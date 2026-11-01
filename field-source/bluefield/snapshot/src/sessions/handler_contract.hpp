// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include <string>
#include <string_view>
namespace preloader_l::server {
struct Handler {
 virtual ~Handler()=default;
 virtual void reset_02bc70()=0;
 virtual std::string initial_message_02d540()=0;
 virtual std::string process_message_033d70(std::string_view)=0;
};
}
