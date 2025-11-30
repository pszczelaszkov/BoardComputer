#include "system_interface.h"
#include "system.h"
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <threads.h>
#include <poll.h>
#include <sys/timerfd.h>
#include <sys/eventfd.h>
#include <string.h>
#include <time.h>

static int timer_fd   = -1;   // 128 ms periodic timer
static int wake_fd    = -1;   // external wake signal
static thrd_t RTC_thread;
static uint8_t board_is_enabled = 1;

uint8_t DDRA, DDRB, DDRC, DDRD;
uint8_t PORTA, PORTB, PORTC, PORTD, DIDR0;

static int sysclk_thread(void* arg)
{
	for(;;)
		{
		struct pollfd fd = { .fd = timer_fd, .events = POLLIN };

		// The CPU “sleeps” here until a timer tick or wake event arrives
		poll(&fd, 1, -1);
		if (fd.revents & POLLIN) {
			uint64_t cnt;
			read(timer_fd, &cnt, sizeof(cnt));   // clear timerfd
			SYSTEM_event_timer++;	
            SYSTEM_exec = 1;
            SYSTEMINTERFACE_external_wakeup();
		}
	}
	return 1;
}

int8_t SYSTEMINTERFACE_is_board_enabled()
{
    return board_is_enabled;
}

void SYSTEMINTERFACE_initialize_IO()
{
    DIDR0 = 0xff;
	DDRD = 0x00;
	PORTD = 0x00;
    //SPI Thing
	SET(DDRB,BIT0);
	SET(DDRB,BIT4);
	SET(DDRB,BIT7);
}

void SYSTEMINTERFACE_start_system_clock()
{
    // 1) Create periodic timerfd (128 ms)
    timer_fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);
    if (timer_fd < 0) {
        perror("timerfd_create");
        return;
    }

    struct itimerspec its;
    memset(&its, 0, sizeof(its));

    // first expiration
    its.it_value.tv_sec  = 0;
    its.it_value.tv_nsec = 128 * 1000000;  // 128 ms

    // periodic interval
    its.it_interval.tv_sec  = 0;
    its.it_interval.tv_nsec = 128 * 1000000;

    if (timerfd_settime(timer_fd, 0, &its, NULL) < 0) {
        perror("timerfd_settime");
    }

    // 2) external wakeup eventfd
    wake_fd = eventfd(0, EFD_NONBLOCK);

	thrd_create(&RTC_thread, sysclk_thread, NULL);
}


void SYSTEMINTERFACE_sleep()
{
    struct pollfd fd = { .fd = wake_fd,  .events = POLLIN };

    // The CPU “sleeps” here until a timer tick or wake event arrives
    poll(&fd, 1, -1);

    if (fd.revents & POLLIN) {
        uint64_t cnt;
        read(wake_fd, &cnt, sizeof(cnt));    // clear wake event
        // External wake has occurred
    }
}

void SYSTEMINTERFACE_external_wakeup(void)
{
    uint64_t one = 1;
    write(wake_fd, &one, sizeof(one));
}
