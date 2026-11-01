#pragma once
#include <stddef.h>
#include <stdint.h>

namespace preloader_l { namespace runtime {

// Normalized QueryImage fallback asset policy. The project avatar remains
// surge.png and is resolved beside the loaded preloader_l.dll, matching the
// Correction-1 surge.cfg location model. No behavioral-branded cache directory or
// external provider module participates in this path. Windows module-path
// policy is acquired as UTF-16 and materialized here as semantic UTF-8.
bool normalized_avatar_path(char* output,size_t capacity,size_t* written) noexcept;

// Curio QueryImage signed size selector. Kept as an explicit clean-source
// contract even though the normalized offline provider resolves a single local
// project asset.
int32_t query_image_size_bucket(int32_t width) noexcept;

} }
