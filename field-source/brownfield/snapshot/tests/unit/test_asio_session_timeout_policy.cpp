// Deterministic boundaries: production private policy, simulated steady clock.
// Not Windows IOCP or actual network timing acceptance.
#include "asio_session_timeout_policy.hpp"
#include <chrono>
#include <cstdlib>
#include <iostream>
using P=preloader_l::runtime::AsioSessionTimeoutPolicy;
using namespace std::chrono_literals;
static int checks=0;
#define CHECK(x) do {++checks;if (!(x)) {std::cerr<<"timeout policy check "<<checks<<" failed\n";std::exit(1);}}while(0)
int main() {
 P policy;
 const P::Time start{100s};
 P::Time next{};
 CHECK(!policy.next(&next));
 CHECK(policy.expired(start)==P::Expired::none);
 policy.begin_receive(start,false);
 CHECK(policy.next(&next) && next==start+30s);
 CHECK(policy.expired(start+29999ms)==P::Expired::none);
 CHECK(policy.expired(start+30s)==P::Expired::receive);
 policy.receive_progress(start+12s,false);
 CHECK(policy.next(&next) && next==start+42s);
 CHECK(policy.expired(start+30s)==P::Expired::none);
 policy.receive_progress(start+14s,true);
 CHECK(policy.next(&next) && next==start+24s);
 policy.receive_progress(start+23s,true); // fragments do not extend 10s frame cap
 CHECK(policy.next(&next) && next==start+24s);
 CHECK(policy.expired(start+23999ms)==P::Expired::none);
 CHECK(policy.expired(start+24s)==P::Expired::receive);
 policy.end_receive();
 CHECK(!policy.next(&next));
 policy.begin_receive(start+25s,true); // coalesced leftover starts next frame
 CHECK(policy.next(&next) && next==start+35s);
 policy.end_receive();
 policy.begin_send(start+50s);
 CHECK(policy.next(&next) && next==start+55s);
 CHECK(policy.expired(start+54999ms)==P::Expired::none);
 policy.send_progress(start+54s); // underlying transport progressed
 CHECK(policy.next(&next) && next==start+59s);
 CHECK(policy.expired(start+55s)==P::Expired::none);
 CHECK(policy.expired(start+59s)==P::Expired::send);
 policy.end_send();
 CHECK(!policy.next(&next));
 policy.begin_receive(start+60s,false);
 policy.begin_send(start+60s);
 CHECK(policy.next(&next) && next==start+65s); // earliest wins
 policy.send_progress(start+64s);
 CHECK(policy.next(&next) && next==start+69s);
 policy.receive_progress(start+64s,true);
 CHECK(policy.next(&next) && next==start+69s);
 policy.clear();
 CHECK(!policy.next(&next));
 std::cout<<"ASIO_SESSION_TIMEOUT_POLICY: PASS "<<checks<<"\n";
}
