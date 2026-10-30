#include "preloader_l/net/transport_recycle_storage.hpp"

namespace preloader_l::socket_recovered {
OwnedRecycleMap::OwnedRecycleMap(std::size_t bucket_count)
    :buckets_(bucket_count),map_{buckets_,&active_},metadata_(map_,free_){
    if(!bucket_count)throw UnsupportedInput("Owned map requires at least one bucket");
    active_.next=active_.previous=&active_;
    free_.next=free_.previous=&free_;
    for(auto& bucket:buckets_)bucket={&active_,&active_};
}
CancelNode& OwnedRecycleMap::insert(Socket socket){
    auto& bucket=buckets_[socket%buckets_.size()];
    if(bucket.first!=&active_){
        auto* const end=bucket.last->next;
        for(auto* current=bucket.first;current!=end;current=current->next)
            if(current->socket==socket)return *current;
    }
    CancelNode* node;
    if(free_.next!=&free_){
        node=free_.next;
        node->previous->next=node->next;
        node->next->previous=node->previous;
        --metadata_.free_size;
    }else{
        auto fresh=std::make_unique<CancelNode>();
        node=fresh.get();nodes_.push_back(std::move(fresh));
    }
    node->socket=socket;node->head=nullptr;node->tail=nullptr;
    auto* const after=bucket.first==&active_?&active_:bucket.last->next;
    node->previous=after->previous;node->next=after;
    after->previous->next=node;after->previous=node;
    if(bucket.first==&active_)bucket.first=node;
    bucket.last=node;
    ++metadata_.map_size;++metadata_.active_size;
    return *node;
}
void OwnedRecycleMap::append(Socket socket,CancelOperation& operation){
    auto& node=insert(socket);
    operation.next=nullptr;
    if(node.tail)node.tail->next=&operation;
    else node.head=&operation;
    node.tail=&operation;
}
} // namespace preloader_l::socket_recovered
