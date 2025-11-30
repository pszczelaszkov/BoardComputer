#include"serial.h"
#include "USART.h"

#include<stdint.h>

void SERIAL_init()
{
    uint8_t baud = 12;
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
	sei();//enable global interrupts
}

ISR(USART0_RX_vect)
{
    void USART_read_nextion_byte();
}

ISR(USART2_RX_vect)
{	
    void USART_read_service_byte();
}

ISR(USART0_TX_vect)
{
    void USART_write_nextion_byte();
}

ISR(USART2_TX_vect)
{
    void USART_write_service_byte();
}