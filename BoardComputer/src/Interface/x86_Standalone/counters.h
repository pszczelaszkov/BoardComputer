#ifndef __COUNTERS__
#define __COUNTERS__

#include <signal.h>
#include <stdint.h>

#define COUNTERS_FUELTICKSPERSECOND 125000
/* Real-time signals for future simulators (sigqueue with pulse amount). */
#define COUNTERS_SIG_FUEL  (SIGRTMIN + 0)
#define COUNTERS_SIG_SPEED (SIGRTMIN + 1)

void COUNTERS_disable_signals(void);
void COUNTERS_enable_signals(void);
void COUNTERS_init(void);
 
#define COUNTERS_ATOMIC_BLOCK for(uint8_t _x = (COUNTERS_disable_signals(),1); \
                                (_x); \
                                _x = (COUNTERS_enable_signals(),0))
#endif
