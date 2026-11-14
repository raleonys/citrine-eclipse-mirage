// SPDX-License-Identifier: GPL-3.0-only

#include "laureondo/platform/file_copy.hpp"

namespace laureondo::file_copy {
std::uint32_t file_identity_04895c(Handle handle,Identity& out,SystemCalls& system) {
    const auto error = system.file_id_info(handle,out);
    if(error!=50 && error!=87)return error;
    LegacyIdentity legacy{};
    const auto fallback_error = system.legacy_file_info(handle,legacy);
    if(fallback_error)return fallback_error;
    out.volume = legacy.volume;
    // RE copies nFileIndexHigh followed by nFileIndexLow, then eight zero bytes.
    // Do not reinterpret this as the opposite-order integer representation.
    for(unsigned i=0;i<4;++i) {
        out.file_id[i] = static_cast<std::uint8_t>(legacy.index_high>>(8*i));
        out.file_id[i+4] = static_cast<std::uint8_t>(legacy.index_low>>(8*i));
    }
    for(unsigned i=8;i<16;++i)out.file_id[i]=0;
    return 0;
}

Result copy_file_048d20(const Path& source,const Path& target,
                       std::uint32_t options,SystemCalls& system) {
    const auto mode = options & 0xfu;
    if(mode==2)return system.copy(source,target,false);
    const auto first = system.copy(source,target,true);
    if(first.error!=80 || mode==0)return first;

    const auto share = mode==1 ? 3u : 1u;
    const auto from = system.open_attributes(source,share);
    if(from.handle==invalid_handle && from.error)return {false,from.error};
    const auto to = system.open_attributes(target,share);
    auto close_one = [&](Handle handle) {
        if(handle!=invalid_handle && !system.close(handle))system.abort_close_failure();
    };
    auto close_both = [&] {close_one(to.handle);close_one(from.handle);};
    if(to.handle==invalid_handle && to.error) {
        close_one(from.handle);
        return {false,to.error};
    }
    if(mode==4) {
        std::int64_t source_time{},target_time{};
        auto error = system.last_write(from.handle,source_time);
        if(!error)error=system.last_write(to.handle,target_time);
        if(error) {close_both();return {false,error};}
        // Assembly048e96/048e9a uses a signed comparison (JGE).
        if(target_time<source_time) {
            close_both();
            return system.copy(source,target,false);
        }
    }
    Identity source_id{},target_id{};
    auto error = file_identity_04895c(from.handle,source_id,system);
    if(!error)error=file_identity_04895c(to.handle,target_id,system);
    if(!error && source_id==target_id)error=32; // ERROR_SHARING_VIOLATION
    close_both();
    return {false,error};
}
}
