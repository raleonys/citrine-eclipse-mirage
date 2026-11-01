#include "engine.hpp"
#include <stdexcept>

namespace anadius::host_platform {
namespace {
struct Error {DWORD value=::GetLastError();~Error(){::SetLastError(value);}void restore()const{::SetLastError(value);}void capture(){value=::GetLastError();}};
struct Proof {
    std::shared_ptr<recovered_hooks::OwnedEngine> engine;
    native_context::Quiescence external;
    bool run(){
        Error error;if(!engine->empty())return false;
        error.restore();bool result;
        try{result=external();}catch(...){error.capture();throw;}
        error.capture();return result&&engine->empty();
    }
};
}
void select_owned_engine(AssemblyInputs& in,std::shared_ptr<recovered_hooks::OwnedEngine> engine){
    Error error;
    if(!engine||in.engine||in.code_lifetime||!in.external_quiescence)
        throw std::invalid_argument("Vacant engine/code, owning engine and external quiescence required");
    auto proof=std::make_shared<Proof>(Proof{engine,in.external_quiescence});
    native_context::Quiescence selected=[proof]{return proof->run();};
    // Complete all allocating/copying work before no-throw publication.
    in.external_quiescence.swap(selected);in.engine=engine;in.code_lifetime=std::move(engine);
}
}
