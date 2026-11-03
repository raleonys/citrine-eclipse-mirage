// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include "preloader_l/net/transport_recycle.hpp"
#include <memory>
#include <vector>

namespace preloader_l::socket {
// Explicit host compatibility storage. Native map construction/insert/rehash,
// allocator/string ABI and teardown are not reconstructed by this backend.
// Nodes have stable owned addresses; operations are borrowed, not deleted.
class OwnedRecycleMap {
public:
    explicit OwnedRecycleMap(std::size_t bucket_count);
    OwnedRecycleMap(const OwnedRecycleMap&)=delete;
    OwnedRecycleMap& operator=(const OwnedRecycleMap&)=delete;
    OwnedRecycleMap(OwnedRecycleMap&&)=delete;
    OwnedRecycleMap& operator=(OwnedRecycleMap&&)=delete;
    CancelMap& map(){return map_;}
    RecycleMap& metadata(){return metadata_;}
    CancelNode& active_sentinel(){return active_;}
    CancelNode& free_sentinel(){return free_;}
    std::size_t owned_nodes()const{return nodes_.size();}
    CancelNode& insert(Socket);
    void append(Socket,CancelOperation&);
private:
    CancelNode active_,free_;
    std::vector<CancelBucket> buckets_;
    std::vector<std::unique_ptr<CancelNode>> nodes_;
    CancelMap map_;
    RecycleMap metadata_;
};
} // namespace preloader_l::socket
