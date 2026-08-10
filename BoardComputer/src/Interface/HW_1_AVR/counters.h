#ifndef __COUNTERS__
#define __COUNTERS__
#include <util/atomic.h>
#define COUNTERS_FUELTICKSPERSECOND 125000

void COUNTERS_init(void);

#define COUNTERS_ATOMIC_BLOCK      ATOMIC_BLOCK(ATOMIC_FORCEON)
#endif
