#include "USART.h"
#include "input.h"
#include "serial.h"

#ifndef __AVR__
static void test()
{
	char eot[] = {0xff,0x0};
	if(!strncmp(&USART_RX_buffer[1],"PING",4))
	{
		USART_send("PONG",0);
		USART_send(eot,1);
	}
}
#endif

#define PASSTHROUGHWATCHDOG_THRESHOLD 8

typedef enum NEXTIONMESSAGETYPE
{
	NEXTIONMESSAGETYPE_TEST = 0x1,
	NEXTIONMESSAGETYPE_TOUCHINPUT = 0x65,
	NEXTIONMESSAGETYPE_PAGEID= 0x66,
	NEXTIONMESSAGETYPE_INCOMINGDATA = 0x71,
	NEXTIONMESSAGETYPE_DEVICEREADY = 0x88,
}NEXTIONMESSAGETYPE;

enum OPERATION_MODE
{
	OPERATION_MODE_NORMAL,
	OPERATION_MODE_PASSTHROUGH
}operation_mode;

enum RX_STATUS
{
	RX_STATUS_FREE,
	RX_STATUS_NEXTION,
	RX_STATUS_SERVICE
}rx_status;

uint8_t USART_RX_buffer[USART_RX_BUFFER_SIZE];
uint8_t USART_TX_buffer[USART_TX_BUFFER_SIZE];
uint8_t USART_TX_message_length;
//note: indexes work as status flags too I.E: TX index with value of TX_BUFFER_SIZE means "TX is ready to go" 
uint8_t USART_RX_buffer_index;
uint8_t USART_TX_buffer_index;
int8_t USART_eot_counter;
volatile uint8_t passthrough_watchdog_counter;

void USART_TX_clear()
{
	USART_TX_buffer_index = USART_TX_BUFFER_SIZE;//Mark as finished
	USART_TX_message_length = 0;
}

void USART_flush()
{	
	if(OPERATION_MODE_NORMAL == operation_mode && 0 < USART_TX_message_length)
	{
		USART_TX_buffer_index = 0;
		#ifdef __DEBUG__
			USART_write_service_byte();
		#else
			USART_write_nextion_byte();
		#endif
	}
}

uint8_t USART_send(char data[],uint8_t flush)
{
	if(USART_TX_buffer_index != USART_TX_BUFFER_SIZE)//Check if there is no pending transmission
		return 0;

	if(operation_mode != OPERATION_MODE_NORMAL)
		return 0;

	uint8_t size = strlen(data);
	if(size+USART_TX_message_length > USART_TX_BUFFER_SIZE)
		return 0;

	memcpy(&USART_TX_buffer[USART_TX_message_length],data,size);
	USART_TX_message_length = USART_TX_message_length + size;
	
	if(flush)
		USART_flush();
	
	return 1;
}

void USART_update()
{
	if(operation_mode == OPERATION_MODE_PASSTHROUGH)
	{
		if(!passthrough_watchdog_counter)
			operation_mode = OPERATION_MODE_NORMAL;
		else
			passthrough_watchdog_counter--;
	}
}

void USART_initialize()
{
	if(USART_TX_buffer_index == USART_TX_BUFFER_SIZE)//just a guard
		return;
    
	USART_TX_buffer_index = USART_TX_BUFFER_SIZE;
	SERIAL_init();
}

void message_register(uint8_t message_size)
{
	Callback_32 handler;
	//Check for "DRAKJHSUYDGBNCJHGJKSHBDN", although more complex rule is not needed.
	if(message_size == 24 && USART_RX_buffer[1] == 'R')
	{
		operation_mode = OPERATION_MODE_PASSTHROUGH;
		passthrough_watchdog_counter = PASSTHROUGHWATCHDOG_THRESHOLD;
	}
	else
	{
		switch((NEXTIONMESSAGETYPE)USART_RX_buffer[0])
		{	
			#ifndef __AVR__
			case NEXTIONMESSAGETYPE_TEST:
				test();
			break;
			#endif
			case NEXTIONMESSAGETYPE_TOUCHINPUT:
				INPUT_ComponentID_t componentID = (INPUT_ComponentID_t)(USART_RX_buffer[2]);
				INPUT_Keystatus_t keystatus = USART_RX_buffer[3];
				INPUT_userinput(keystatus, INPUT_KEY_ENTER, componentID);
			break;
			case NEXTIONMESSAGETYPE_PAGEID://Pinging purpose
				NEXTION_handler_sendme(USART_RX_buffer[1]);
			break;
			case NEXTIONMESSAGETYPE_INCOMINGDATA:
				if(NEXTION_incomingdata_handler)
				{
					handler(*(uint32_t*)&USART_RX_buffer[1]);
				}
			break;
			case NEXTIONMESSAGETYPE_DEVICEREADY:
				NEXTION_handler_ready(*(uint16_t*)&USART_RX_buffer[1]);
			break;
		}
	}
	USART_RX_buffer_index = 0;//unlock
}

void handle_RX(uint8_t buffer)
{
	if(USART_RX_buffer_index < USART_RX_BUFFER_SIZE)
	{
		USART_RX_buffer[USART_RX_buffer_index] = buffer;
		USART_RX_buffer_index++;
	}

	if(buffer == USART_EOT)
		USART_eot_counter++;
	else
		USART_eot_counter = 0;
	
	if(USART_eot_counter == USART_EOT_COUNT)
	{	
		uint8_t message_size = USART_RX_buffer_index - USART_EOT_COUNT;
		USART_RX_buffer_index = USART_RX_BUFFER_SIZE;//Lock buffer
		rx_status = RX_STATUS_FREE;
		message_register(message_size);
	}
}

void USART_read_nextion_byte()
{
	uint8_t buffer = SERIAL_NEXTION_IN;

	if(operation_mode == OPERATION_MODE_PASSTHROUGH)
	{
		SERIAL_SERVICE_OUT(buffer);
		return;
	}
	if(rx_status <= RX_STATUS_NEXTION)
	{
		rx_status = RX_STATUS_NEXTION;
		handle_RX(buffer);
	}
}

void USART_read_service_byte()
{
	uint8_t buffer = SERIAL_SERVICE_IN;

	if(operation_mode == OPERATION_MODE_PASSTHROUGH)
	{	
		passthrough_watchdog_counter = PASSTHROUGHWATCHDOG_THRESHOLD;
		SERIAL_NEXTION_OUT(buffer);
		return;
	}
	if(rx_status < RX_STATUS_SERVICE)
	{
		rx_status = RX_STATUS_SERVICE;
		USART_RX_buffer_index = 0;
	}
	handle_RX(buffer);
}

void USART_write_nextion_byte()
{
    if(USART_TX_buffer_index == USART_TX_message_length)
	{
		USART_TX_clear();
		return;
	}
	
	SERIAL_NEXTION_OUT(USART_TX_buffer[USART_TX_buffer_index++]);
}

void USART_write_service_byte()
{
    if(USART_TX_buffer_index == USART_TX_message_length)
	{
		USART_TX_clear();
		return;
	}

	SERIAL_SERVICE_OUT(USART_TX_buffer[USART_TX_buffer_index++]);
}