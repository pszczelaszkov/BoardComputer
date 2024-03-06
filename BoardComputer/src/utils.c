#include "utils.h"

#ifndef __AVR__
    volatile uint8_t PINA, PINB, PINC, PIND;
    volatile uint16_t TCNT1, TCNT2;
    uint8_t DDRA, DDRB, DDRC, DDRD;
    uint8_t PORTA, PORTB, PORTC, PORTD, DIDR0;
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
Extracts Integer and Fractional part from 16bit(8+8) fixedpoint variable.
@param fixedpoint Value for conversion.
@param integer Ptr to resulting integer
@param fractional Ptr to resulting fractional part.
*/
void extractfp16(int16_t fixedpoint, int8_t* integral, uint16_t* fractional)
{    
    const uint8_t ch = 0x27;
    const uint8_t cl = 0x36;
    uint8_t fixedpoint_l = fixedpoint & 0xff;

    *integral = fixedpoint >> 8;
    *fractional = (ch * fixedpoint_l) + ((cl * fixedpoint_l) >> 8);
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
    const uint8_t ch = 0x27;
    const uint8_t max_precision = 4;
    const uint8_t fractionstart = integrallength + 1;
    int8_t integral = 0;
    int8_t sign = 1;
	uint16_t fraction = 0;
    uint8_t fixedpoint_l = 0;

    fractionlength = MAX(1,MIN(fractionlength, max_precision));
    if(fixedpoint < 0)
    {
        sign = -1;
        fixedpoint = (fixedpoint ^ 0xffff) + 1;
    }
    fixedpoint_l = fixedpoint & 0xff;
    integral = fixedpoint >> 8;
    fraction = (ch * fixedpoint_l) +  (fixedpoint_l >> 4);
    
    
    char fractionascii[7];
    memset(fractionascii, '0', max_precision);

	rightnconcat_short(dest, integral, integrallength, integrallength);
    dest[integrallength] = '.';
    if(sign < 0)
        dest[0] = '-';
        
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