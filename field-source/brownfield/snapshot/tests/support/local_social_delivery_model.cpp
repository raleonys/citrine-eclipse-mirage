#include "local_social_delivery_model.hpp"
namespace preloader_l { namespace runtime {
LocalSocialModelDelivery model_social_delivery(LocalSocialStore& store,
    LocalPrincipalLease lease, LocalSocialAction action, uint64_t target,
    const uint8_t* frame, size_t frame_size, LocalSocialModelWrite writer,
    void* writer_context, LocalSocialResult* prepare_detail) noexcept {
    if(prepare_detail)*prepare_detail=LocalSocialResult::invalid_output;
    if(!frame||!frame_size||frame_size>65536u||!writer)
        return LocalSocialModelDelivery::invalid_input;
    LocalSocialPending pending{};
    const auto prepared=store.prepare(lease,action,target,&pending);
    if(prepare_detail)*prepare_detail=prepared;
    if(prepared!=LocalSocialResult::ok)
        return LocalSocialModelDelivery::prepare_rejected;
    const auto receipt=writer(writer_context,frame,frame_size);
    const bool coherent=(
        (receipt.outcome==LocalSocialWireOutcome::no_bytes && receipt.bytes_sent==0u)||
        (receipt.outcome==LocalSocialWireOutcome::partial_bytes &&
         receipt.bytes_sent>0u && receipt.bytes_sent<frame_size)||
        (receipt.outcome==LocalSocialWireOutcome::complete_bytes &&
         receipt.bytes_sent==frame_size));
    if(!coherent){
        store.rollback(&pending);
        return LocalSocialModelDelivery::inconsistent_wire_receipt;
    }
    if(receipt.outcome==LocalSocialWireOutcome::no_bytes){
        store.rollback(&pending);
        return LocalSocialModelDelivery::no_bytes_rollback;
    }
    if(receipt.outcome==LocalSocialWireOutcome::partial_bytes){
        store.rollback(&pending);
        return LocalSocialModelDelivery::partial_wire_ambiguous;
    }
    const auto committed=store.commit(&pending);
    if(committed!=LocalSocialResult::ok){
        store.rollback(&pending);
        return LocalSocialModelDelivery::commit_after_wire_failed;
    }
    return LocalSocialModelDelivery::committed_local_only;
}
} }
