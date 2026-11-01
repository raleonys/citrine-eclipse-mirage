#include "bindings.hpp"
#include <bit>

namespace anadius::socket_recovered {
AcceptExResult BoundedAcceptRetryApi::accept(Socket listener,Socket accepted,std::span<std::byte> bytes,
    std::uint32_t n,OVERLAPPED& ov){
    if(n!=0x90||bytes.size()!=288)throw UnsupportedInput("Owned retry requires the exact0x90 endpoint profile");
    return api_.accept(listener,accepted,bytes,ov);
}
Socket RecoveredAcceptLower::create_socket(std::uint32_t family,std::int32_t type,std::int32_t protocol,Error& error){
    return create_socket_029df0(std::bit_cast<std::int32_t>(family),type,protocol,error,factory_,sockets_,errors_);
}
void RecoveredAcceptLower::reset_owned_socket(Socket& holder){reset_socket_holder_029590(holder,sockets_,errors_);}
void RecoveredAcceptLower::retry(AsyncAcceptOperation& op,CancelOperation& selected){
    // Generic native source supports early cancellation and alternate packets.
    // Owning composition does not claim their multi-publication/kernel-finality
    // protocol. Reject before native effects; never cast opaque image tokens.
    if(op.retry_context||&selected!=&op.operation)
        throw UnsupportedInput("Owned retry cancellation and alternate packet profiles are unresolved");
    AcceptRetryBinding* found=nullptr;
    for(auto& b:bindings_)if(b.identity==op.retry_engine_identity){
        if(found)throw UnsupportedInput("Ambiguous retry engine binding");found=&b;
    }
    if(!found||!found->engine||!found->api||!found->cancel||!op.packet)
        throw UnsupportedInput("Retry resource binding is incomplete");
    AcceptRetryContext context{factory_,sockets_,errors_,*found->api,*found->cancel,completion_};
    retry_accept_029660(*found->engine,op.listener,op.accepted,std::bit_cast<std::int32_t>(op.protocol),
        1,6,op.address_buffer,0x90,nullptr,selected,*op.packet,context);
}
} // namespace anadius::socket_recovered
