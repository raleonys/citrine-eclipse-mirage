#include "iteration_test_fixture.hpp"

namespace {
using namespace b9_test;
struct Collected {std::vector<Path> entries;std::vector<DWORD> incoming;};
void collect(void* context,const Path& p){
    auto& out=*static_cast<Collected*>(context);out.incoming.push_back(::GetLastError());out.entries.push_back(p);::SetLastError(0xcab);
}
void streaming_and_compatibility(){
    SdkScope own;own.plan(u"outer",{L"z",L".",L"..",L"a"});
    Collected out;::SetLastError(0x42);own.fs->visit_directory_entries(u"outer",{&out,collect});
    require(out.entries==std::vector<Path>({u"outer\\z",u"outer\\a"}),"stream preserves SDK logical order and skips exactly dot entries");
    require(out.incoming==std::vector<DWORD>({0x111,0x222}),"each callback begins with preceding SDK status after local path bookkeeping");
    require(own.state.next_incoming.front()==0xcab&&own.state.next_incoming.back()==0xcab,"next SDK operation receives callback LastError effects");
    require(own.state.nexts==4&&own.state.closes==1&&own.state.cursors.empty(),"stream consumes one increment per record and closes exact owned find handle");
    require(::GetLastError()==0x333,"completed visitor preserves cleanup close LastError");
    own.plan(u"empty",{});Collected empty;own.fs->visit_directory_entries(u"empty",{&empty,collect});
    require(empty.entries.empty()&&own.state.closes==1,"empty stream invokes no callback and does not close an invalid handle");
    const auto firsts=own.state.firsts;
    rejects<std::invalid_argument>([&]{own.fs->visit_directory_entries(u"outer",{});},"missing callback is rejected before enumeration side effects");
    require(own.state.firsts==firsts,"invalid callback performs no FindFirst");
    own.plan(u"vector",{L"second",L"first"});
    require(own.fs->directory_entries(u"vector")==std::vector<Path>({u"vector\\second",u"vector\\first"}),"legacy vector API collects the same source stream without sorting");
    require(own.state.closes==2&&own.state.cursors.empty(),"legacy vector collection shares exact handle cleanup");
}
void callback_failure_and_reentry(){
    {
        SdkScope own;own.plan(u"throw",{L"a",L"b"});
        rejects<std::runtime_error>([&]{own.fs->visit_directory_entries(u"throw",{nullptr,[](void*,const Path&){::SetLastError(0xabc);throw std::runtime_error("callback");}});},"callback exception propagates without fetching later entries");
        require(own.state.nexts==0&&own.state.closes==1&&own.state.cursors.empty(),"throwing callback closes the active handle once before any increment");
        require(::GetLastError()==0x333,"throwing callback still exposes selected last cleanup SDK status");
    }
    {
        SdkScope own;own.plan(u"outer",{L"a",L"b"});own.plan(u"inner",{L"x"});
        struct Nested {cs::Filesystem* fs;std::vector<Path> entries;bool nested=false;};Nested state{own.fs.get(),{},false};
        own.fs->visit_directory_entries(u"outer",{&state,[](void* c,const Path& p){
            auto& s=*static_cast<Nested*>(c);s.entries.push_back(p);
            if(!s.nested){s.nested=true;s.fs->visit_directory_entries(u"inner",{&s,[](void* q,const Path& entry){static_cast<Nested*>(q)->entries.push_back(entry);}});}
            ::SetLastError(0x555);
        }});
        require(state.entries==std::vector<Path>({u"outer\\a",u"inner\\x",u"outer\\b"}),"nested visitor maintains independent live cursor state and resumes outer order");
        require(own.state.firsts==2&&own.state.closes==2&&own.state.cursors.empty(),"reentrant visitor owns and closes both independent SDK handles");
        require(own.state.next_incoming.back()==0x555,"outer SDK continuation receives final nested-callback state");
    }
}
void source_failure_prefixes(){
    {
        SdkScope own;own.plan(user_directory(),{L"early_AVATAR_one.png",L"keep.txt"},ERROR_READ_FAULT);
        Policies policy(own.fs);Pass passes;auto state=seed();bool caught=false;
        try{configuration::initialize_configuration(state,policy,passes);}catch(const std::system_error& e){caught=e.code().value()==ERROR_READ_FAULT;}
        const auto early=path_recovered::join_016b00(user_directory(),u"early_AVATAR_one.png");
        require(caught&&own.state.removed==std::vector<Path>{early},"actual configuration source removes early avatar before a later FindNext error");
        const auto pattern=path_recovered::join_016b00(user_directory(),u"*");
        require(position(own.state.events,"remove:"+label(early))<position(own.state.events,"next:"+label(pattern)),"source filter/removal callback occurs before the next native enumeration step");
        require(policy.vectors==0&&policy.visits==1&&passes.calls==2,"actual configuration source uses visitor after both configuration passes");
        require(state.sdk=="1.2.3"&&policy.state->persistence==0&&policy.state->environment.empty(),"late enumeration error stops later normalization, persistence and environment effects");
        require(policy.state->pngs==2&&!state.default_avatar.empty()&&own.state.closes==1,"earlier source PNG/path prefix survives while enumeration handle is closed");
    }
    for(const auto failure:{Failure::filename,Failure::narrow,Failure::remove}){
        SdkScope own;own.plan(user_directory(),{L"early_AVATAR_one.png",L"later"});
        Policies policy(own.fs);policy.failure=failure;Pass passes;auto state=seed();
        rejects<std::runtime_error>([&]{configuration::initialize_configuration(state,policy,passes);},"filename/narrow/remove callback failure propagates from actual source loop");
        require(own.state.nexts==0&&own.state.closes==1&&own.state.cursors.empty(),"callback failure blocks increment and closes borrowed iterator owner");
        const std::vector<std::string> expected=failure==Failure::filename?std::vector<std::string>{"filename"}:
            failure==Failure::narrow?std::vector<std::string>{"filename","narrow"}:std::vector<std::string>{"filename","narrow","remove"};
        require(policy.trace==expected,"actual source callback preserves filename then narrow then conditional remove failure order");
        require(policy.state->persistence==0&&policy.state->environment.empty(),"failed callback cannot fabricate later storage/environment effects");
    }
    {
        SdkScope own;own.plan(user_directory(),{L"early_AVATAR_one.png",L"later"});own.state.base.set_errors={ERROR_ACCESS_DENIED};
        Policies policy(own.fs);Pass passes;auto state=seed();bool caught=false;
        try{configuration::initialize_configuration(state,policy,passes);}catch(const std::system_error& e){caught=e.code().value()==ERROR_ACCESS_DENIED;}
        require(caught&&own.state.nexts==0&&own.state.removed.empty(),"actual remove failure propagates before fetching a later directory entry");
        require(own.state.base.handle_closes==1&&own.state.closes==1,"remove failure closes both delete and enumeration handles exactly once");
    }
}
void old_vector_fallback(){
    Policies policy;policy.vector_entries={u"OLD\\one_AVATAR_a.png",u"OLD\\lower_avatar.png"};
    Pass pass;auto state=seed();configuration::initialize_configuration(state,policy,pass);
    require(policy.vectors==1&&policy.visits==1&&policy.removes==1,"optional visitor default preserves old vector-only dependency fixtures");
    require(state.sdk=="1,2,3"&&policy.state->persistence==1&&policy.state->environment.size()==2,"vector fallback retains successful original post-enumeration source sequence");
    Policies invalid;
    rejects<std::invalid_argument>([&]{invalid.configuration::Dependencies::visit_directory_entries(u"unused",{});},"base visitor validates missing callback before invoking old vector backend");
    require(invalid.vectors==0,"invalid default visitor cannot trigger collection");
}
constexpr std::array<int,7> expected_codes{16,22,3,12,1,36,11};
constexpr std::array<std::string_view,7> expected_contexts{
    "device or resource busy","invalid argument","no such process","not enough memory",
    "operation not permitted","resource deadlock would occur","resource unavailable try again"};
template<class F>void expect_thread_error(int index,F&& f,const char* label){
    bool caught=false;
    try{f();}catch(const std::system_error& e){
        caught=e.code().value()==expected_codes[static_cast<std::size_t>(index)]&&
            std::string_view(e.code().category().name())=="generic"&&
            std::string_view(e.what()).find(expected_contexts[static_cast<std::size_t>(index)])!=std::string_view::npos;
    }
    require(caught,label);
}
void error_table_and_services(){
    for(int i=0;i<7;++i){
        const auto spec=bootstrap_policies::thread_error_spec(i);
        require(spec.code==expected_codes[static_cast<std::size_t>(i)]&&spec.context==expected_contexts[static_cast<std::size_t>(i)],"thread error index preserves exact retained numeric/context pair");
        expect_thread_error(i,[&]{bootstrap_policies::throw_thread_error_04a648(i);},"selected thread error preserves code, generic category and retained context");
    }
    for(const auto i:{-1,7,std::numeric_limits<int>::min(),std::numeric_limits<int>::max()}){
        rejects<std::out_of_range>([&]{bootstrap_policies::thread_error_spec(i);},"invalid thread error index rejected before table access");
        rejects<std::out_of_range>([&]{bootstrap_policies::throw_thread_error_04a648(i);},"throw helper rejects invalid index before exception table access");
    }
    b7_test::DomainFixture domain;auto memory=domain.domain->make_memory();
    b2_test::Track track;auto lower=std::make_unique<b2_test::Backend>(track);
    {
        bootstrap_policies::SourceServices services(memory,std::move(lower));
        for(int i=0;i<7;++i)expect_thread_error(i,[&]{services.thread_error(i);},"SourceServices binds every retained index instead of invoking lower thread-error policy");
        bootstrap_runtime::NativeExecution execution(services);
        expect_thread_error(6,[&]{execution.raise_thread_error_04a648(6);},"existing NativeExecution reaches same concrete retained thread-error route");
        require(memory->outstanding_allocations()==0,"error propagation performs no application payload allocation");
    }
    memory.reset();require(domain.domain->close(),"thread-error service test closes owned memory/domain resources");
}
}
int main(){try{
    streaming_and_compatibility();callback_failure_and_reentry();source_failure_prefixes();old_vector_fallback();error_table_and_services();
    std::cout<<"PASS: "<<b9_test::total()<<" live-iteration core checks; callback-before-increment, retained failure prefixes, LastError/reentry/default-vector paths and indexed thread errors; no original target execution\n";
}catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}}
