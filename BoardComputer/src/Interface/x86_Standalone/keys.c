#include "keys.h"
#include "counters.h"
#include "input.h"

#include <signal.h>
#include <threads.h>

static thrd_t keys_signal_thread;
static uint8_t key_pressed[INPUT_KEY_LAST];

static int keys_signal_loop(void* arg)
{
	sigset_t set;
	siginfo_t info;

	(void)arg;
	sigemptyset(&set);
	sigaddset(&set, KEYS_SIG);

	for(;;)
	{
		int sig = sigwaitinfo(&set, &info);
		if(sig < 0)
			continue;

		int key_id = info.si_value.sival_int;
		if(key_id < 0 || key_id >= INPUT_KEY_LAST)
			continue;

		INPUT_Key_t key = (INPUT_Key_t)key_id;
		key_pressed[key] = !key_pressed[key];
		INPUT_Keystatus_t keystatus = key_pressed[key]
			? INPUT_KEYSTATUS_PRESSED
			: INPUT_KEYSTATUS_RELEASED;
		
		INPUT_userinput(keystatus, key, INPUT_COMPONENT_NONE);
	}

	return 0;
}

void KEYS_init(void)
{
	sigset_t set;

	sigemptyset(&set);
	sigaddset(&set, COUNTERS_SIG_FUEL);
	sigaddset(&set, COUNTERS_SIG_SPEED);
	sigaddset(&set, KEYS_SIG);
	/* Block the full sim SIGRT set before any waiter thread exists so
	 * later threads inherit it. sigwaitinfo only sees signals that stay
	 * blocked; an unblocked SIGRT with default disposition kills the process. */
	sigprocmask(SIG_BLOCK, &set, NULL);

	thrd_create(&keys_signal_thread, keys_signal_loop, NULL);
	thrd_detach(keys_signal_thread);
}
