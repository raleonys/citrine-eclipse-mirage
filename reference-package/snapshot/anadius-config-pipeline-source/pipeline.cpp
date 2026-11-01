#include "pipeline.hpp"

namespace anadius::config_pipeline {
configuration::Node DocumentPipeline::read(const configuration::Path& path) const {
    // read_binary preserves stream initial-state and short-read/NUL-tail behavior.
    // parse already removes exactly ONE UTF-8 BOM. Do not pre-strip or decode.
    const auto bytes = file_recovered::read_binary_03b3f0(path, files_);
    parser::State parsed; // Never reuse a previous document's parser actions.
    (void)parser::parse(bytes, parsed); // Current recovered grammar returns true or throws.
    configuration::Node selected;
    if (const auto* config = parsed.root.find("Config2"))
        selected = parser::to_configuration_node(*config, order_);
    // Only Config2 is copied in 019d52. Unselected root subtrees are NOT converted.
    // Missing Config2 has the same empty value as the original default insertion;
    // root mutation/allocator layout is deliberately not emulated.
    configuration::Node document;
    document.children.emplace("Config2", std::move(selected));
    document.iteration_order.emplace_back("Config2"); // Logical API wrapper only.
    return document; // Owns all strings and nodes after parser/input destruction.
}

UnsupportedCollectionOrder::UnsupportedCollectionOrder()
    : std::logic_error("Config2 section-copy consumer order is not reconstructed") {}

void require_scalar_application(const configuration::Node& document) {
    const auto& config = document.child("Config2");
    if (!config.child("Content").children.empty() ||
        !config.child("Entitlements").children.empty())
        throw UnsupportedCollectionOrder();
    const auto& achievements = config.child("Achievements");
    for (const auto* key : {"AchievementNames", "AchievementTotals", "SteamIdOverrides"})
        if (!achievements.child(key).children.empty()) throw UnsupportedCollectionOrder();
}

configuration::Node ScalarDependencies::load_document(const configuration::Path& path) {
    auto document = pipeline_.read(path);
    require_scalar_application(document);
    return document;
}
} // namespace anadius::config_pipeline
