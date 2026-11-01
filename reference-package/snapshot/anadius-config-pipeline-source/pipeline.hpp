#pragma once
#include "../anadius-file-source/binary_io.hpp"
#include "../anadius-parser-source/parser.hpp"

namespace anadius::config_pipeline {
// Logical source composition of 019bce -> 019d1a -> 019d42/019d52.
// The order provider describes the FIRST selected Config2 copy, not the later
// section copies in 019ad0. Factory and provider must outlive this object.
class DocumentPipeline {
public:
    DocumentPipeline(file_recovered::Factory& files,
                     const parser::BinaryIterationOrder& selected_copy_order)
        : files_(files), order_(selected_copy_order) {}
    configuration::Node read(const configuration::Path&) const;
private:
    file_recovered::Factory& files_;
    const parser::BinaryIterationOrder& order_;
};

class UnsupportedCollectionOrder : public std::logic_error {
public:
    UnsupportedCollectionOrder();
};
// A deliberate integration bound: the first-copy document is sufficient for
// scalar lookups. Ordered collections require the later reused section-copy
// history, which is not reconstructed here. Reject them BEFORE log/apply.
void require_scalar_application(const configuration::Node& document);

// Implements only the file/parser boundary. Every path, locale, log, licensing,
// persistence and OS-effect method inherited from Dependencies remains pure.
// This bridge never silently passes first-copy order off as final consumer order.
class ScalarDependencies : public configuration::Dependencies {
public:
    explicit ScalarDependencies(const DocumentPipeline& pipeline) : pipeline_(pipeline) {}
    configuration::Node load_document(const configuration::Path&) final;
private:
    const DocumentPipeline& pipeline_;
};
} // namespace anadius::config_pipeline
