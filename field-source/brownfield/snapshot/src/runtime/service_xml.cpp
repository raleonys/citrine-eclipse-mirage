#include "service_policy_internal.hpp"

namespace preloader_l { namespace runtime { namespace model_release_service_policy_internal {

void initialize_model_xml_document(model_abi::ModelXmlDocument370* doc) noexcept {
    if (!doc) return;
    zero_bytes(doc, sizeof(*doc));
    doc->node.document = reinterpret_cast<uintptr_t>(doc);
    doc->process_entities = 1u;
    doc->whitespace_mode = 0;
}

void repair_model_xml_document_self(model_abi::ModelXmlDocument370* doc) noexcept {
    if (doc) doc->node.document = reinterpret_cast<uintptr_t>(doc);
}

namespace {
// D02: ModelXmlDocument370 exposes only the model ABI's raw char-buffer pointer;
// it has no field that can carry the allocator byte count.  Keep exact
// allocation ownership in a private side ledger instead of recovering it with
// a C-string scan, so embedded NUL bytes never truncate the release size.  The model
// service mutation boundary is externally serialized; this ledger is private
// to that owner and is deliberately not a new public/shipping API.
struct ModelXmlAllocationRecord {
    char* pointer;
    size_t bytes;
    bool published;
};

constexpr size_t kModelXmlAllocationRecordCapacity = 32u;
ModelXmlAllocationRecord g_model_xml_allocations[kModelXmlAllocationRecordCapacity]{};

size_t find_model_xml_allocation(char* pointer) noexcept {
    if (!pointer) return kModelXmlAllocationRecordCapacity;
    for (size_t i = 0; i < kModelXmlAllocationRecordCapacity; ++i)
        if (g_model_xml_allocations[i].pointer == pointer) return i;
    return kModelXmlAllocationRecordCapacity;
}

size_t find_free_model_xml_allocation_slot() noexcept {
    for (size_t i = 0; i < kModelXmlAllocationRecordCapacity; ++i)
        if (!g_model_xml_allocations[i].pointer) return i;
    return kModelXmlAllocationRecordCapacity;
}

void clear_model_xml_allocation(size_t index) noexcept {
    if (index >= kModelXmlAllocationRecordCapacity) return;
    g_model_xml_allocations[index] = ModelXmlAllocationRecord{};
}

bool release_recorded_model_xml_buffer(char* pointer) noexcept {
    const size_t index = find_model_xml_allocation(pointer);
    if (index >= kModelXmlAllocationRecordCapacity) return false;
    const size_t bytes = g_model_xml_allocations[index].bytes;
    if (!bytes) return false;
    if (!release_bytes(pointer, bytes)) return false;
    clear_model_xml_allocation(index);
    return true;
}

bool retry_unpublished_model_xml_allocations() noexcept {
    bool ok = true;
    for (size_t i = 0; i < kModelXmlAllocationRecordCapacity; ++i) {
        auto& record = g_model_xml_allocations[i];
        if (!record.pointer || record.published) continue;
        char* pointer = record.pointer;
        const size_t bytes = record.bytes;
        if (!bytes || !release_bytes(pointer, bytes)) {
            ok = false;
            continue;
        }
        clear_model_xml_allocation(i);
    }
    return ok;
}

bool release_model_xml_buffer_only(model_abi::ModelXmlDocument370* doc) noexcept {
    if (!doc || !doc->char_buffer) return true;
    auto* payload = reinterpret_cast<char*>(doc->char_buffer);
    return release_recorded_model_xml_buffer(payload);
}
} // namespace

bool release_model_xml_document(model_abi::ModelXmlDocument370* doc) noexcept {
    if (!doc) return true;
    if (!release_model_xml_buffer_only(doc)) return false;
    initialize_model_xml_document(doc);
    // A failed cleanup of an uncommitted replacement remains ledger-owned and
    // is retried on the next model-document operation rather than becoming an
    // untracked leak.  Report failure until all such ownership is retired.
    return retry_unpublished_model_xml_allocations();
}

bool assign_model_xml_document(model_abi::ModelXmlDocument370* doc,
                               const char* xml, size_t length) noexcept {
    if (!doc || (!xml && length)) return false;
    if (length == static_cast<size_t>(-1)) return false;

    // Retire any earlier uncommitted candidate first, then reserve ledger
    // capacity before allocating so every successful allocation is tracked.
    (void)retry_unpublished_model_xml_allocations();
    const size_t record_index = find_free_model_xml_allocation_slot();
    if (record_index >= kModelXmlAllocationRecordCapacity) return false;

    const size_t allocation_bytes = length + 1u;
    auto* replacement_buffer = static_cast<char*>(allocate_bytes(allocation_bytes));
    if (!replacement_buffer) return false;
    g_model_xml_allocations[record_index] =
        ModelXmlAllocationRecord{replacement_buffer, allocation_bytes, false};

    for (size_t i = 0; i < length; ++i) replacement_buffer[i] = xml[i];
    replacement_buffer[length] = 0;

    model_abi::ModelXmlDocument370 replacement{};
    initialize_model_xml_document(&replacement);
    replacement.char_buffer = reinterpret_cast<uintptr_t>(replacement_buffer);
    replacement.write_bom = length >= 3u &&
        static_cast<uint8_t>(xml[0]) == 0xEFu &&
        static_cast<uint8_t>(xml[1]) == 0xBBu &&
        static_cast<uint8_t>(xml[2]) == 0xBFu ? 1u : 0u;
    replacement.error_id = 0;
    replacement.error_line_num = 0;
    replacement.parsing_depth = 0;
    replacement.parse_cur_line_num = 1;
    for (size_t i = 0; i < length; ++i)
        if (xml[i] == '\n') ++replacement.parse_cur_line_num;

    // Transactional replacement: an old-buffer release failure preserves the
    // published document.  The new candidate is either released immediately
    // or remains as an unpublished ledger entry for a later retry.
    if (!release_model_xml_buffer_only(doc)) {
        (void)release_recorded_model_xml_buffer(replacement_buffer);
        return false;
    }

    *doc = replacement;
    repair_model_xml_document_self(doc);
    g_model_xml_allocations[record_index].published = true;
    return true;
}

} } }
