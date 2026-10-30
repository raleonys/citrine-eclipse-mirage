#include "preloader_l/runtime/control.h"
#include <stdio.h>

int main(void) {
    SURGE();
    if(DllShutdown(20000)!=DLL_SHUTDOWN_OK)return 3;
    if(DllShutdown(0)!=DLL_SHUTDOWN_OK)return 4;
    puts("PASS: C17 consumer linked against the three-export application import library; real worker startup and shutdown");
    return 0;
}
