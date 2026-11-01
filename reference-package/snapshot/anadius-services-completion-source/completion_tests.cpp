#include "completion_test_fixture.hpp"

namespace {
using namespace b10_test;
void logger_policies(){
    rejects<std::invalid_argument>([]{sc::BootstrapLogger bad({});},"logger requires an explicit stream owner");
    auto sink=std::make_shared<Sink>();auto log=logger(sink);
    require(sink->buffer.str().empty()&&sink->buffer.syncs==0,"logger construction performs no output");
    auto* token=log->insert(reinterpret_cast<void*>(sc::BootstrapLogger::stream_identity),0x180083cd8);
    require(token==log->token()&&token!=reinterpret_cast<void*>(sc::BootstrapLogger::stream_identity),"native stream identity becomes stable source token");
    require(log->insert(token,0x180086720)==token&&log->insert(token,0x180086718)==token,"three source insertions keep one owned token");
    log->finish(token);
    require(sink->buffer.str()=="LSX emu version 2.0.1.0\n"&&sink->buffer.syncs==1,"exact retained startup bytes and one endl flush");
    const auto before=sink->buffer.str();
    rejects<std::domain_error>([&]{log->insert(reinterpret_cast<void*>(0x1234),0x180083cd8);},"foreign stream identity is rejected");
    rejects<std::domain_error>([&]{log->insert(token,0x180000000);},"unknown literal identity is rejected");
    rejects<std::domain_error>([&]{log->finish(reinterpret_cast<void*>(0x1234));},"foreign finish identity is rejected");
    require(sink->buffer.str()==before&&sink->buffer.syncs==1,"rejected operands have no output or flush effects");
    sink->stream<<std::setfill('_')<<std::setw(10);
    log->insert(token,0x180083cd8);
    require(sink->buffer.str()==before+"___LSX emu"&&sink->stream.width()==0,"selected formatted insertion respects width/fill and resets width");
    sink->stream.setstate(std::ios::failbit);const auto failed=sink->buffer.str();log->insert(token,0x180083cd8);log->finish(token);
    require(sink->buffer.str()==failed&&sink->stream.fail(),"failed compiler stream does not fabricate output");
    struct Failing:std::streambuf {std::streamsize xsputn(const char*,std::streamsize)override{return 0;}};
    auto bad_buffer=std::make_shared<Failing>();auto bad_stream=std::make_shared<std::ostream>(bad_buffer.get());bad_stream->exceptions(std::ios::badbit);
    sc::BootstrapLogger bad_log(bad_stream);
    rejects<std::ios_base::failure>([&]{bad_log.terminate_log();},"fatal output exception propagates before the source abort point");
}
void registry_policies(){
    Probe trace;probe=&trace;
    {
        auto bad=probe_api();bad.initialize=fail_initialize;
        bool caught=false;try{sc::ExitWaitRegistry fail(sc::InitialRegistry::empty_owned,bad);}catch(const std::system_error& e){caught=e.code().value()==ERROR_NOT_ENOUGH_MEMORY;}
        require(caught&&trace.initializes==1&&trace.destroys==0,"failed native CS init reports error without deleting uninitialized state");
    }
    trace={};
    {
        auto reg=registry(probe_api());auto target=std::make_shared<sc::WaitTarget>(1,0,probe_api());trace.target=target.get();
        require(trace.initializes==1&&reg->active()==0&&reg->blocks()==1,"full nonzero BOOL0x100 success owns one real empty CS/head");
        rejects<std::logic_error>([&]{target->unlock();},"unlock rejects absent owner");
        rejects<std::logic_error>([&]{reg->register_current_thread(target);},"registration rejects an unlocked target");
        target->lock();rejects<std::logic_error>([&]{target->lock();},"count1 target rejects recursive lock");
        reg->register_current_thread(target);
        rejects<std::logic_error>([&]{reg->register_current_thread(target);},"duplicate publication cannot create a second entry");
        rejects<std::logic_error>([&]{target->unlock();},"registered target unlock belongs to source exit cleanup");
        require(target->registered()&&target->is_locked()&&reg->active()==1,"published entry retains locked target");
        require(!reg->close(),"live entry prevents closed registry success");
        auto rejected=std::make_shared<sc::WaitTarget>(1,7);rejected->lock();
        rejects<std::logic_error>([&]{reg->register_current_thread(rejected);},"stopping registry rejects new publication");
        rejected->unlock();trace.events.clear();
        reg->cleanup_current_thread();
        require(trace.events.front()=="thread"&&trace.events[1]=="enter","cleanup gets thread id before registry critical section");
        require(pos(trace.events,"release")<pos(trace.events,"wake")&&pos(trace.events,"wake")<pos(trace.events,"leave"),"source cleanup unlocks target before notify and exits registry last");
        require(trace.ready_before_release&&trace.unlocked_before_wake,"ready is published before native unlock and notify sees unlocked target");
        require(target->ready_value_quiescent()==1&&!target->registered()&&!target->is_locked()&&reg->active()==0,"source consumes exactly current entry and releases ownership");
        const auto count=trace.wakes;reg->cleanup_current_thread();require(trace.wakes==count,"consumed entries are not notified twice");
        require(reg->close()&&reg->close(),"empty stopping registry closes idempotently");
        rejects<std::logic_error>([&]{reg->cleanup_current_thread();},"closed registry rejects late cleanup with CS retained safely");
    }
    require(trace.destroys==1,"only successfully initialized quiescent CS is deleted once");probe=nullptr;
    auto incomplete=sc::native_exit_wait_api();incomplete.wake_all=nullptr;
    rejects<std::invalid_argument>([&]{sc::ExitWaitRegistry bad(sc::InitialRegistry::empty_owned,incomplete);},"partial native API rejected before resource construction");
}
void multiple_blocks_and_threads(){
    auto reg=registry();std::vector<std::shared_ptr<sc::WaitTarget>> targets;
    for(unsigned i=0;i<25;++i){auto target=std::make_shared<sc::WaitTarget>(1,9);target->lock();reg->register_current_thread(target,i!=24);targets.push_back(std::move(target));}
    std::weak_ptr<sc::WaitTarget> retained=targets.front();targets.front().reset();
    require(!retained.expired()&&reg->active()==25&&reg->blocks()==2,"registry owns published targets across the twenty-slot head boundary");
    reg->cleanup_current_thread();require(retained.expired()&&reg->active()==0,"consumed registry ownership releases otherwise unowned target");
    for(std::size_t i=1;i<targets.size();++i)require(!targets[i]->is_locked()&&!targets[i]->registered()&&targets[i]->ready_value_quiescent()==(i==24?9:1),"multi-block cleanup preserves optional ready pointer semantics");
    require(reg->close(),"fully consumed multiple blocks close");
    reg=registry();auto main_target=std::make_shared<sc::WaitTarget>(1,0),worker_target=std::make_shared<sc::WaitTarget>(1,0);
    main_target->lock();reg->register_current_thread(main_target);
    std::promise<void> published,release;auto ready=published.get_future();auto released=release.get_future();
    std::thread worker([&]{worker_target->lock();reg->register_current_thread(worker_target);published.set_value();released.wait();reg->cleanup_current_thread();});
    ready.get();require(reg->active()==2,"different real thread ids own separate live entries");
    reg->cleanup_current_thread();
    require(main_target->ready_value_quiescent()==1&&worker_target->ready_value_quiescent()==0&&reg->active()==1,"current-thread cleanup leaves another thread entry untouched");
    require(!reg->close(),"remaining other-thread entry prevents teardown");
    release.set_value();worker.join();
    require(worker_target->ready_value_quiescent()==1&&!worker_target->is_locked()&&reg->active()==0&&reg->close(),"joined worker consumes its own entry before final close");
}
void wrapper_policies(){
    auto sink=std::make_shared<Sink>();auto log=logger(sink);auto reg=registry();b2_test::Track track;
    {
        auto lower=std::make_unique<b2_test::Backend>(track);sc::CompletionServices services(log,reg,std::move(lower));
        auto* stream=services.log_insert(reinterpret_cast<void*>(sc::BootstrapLogger::stream_identity),0x180083cd8);services.log_finish(stream);
        auto target=std::make_shared<sc::WaitTarget>(1,0);target->lock();reg->register_current_thread(target);services.cleanup_threads();
        require(sink->buffer.str()=="LSX emu\n"&&target->ready_value_quiescent()==1,"Services routes reach selected logger and actual source cleanup");
        require(std::find(track.events.begin(),track.events.end(),"bootstrap-log-part")==track.events.end()&&std::find(track.events.begin(),track.events.end(),"cleanup-thread")==track.events.end(),"selected routes bypass lower coarse policies");
        services.guard_return();require(std::find(track.events.begin(),track.events.end(),"guard-return")!=track.events.end(),"unselected route preserves lower policy");
    }
    require(reg->close(),"wrapper selected registry closes after use");
    rejects<std::invalid_argument>([&]{sc::CompletionServices bad({},reg,std::make_unique<b2_test::Backend>(track));},"missing logger rejected");
    rejects<std::invalid_argument>([&]{sc::CompletionServices bad(log,{},std::make_unique<b2_test::Backend>(track));},"missing registry rejected");
}
volatile std::sig_atomic_t abort_code=90,fatal_ok=1;
void expected_abort(int){std::_Exit(fatal_ok?abort_code:99);}
void WINAPI throwing_release(PSRWLOCK p){::ReleaseSRWLockExclusive(p);throw std::runtime_error("unknown native release outcome");}
struct FatalBuffer:Buffer {int sync()override{fatal_ok=str()=="Unhandled exception\n";return Buffer::sync();}};
int terminal(std::string_view mode){
    ::SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX);std::signal(SIGABRT,expected_abort);
    if(mode=="--live-registry"){abort_code=90;auto reg=registry();return 98;}
    if(mode=="--locked-target"){abort_code=91;auto target=std::make_shared<sc::WaitTarget>(1,0);target->lock();return 98;}
    if(mode=="--unknown-release"){abort_code=92;auto api=sc::native_exit_wait_api();api.release=throwing_release;sc::WaitTarget target(1,0,api);target.lock();target.unlock();return 98;}
    if(mode=="--fatal"){
        abort_code=93;auto buffer=std::make_shared<FatalBuffer>();auto stream=std::make_shared<std::ostream>(buffer.get());sc::BootstrapLogger log(stream);log.terminate_log();
    }
    if(mode=="--unexpected"){
        abort_code=94;auto sink=std::make_shared<Sink>();auto reg=registry();b2_test::Track track;sc::CompletionServices services(logger(sink),reg,std::make_unique<b2_test::Backend>(track));services.unexpected_terminate_return();
    }
    return 98;
}
}
int main(int argc,char** argv){if(argc==2&&std::string_view(argv[1]).starts_with("--"))return terminal(argv[1]);try{
    logger_policies();registry_policies();multiple_blocks_and_threads();wrapper_policies();
    int code=90;for(const auto* mode:{"--live-registry","--locked-target","--unknown-release","--fatal","--unexpected"})
        require(_spawnl(_P_WAIT,argv[0],argv[0],mode,static_cast<const char*>(nullptr))==code++,"own terminal child enforces explicit resource/fatal boundary");
    std::cout<<"PASS: "<<checks<<" services-completion core checks; retained logger bytes, owned exit-wait source cleanup, native synchronization and terminal boundaries\n";
}catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}}
