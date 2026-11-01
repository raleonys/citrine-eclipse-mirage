#pragma once
#include "../anadius-native-context-source/context.hpp"
#include "../anadius-thread-domain-source/domain.hpp"
#include "../anadius-encoding-source/encoding.hpp"
#include "../anadius-library-effects-source/dialog.hpp"
#include <list>
#include <ostream>

namespace anadius::callback_policy {
// Compiler CRT functions selected for this rebuilt image, not original CRT ABI.
struct CrtApi {
    int (*compare_a)(const char*,const char*);
    int (*copy_a)(char*,std::size_t,const char*);
    int (*compare_w)(const wchar_t*,const wchar_t*);
    int (*copy_w)(wchar_t*,std::size_t,const wchar_t*);
};
CrtApi compiler_crt_api() noexcept;
// Optional source-level owned context. It is independent of recovered PTD state.
class CrtProvider {
public:
    virtual ~CrtProvider()=default;
    virtual int compare_a(const char*,const char*)=0;
    virtual int copy_a(char*,std::size_t,const char*)=0;
    virtual int compare_w(const wchar_t*,const wchar_t*)=0;
    virtual int copy_w(wchar_t*,std::size_t,const wchar_t*)=0;
};
// Fully readable NUL-terminated UTF-16 inputs and writable destination capacity
// are required. Capacity <= INT_MAX; overlapping copy storage is rejected.
// Stage only actual wchar_t objects for compiler CRT calls; no typed access to
// caller wchar_t/char16_t/unaligned backing. Temporary allocation/EH and compiler
// invalid-handler/debug-fill details do not reproduce original static CRT ABI.
class ByteWide final:public native_recovered::WideOperations {
public:
    explicit ByteWide(CrtApi=compiler_crt_api(),std::shared_ptr<CrtProvider> owned={});
    int compare_utf16(const void*,const void*) override;
    int copy_utf16(void*,std::size_t,const void*) override;
private:CrtApi api_;std::shared_ptr<CrtProvider> owned_;
};
using ExitFunction=void(*)(std::uint32_t);
[[noreturn]] void compiler_exit(std::uint32_t);
struct Resources {
    std::shared_ptr<thread_domain::ApplicationMemory> memory;
    std::shared_ptr<std::ostream> stream; // caller owner must also retain streambuf
    std::unique_ptr<encoding_recovered::Calls> encoding=std::make_unique<encoding_recovered::NativeCalls>();
    std::unique_ptr<encoding_recovered::Errors> errors=std::make_unique<encoding_recovered::PortableErrors>();
    library_effects::Dialog dialog{};
    CrtApi crt=compiler_crt_api();
    std::shared_ptr<CrtProvider> owned_crt;
    // Optional exact-slot-bound owner, constructed only after RuntimeSeed moves.
    std::function<std::shared_ptr<native_recovered::OriginalProvider>(const native_recovered::OriginalSlots&)> originals;
    ExitFunction exit=compiler_exit;
    // Optional explicit permission for an admitted same-domain PTD scope.
    // Empty keeps B16 controller-only behavior. Must retain its gate owner.
    std::function<bool()> callback_scope;
};
struct Storage {
    Resources resources;
    ByteWide wide;
    std::shared_ptr<native_recovered::OriginalProvider> original_owner;
    std::shared_ptr<original_checkpoint::Boundary> original_boundary;
    Storage(Resources,const native_recovered::OriginalSlots&);
};
// Bounded concrete policy: one controller thread serializes ALL callbacks and
// shared map/config/set/counter/stream accesses, with NO callback reentry.
// This class's overrides reject other-thread calls. Inherited final original/
// wide methods do not; this is not a whole-callback synchronization/admission guard.
// Memory is the caller's same-domain ApplicationMemory; foreign host-thread PTD
// support remains open. No incoming fabricated handles are adopted. Providers
// must outlive callbacks; native Context must close before destruction.
class Policy final:private Storage,public native_recovered::OriginalEffects {
public:
    Policy(native_context::PolicyView,Resources);
    ~Policy() noexcept override;
    Policy(const Policy&)=delete;Policy& operator=(const Policy&)=delete;
    Policy(Policy&&)=delete;Policy& operator=(Policy&&)=delete;
    std::string filesystem_narrow(std::u16string_view) override;
    int crt_stricmp(const char*,const char*) override;
    int crt_strcpy_s(char*,std::size_t,const char*) override;
    registry_recovered::Handle allocate_4_04b0bc() override;
    void free_04b0f8(registry_recovered::Handle) override;
    void wait_036880(std::int32_t) override;
    void wait_steps_036880(std::int32_t) override;
    void log_literal_006880(std::uint64_t) override;
    void log_text_007990(std::string_view) override;
    void finish_log_0088e0() override;
    void log_literal_006880(std::string_view) override;
    void log_port_00ad70(std::uint16_t) override;
    void log_finish_0088e0() override;
    void dialog_038bc0(std::string_view,std::string_view,std::uint32_t) override;
    [[noreturn]] void exit_05cdc0(std::uint32_t) override;
    std::size_t owned_handles() const;
    static std::string_view registry_literal(std::uint64_t);
private:
    void check_thread() const;
    execution_recovered::Execution& execution_;
    DWORD controller_;
    std::list<registry_recovered::Handle> handles_;
};
// One-shot resource ownership even when the returned function object is copied.
native_context::PolicyFactory make_policy_factory(Resources);
} // namespace anadius::callback_policy
