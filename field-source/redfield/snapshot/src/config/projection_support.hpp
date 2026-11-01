// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include "config2_pegtl.hpp"
#include <string>
#include <string_view>
#include <vector>

namespace preloader_l::config_projection {
// Parser-validated spans borrow source bytes only during the leaf callback.
// Published records own their strings; this view never performs decoding.
class SourceView final {
public:
    explicit SourceView(const char* source) noexcept : source_(source) {}
    std::string_view text(config2_pegtl::Span span) const noexcept {
        return {source_+span.off,span.len};
    }
    bool is(config2_pegtl::Span span,std::string_view literal) const noexcept {
        return text(span)==literal;
    }
private:
    const char* source_;
};

template<class Record>
Record* find_record(std::vector<Record>& records,std::string_view id,
                    std::string Record::* identity) noexcept {
    for(auto& record:records)
        if(record.*identity==id)return &record;
    return nullptr;
}

inline bool replace_text(std::string& target,std::string_view value) noexcept {
    try {target.assign(value.data(),value.size());return true;}
    catch(...) {return false;}
}
}
