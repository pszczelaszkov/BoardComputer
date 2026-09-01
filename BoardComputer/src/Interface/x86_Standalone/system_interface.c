#include "system_interface.h"
#include "system.h"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <threads.h>
#include <poll.h>
#include <sys/stat.h>
#include <sys/timerfd.h>
#include <sys/eventfd.h>
#include <string.h>
#include <time.h>

static int timer_fd   = -1;   // 128 ms periodic timer
static int wake_fd    = -1;   // external wake signal
static int ignition_fd = -1;
static thrd_t RTC_thread;

volatile uint8_t DDRA, DDRB, DDRC, DDRD;
volatile uint8_t PORTA, PORTB, PORTC, PORTD, DIDR0;
volatile uint8_t PINA, PINB, PINC, PIND;
volatile uint16_t TCNT1, TCNT2;

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
			if(7 > SYSTEM_event_timer)
            {
                SYSTEM_event_timer++;
            }
            else
            {
                SYSTEM_event_timer=0;
            }
            SYSTEM_exec = 1;
            SYSTEMINTERFACE_external_wakeup();
		}
	}
	return 1;
}

static const char* bc_dir_path(void)
{
	const char* dir = getenv("BC_DIR");
	if(dir && dir[0] != '\0')
		return dir;
	return ".";
}

static int open_ignition_file(const char* dir)
{
	char path[512];
	int n = snprintf(path, sizeof(path), "%s/IGNITION", dir);
	if(n < 0 || (size_t)n >= sizeof(path))
		return -1;

	int fd = open(path, O_RDWR | O_CREAT | O_NONBLOCK, 0644);
	if(fd < 0)
		return -1;

	struct stat st;
	if(fstat(fd, &st) == 0 && S_ISREG(st.st_mode) && st.st_size == 0)
		(void)write(fd, "1\n", 2);

	return fd;
}

static int8_t read_ignition_file(void)
{
	char buf[32];
	ssize_t n;
	char* end;
	unsigned long value;

	if(ignition_fd < 0)
		return 1;

	if(lseek(ignition_fd, 0, SEEK_SET) < 0)
		return 1;

	n = read(ignition_fd, buf, sizeof(buf) - 1);
	if(n <= 0)
		return 1;

	buf[n] = '\0';
	errno = 0;
	value = strtoul(buf, &end, 10);
	if(end == buf || errno == ERANGE)
		return 1;
	if(value == 0)
		return 0;
	return 1;
}

int8_t SYSTEMINTERFACE_is_board_enabled()
{
	return read_ignition_file();
}

void SYSTEMINTERFACE_initialize_IO()
{
	const char* dir = bc_dir_path();

	if(mkdir(dir, 0755) < 0 && errno != EEXIST)
		perror(dir);

	ignition_fd = open_ignition_file(dir);
	if(ignition_fd < 0)
		perror("IGNITION open");

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
