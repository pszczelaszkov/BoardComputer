/*
 * ProgramData.h
 *
 * Created: 2020-01-09 00:33:36
 *  Author: pszczelaszkov
 */ 
#ifndef PROGRAMDATA_H_
#define PROGRAMDATA_H_
#ifdef __AVR__
#include <avr/pgmspace.h>
#else
#include "utils.h"
#define PROGMEM
uint16_t pgm_read_word(const int16_t* value);
#endif

typedef enum PROGRAMDATA_LUT
{
    PROGRAMDATA_LUT_NTC_2200_INVERTED,
    PROGRAMDATA_LUT_LAST, 
}PROGRAMDATA_LUT_t;

extern int16_t PROGRAMDATA_get_lut_value(PROGRAMDATA_LUT_t lut, uint16_t index);

#endif /* PROGRAMDATA_H_ */