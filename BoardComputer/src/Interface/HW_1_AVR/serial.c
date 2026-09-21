#include"serial.h"
#include "UART.h"
#include<stdint.h>
#include<stdatomic.h>

volatile uint8_t SERIAL_NEXTION_OUT_status = SERIAL_OUT_STATUS_IDLE;
volatile uint8_t SERIAL_SERVICE_OUT_status = SERIAL_OUT_STATUS_IDLE;

uint8_t nextion_tx_message_length;
uint8_t* nextion_tx_message;

uint8_t service_tx_message_length;
uint8_t* service_tx_message;

static inline void advance_tx_message(uint8_t** message, uint8_t* length)
{
    (*length)--;
    (*message)++;
}

static inline void uart_put_rx_udr(UART_CHANNEL channel, uint8_t udr_shadow)
{
    UART_put_byte_to_RX_buffer(channel, udr_shadow);
}

void SERIAL_send_msg(UART_CHANNEL channel, const uint8_t* data, const uint8_t length)
{ 
    uint8_t first_byte = 0x0;
    switch(channel)
    {

        case UART_CHANNEL_NEXTION:
            if(SERIAL_OUT_STATUS_IDLE == SERIAL_NEXTION_OUT_status)
            {
                SERIAL_NEXTION_OUT_status = SERIAL_OUT_STATUS_BUSY;
                nextion_tx_message_length = length;
                nextion_tx_message = (uint8_t*)data;
                first_byte = nextion_tx_message[0];
                advance_tx_message(&nextion_tx_message, &nextion_tx_message_length);
                __atomic_signal_fence(memory_order_seq_cst);
                SERIAL_NEXTION_OUT(first_byte);
            }
            break;
        case UART_CHANNEL_SERVICE:
            if(SERIAL_OUT_STATUS_IDLE == SERIAL_SERVICE_OUT_status)
            {
                SERIAL_SERVICE_OUT_status = SERIAL_OUT_STATUS_BUSY;
                service_tx_message_length = length;
                service_tx_message = (uint8_t*)data;
                first_byte = service_tx_message[0];
                advance_tx_message(&service_tx_message, &service_tx_message_length);
                __atomic_signal_fence(memory_order_seq_cst);
                SERIAL_SERVICE_OUT(first_byte);
            }
            break;
        default:
            break;
    }
}

void SERIAL_init()
{
    uint8_t baud = 51;
	//Service
    UBRR2H = (uint8_t)(baud>>8);
    UBRR2L = (uint8_t)baud;
    UCSR2B = (1<<RXEN)|(1<<TXEN)|(1<<RXCIE)|(1<<TXCIE);
    UCSR2C = (3<<UCSZ0);//frame format: 8data, 1stop bit
	//Nextion
	UBRR0H = (uint8_t)(baud>>8);
    UBRR0L = (uint8_t)baud;
    UCSR0B = (1<<RXEN)|(1<<TXEN)|(1<<RXCIE)|(1<<TXCIE);
    UCSR0C = (3<<UCSZ0);//frame format: 8data, 1stop bit
}

ISR(USART0_RX_vect)
{
    const uint8_t udr_shadow = UDR0;
    uart_put_rx_udr(UART_CHANNEL_NEXTION, udr_shadow);
}

ISR(USART2_RX_vect)
{
    const uint8_t udr_shadow = UDR2;
    uart_put_rx_udr(UART_CHANNEL_SERVICE, udr_shadow);
}

ISR(USART0_TX_vect)
{
    if(nextion_tx_message_length > 0)
    {
        SERIAL_NEXTION_OUT(nextion_tx_message[0]);
        advance_tx_message(&nextion_tx_message, &nextion_tx_message_length);
    }
    else {
        UART_send_next_msg(UART_CHANNEL_NEXTION);
    }
}

ISR(USART2_TX_vect)
{
    if(service_tx_message_length > 0)
    {
        SERIAL_SERVICE_OUT(service_tx_message[0]);
        advance_tx_message(&service_tx_message, &service_tx_message_length);
    }
    else {
        UART_send_next_msg(UART_CHANNEL_SERVICE);
    }
}
