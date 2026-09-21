#include"serial.h"

uint8_t serial_nextion_in,serial_nextion_out,serial_service_in,serial_service_out;
volatile uint8_t SERIAL_NEXTION_OUT_status = SERIAL_OUT_STATUS_IDLE;
volatile uint8_t SERIAL_SERVICE_OUT_status = SERIAL_OUT_STATUS_IDLE;

uint8_t* nextion_tx_message;
uint8_t nextion_tx_message_length;
uint8_t* service_tx_message;
uint8_t service_tx_message_length;

void SERIAL_send_msg(UART_CHANNEL channel, const uint8_t* data, const uint8_t length)
{
    switch(channel)
    {
        case UART_CHANNEL_NEXTION:
            if(SERIAL_OUT_STATUS_IDLE == SERIAL_NEXTION_OUT_status)
            {
                SERIAL_NEXTION_OUT_status = SERIAL_OUT_STATUS_BUSY;
                nextion_tx_message = (uint8_t*)data;
                nextion_tx_message_length = length;
                if(length > 0)
                {
                    SERIAL_NEXTION_OUT(data[0]);
                }
            }
            break;
        case UART_CHANNEL_SERVICE:
            if(SERIAL_OUT_STATUS_IDLE == SERIAL_SERVICE_OUT_status)
            {
                SERIAL_SERVICE_OUT_status = SERIAL_OUT_STATUS_BUSY;
                service_tx_message = (uint8_t*)data;
                service_tx_message_length = length;
                if(length > 0)
                {
                    SERIAL_SERVICE_OUT(data[0]);
                }
            }
            break;
        default:
            break;
    }
}

void SERIAL_init(){}
