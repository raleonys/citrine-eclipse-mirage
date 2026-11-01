#include "publication_test_fixture.hpp"

namespace {
using namespace b19_test;using namespace std::chrono_literals;
void mapping_and_failure(){
    State state;Platform platform;auto events=std::make_shared<sp::Milestones>();
    state.server=state.online=state.libraries=1;ex::Execution execution(state.mutex,state.flags(),state.cache,platform,events.get());
    require(events->bits()==0&&!events->observed_all(),"preseed ready cells are not observed publication events");
    execution.finish_flag_036820(state.other);require(state.other==1&&events->bits()==0,"unrelated supplied flag preserves write without event");
    execution.finish_flag_036820(state.server);execution.finish_flag_036820(state.server);
    require(events->bits()==1&&!events->observed_all(),"duplicate finish is idempotent and never fills missing stages");
    execution.finish_flag_036820(state.online);execution.finish_flag_036820(state.libraries);
    require(events->bits()==7,"three exact flag addresses map to worker prefix bits");
    events->published(ex::ReadyStage::Host,&execution);require(events->observed_all()&&!events->invalid(),"host return event completes four milestones");
    bool provider=false;unsigned calls=0;auto proof=sp::readiness_proof(events,[&]{++calls;return provider;});
    require(!proof()&&calls==1,"all source bits do not replace external provider proof");provider=true;require(proof()&&calls==2,"complete milestones combine with separate provider ownership proof");
    State second;rejects<std::logic_error>([&]{ex::Execution reused(second.mutex,second.flags(),second.cache,platform,events.get());},"one observer cannot bind a second Execution");
    sp::Milestones aliases;rejects<std::invalid_argument>([&]{ex::Execution invalid(second.mutex,{second.server,second.server,second.libraries},second.cache,platform,&aliases);},"aliased flags rejected before observer binding");
    sp::Milestones missing;rejects<std::invalid_argument>([&]{sp::readiness_proof({},[]{return true;});},"owned observer required");
    events->published(ex::ReadyStage::Server,&second);require(events->invalid()&&!events->observed_all(),"foreign issuer poisons readiness rather than claiming success");
    State failure;Platform failed;sp::Milestones no_event;ex::Execution failing(failure.mutex,failure.flags(),failure.cache,failed,&no_event);
    failure.mutex.owner_48=::GetCurrentThreadId();failure.mutex.count_4c=1;
    bool error=false;try{failing.finish_flag_036820(failure.server);}catch(int index){error=index==5;}
    require(error&&failure.server==0&&failure.mutex.count_4c==1&&no_event.bits()==0,"native error5 prefix remains without invented unlock/publication");
    State release;Platform release_platform;sp::Milestones release_events;ex::Execution release_execution(release.mutex,release.flags(),release.cache,release_platform,&release_events);
    release_platform.throw_release=true;rejects<std::runtime_error>([&]{release_execution.finish_flag_036820(release.server);},"injected unlock exception propagates");
    require(release.server==1&&release.mutex.count_4c==0&&release_events.bits()==0,"written flag retained but failed release emits no event");
}
void waiter_and_clock(){
    State state;Platform platform;sp::Milestones milestones;ex::Execution execution(state.mutex,state.flags(),state.cache,platform,&milestones);
    std::mutex gate;std::condition_variable cv;bool sleeping=false,finished=false;std::exception_ptr failure;
    platform.sleep_hook=[&]{std::unique_lock lock(gate);sleeping=true;cv.notify_all();if(!cv.wait_for(lock,2s,[&]{return finished;}))throw std::runtime_error("finisher blocked by sleep gate");};
    std::thread waiter([&]{try{execution.wait_steps_036880(1);}catch(...){failure=std::current_exception();}});
    bool entered_sleep;
    {std::unique_lock lock(gate);entered_sleep=cv.wait_for(lock,2s,[&]{return sleeping;});if(!entered_sleep)finished=true;}
    if(!entered_sleep){cv.notify_all();waiter.join();throw std::runtime_error("waiter did not sleep");}
    ::SetLastError(0x7711);execution.finish_flag_036820(state.server);
    const auto final_error=::GetLastError();{std::lock_guard lock(gate);finished=true;}cv.notify_all();waiter.join();if(failure)std::rethrow_exception(failure);
    require(platform.sleeps>0&&state.server==1&&milestones.bits()==1,"other own thread finishes while waiter is sleeping outside source gate");
    require(final_error==0x7711,"source guard bookkeeping preserves platform error state");
    State clock;Platform clock_platform;ex::Execution shared(clock.mutex,clock.flags(),clock.cache,clock_platform);
    std::atomic<bool> valid=true;std::vector<std::thread> readers;
    for(unsigned i=0;i<8;++i)readers.emplace_back([&]{for(unsigned n=0;n<100;++n)if(shared.now_nanoseconds_036740()<0)valid=false;});
    for(auto& reader:readers)reader.join();
    require(valid&&clock.cache==10000000&&clock_platform.frequencies==1&&clock_platform.counters==800,"concurrent clock callers initialize one cache without racing wrapper state");
}
struct Observer final:ex::Publication {
    ex::Execution* execution=nullptr;std::mutex gate;std::condition_variable cv;bool done=false,during=false;std::thread worker;
    void bind(const void* value,ex::FlagReferences)override{execution=const_cast<ex::Execution*>(static_cast<const ex::Execution*>(value));}
    void published(ex::ReadyStage,const void*)noexcept override{
        worker=std::thread([&]{execution->now_nanoseconds_036740();{std::lock_guard lock(gate);done=true;}cv.notify_all();});
        std::unique_lock lock(gate);during=cv.wait_for(lock,2s,[&]{return done;});
    }
};
void observer_release_and_payload(){
    State state;Platform platform;Observer observer;ex::Execution execution(state.mutex,state.flags(),state.cache,platform,&observer);
    execution.finish_flag_036820(state.server);observer.worker.join();require(observer.during,"observer runs after source/native locks release; another Execution caller completes during observer");
    State payload;Platform payload_platform;sp::Milestones milestones;ex::Execution shared(payload.mutex,payload.flags(),payload.cache,payload_platform,&milestones);
    unsigned port=0,online=0,libraries=0;
    std::thread a([&]{port=4660;shared.finish_flag_036820(payload.server);});
    std::thread b([&]{online=17;shared.finish_flag_036820(payload.online);});
    std::thread c([&]{libraries=23;shared.finish_flag_036820(payload.libraries);});
    milestones.published(ex::ReadyStage::Host,&shared);
    const auto deadline=std::chrono::steady_clock::now()+2s;
    while(!milestones.observed_all()&&std::chrono::steady_clock::now()<deadline)std::this_thread::yield();
    const bool visible=milestones.observed_all()&&port==4660&&online==17&&libraries==23;
    a.join();b.join();c.join();require(visible,"all-bits acquire publishes plain prefix payloads from separate own threads");
}
}
int main(){try{mapping_and_failure();waiter_and_clock();observer_release_and_payload();std::cout<<"PASS: "<<b19_test::checks<<" startup publication core checks\n";return 0;}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
