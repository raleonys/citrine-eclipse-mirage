// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include <ctime>
#include <chrono>
#include <string>
inline std::string utc_text(std::time_t value){
    std::tm result{};
#if defined(_WIN32)
    if(gmtime_s(&result,&value))return {};
#else
    if(!gmtime_r(&value,&result))return {};
#endif
    char text[32]{};if(std::strftime(text,sizeof(text),"%Y-%m-%dT%H:%M:%S",&result)!=19)return {};
    return text;
}

inline std::time_t utc_wall_seconds(){
    return static_cast<std::time_t>(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count());
}
