/*
 * ProgramData.h
 *
 * Created: 2020-01-09 00:33:36
 *  Author: pszczelaszkov
 */ 
#ifndef PROGRAMDATA_H_
#define PROGRAMDATA_H_

#include <stdint.h>
#define PROGRAMDATA_BAD_VAL 0xffff
typedef enum PROGRAMDATA_ADC_LUT
{
    PROGRAMDATA_ADC_LUT_NTC_2200R25_2200RS_3950B,
    PROGRAMDATA_ADC_LUT_LAST, 
}PROGRAMDATA_ADC_LUT_t;

extern int16_t PROGRAMDATA_get_ADC_lut_value(const PROGRAMDATA_ADC_LUT_t lut, uint16_t adc_value);

#endif /* PROGRAMDATA_H_ */