#include "preloader_l/platform/crypt_path_restore.hpp"

namespace preloader_l { namespace platform {

bool should_restore_crypt_query_path(uint32_t object_type,
                                     const void* object,
                                     uint32_t expected_content,
                                     uint32_t expected_format,
                                     uint32_t flags,
                                     bool caller_is_main) noexcept
{
    return caller_is_main &&
           object == nullptr &&
           object_type == crypt_query_restore_object_type &&
           expected_content == crypt_query_restore_expected_content &&
           expected_format == crypt_query_restore_expected_format &&
           flags == crypt_query_restore_flags;
}

} }
