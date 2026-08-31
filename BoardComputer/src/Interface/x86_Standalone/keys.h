#ifndef __KEYS__
#define __KEYS__

#include <signal.h>

/* Real-time signal for key edges (sigqueue with key id 0=Enter, 1=Down). */
#define KEYS_SIG (SIGRTMIN + 3)

void KEYS_init(void);

#endif
