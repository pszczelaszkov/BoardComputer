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
char USART_RX_buffer[USART_RX_BUFFER_SIZE];
char USART_TX_buffer[USART_TX_BUFFER_SIZE];
uint8_t USART_TX_message_length;
//note: indexes work as status flags too I.E: TX index with value of TX_BUFFER_SIZE means "TX is ready to go" 
uint8_t USART_RX_buffer_index;
uint8_t USART_TX_buffer_index;
int8_t USART_eot_counter;


void USART_TX_clear()
{
		USART_TX_buffer_index = USART_TX_BUFFER_SIZE;//Mark as finished
		USART_TX_message_length = 0;
}

void USART_flush()
{
	if(!USART_TX_message_length || USART_TX_buffer_index != USART_TX_BUFFER_SIZE)
		return;
	USART_TX_buffer_index = 1;//set index at 2nd byte for further IRQ callback
	UDR2 = USART_TX_buffer[0];//First byte is send here, rest is handled on IRQ
}

uint8_t USART_send(char data[],uint8_t flush)
{
	if(USART_TX_buffer_index != USART_TX_BUFFER_SIZE)//Check if there is no pending transmission
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

void USART_initialize()
{
    //USART_parsing_function = USARTonUSART;
	if(USART_TX_buffer_index == USART_TX_BUFFER_SIZE)//just a guard
		return;
    
	USART_TX_buffer_index = USART_TX_BUFFER_SIZE;
	#ifdef __AVR__
    uint8_t baud = USART_BAUDRATE;
    UBRR2H = (uint8_t)(baud>>8);
    UBRR2L = (uint8_t)baud;
    UCSR2B = (1<<RXEN)|(1<<TXEN)|(1<<RXCIE)|(1<<TXCIE);
    UCSR2C = (3<<UCSZ0);//frame format: 8data, 1stop bit
	sei();//enable global interrupts
    #endif
}

void USART_register()
{
	Callback_32 handler;
	switch(USART_RX_buffer[0])
	{	
		#ifndef __AVR__
		case 0x01:
			USART_test();
		break;
		#endif
		case 0x65:;
			INPUT_ComponentID_t componentID = (INPUT_ComponentID_t)(USART_RX_buffer[2]|USART_RX_buffer[1] << 4);
			INPUT_Keystatus_t keystatus = USART_RX_buffer[3];
			INPUT_userinput(keystatus, INPUT_KEY_ENTER, componentID);
		break;
		case 0x71:
			handler = NEXTION_requested_data_handler;
			if(handler)
			{
				handler(*(uint32_t*)&USART_RX_buffer[1]);
				NEXTION_requested_data_handler = NULL;
			}
		break;
	}
	USART_RX_buffer_index = 0;//unlock
}

ISR(USART2_RX_vect)
{
	#ifdef __AVR__
	uint8_t buffer = UDR2;
	#else
	uint8_t buffer = UDRRX;
	#endif
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
		USART_RX_buffer_index = USART_RX_BUFFER_SIZE;//Lock buffer
		USART_register();
		//SCHEDULER_addLowPriorityTask(SCHEDULER_CALLBACK_USART_REGISTER);
	}
}
ISR(USART2_TX_vect)
{
    if(USART_TX_buffer_index == USART_TX_message_length)
	{
		USART_TX_clear();
		return;
	}
	
    char buffer = USART_TX_buffer[USART_TX_buffer_index];
	USART_TX_buffer_index++;
	UDR2 = buffer;
}