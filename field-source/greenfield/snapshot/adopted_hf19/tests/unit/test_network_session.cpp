#include "preloader_l/net/session_state.hpp"
using namespace preloader_l::net;
namespace {
struct Fixture { unsigned closed; };
TransportStatus rx(void*, uint8_t*, size_t, size_t* n) noexcept { if(n)*n=0; return TransportStatus::would_block; }
TransportStatus tx(void*, const uint8_t*, size_t n, size_t* sent) noexcept { if(sent)*sent=n; return TransportStatus::ok; }
void close(void* p) noexcept { if(p) ++static_cast<Fixture*>(p)->closed; }
}
int main(){
 Fixture f{}; TransportEndpoint ep{&f,rx,tx,close}; SessionState s{}; reset_session(&s);
 if(s.phase!=SessionPhase::idle) return 1;
 if(bind_session(&s,42,ep)!=SessionStatus::ok || s.phase!=SessionPhase::connected || s.session_id!=42) return 2;
 if(bind_session(&s,43,ep)!=SessionStatus::already_connected) return 3;
 if(note_session_receive(&s,7)!=SessionStatus::ok || note_session_receive(&s,5)!=SessionStatus::ok) return 4;
 if(note_session_send(&s,9)!=SessionStatus::ok) return 5;
 if(s.received_bytes!=12 || s.sent_bytes!=9 || s.receive_events!=2 || s.send_events!=1) return 6;
 if(close_session(&s)!=SessionStatus::ok || s.phase!=SessionPhase::closed || f.closed!=1 || s.close_count!=1) return 7;
 if(note_session_receive(&s,1)!=SessionStatus::not_connected) return 8;
 if(close_session(&s)!=SessionStatus::not_connected || f.closed!=1) return 9;
 reset_session(&s); TransportEndpoint bad{}; if(bind_session(&s,1,bad)!=SessionStatus::invalid_transport) return 10;
 return 0;
}
