#include "USART.h"
#include "input.h"


#ifndef __AVR__
uint8_t UDR0,UDR2,UDRRX;
void USART_test()
{
    
	char eot = 0xff;
	if(!strncmp(&USART_RX_buffer[1],"PING",4))
	{
		USART_send("PONG",0);
		USART_send(&eot,1);
	}
}
#endif

#define PASSTHROUGHWATCHDOG_THRESHOLD 8

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

char USART_RX_buffer[USART_RX_BUFFER_SIZE];
char USART_TX_buffer[USART_TX_BUFFER_SIZE];
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
	if(!USART_TX_message_length || USART_TX_buffer_index != USART_TX_BUFFER_SIZE)
		return;
	
	if(operation_mode == OPERATION_MODE_NORMAL)
	{
		USART_TX_buffer_index = 1;//set index at 2nd byte for further IRQ callback
		//First byte is send here, rest is handled on IRQ
		#ifdef __DEBUG__
			UDR2 = USART_TX_buffer[0];
		#else
			UDR0 = USART_TX_buffer[0];
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
	#ifdef __AVR__
    uint8_t baud = USART_BAUDRATE;
	//service
    UBRR2H = (uint8_t)(baud>>8);
    UBRR2L = (uint8_t)baud;
    UCSR2B = (1<<RXEN)|(1<<TXEN)|(1<<RXCIE)|(1<<TXCIE);
    UCSR2C = (3<<UCSZ0);//frame format: 8data, 1stop bit
	//Nextion
	UBRR0H = (uint8_t)(baud>>8);
    UBRR0L = (uint8_t)baud;
    UCSR0B = (1<<RXEN)|(1<<TXEN)|(1<<RXCIE)|(1<<TXCIE);
    UCSR0C = (3<<UCSZ0);//frame format: 8data, 1stop bit
	sei();//enable global interrupts
    #endif
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
		switch((uint8_t)USART_RX_buffer[0])
		{	
			#ifndef __AVR__
			case 0x01:
				USART_test();
			break;
			#endif
			case 0x65:;
				INPUT_ComponentID_t componentID = (INPUT_ComponentID_t)(USART_RX_buffer[2]);
				INPUT_Keystatus_t keystatus = USART_RX_buffer[3];
				INPUT_userinput(keystatus, INPUT_KEY_ENTER, componentID);
			break;
			case 0x66:
				NEXTION_handler_sendme(USART_RX_buffer[1]);//Pinging purpose
			break;
			case 0x71:
				handler = NEXTION_handler_requested_data;
				if(handler)
				{
					handler(*(uint32_t*)&USART_RX_buffer[1]);
					NEXTION_handler_requested_data = NULL;
				}
			break;
			case 0x88:
				NEXTION_handler_ready();
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

ISR(USART0_RX_vect)
{
	#ifdef __AVR__
	uint8_t buffer = UDR0;
	#else
	uint8_t buffer = UDRRX;
	#endif
	if(operation_mode == OPERATION_MODE_PASSTHROUGH)
	{
		UDR2 = buffer;
		return;
	}
	if(rx_status <= RX_STATUS_NEXTION)
	{
		rx_status = RX_STATUS_NEXTION;
		handle_RX(buffer);
	}
}

ISR(USART2_RX_vect)
{	
	#ifdef __AVR__
	uint8_t buffer = UDR2;
	#else
	uint8_t buffer = UDRRX;
	#endif
	if(operation_mode == OPERATION_MODE_PASSTHROUGH)
	{	
		passthrough_watchdog_counter = PASSTHROUGHWATCHDOG_THRESHOLD;
		UDR0 = buffer;
		return;
	}
	if(rx_status < RX_STATUS_SERVICE)
	{
		rx_status = RX_STATUS_SERVICE;
		USART_RX_buffer_index = 0;
	}
	handle_RX(buffer);
}

ISR(USART0_TX_vect)
{
	if(USART_TX_message_length == 0)
		return;

    if(USART_TX_buffer_index == USART_TX_message_length)
	{
		USART_TX_clear();
		return;
	}
	
    char buffer = USART_TX_buffer[USART_TX_buffer_index];
	USART_TX_buffer_index++;
	UDR0 = buffer;
}

ISR(USART2_TX_vect)
{
	if(USART_TX_message_length == 0)
		return;

    if(USART_TX_buffer_index == USART_TX_message_length)
	{
		USART_TX_clear();
		return;
	}

    char buffer = USART_TX_buffer[USART_TX_buffer_index];
	USART_TX_buffer_index++;
	UDR2 = buffer;
}