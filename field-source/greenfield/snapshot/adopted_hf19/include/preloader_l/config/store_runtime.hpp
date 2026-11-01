#pragma once
#include <stddef.h>
#include <stdint.h>
#include "preloader_l/config/settings_values.hpp"

namespace preloader_l { namespace runtime { namespace model_abi { struct ReleaseServiceObject548; } } }
namespace preloader_l { namespace config {

enum class StoreRuntimeStatus : uint8_t {
    not_loaded = 0,
    ok,
    invalid_argument,
    invalid_xml,
    resource_error,
    invalid_hex,
    number_range,
};

struct StoreByteView {
    const uint8_t* data;
    size_t size;
};

struct StoreEntitlementView {
    uint64_t numeric_id;
    uint64_t offer_id;
    uint64_t release_ms;
    uint64_t flags;
    uint32_t category;

    ValueView tag;
    ValueView icon;
    ValueView title;
    ValueView description;
    ValueView img_thumb;
    ValueView img_cropped;
    ValueView img_hero;
    ValueView vendor;
    ValueView key;
    ValueView hash;

    size_t feature_desc_count;
    size_t feature_flag_count;

    StoreByteView proto_bytes;
    StoreByteView field2;
    StoreByteView pad;
};


struct StoreMaterializationSource {
    void* context;
    bool (*loaded)(void* context) noexcept;
    bool (*present)(void* context) noexcept;
    size_t (*entitlement_count)(void* context) noexcept;
    bool (*entitlement)(void* context, size_t index, StoreEntitlementView* output) noexcept;
    StoreByteView (*preamble)(void* context) noexcept;
    StoreByteView (*trailer)(void* context) noexcept;
    ValueView (*feature_desc)(void* context, size_t entitlement_index, size_t feature_index) noexcept;
    ValueView (*feature_flag)(void* context, size_t entitlement_index, size_t feature_index) noexcept;
};


struct RuntimeStoreReadLease {
    uint64_t generation;
    uint8_t slot;
    bool active;
};

// Pins one immutable runtime-store generation. Borrowed views obtained through
// runtime_store_source_for_lease remain valid until end_runtime_store_read().
bool begin_runtime_store_read(RuntimeStoreReadLease* lease) noexcept;
void end_runtime_store_read(RuntimeStoreReadLease* lease) noexcept;
StoreMaterializationSource runtime_store_source_for_lease(RuntimeStoreReadLease* lease) noexcept;

// Evidence-backed reconstruction of the first AE00 output vector.
//
// Selection:
//   document root -> FIRST direct Store child -> direct Entitlement children.
// Root itself being Store is not selected. Names are case-sensitive.
// Duplicate NumericId records remain separate.
//
// AE00 auxiliary vectors #2/#3 are Store-level Base64 byte spans:
//   PreambleB64 -> prefix/preamble bytes
//   TrailerB64  -> trailer bytes
// They are exposed separately below and feed the curio D5F0 payload builder.
// Successful updates replace the committed fixed snapshot. Failed updates
// preserve the previous successful snapshot and only update runtime_store_status().
// The implementation remains freestanding and does not use TinyXML2, STL,
// exceptions, or the original MSVC object ABI. 2.8.4 removes the historical
// clean-only 4096-record / 65536-feature / 2 MiB scratch caps: request-scoped
// parser storage now grows dynamically and reports true allocation failure.
StoreRuntimeStatus set_runtime_store_xml(const char* xml, size_t length) noexcept;
bool clear_runtime_store_retryable() noexcept;
void clear_runtime_store() noexcept;

StoreRuntimeStatus runtime_store_status() noexcept;
// True when at least one Store parse has committed a snapshot.
// This is independent of runtime_store_status(), which reports the
// most recent parse attempt and can be an error while the prior
// committed snapshot is still retained.
bool runtime_store_loaded() noexcept;
bool runtime_store_present() noexcept;
size_t runtime_store_entitlement_count() noexcept;
bool runtime_store_entitlement(size_t index, StoreEntitlementView* output) noexcept;
StoreByteView runtime_store_preamble() noexcept;
StoreByteView runtime_store_trailer() noexcept;

ValueView runtime_store_feature_desc(size_t entitlement_index,
                                     size_t feature_index) noexcept;
ValueView runtime_store_feature_flag(size_t entitlement_index,
                                     size_t feature_index) noexcept;
#if defined(_WIN32)
void bind_runtime_store_model_service(
    preloader_l::runtime::model_abi::ReleaseServiceObject548* service) noexcept;
void unbind_runtime_store_model_service() noexcept;

// 2.8.2 transactional staging API (carried forward from 2.8.0). This parses into request-scoped scratch and
// materializes only into the supplied off-live candidate service. It never
// publishes process-global parser state.
StoreRuntimeStatus stage_runtime_store_xml_to_model_service(
    const char* xml, size_t length,
    preloader_l::runtime::model_abi::ReleaseServiceObject548* candidate_service) noexcept;
void note_runtime_store_model_commit(bool authoritative) noexcept;
#endif

} }
