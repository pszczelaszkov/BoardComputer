#include "utils.h"

#ifndef __AVR__
    volatile uint8_t PINA,PINB;
    volatile uint16_t TCNT1,TCNT2;
    uint8_t DDRD, DDRB, DIDR0;
    uint8_t PORTD, PORTB, PIND;
    uint16_t ADC;
    uint8_t ADMUX, ADSC, ADCSRA;
    void _delay_ms(int dummy){}
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
#endif

const uint8_t FP8_weight = 10000/0xff;
const uint16_t FP16_weight = SENSORSFEED_HIGH_PRECISION_BASE/0xffff;

/* reverse:  reverse string s in place */
void reverse(char s[])
{
    uint8_t i, j;
    char c;

    for (i = 0, j = strlen(s)-1; i<j; i++, j--) {
        c = s[i];
        s[i] = s[j];
        s[j] = c;
    }
}
/* uitoa:  convert unsigned n to characters in s */
void uitoa(uint16_t n, char s[])
{
    uint8_t i;

    i = 0;
    do {       /* generate digits in reverse order */
        s[i++] = n % 10 + '0';   /* get next digit */
    } while ((n /= 10) > 0);     /* delete it */
    s[i] = '\0';
    reverse(s);
}

/*
Converts 16bit(8+8) fixedpoint variable to ascii.
@param fixedpoint Value for conversion.
@param dest Destination buffer.
@param integrallength Maximal length of converted integral part including sign.
@param fractionlength Maximal length of converted fractionpart(Max 4).
*/
void fp16toa(int16_t fixedpoint, char* dest, uint8_t integrallength, uint8_t fractionlength)
{
    const uint8_t max_precision = 4;
    const uint8_t fractionstart = integrallength + 1;

    int8_t integral = fixedpoint >> 8;
	uint16_t fraction = (fixedpoint & 0xff) * FP8_weight;

    char fractionascii[7];
    memset(fractionascii, '0', max_precision);

    //clamp fractionlength
    if(!fractionlength)
    {
        fractionlength = 1;
    }
    else
    {
        if(fractionlength > max_precision)
            fractionlength = max_precision;
    }

	rightnconcat_short(dest, integral, integrallength, integrallength);
	dest[integrallength] = '.';
    
    rightconcat_short(fractionascii, fraction, max_precision);
    memcpy(&dest[fractionstart],fractionascii,fractionlength);
}

int16_t UTILS_atoi(char* stringvalue) {
    int8_t sign = (stringvalue[0] == '-'? -1 : 1);
    int16_t result = 0;
    while (*stringvalue) {
        if(*stringvalue >= '0' && *stringvalue <= '9')
            result = result*10 + (*stringvalue) - '0';
        stringvalue++;
     }
     return result * sign;
}

/*
Insert right aligned integer into string.
@param dest Destination buffer(at least 7 bytes)
@param value Value which will be processed to string
@param spacing Postion of alignment
*/
void rightconcat_short(char* dest, int16_t value, uint8_t spacing)
{
	char temp[7];
	uint8_t length;
	itoa(value, temp,10);
	length = strlen(temp);
	memcpy(&dest[spacing-length],temp,length);
}

/*
Insert right aligned integer into string.
@param dest Destination buffer(at least 7 bytes)
@param value Value which will be processed to string
@param spacing Postion of alignment
@param n Number of characters to be inserted(starts from left side)
*/
void rightnconcat_short(char* dest, int16_t value, uint8_t spacing, uint8_t n)
{
	char temp[7];
    uint8_t length;
	if (n > 6)
		n = 6;
	itoa(value, temp,10);
    length = strlen(temp);
    if(n > length)
        n = length;
	memcpy(&dest[spacing-n],temp,n);
}