#include "serial.h"
#include "USART.h"
#include "system_interface.h"

#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <stdatomic.h>
#include <threads.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <stdlib.h>
#include <termios.h>

uint8_t serial_nextion_in,serial_service_in,serial_service_out;

static thrd_t nextion_serial_tx_thread,nextion_serial_rx_thread;
static mtx_t tx_lock;
static cnd_t tx_cond;

static volatile uint8_t TX_REG;
static volatile uint8_t tx_reg_full = 0;

static int serial_fd = -1;

static void idle_ms(int ms)
{
    while (poll(NULL, 0, ms) < 0 && errno == EINTR)
        ;
}

/* Park forever after an unusable fd so we do not spin. */
static void rx_park_forever(void)
{
    for (;;)
        idle_ms(-1);
}

static int serial_configure(int fd)
{
    struct termios tio;
    int flags;

    /* Open used O_NONBLOCK so DCD does not stall us; TX/RX need blocking I/O. */
    flags = fcntl(fd, F_GETFL, 0);
    if (flags >= 0)
        fcntl(fd, F_SETFL, flags & ~O_NONBLOCK);

    if (tcgetattr(fd, &tio) < 0)
        return -1;

    cfmakeraw(&tio);
    tio.c_cflag |= (CLOCAL | CREAD);
    tio.c_cflag &= ~CRTSCTS;
    tio.c_iflag &= ~(IXON | IXOFF | IXANY);
    cfsetispeed(&tio, B9600);
    cfsetospeed(&tio, B9600);
    tio.c_cc[VMIN] = 1;
    tio.c_cc[VTIME] = 0;

    return tcsetattr(fd, TCSANOW, &tio);
}

/* Do not advance the USART TX ISR until the byte is actually on the fd. */
static void write_serial_byte(uint8_t byte)
{
    for (;;) {
        ssize_t n = write(serial_fd, &byte, 1);
        if (n == 1)
            return;
        if (n < 0 && errno == EINTR)
            continue;
        /* EAGAIN/EIO/0: no peer, hangup, or would-block. Retry, do not drop. */
        idle_ms(100);
    }
}

/*---------------- UART TX ISR THREAD ----------------*/
static int tx_thread(void *arg)
{
    uint8_t byte;

    for (;;) {
        mtx_lock(&tx_lock);

        while (0 == tx_reg_full) {
            cnd_wait(&tx_cond, &tx_lock);
        }

        byte = TX_REG;
        tx_reg_full = 0;

        mtx_unlock(&tx_lock);
        write_serial_byte(byte);
        USART_write_nextion_byte();
        SYSTEMINTERFACE_external_wakeup();
    }

    return 0;
}

static int rx_thread(void *arg)
{
    uint8_t byte;
    struct pollfd pfd = { .fd = serial_fd, .events = POLLIN };
    int hangup_backoff = 0;

    for (;;) {
        int pr = poll(&pfd, 1, hangup_backoff ? 100 : -1);
        if (pr < 0) {
            if (errno == EINTR)
                continue;
            rx_park_forever();
        }

        if (pfd.revents & POLLNVAL)
            rx_park_forever();

        if (pfd.revents & POLLIN) {
            ssize_t n = read(serial_fd, &byte, 1);
            if (n == 1) {
                hangup_backoff = 0;
                SERIAL_NEXTION_IN = byte;
                USART_read_nextion_byte();
                SYSTEMINTERFACE_external_wakeup();
                continue;
            }
            if (n < 0 && (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK))
                continue;
            /* EOF or other I/O error: retry with a timeout, peer may appear later. */
            hangup_backoff = 1;
            continue;
        }

        if (pfd.revents & (POLLHUP | POLLERR))
            hangup_backoff = 1;
    }

    return 0;
}


void SERIAL_NEXTION_OUT(uint8_t data)
{
    mtx_lock(&tx_lock);
    TX_REG = data;
    tx_reg_full = 1;
    cnd_signal(&tx_cond);
    mtx_unlock(&tx_lock);
}


void SERIAL_init(void)
{
    // Check environment variable
    const char* serial_path = "/dev/ttyS0";
    const char* env = getenv("SERIAL_TTY");
    if (env && env[0] != '\0') {
        serial_path = env;
    }

    printf("Opening serial port: %s\n", serial_path);

    serial_fd = open(serial_path, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (serial_fd < 0) {
        perror(serial_path);
        return;
    }

    serial_configure(serial_fd);

    mtx_init(&tx_lock, mtx_plain);
    cnd_init(&tx_cond);

    thrd_create(&nextion_serial_rx_thread, rx_thread, NULL);
    thrd_detach(nextion_serial_rx_thread);
    thrd_create(&nextion_serial_tx_thread, tx_thread, NULL);
}
