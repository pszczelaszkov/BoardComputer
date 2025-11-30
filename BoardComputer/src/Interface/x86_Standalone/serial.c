#include"serial.h"
#include "USART.h"


#include <stdio.h>
#include <stdint.h>
#include <stdatomic.h>
#include <threads.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>

uint8_t serial_nextion_in,serial_service_in,serial_service_out;

static thrd_t nextion_serial_tx_thread,nextion_serial_rx_thread;
static mtx_t tx_lock;
static cnd_t tx_cond;

static volatile uint8_t TX_REG;
static volatile uint8_t tx_reg_full = 0;

static int serial_fd = -1;

/*---------------- UART TX ISR THREAD ----------------*/
static int tx_thread(void *arg)
{
    uint8_t byte;

    for (;;) {
        mtx_lock(&tx_lock);

        while (0 == tx_reg_full) {
            cnd_wait(&tx_cond, &tx_lock);
        }

        // Get the byte
        byte = TX_REG;
        tx_reg_full = 0;

        mtx_unlock(&tx_lock);

        write(serial_fd, &byte, 1);
        USART_write_nextion_byte();
    }

    return 0;
}

static int rx_thread(void *arg)
{
    uint8_t byte;
    for (;;) {
        if (read(serial_fd, &byte, 1) == 1) {
            SERIAL_NEXTION_IN = byte;
            USART_read_nextion_byte();
        }
    }
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

    mtx_init(&tx_lock, mtx_plain);
    cnd_init(&tx_cond);

    thrd_create(&nextion_serial_rx_thread, rx_thread, NULL);
    thrd_create(&nextion_serial_tx_thread, tx_thread, NULL);
}
