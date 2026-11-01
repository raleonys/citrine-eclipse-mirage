#include "preloader_l/runtime/bootstrap_thread.hpp"
#include <stdint.h>
using preloader_l::runtime::ReleaseRuntimeShutdownResult;
static_assert(static_cast<uint32_t>(ReleaseRuntimeShutdownResult::shutdown)==0u,"shutdown ABI");
static_assert(static_cast<uint32_t>(ReleaseRuntimeShutdownResult::busy)==1u,"busy ABI");
static_assert(static_cast<uint32_t>(ReleaseRuntimeShutdownResult::bootstrap_timeout)==2u,"timeout ABI");
static_assert(static_cast<uint32_t>(ReleaseRuntimeShutdownResult::bootstrap_wait_failed)==3u,"wait ABI");
static_assert(static_cast<uint32_t>(ReleaseRuntimeShutdownResult::service_active_leases)==4u,"lease ABI");
static_assert(static_cast<uint32_t>(ReleaseRuntimeShutdownResult::hook_teardown_failed)==5u,"hook ABI");
static_assert(static_cast<uint32_t>(ReleaseRuntimeShutdownResult::ptd_cleanup_pending)==6u,"PTD ABI");
static_assert(static_cast<uint32_t>(ReleaseRuntimeShutdownResult::bootstrap_handle_close_failed)==7u,"handle close ABI");
static_assert(static_cast<uint32_t>(ReleaseRuntimeShutdownResult::bootstrap_pin_release_failed)==8u,"pin release ABI");
static_assert(static_cast<uint32_t>(ReleaseRuntimeShutdownResult::environment_cleanup_pending)==9u,"environment cleanup ABI");
static_assert(static_cast<uint32_t>(ReleaseRuntimeShutdownResult::service_cleanup_pending)==10u,"service cleanup ABI");
static_assert(static_cast<uint32_t>(preloader_l::runtime::ReleaseBootstrapWaitResult::handle_close_failed)==4u,"bootstrap handle ABI");
int main(){ return 0; }
