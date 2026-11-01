#include "preloader_l/net/frame_ingress.hpp"
#include "preloader_l/xml/request_parser.hpp"
#include <cstring>
using namespace preloader_l;
int main(){
 net::IngressState s{}; net::reset_ingress(&s); xml::TextSpan frame{};
 constexpr char a[]="<LSX><Request id=\"44\"><Get";
 constexpr char b[]="Config/></Request></LSX>";
 if(net::feed_ingress(&s,reinterpret_cast<const uint8_t*>(a),sizeof(a)-1,&frame)!=net::IngressStatus::need_more || frame.size!=0) return 1;
 if(net::feed_ingress(&s,reinterpret_cast<const uint8_t*>(b),sizeof(b)-1,&frame)!=net::IngressStatus::frame_ready) return 2;
 constexpr char full[]="<LSX><Request id=\"44\"><GetConfig/></Request></LSX>";
 if(frame.size!=sizeof(full)-1 || std::memcmp(frame.data,full,frame.size)!=0) return 3;
 if(s.feed_events!=2 || s.bytes_fed!=sizeof(full)-1 || s.frames_ready!=1) return 4;
 if(net::consume_ingress_frame(&s,frame.size)!=net::IngressStatus::ok || s.receive.size!=0 || s.frames_consumed!=1) return 5;
 constexpr char two[]="<LSX><Request id=\"1\"><GetConfig/></Request></LSX><LSX><Request id=\"2\"><GetConfig/></Request></LSX>";
 if(net::feed_ingress(&s,reinterpret_cast<const uint8_t*>(two),sizeof(two)-1,&frame)!=net::IngressStatus::frame_ready) return 6;
 const size_t first=frame.size; if(first==0 || first>=sizeof(two)-1) return 7;
 if(net::consume_ingress_frame(&s,first)!=net::IngressStatus::ok) return 8;
 if(net::peek_ingress_frame(&s,&frame)!=net::IngressStatus::frame_ready || frame.size==0) return 9;
 if(net::consume_ingress_frame(&s,frame.size)!=net::IngressStatus::ok || s.receive.size!=0) return 10;
 net::reset_ingress(&s); constexpr char bad[]="BAD";
 if(net::feed_ingress(&s,reinterpret_cast<const uint8_t*>(bad),sizeof(bad)-1,&frame)!=net::IngressStatus::invalid_frame || s.errors!=1) return 11;
 return 0;
}
