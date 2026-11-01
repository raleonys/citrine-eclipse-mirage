// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include <string>
#include <string_view>
namespace preloader_l::test_packet_xml {
struct Result {int code;bool error;std::string diagnostic,tree;};
Result parse(std::string_view);
inline void field(std::string& out,std::string_view value){out+=std::to_string(value.size());out+=':';out.append(value);}
}
