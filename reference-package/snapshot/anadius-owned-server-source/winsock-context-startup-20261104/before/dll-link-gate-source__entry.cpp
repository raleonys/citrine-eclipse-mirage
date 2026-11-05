#include "production_image.hpp"
#include "../anadius-resources-factory-source/configuration.hpp"
#include "../anadius-host-platform-source/engine.hpp"
#include "../anadius-initial-cells-source/defaults.hpp"
#include "../anadius-initial-cells-source/runtime.hpp"
#include "../anadius-initial-cells-source/worker.hpp"
#include "../anadius-thread-domain-source/factory.hpp"
#include "../anadius-bootstrap-providers-source/native_threads.hpp"
#include "../anadius-permanent-errors-source/errors.hpp"
#include <atomic>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <memory>
#include <new>
#include <stdexcept>
#include <streambuf>
#include <string>

namespace anadius::dll_link_gate {
namespace {
// Uses this image's compiler stdout, including the real CompilerConsole
// freopen changes. Own the streambuf; do not borrow std::cout's global buffer.
class ConsoleBuffer final:public std::streambuf {
    std::streamsize xsputn(const char* bytes,std::streamsize size)override{
        return size>0?static_cast<std::streamsize>(std::fwrite(bytes,1,static_cast<std::size_t>(size),stdout)):0;
    }
    int_type overflow(int_type value)override{
        if(traits_type::eq_int_type(value,traits_type::eof()))return traits_type::not_eof(value);
        return std::fputc(traits_type::to_char_type(value),stdout)==EOF?traits_type::eof():value;
    }
    int sync()override{return std::fflush(stdout)==0?0:-1;}
};
enum class ImagePhase {Allocated,Constructing,Constructed,Publishing,Starting,Attached,Failed};
class Image final {
    HINSTANCE module_;
    permanent_errors::Policies errors_;
    std::atomic<ImagePhase> phase_{ImagePhase::Allocated};
    std::shared_ptr<shared_log::Stream> stream_;
    std::shared_ptr<library_effects::Dialog> dialog_;
    std::shared_ptr<thread_domain::DomainStorage> domain_;
    std::shared_ptr<bootstrap_providers::NativeThreads> message_threads_;
    bootstrap_backend::Resources resources_;
    std::unique_ptr<resources_factory::Inputs> inputs_;
    std::unique_ptr<host_platform::AssemblyInputs> host_;
    host_platform::Assembly host_assembly_;
    std::unique_ptr<bootstrap_runtime::Runtime> runtime_;
public:
    explicit Image(HINSTANCE module):module_(module),errors_(permanent_errors::compiler_abort_policy()){}
    void failed()noexcept{phase_.store(ImagePhase::Failed,std::memory_order_release);}
    BOOL start(const std::shared_ptr<Image>& self,LPVOID reserved){
        const DWORD incoming=::GetLastError();
        phase_.store(ImagePhase::Constructing,std::memory_order_release);
        stream_=shared_log::make_stream(std::make_shared<ConsoleBuffer>());
        dialog_=std::make_shared<library_effects::Dialog>();
        thread_domain::InitialInputs initial(thread_domain::make_native_initial_policy());
        initial.backing=self;
        domain_=thread_domain::make_initial_domain(initial);
        if(!domain_->initialize())throw std::runtime_error("Application Domain initialization failed");
        // Provider creation belongs to the Domain controller. NativeJobs itself
        // is constructed later on the actual server worker/fiber by configure.
        message_threads_=std::shared_ptr<bootstrap_providers::NativeThreads>(domain_->make_threads());

        resources_.state_lifetime=self;
        resources_.lifetime_profile=bootstrap_backend::LifetimeProfile::OriginalCrt;
        resources_factory::install_native_configuration_console(resources_,stream_);
        inputs_=std::make_unique<resources_factory::Inputs>();
        inputs_->domain=domain_;inputs_->stream=stream_;inputs_->dialog=dialog_;
        inputs_->waits=std::make_shared<services_completion::ExitWaitRegistry>(services_completion::InitialRegistry::empty_owned);
        inputs_->worker.lifetime=self;
        inputs_->worker.configure=[stream=stream_,threads=message_threads_](application_worker::Inputs& value){
            // Native Challenge.version uses configuration.sdk at0xa3c98,
            // not the separate bootstrap log version literal. Worker binds
            // the live Runtime's SDK after configure, before Graph creation.
            resources_factory::configure_native_worker(value,stream,{});
            resources_factory::select_native_message_jobs(value,threads);
        };
        online_factory::select_native_original_online(inputs_->online);

        host_=std::make_unique<host_platform::AssemblyInputs>();
        host_->domain=domain_;host_->stream=stream_;
        host_->callbacks.memory=domain_->make_memory();host_->permanent_errors=&errors_.host;
        initial_cells::select_defaults(*host_,*inputs_);
        initial_cells::select_worker_defaults(*inputs_);
        std::unique_ptr<recovered_hooks::Api> lower=std::make_unique<recovered_hooks::NativeApi>();
        // MinHook is statically linked into this same mapped image. The real
        // image owner retains its API/data/backing; no original DLL is loaded
        // and no self-pin changes the loader's normal reference behavior.
        std::shared_ptr<void> code=self;
        auto engine=recovered_hooks::make_owned_engine(lower,code);
        host_platform::select_original_owned_engine(*host_,engine);
        host_assembly_=host_platform::install_host(resources_,*host_);
        resources_factory::install(resources_,*inputs_);
        runtime_=initial_cells::make_fresh_runtime(resources_,errors_.bootstrap);
        phase_.store(ImagePhase::Constructed,std::memory_order_release);

        // Every real provider and both ownership roots are now adopted. Keep
        // compiler CRT exit registration in the actual application body; do not
        // manually invoke or pre-register the recovered exit callback here.
        phase_.store(ImagePhase::Publishing,std::memory_order_release);
        runtime_->publish();
        const auto entry=runtime_->application_entry();
        phase_.store(ImagePhase::Starting,std::memory_order_release);
        ::SetLastError(incoming);
        const BOOL attached=entry(module_,DLL_PROCESS_ATTACH,reserved);
        const DWORD outgoing=::GetLastError();
        if(!attached)throw std::runtime_error("Application attach returned false after publication");
        phase_.store(ImagePhase::Attached,std::memory_order_release);
        ::SetLastError(outgoing);return attached;
    }
};
// Deliberate mapped-image retention: CRT source exit callbacks still need the
// complete graph. Do not register a cooperative Runtime/Domain destructor that
// would join workers or replace original reason0 behavior under loader lock.
// This is not a proof of full native resource teardown or safe live FreeLibrary.
alignas(std::shared_ptr<Image>) constinit std::byte owner_storage[sizeof(std::shared_ptr<Image>)]{};
constinit std::atomic<bool> attach_claimed{false};
std::shared_ptr<Image>& image_owner(){
    static auto* const value=::new(static_cast<void*>(owner_storage)) std::shared_ptr<Image>();
    return *value;
}
static_assert(std::atomic<ImagePhase>::is_always_lock_free&&std::atomic<bool>::is_always_lock_free);
}
BOOL require_loader_attach_runtime(HINSTANCE module,LPVOID reserved)noexcept{
    if(!module||attach_claimed.exchange(true,std::memory_order_acq_rel))std::abort();
    try{
        auto owner=std::make_shared<Image>(module);
        image_owner()=owner; // retain before Domain SDK initialization or publication
        return owner->start(owner,reserved);
    }catch(...){
        if(image_owner())image_owner()->failed();
        permanent_errors::compiler_abort_policy().bootstrap.callback_exception(
            bootstrap_native_recovered::Entry::Application,std::current_exception());
        std::abort();
    }
}
}

void anadius::dll_link_gate::require_loader_detach_quiescence(HINSTANCE, LPVOID) noexcept {
    // Original application DllMain038870 does nothing for reason0: CMP EDX,1;
    // JNZ03887e; MOV EAX,1. This intentionally has no application detach work.
    // Compiler CRT invokes the registered source exit callback at its actual
    // CRT position. Do not invoke it here as well, or call Runtime::close/join.
    // The historical helper name does NOT assert quiescence or safe FreeLibrary.
}

// Candidate compiler-CRT application entry; runtime behavior is unvalidated.
// The toolchain supplies DllMainCRTStartup. Do not also call recovered
// Entry::entry_04bcfc: that would initialize a second CRT around this C++ graph.
// Neither original nor rebuilt DLL is loaded or executed by the build script.
extern "C" BOOL WINAPI DllMain(HINSTANCE module, DWORD reason, LPVOID reserved) noexcept {
    if (reason == DLL_PROCESS_ATTACH) {
        // One owner covers construction, publication and the complete real
        // application entry. Calling ApplicationEntry here as well would move
        // startup outside that transaction or initialize it a second time.
        return anadius::dll_link_gate::require_loader_attach_runtime(module, reserved);
    }
    if (reason == DLL_PROCESS_DETACH)
        anadius::dll_link_gate::require_loader_detach_quiescence(module, reserved);
    // Recovered application main has no thread-attach/detach work. Compiler CRT
    // thread/TLS work remains owned by DllMainCRTStartup, not this projection.
    return TRUE;
}
