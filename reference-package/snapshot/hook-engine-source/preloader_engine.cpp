#include "adapters.hpp"
namespace recovered_hooks {
int PreloaderEngine::initialize() {return engine_.initialize();}
int PreloaderEngine::create(void* t,void* d,void** original) {return engine_.create(t,d,original);}
int PreloaderEngine::enable(void* t) {return engine_.enable(t);}
}
