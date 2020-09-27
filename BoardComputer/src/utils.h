/*
 * utils.c
 *
 * Created: 2020-09-03 03:21:00
 * Author : pszczelaszkov
 */


#ifndef __UTILS__
#define __UTILS__
#ifdef __AVR__
    #include <stdlib.h>
#else
    #define ISR(...) void __VA_ARGS__()
    #define sleep_cpu()
    volatile uint8_t PINA,PINB;
    volatile uint16_t TCNT1;
    uint8_t DDRD;
    uint8_t PORTD;
    uint8_t ADC;
    uint8_t ADMUX, ADSC, ADCSRA;
    /* reverse:  reverse string s in place */
    void reverse(char s[])
    {
        int i, j;
        char c;

        for (i = 0, j = strlen(s)-1; i<j; i++, j--) {
            c = s[i];
            s[i] = s[j];
            s[j] = c;
        }
    }
    /* itoa:  convert n to characters in s */
    void itoa(int n, char s[],int dummy)
    {
        int i, sign;

        if ((sign = n) < 0)  /* record sign */
            n = -n;          /* make n positive */
        i = 0;
        do {       /* generate digits in reverse order */
            s[i++] = n % 10 + '0';   /* get next digit */
        } while ((n /= 10) > 0);     /* delete it */
        if (sign < 0)
            s[i++] = '-';
        s[i] = '\0';
        reverse(s);
    }
    void _delay_ms(int dummy){}
#endif

void concat_short_1(char* dest, uint16_t value)
{
		char temp[5];
		itoa(value,&temp[0],10);
		dest[0] = temp[0];
}

int8_t concat_short_r3(char* dest, uint16_t value)
{
	//Concatenate right aligned integer.
	char temp[5];
	itoa(value, &temp[0],10);
	if(value < 10)
	{	
		dest[2] = temp[0];
	}
	else if(value < 100)
	{
		dest[1] = temp[0];
		dest[2] = temp[1];
	}
	else
	{
		dest[0] = temp[0];
		dest[1] = temp[1];
		dest[2] = temp[2];
	}
	
	return 1;
}

int8_t concat_short_r4(char* dest, uint16_t value)
{
	//Concatenate right aligned integer.
	char temp[5];
	itoa(value, &temp[0],10);
	if(value < 10)
	{	
		dest[3] = temp[0];
	}
	else if(value < 100)
	{
		dest[2] = temp[0];
		dest[3] = temp[1];
	}
	else if(value < 1000)
	{
		dest[1] = temp[0];
		dest[2] = temp[1];
		dest[3] = temp[2];
	}
	else
	{
		dest[0] = temp[0];
		dest[1] = temp[1];
		dest[2] = temp[2];
		dest[3] = temp[3];
	}
	
	return 1;
}

#endif
