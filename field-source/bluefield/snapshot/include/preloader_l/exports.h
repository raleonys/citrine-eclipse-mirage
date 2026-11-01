/* SPDX-License-Identifier: GPL-3.0-only */
#pragma once
#include <stdint.h>
#ifdef _WIN32
#define PRELOADER_L_CALL __stdcall
#else
#define PRELOADER_L_CALL
#endif
#ifdef __cplusplus
extern "C" {
#endif
/* SURGE is the established no-op loader compatibility export. */
void SURGE(void);
/* Call outside DllMain while retaining the caller's LoadLibrary reference.
   0 = clean and unloadable, 1 = concurrent shutdown, 2 = worker timeout,
   3 = wait failure, 7 = handle close pending, 8 = self-pin release pending,
   9 = environment restoration pending.
   timeout_ms == UINT32_MAX waits indefinitely. Nonzero forbids unload. */
uint32_t PRELOADER_L_CALL DllShutdown(uint32_t timeout_ms);
#ifdef __cplusplus
}
#endif
