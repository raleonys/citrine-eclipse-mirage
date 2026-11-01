#include "preloader_l/runtime/service_abi.hpp"
#include <cstddef>
#include <cstdint>

using namespace preloader_l::runtime::model_abi;

int main() {
    if (sizeof(ModelXmlStrPair18) != 0x18) return 1;
    if (offsetof(ModelXmlStrPair18, flags) != 0x00) return 2;
    if (offsetof(ModelXmlStrPair18, start) != 0x08) return 3;
    if (offsetof(ModelXmlStrPair18, end) != 0x10) return 4;
    if (sizeof(ModelXmlNodeBase68) != 0x68) return 5;
    if (offsetof(ModelXmlNodeBase68, value) != 0x18) return 6;
    if (offsetof(ModelXmlNodeBase68, first_child) != 0x38) return 7;
    if (offsetof(ModelXmlNodeBase68, last_child) != 0x40) return 8;
    if (offsetof(ModelXmlNodeBase68, next) != 0x50) return 9;
    if (sizeof(ModelXmlElement78) != 0x78) return 10;
    if (offsetof(ModelXmlElement78, root_attribute) != 0x70) return 11;
    if (sizeof(ModelXmlAttribute50) != 0x50) return 12;
    if (offsetof(ModelXmlAttribute50, name) != 0x08) return 13;
    if (offsetof(ModelXmlAttribute50, value) != 0x20) return 14;
    if (offsetof(ModelXmlAttribute50, next) != 0x40) return 15;
    if (sizeof(ModelXmlUnlinked68) != 0x68) return 16;
    if (sizeof(ModelXmlMemPoolOpaque98) != 0x98) return 17;
    if (sizeof(ModelXmlDocument370) != 0x370) return 18;
    if (offsetof(ModelXmlDocument370, unlinked) != 0x0A8) return 19;
    if (offsetof(ModelXmlDocument370, element_pool) != 0x110) return 20;
    if (offsetof(ModelXmlDocument370, attribute_pool) != 0x1A8) return 21;
    if (offsetof(ModelXmlDocument370, text_pool) != 0x240) return 22;
    if (offsetof(ModelXmlDocument370, comment_pool) != 0x2D8) return 23;
    return 0;
}
