#pragma once
#include <stddef.h>
#include <stdint.h>

namespace preloader_l { namespace runtime { namespace model_abi {

// Physical x64/MSVC layout curio evidence from the model 0x7000 allocation.
// uses this curio 0x558 partition as live ownership storage.
// Client jthread records are materialized in +0x30 and the asynchronous accept
// jthread record is materialized in +0x18. binds the curio
// stop-state source/lifetime counts; additionally binds the exact
// request-stop callback-head/current-callback transient state. The clean
// ReleaseServerContext carries only clean coordination/bookkeeping that is not
// represented by curio service-object bytes.
struct MsvcVector24 {
    uintptr_t begin;
    uintptr_t end;
    uintptr_t capacity;
};

struct ModelJthread24 {
    uintptr_t handle;                      // +0x00
    uint32_t thread_id;                    // +0x08
    uint32_t pad_00c;                      // +0x0C
    uintptr_t stop_state;                  // +0x10
};

// Active 0xF020/0x473C4/0x473F0 evidence: recursive SRW-backed
// synchronization state embedded at service+0x048. Only fields with direct
// producer/consumer evidence are named; the middle bytes remain opaque.
struct ModelRecursiveMutex50 {
    uint32_t type_state;                   // +0x00, constructor writes 2
    uint8_t reserved_004[0x0C];            // +0x04
    uintptr_t srw_lock_word;               // +0x10
    uint8_t reserved_018[0x30];            // +0x18
    uint32_t owner_thread_id;              // +0x48, initial 0xFFFFFFFF
    uint32_t recursion_count;              // +0x4C, initial 0
};

struct ModelStopState20;

// MSVC stop_callback base layout corroborated against the frozen image's
// 0x37510 request-stop unlink/invoke sequence. No callback attach/detach producer
// is instantiated in this image, but the request-stop consumer is present.
struct ModelStopCallback20 {
    ModelStopState20* parent;          // +0x00
    ModelStopCallback20* next;         // +0x08
    ModelStopCallback20* prev;         // +0x10
    uintptr_t invoke_fn;                    // +0x18, void (__cdecl*)(ModelStopCallback20*)
};

struct ModelStopState20 {
    int32_t lifetime_refs;                 // +0x00
    int32_t source_refs_and_stop;          // +0x04 (low bit = stop requested)
    uintptr_t callback_state;              // +0x08 tagged _Locked_pointer callback head
    uintptr_t current_callback;            // +0x10 currently invoking callback / atomic wait slot
    uint32_t requesting_thread_id;         // +0x18
    uint32_t pad_01c;                      // +0x1C
};

struct MsvcString32 {
    uint8_t storage[16];
    uint64_t size;
    uint64_t capacity;
};

// 0x36CA0/0x36BF0 curio worker-launch closure. The endpoint string is
// moved into this closure before thread creation and destroyed by the worker.
struct ModelWorkerClosure38 {
    uintptr_t stop_state;                   // +0x00
    uintptr_t service;                      // +0x08
    uintptr_t accepted_socket;              // +0x10
    MsvcString32 remote_address;            // +0x18
};

// partitions the previously opaque +0x098..+0x467 region from
// byte-pinned constructor/destructor evidence. These remain layout mirrors;
// configuration path/XMLDocument/Settings semantic policy is source-converged;
// compiler-private XML pool bytes remain deliberately opaque.

struct ModelStoreNestedD8 {
    MsvcString32 icon;                      // +0x000 -> record +0x030
    MsvcString32 title;                     // +0x020 -> record +0x050
    MsvcString32 description;               // +0x040 -> record +0x070
    MsvcString32 img_thumb;                 // +0x060 -> record +0x090
    MsvcString32 img_cropped;               // +0x080 -> record +0x0B0
    MsvcString32 img_hero;                  // +0x0A0 -> record +0x0D0
    MsvcVector24 feature_desc;               // +0x0C0 -> record +0x0F0
};

struct ModelStoreEntitlement1E0 {
    uint64_t numeric_id;                    // +0x000
    uint64_t category;                      // +0x008
    MsvcString32 tag;                       // +0x010
    ModelStoreNestedD8 nested;          // +0x030
    uint64_t release_ms;                    // +0x108
    uint64_t offer_id;                      // +0x110
    MsvcString32 vendor;                    // +0x118
    MsvcVector24 feature_flag;               // +0x138
    MsvcString32 key;                       // +0x150
    MsvcString32 hash;                      // +0x170
    uint64_t flags;                         // +0x190
    MsvcVector24 proto_bytes;                // +0x198
    MsvcVector24 field2;                     // +0x1B0
    MsvcVector24 pad;                        // +0x1C8
};

// Active 0x17C80/0x37C20/0x38900 evidence binds the configuration path
// as the x64 MSVC wide-string shape: 8 UTF-16 code units of SSO storage,
// size/capacity in code units, SSO capacity 7 and heap storage beyond 7.
struct ModelWidePathString32 {
    uint16_t storage[8];
    uint64_t size;
    uint64_t capacity;
};

// TinyXML2 core offsets are independently pinned by 0x3BA0/0x3DC0/0x4070/
// 0x48F0. The model build's private MemPoolT objects are larger than the
// vendored clean copy, so 2.8.7 names only the byte-stable core and leaves the
// pool tail opaque rather than fabricating compiler-private pool bytes.
struct ModelXmlStrPair18 {
    uint32_t flags;                        // +0x00, 0x1A50 tests flags here
    uint32_t reserved_004;                 // +0x04
    uintptr_t start;                       // +0x08
    uintptr_t end;                         // +0x10
};

struct ModelXmlNodeBase68 {
    uintptr_t vtable_identity;              // +0x00, model-specific vtable
    uintptr_t document;                     // +0x08, XMLDocument ctor writes self
    uintptr_t parent;                       // +0x10
    ModelXmlStrPair18 value;            // +0x18
    int32_t parse_line_num;                 // +0x30
    uint32_t reserved_034;                  // +0x34
    uintptr_t first_child;                  // +0x38
    uintptr_t last_child;                   // +0x40
    uintptr_t prev;                         // +0x48
    uintptr_t next;                         // +0x50
    uintptr_t user_data;                    // +0x58
    uintptr_t mem_pool;                     // +0x60, null for XMLDocument root
};

// 0xDB20 + 0x1A50 byte-pin the element/attribute traversal geometry used
// while configuration XML is converted into Settings. These declarations are
// physical layout mirrors only; shipping runtime does not fabricate a retained
// node graph.
struct ModelXmlElement78 {
    ModelXmlNodeBase68 node;            // +0x00
    int32_t closing_type;                    // +0x68
    uint32_t reserved_06c;                   // +0x6C
    uintptr_t root_attribute;                // +0x70
};

struct ModelXmlAttribute50 {
    uintptr_t vtable_identity;               // +0x00
    ModelXmlStrPair18 name;              // +0x08
    ModelXmlStrPair18 value;             // +0x20
    int32_t parse_line_num;                  // +0x38
    uint32_t reserved_03c;                   // +0x3C
    uintptr_t next;                          // +0x40
    uintptr_t mem_pool;                      // +0x48
};

// XMLDocument ctor 0x3BA0 partitions the private tail into a 0x68-byte
// unlinked-node container followed by four 0x98-byte pool objects. Their
// internal compiler/private fields remain intentionally opaque.
struct ModelXmlUnlinked68 {
    uint8_t storage[0x68];
};

struct ModelXmlMemPoolOpaque98 {
    uint8_t storage[0x98];
};

struct ModelXmlDocument370 {
    ModelXmlNodeBase68 node;             // +0x000
    uint8_t write_bom;                       // +0x068
    uint8_t process_entities;                // +0x069, ctor true
    uint16_t reserved_06a;                   // +0x06A
    int32_t error_id;                        // +0x06C
    int32_t whitespace_mode;                 // +0x070, preserve=0
    uint32_t reserved_074;                   // +0x074
    ModelXmlStrPair18 error_str;         // +0x078
    int32_t error_line_num;                  // +0x090
    uint32_t reserved_094;                   // +0x094
    uintptr_t char_buffer;                   // +0x098, owned parse buffer
    int32_t parse_cur_line_num;              // +0x0A0
    int32_t parsing_depth;                    // +0x0A4
    ModelXmlUnlinked68 unlinked;         // +0x0A8
    ModelXmlMemPoolOpaque98 element_pool;// +0x110
    ModelXmlMemPoolOpaque98 attribute_pool;// +0x1A8
    ModelXmlMemPoolOpaque98 text_pool;   // +0x240
    ModelXmlMemPoolOpaque98 comment_pool;// +0x2D8
};

// Curio 0xDD70/0xD870/0x18070/0xE720 semantics identify the +0x390
// object as the x64 MSVC unordered-map shape used by the model. 2.8.6
// models the observed semantic ABI directly rather than embedding a private
// PLRMAP sidecar header in these bytes.
struct ModelSettingsNode50 {
    uintptr_t next;                       // +0x00
    uintptr_t prev;                       // +0x08
    MsvcString32 key;                     // +0x10
    MsvcString32 value;                   // +0x30
};

struct ModelSettingsUnorderedMap40 {
    float max_load_factor;                // +0x00, constructor writes 1.0f
    uint32_t reserved_004;                // +0x04
    uintptr_t sentinel;                   // +0x08, 0x50-byte list sentinel
    uint64_t size;                        // +0x10
    MsvcVector24 bucket_pairs;            // +0x18, 2 pointers per bucket
    uint64_t mask;                        // +0x30, bucket_count - 1
    uint64_t bucket_count;                // +0x38, constructor starts at 8
};

struct ReleaseServiceConfig3D0 {
    ModelWidePathString32 path;                // +0x000 -> service +0x098
    ModelXmlDocument370 document;          // +0x020 -> service +0x0B8
    ModelSettingsUnorderedMap40 settings;      // +0x390 -> service +0x428
};

struct ReleaseServiceObject548 {
    uint16_t port;                         // +0x000
    uint8_t pad_002[6];                   // +0x002
    uintptr_t listener_socket;            // +0x008
    uint8_t active;                       // +0x010
    uint8_t pad_011[7];                   // +0x011
    ModelJthread24 accept_thread;      // +0x018
    MsvcVector24 client_records;           // +0x030: vector<ModelJthread24>
    ModelRecursiveMutex50 mutex_state; // +0x048
    ReleaseServiceConfig3D0 config;       // +0x098
    MsvcString32 content;                  // +0x468
    MsvcString32 entitlements;             // +0x488
    MsvcString32 offers;                   // +0x4A8
    MsvcString32 achievements;             // +0x4C8
    MsvcVector24 store_entitlements;       // +0x4E8
    MsvcVector24 preamble_bytes;           // +0x500
    MsvcVector24 trailer_bytes;            // +0x518
    MsvcVector24 store_payload;            // +0x530
};

struct RefCountControlBlock10 {
    uintptr_t vftable;                    // +0x00; model VA 0x1801080C8
    int32_t strong_count;                 // +0x08; initialized to 1
    int32_t weak_count;                   // +0x0C; initialized to 1
};

struct ReleaseServiceAllocation558 {
    RefCountControlBlock10 control;        // allocation +0x000
    ReleaseServiceObject548 service;       // allocation +0x010
};

static_assert(sizeof(MsvcVector24) == 0x18, "model vector ABI drift");
static_assert(sizeof(ModelJthread24) == 0x18, "model jthread ABI drift");
static_assert(sizeof(ModelRecursiveMutex50) == 0x50, "model mutex ABI drift");
static_assert(sizeof(ModelStopCallback20) == 0x20, "model stop-callback ABI drift");
static_assert(sizeof(ModelStopState20) == 0x20, "model stop-state ABI drift");
static_assert(sizeof(MsvcString32) == 0x20, "model string ABI drift");
static_assert(sizeof(ModelWorkerClosure38) == 0x38, "model worker closure ABI drift");

static_assert(sizeof(ModelStoreNestedD8) == 0xD8, "model store nested ABI drift");
static_assert(sizeof(ModelStoreEntitlement1E0) == 0x1E0, "model entitlement record ABI drift");
static_assert(offsetof(ModelStoreEntitlement1E0, category) == 0x008, "record Category offset drift");
static_assert(offsetof(ModelStoreEntitlement1E0, tag) == 0x010, "record Tag offset drift");
static_assert(offsetof(ModelStoreEntitlement1E0, nested) == 0x030, "record nested offset drift");
static_assert(offsetof(ModelStoreEntitlement1E0, nested) + offsetof(ModelStoreNestedD8, feature_desc) == 0x0F0, "record FeatureDesc offset drift");
static_assert(offsetof(ModelStoreEntitlement1E0, release_ms) == 0x108, "record ReleaseMs offset drift");
static_assert(offsetof(ModelStoreEntitlement1E0, offer_id) == 0x110, "record OfferId offset drift");
static_assert(offsetof(ModelStoreEntitlement1E0, vendor) == 0x118, "record Vendor offset drift");
static_assert(offsetof(ModelStoreEntitlement1E0, feature_flag) == 0x138, "record FeatureFlag offset drift");
static_assert(offsetof(ModelStoreEntitlement1E0, key) == 0x150, "record Key offset drift");
static_assert(offsetof(ModelStoreEntitlement1E0, hash) == 0x170, "record Hash offset drift");
static_assert(offsetof(ModelStoreEntitlement1E0, flags) == 0x190, "record Flags offset drift");
static_assert(offsetof(ModelStoreEntitlement1E0, proto_bytes) == 0x198, "record ProtoBytes offset drift");
static_assert(offsetof(ModelStoreEntitlement1E0, field2) == 0x1B0, "record Field2 offset drift");
static_assert(offsetof(ModelStoreEntitlement1E0, pad) == 0x1C8, "record Pad offset drift");

static_assert(sizeof(ModelWidePathString32) == 0x20, "model wide-path ABI drift");
static_assert(sizeof(ModelXmlStrPair18) == 0x18, "model XML StrPair ABI drift");
static_assert(offsetof(ModelXmlStrPair18, flags) == 0x00, "model XML StrPair flags offset drift");
static_assert(offsetof(ModelXmlStrPair18, start) == 0x08, "model XML StrPair start offset drift");
static_assert(offsetof(ModelXmlStrPair18, end) == 0x10, "model XML StrPair end offset drift");
static_assert(sizeof(ModelXmlNodeBase68) == 0x68, "model XMLNode ABI drift");
static_assert(offsetof(ModelXmlNodeBase68, value) == 0x18, "model XMLNode value offset drift");
static_assert(offsetof(ModelXmlNodeBase68, last_child) == 0x40, "model XMLNode last-child offset drift");
static_assert(offsetof(ModelXmlNodeBase68, next) == 0x50, "model XMLNode next offset drift");
static_assert(sizeof(ModelXmlElement78) == 0x78, "model XMLElement ABI drift");
static_assert(offsetof(ModelXmlElement78, root_attribute) == 0x70, "model XMLElement attribute-root offset drift");
static_assert(sizeof(ModelXmlAttribute50) == 0x50, "model XMLAttribute ABI drift");
static_assert(offsetof(ModelXmlAttribute50, name) == 0x08, "model XMLAttribute name offset drift");
static_assert(offsetof(ModelXmlAttribute50, value) == 0x20, "model XMLAttribute value offset drift");
static_assert(offsetof(ModelXmlAttribute50, next) == 0x40, "model XMLAttribute next offset drift");
static_assert(sizeof(ModelXmlUnlinked68) == 0x68, "model XML unlinked storage drift");
static_assert(sizeof(ModelXmlMemPoolOpaque98) == 0x98, "model XML pool extent drift");
static_assert(sizeof(ModelXmlDocument370) == 0x370, "model XMLDocument ABI drift");
static_assert(offsetof(ModelXmlNodeBase68, document) == 0x08, "model XMLNode document offset drift");
static_assert(offsetof(ModelXmlNodeBase68, first_child) == 0x38, "model XMLNode first-child offset drift");
static_assert(offsetof(ModelXmlDocument370, write_bom) == 0x68, "model XMLDocument BOM offset drift");
static_assert(offsetof(ModelXmlDocument370, error_id) == 0x6C, "model XMLDocument error offset drift");
static_assert(offsetof(ModelXmlDocument370, error_str) == 0x78, "model XMLDocument error-string offset drift");
static_assert(offsetof(ModelXmlDocument370, char_buffer) == 0x98, "model XMLDocument char-buffer offset drift");
static_assert(offsetof(ModelXmlDocument370, parse_cur_line_num) == 0xA0, "model XMLDocument line offset drift");
static_assert(offsetof(ModelXmlDocument370, parsing_depth) == 0xA4, "model XMLDocument depth offset drift");
static_assert(offsetof(ModelXmlDocument370, unlinked) == 0xA8, "model XMLDocument unlinked offset drift");
static_assert(offsetof(ModelXmlDocument370, element_pool) == 0x110, "model XMLDocument element-pool offset drift");
static_assert(offsetof(ModelXmlDocument370, attribute_pool) == 0x1A8, "model XMLDocument attribute-pool offset drift");
static_assert(offsetof(ModelXmlDocument370, text_pool) == 0x240, "model XMLDocument text-pool offset drift");
static_assert(offsetof(ModelXmlDocument370, comment_pool) == 0x2D8, "model XMLDocument comment-pool offset drift");
static_assert(sizeof(ModelSettingsNode50) == 0x50, "model settings node ABI drift");
static_assert(offsetof(ModelSettingsNode50, key) == 0x10, "model settings node key offset drift");
static_assert(offsetof(ModelSettingsNode50, value) == 0x30, "model settings node value offset drift");
static_assert(sizeof(ModelSettingsUnorderedMap40) == 0x40, "model settings unordered-map ABI drift");
static_assert(offsetof(ModelSettingsUnorderedMap40, sentinel) == 0x08, "model settings sentinel offset drift");
static_assert(offsetof(ModelSettingsUnorderedMap40, size) == 0x10, "model settings size offset drift");
static_assert(offsetof(ModelSettingsUnorderedMap40, bucket_pairs) == 0x18, "model settings bucket-vector offset drift");
static_assert(offsetof(ModelSettingsUnorderedMap40, mask) == 0x30, "model settings mask offset drift");
static_assert(offsetof(ModelSettingsUnorderedMap40, bucket_count) == 0x38, "model settings bucket-count offset drift");
static_assert(sizeof(ReleaseServiceConfig3D0) == 0x3D0, "model config aggregate ABI drift");
static_assert(offsetof(ReleaseServiceConfig3D0, document) == 0x020, "XMLDocument config offset drift");
static_assert(offsetof(ReleaseServiceConfig3D0, settings) == 0x390, "settings-map config offset drift");
static_assert(sizeof(RefCountControlBlock10) == 0x10, "model control-block ABI drift");
static_assert(sizeof(ReleaseServiceObject548) == 0x548, "model service ABI drift");
static_assert(sizeof(ReleaseServiceAllocation558) == 0x558, "model allocation ABI drift");

static_assert(offsetof(ReleaseServiceObject548, listener_socket) == 0x008, "socket offset drift");
static_assert(offsetof(ReleaseServiceObject548, active) == 0x010, "active offset drift");
static_assert(offsetof(ReleaseServiceObject548, accept_thread) == 0x018, "thread-state offset drift");
static_assert(offsetof(ReleaseServiceObject548, client_records) == 0x030, "client-vector offset drift");
static_assert(offsetof(ReleaseServiceObject548, mutex_state) == 0x048, "mutex offset drift");
static_assert(offsetof(ModelRecursiveMutex50, srw_lock_word) == 0x10, "model mutex lock-word offset drift");
static_assert(offsetof(ModelRecursiveMutex50, owner_thread_id) == 0x48, "model mutex owner offset drift");
static_assert(offsetof(ModelRecursiveMutex50, recursion_count) == 0x4C, "model mutex recursion offset drift");
static_assert(offsetof(ReleaseServiceObject548, config) == 0x098, "config offset drift");
static_assert(offsetof(ReleaseServiceObject548, config) + offsetof(ReleaseServiceConfig3D0, document) == 0x0B8, "XMLDocument service offset drift");
static_assert(offsetof(ReleaseServiceObject548, config) + offsetof(ReleaseServiceConfig3D0, settings) == 0x428, "settings-map service offset drift");
static_assert(offsetof(ReleaseServiceObject548, content) == 0x468, "Content offset drift");
static_assert(offsetof(ReleaseServiceObject548, entitlements) == 0x488, "Entitlements offset drift");
static_assert(offsetof(ReleaseServiceObject548, offers) == 0x4A8, "Offers offset drift");
static_assert(offsetof(ReleaseServiceObject548, achievements) == 0x4C8, "Achievements offset drift");
static_assert(offsetof(ReleaseServiceObject548, store_entitlements) == 0x4E8, "AE00 record vector offset drift");
static_assert(offsetof(ReleaseServiceObject548, preamble_bytes) == 0x500, "AE00 preamble vector offset drift");
static_assert(offsetof(ReleaseServiceObject548, trailer_bytes) == 0x518, "AE00 trailer vector offset drift");
static_assert(offsetof(ReleaseServiceObject548, store_payload) == 0x530, "D5F0 payload vector offset drift");
static_assert(offsetof(ReleaseServiceAllocation558, service) == 0x10, "service alias offset drift");

// Model addresses are evidence constants, not runtime relocation targets.
constexpr uintptr_t kModelControlVftableVa = UINT64_C(0x1801080C8);
constexpr size_t kModelVtableDestroySlot = 0x00u;
constexpr size_t kModelVtableDeleteThisSlot = 0x08u;
constexpr size_t kModelVtableScalarDeleteSlot = 0x10u;
constexpr size_t kModelStrongCountOffset = 0x08u;
constexpr size_t kModelWeakCountOffset = 0x0Cu;
constexpr uint32_t kModelDestroyRva = 0x7BD0u;
constexpr uint32_t kModelDeleteThisRva = 0x7C10u;
constexpr uint32_t kModelScalarDeletingDestructorRva = 0x7BE0u;
constexpr uint32_t kModelServiceDestructorRva = 0x7A20u;
constexpr uint32_t kModelServiceConstructorRva = 0xF020u;
constexpr uint32_t kModelServiceRunRva = 0x37750u;
constexpr uint32_t kModelConfigPathCopyRva = 0x37C20u;
constexpr uint32_t kModelConfigPathDestroyRva = 0x38900u;
constexpr uint32_t kModelXmlDocumentConstructRva = 0x3BA0u;
constexpr uint32_t kModelXmlDocumentDestroyRva = 0x3DC0u;
constexpr uint32_t kModelSettingsMapConstructRva = 0xDD70u;
constexpr uint32_t kModelSettingsMapClearRva = 0xE720u;
constexpr uint32_t kModelConfigLoadRva = 0x7EB0u;
constexpr uint32_t kModelConfigExtractSettingsRva = 0xDB20u;

} } }
