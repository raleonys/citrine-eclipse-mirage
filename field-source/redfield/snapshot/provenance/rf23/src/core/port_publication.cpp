// SPDX-License-Identifier: GPL-3.0-only
#include "redfield/port_publication.hpp"
#include <charconv>
#include <exception>
namespace redfield {
PortPublication::~PortPublication(){if(held_)std::terminate();}
bool PortPublication::publish(std::uint16_t port) noexcept {
    if(!port || held_)return false;
    if(!backend_.read && !backend_.write)return true; // explicitly disabled
    if(!backend_.read || !backend_.write)return false;
    try {
        std::optional<std::string> prior;
        if(!backend_.read(backend_.context,prior))return false;
        char text[6]{};auto converted=std::to_chars(text,text+5,port);
        if(converted.ec!=std::errc{})return false;
        std::string value(text,converted.ptr);
        if(!backend_.write(backend_.context,value.c_str()))return false;
        previous_=std::move(prior);published_=std::move(value);held_=true;
        return true;
    }catch(...){return false;}
}
bool PortPublication::restore() noexcept {
    if(!held_)return true;
    try {
        std::optional<std::string> current;
        if(!backend_.read(backend_.context,current))return false;
        // A later coordinated owner supersedes our publication. Preserve it.
        if(current && *current==published_ && !backend_.write(backend_.context,previous_?previous_->c_str():nullptr))return false;
        held_=false;previous_.reset();published_.clear();return true;
    }catch(...){return false;}
}
}
