// SPDX-License-Identifier: GPL-3.0-only
#include "src/transport/read_window.hpp"
#include <algorithm>
namespace preloader_l::server {
// Algorithm retained from the packet's server.cpp receive window calculation.
std::size_t receive_request_size(std::size_t size,std::size_t capacity,std::size_t maximum){
 if(size>=maximum)return 0;
 const auto spare=std::min(capacity,maximum)-size;
 return std::min({maximum-size,std::size_t{0x10000},std::max(spare,std::size_t{0x200})});
}
}
