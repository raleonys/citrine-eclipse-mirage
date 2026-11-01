// Portable lexical HF12 filesystem contract. Actual Windows file opening,
// UTF-16 GetModuleFileNameW and /MD CRT acceptance remain native-only gates.
#include "../../src/config/module_relative_filesystem.hpp"
#include <cassert>
#include <cstdint>
#include <string>
#include <vector>
#include <iostream>

using preloader_l::config::path_detail::join_module_relative;

static std::vector<uint16_t> to_units(const char16_t* s) {
    std::vector<uint16_t> out;
    while (*s) out.push_back(static_cast<uint16_t>(*s++));
    return out;
}

static void run_one(const char16_t* input, const char16_t* leaf,
                    const char16_t* expected, bool allowed) {
    auto module=to_units(input), name=to_units(leaf);
    uint16_t result[32768]={};
    size_t used=123u;
    bool ok=join_module_relative(module.data(),module.size(),name.data(),name.size(),
                                 result,32768,&used);
    assert(ok==allowed);
    if (allowed) {
        auto reference=to_units(expected);
        assert(used==reference.size());
        for(size_t i=0;i<used;++i) assert(result[i]==reference[i]);
        assert(!result[used]);
    } else assert(used==0);
}
int main() {
#if defined(_WIN32)
    run_one(u"C:\\User\\test\\preloader_l.dll",u"surge.cfg",u"C:\\User\\test\\surge.cfg",true);
    run_one(u"C:\\テスト\\preloader_l.dll",u"surge.png",u"C:\\テスト\\surge.png",true);
    run_one(u"C:\\preloader_l.dll",u"surge.cfg",u"C:\\surge.cfg",true);
    run_one(u"\\\\server\\share\\mod\\preloader_l.dll",u"surge.cfg",u"\\\\server\\share\\mod\\surge.cfg",true);
    run_one(u"preloader_l.dll",u"surge.cfg",u"",false);
#else
    run_one(u"C:/User/test/preloader_l.dll",u"surge.cfg",u"C:/User/test/surge.cfg",true);
    run_one(u"C:/テスト/preloader_l.dll",u"surge.png",u"C:/テスト/surge.png",true);
    run_one(u"C:/preloader_l.dll",u"surge.cfg",u"C:/surge.cfg",true);
    run_one(u"preloader_l.dll",u"surge.cfg",u"",false);
#endif
    run_one(u"preloader_l.dll",u"/surge.cfg",u"",false);
    run_one(u"preloader_l.dll",u"sub\\surge.cfg",u"",false);
    auto name=to_units(u"surge.cfg"); auto module=to_units(u"C:/usr/preloader_l.dll");
    uint16_t output[32]={}; size_t used=999;
    assert(!join_module_relative(nullptr,0,name.data(),name.size(),output,32,&used) && used==0);
    assert(!join_module_relative(module.data(),module.size(),name.data(),name.size(),output,1,&used) && used==0);
    assert(!join_module_relative(module.data(),module.size(),name.data(),name.size(),output,32,nullptr));
    const uint16_t with_nul[]={ 'C', ':', '/', 'x', 0, '/', 'y' };
    assert(!join_module_relative(with_nul,7,name.data(),name.size(),output,32,&used) && used==0);
    const uint16_t empty[]={0};
    assert(!join_module_relative(module.data(),module.size(),empty,0,output,32,&used) && used==0);
    // Aliased input/output is how the shipping Win32 path buffer is used.
    uint16_t alias[64]={};
#if defined(_WIN32)
    auto native=to_units(u"C:\\usr\\preloader_l.dll");
#else
    auto native=module;
#endif
    for(size_t i=0;i<native.size();++i) alias[i]=native[i];
    assert(join_module_relative(alias,native.size(),name.data(),name.size(),alias,64,&used));
    auto check=to_units(u"C:/usr/surge.cfg");
#if defined(_WIN32)
    // On Windows forward slashes may be normalized: alias check is only
    // exercised for native backslash paths by the Windows-specific cases.
#else
    assert(used==check.size());
    for(size_t i=0;i<used;++i) assert(alias[i]==check[i]);
#endif
    std::cout << "HF12_FILESYSTEM_PATH_PORTABLE: PASS\n";
}
