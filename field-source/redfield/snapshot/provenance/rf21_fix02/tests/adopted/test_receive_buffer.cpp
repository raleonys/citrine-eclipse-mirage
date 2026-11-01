#include "preloader_l/net/receive_buffer.hpp"
using namespace preloader_l::net;
int main(){
 ReceiveBuffer b{}; clear_receive_buffer(&b); if(b.size!=0 || receive_buffer_available(b)!=receive_buffer_capacity) return 1;
 const uint8_t a[]={'a','b','c'}; const uint8_t d[]={'d','e'};
 if(append_receive_bytes(&b,a,sizeof a)!=ReceiveBufferStatus::ok || b.size!=3) return 2;
 if(append_receive_bytes(&b,d,sizeof d)!=ReceiveBufferStatus::ok || b.size!=5) return 3;
 if(consume_receive_bytes(&b,2)!=ReceiveBufferStatus::ok || b.size!=3 || b.bytes[0]!='c' || b.bytes[1]!='d' || b.bytes[2]!='e') return 4;
 if(consume_receive_bytes(&b,4)!=ReceiveBufferStatus::invalid_argument || b.size!=3) return 5;
 uint8_t large[receive_buffer_capacity]{};
 clear_receive_buffer(&b); if(append_receive_bytes(&b,large,sizeof large)!=ReceiveBufferStatus::ok || receive_buffer_available(b)!=0) return 6;
 if(append_receive_bytes(&b,a,1)!=ReceiveBufferStatus::capacity || b.size!=receive_buffer_capacity) return 7;
 if(append_receive_bytes(nullptr,a,1)!=ReceiveBufferStatus::invalid_argument) return 8;
 if(append_receive_bytes(&b,nullptr,1)!=ReceiveBufferStatus::invalid_argument) return 9;
 return 0;
}
