#include "counters.h"
#include "countersfeed.h"
#include "keys.h"

#include <signal.h>
#include <threads.h>

static mtx_t counters_lock;
static thrd_t counters_signal_thread;

static int counters_signal_loop(void* arg)
{
    sigset_t set;
    siginfo_t info;

    (void)arg;
    sigemptyset(&set);
    sigaddset(&set, COUNTERS_SIG_FUEL);
    sigaddset(&set, COUNTERS_SIG_SPEED);

    for(;;)
    {
        int sig = sigwaitinfo(&set, &info);
        if(sig < 0)
            continue;

        mtx_lock(&counters_lock);
        if(sig == COUNTERS_SIG_FUEL)
            COUNTERSFEED_count_fuelusage((uint16_t)info.si_value.sival_int);
        else if(sig == COUNTERS_SIG_SPEED)
            COUNTERSFEED_count_speed((uint16_t)info.si_value.sival_int);
        mtx_unlock(&counters_lock);
    }

    return 0;
}

void COUNTERS_enable_signals(void)
{
    mtx_unlock(&counters_lock);
}

void COUNTERS_disable_signals(void)
{
    mtx_lock(&counters_lock);
}

void COUNTERS_init(void)
{
    sigset_t set;

    mtx_init(&counters_lock, mtx_plain);

    sigemptyset(&set);
    sigaddset(&set, COUNTERS_SIG_FUEL);
    sigaddset(&set, COUNTERS_SIG_SPEED);
    sigaddset(&set, KEYS_SIG);
    /* Block the full sim SIGRT set before any waiter thread exists so
     * later threads inherit it. sigwaitinfo only sees signals that stay
     * blocked; an unblocked SIGRT with default disposition kills the process. */
    sigprocmask(SIG_BLOCK, &set, NULL);

    thrd_create(&counters_signal_thread, counters_signal_loop, NULL);
    thrd_detach(counters_signal_thread);
}
