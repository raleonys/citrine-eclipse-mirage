// Bounded, source-linked completion contract for sync sender and Asio IOCP callbacks.
// The real GrantAchievement/Event queue owners are intentionally NOT mocked here.
#include "socket_exchange_write_completion.hpp"
#include <cstdlib>
#include <iostream>
namespace w = preloader_l::protocol::detail;
static unsigned assertions=0;
static void check(bool condition) {
    ++assertions;
    if(!condition) { std::cerr<<"FAIL assertion "<<assertions<<"\n"; std::exit(1); }
}
int main() {
    using P=w::DeliveryWritePhase; using D=w::DeliveryWriteDecision;
    w::DeliveryWriteCursor c{};
    check(!w::begin_delivery_writes(nullptr,1,0));
    check(!w::begin_delivery_writes(&c,0,0));
    check(c.phase==P::failed);
    check(!w::begin_delivery_writes(&c,static_cast<size_t>(-1),1));
    check(c.phase==P::failed);
    check(w::complete_delivery_write(nullptr,true,1)==D::reject);
    for(size_t response=1;response<=96;++response) {
        for(size_t event=0;event<=20;++event) {
            check(w::begin_delivery_writes(&c,response,event));
            check(c.phase==P::response);
            check(w::complete_delivery_write(&c,true,response)==
                (event ? D::event_needed : D::response_done));
            check(c.phase==(event ? P::event : P::complete));
            if(event) {
                check(w::complete_delivery_write(&c,true,event)==D::transaction_done);
                check(c.phase==P::complete);
            }
            check(w::complete_delivery_write(&c,true,response)==D::reject);
            // An explicit write error cannot authorize Response commit, even
            // when the callback's `transferred` equals the entire frame size.
            check(w::begin_delivery_writes(&c,response,event));
            check(w::complete_delivery_write(&c,false,response)==D::reject);
            check(c.phase==P::failed);
            check(w::complete_delivery_write(&c,true,response)==D::reject);
            if(response>1) {
                check(w::begin_delivery_writes(&c,response,event));
                check(w::complete_delivery_write(&c,true,response-1)==D::reject);
                check(c.phase==P::failed);
            }
            if(event) {
                check(w::begin_delivery_writes(&c,response,event));
                check(w::complete_delivery_write(&c,true,response)==D::event_needed);
                check(w::complete_delivery_write(&c,false,event)==D::reject);
                check(c.phase==P::failed); // Response already accepted; Event retry is caller's job
                check(w::begin_delivery_writes(&c,response,event));
                check(w::complete_delivery_write(&c,true,response)==D::event_needed);
                check(w::complete_delivery_write(&c,true,event-1)==D::reject);
                check(c.phase==P::failed);
            }
        }
    }
    std::cout << "HF12_STAGE03_SOURCE_DELIVERY_COMPLETION PASS assertions=" << assertions << "\n";
}
