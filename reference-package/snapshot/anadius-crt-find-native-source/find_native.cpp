#include "find_native.hpp"
#include <cstring>
#include <limits>
#include <stdexcept>

namespace anadius::crt_find_native {
static_assert(sizeof(void*) == 8 && sizeof(HANDLE) == 8);
static_assert(sizeof(WCHAR) == 2 && sizeof(BOOL) == 4 && sizeof(DWORD) == 4);
static_assert(sizeof(WIN32_FIND_DATAW) == 592);
static_assert(offsetof(WIN32_FIND_DATAW, cFileName) == 44);
static_assert(sizeof(WIN32_FIND_DATAW::cFileName) == 520);
namespace {
Address address(const void* pointer) noexcept {
    return static_cast<Address>(reinterpret_cast<std::uintptr_t>(pointer));
}
HANDLE handle_pointer(Address handle) noexcept {
    return reinterpret_cast<HANDLE>(static_cast<std::uintptr_t>(handle));
}
std::span<const std::byte> name_span(const WIN32_FIND_DATAW& data) noexcept {
    return std::as_bytes(std::span{data.cFileName});
}
}
Api system_api() noexcept {
    return {&::FindFirstFileExW, &::FindNextFileW, &::FindClose};
}
Session::Session(Api api) : api_(api) {
    if (api.first == nullptr || api.next == nullptr || api.close == nullptr)
        throw std::invalid_argument("find bridge requires all three typed APIs");
}
Address Session::first(Address wide_pattern, Address& filename) {
    // Prepare the owning object and map node before the SDK call. Inserting an
    // extracted node below does not allocate or relocate the typed record.
    Records pending;
    pending.emplace(0, std::make_unique<WIN32_FIND_DATAW>());
    auto node = pending.extract(pending.begin());
    auto* data = node.mapped().get();
    const auto native_handle = api_.first(
        reinterpret_cast<LPCWSTR>(static_cast<std::uintptr_t>(wide_pattern)),
        FindExInfoStandard, data, FindExSearchNameMatch, nullptr, 0);
    const auto handle = address(native_handle);
    if (handle == std::numeric_limits<Address>::max()) return handle;
    node.key() = handle;
    const auto insertion = active_.insert(std::move(node));
    if (!insertion.inserted)
        throw std::domain_error("find API returned a duplicate live handle");
    filename = address(data->cFileName);
    return handle;
}
std::int32_t Session::next(Address handle, Address& filename) {
    const auto it = active_.find(handle);
    if (it == active_.end()) throw std::domain_error("next requires a live find handle");
    const auto result = api_.next(handle_pointer(handle), it->second.get());
    if (result != 0) filename = address(it->second->cFileName);
    return static_cast<std::int32_t>(result);
}
void Session::close(Address handle) {
    const auto it = active_.find(handle);
    if (it == active_.end()) throw std::domain_error("close requires a live find handle");
    (void)api_.close(handle_pointer(handle));
    // The observed caller ignores BOOL. Even failure ends its record lifetime.
    active_.erase(it);
}
const WIN32_FIND_DATAW& Session::record(Address handle) const {
    const auto it = active_.find(handle);
    if (it == active_.end()) throw std::domain_error("storage requires a live find handle");
    return *it->second;
}
std::span<const std::byte> Session::filename_storage(Address handle) const {
    return name_span(record(handle));
}
bool Session::contains(Address location, std::uint64_t bytes) const noexcept {
    for (const auto& [handle, data] : active_) {
        (void)handle;
        const auto span = name_span(*data);
        const auto begin = address(span.data());
        if (location >= begin && location - begin <= span.size() &&
            bytes <= span.size() - (location - begin)) return true;
    }
    return false;
}
std::uint8_t Session::read_u8(Address location) const {
    if (!contains(location, 1)) throw std::domain_error("byte outside live find filename storage");
    std::uint8_t result;
    std::memcpy(&result, reinterpret_cast<const void*>(static_cast<std::uintptr_t>(location)), 1);
    return result;
}
std::uint16_t Session::read_u16(Address location) const {
    if (!contains(location, 2)) throw std::domain_error("word outside live find filename storage");
    std::uint16_t result;
    std::memcpy(&result, reinterpret_cast<const void*>(static_cast<std::uintptr_t>(location)), 2);
    return result;
}
} // namespace anadius::crt_find_native
