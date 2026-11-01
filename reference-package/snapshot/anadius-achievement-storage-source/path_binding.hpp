#pragma once
#include "storage.hpp"

namespace anadius::achievement_storage {
// Optional source-only sharing of the two native global cells:
// XML at0x1800a3de8 (+408), INI at0x1800a3e88 (+4a8).
// Bind once, passively, before operations/publication or borrowed path views.
// The cell owner must outlive all providers, graph borrowers and worker joins.
class PathBinding {
public:
    virtual ~PathBinding()=default;
    virtual void bind_achievement_paths(Paths&)=0;
};
} // namespace anadius::achievement_storage
