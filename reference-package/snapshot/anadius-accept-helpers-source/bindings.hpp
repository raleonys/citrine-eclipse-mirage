#pragma once
#include "helpers.hpp"

namespace anadius::socket_recovered {
// Selected0x90 adapter. For owning retry use OwnedAcceptMemory::prearmed_api(),
// not pinned_api(): the owning retry entry already arms before the first close.
class BoundedAcceptRetryApi final:public RetryAcceptApi {
public:explicit BoundedAcceptRetryApi(AsyncAcceptApi& api):api_(api){}
    AcceptExResult accept(Socket,Socket,std::span<std::byte>,std::uint32_t,OVERLAPPED&)override;
private:AsyncAcceptApi& api_;
};
struct AcceptRetryBinding {
    std::uintptr_t identity=0;RetryEngine* engine=nullptr;RetryAcceptApi* api=nullptr;RetryCancelApi* cancel=nullptr;
};
// Composition only. Caller owns all references/spans, binds before publication,
// and serializes calls under the same parent operation. Remaining application
// lower supplies environment/pending-owner/categories/handler only.
class RecoveredAcceptLower final:public AcceptLower {
public:
    RecoveredAcceptLower(AcceptLower& remaining,SocketFactoryApi& factory,AcceptorApi& sockets,
        Api& errors,DispatcherApi& completion,std::span<AcceptRetryBinding> bindings)
        :remaining_(remaining),factory_(factory),sockets_(sockets),errors_(errors),completion_(completion),bindings_(bindings){}
    void construct_environment(IoHandlerEnvironment& e,std::uintptr_t a)override{remaining_.construct_environment(e,a);}
    Socket create_socket(std::uint32_t,std::int32_t,std::int32_t,Error&)override;
    void reset_owned_socket(Socket&)override;
    void reset_pending_owner(AcceptPending& p)override{remaining_.reset_pending_owner(p);}
    Category misc_category()override{return remaining_.misc_category();}
    Category success_category()override{return remaining_.success_category();}
    void retry(AsyncAcceptOperation&,CancelOperation&)override;
    void invoke(AcceptHandler& h)override{remaining_.invoke(h);}
private:
    AcceptLower& remaining_;SocketFactoryApi& factory_;AcceptorApi& sockets_;Api& errors_;
    DispatcherApi& completion_;std::span<AcceptRetryBinding> bindings_;
};
} // namespace anadius::socket_recovered
