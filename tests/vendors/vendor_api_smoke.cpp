// Vendor API/compilation fixture only. This does not define a config grammar,
// filesystem policy, production transport owner or final runtime integration.
#include <asio.hpp>
#include <filesystem>
#include <fstream>
#include <tinyxml2.h>
#include <chrono>
#include <iostream>
#include <stdexcept>
#include <string>
#ifdef _WIN32
#include <MinHook.h>
#endif

int main() {
    const char* stage="Asio context";
    try {
        asio::io_context context;
        unsigned calls=0;
        asio::post(context,[&] { ++calls; });
        stage="Asio timer";
        asio::steady_timer timer(context,std::chrono::milliseconds(0));
        timer.async_wait([&](const asio::error_code& error) { if(!error) ++calls; });
        context.run();
        if(calls!=2) return 10;
        stage="filesystem";
        const auto path=std::filesystem::path("alpha")/"beta";
        if(path.filename().generic_string()!="beta" || path.parent_path().generic_string()!="alpha") return 11;
        static_assert(TINYXML2_MAX_ELEMENT_DEPTH==500,"Use the supplied vendor default without a compatibility patch");
        stage="TinyXML2";
        tinyxml2::XMLDocument document;
        if(document.Parse("<Vendor value='ok'/>")!=tinyxml2::XML_SUCCESS) return 13;
        if(std::string(document.RootElement()->Attribute("value"))!="ok") return 14;
#ifdef _WIN32
        stage="MinHook lifecycle";
        if(MH_Initialize()!=MH_OK) return 15;
        if(MH_Uninitialize()!=MH_OK) return 16;
#endif
        std::cout << "PASS: supplied vendor API fixture (Asio, TinyXML2; standard filesystem; MinHook lifecycle on Windows); PEGTL has a separate toolchain probe; not production integration\n";
        return 0;
    } catch(const std::system_error& error) {
        std::cerr << stage << ": " << error.what() << "; code=" << error.code().value()
                  << "; category=" << error.code().category().name() << '\n';
        return 1;
    } catch(const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
