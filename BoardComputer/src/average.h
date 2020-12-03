/*
 * average.h
 *
 * Created: 2020-08-18 02:15:00
 *  Author: pszczelaszkov
 */ 
#include "inttypes.h"

#ifndef AVERAGE_H
#define AVERAGE_H
/*
    Somekind of circularbuffer that accepts 2^16 of 16bit integers as max capacity and holds their average value.
*/
enum AVERAGE_BUFFER
{
    AVERAGE_BUFFER_SPEED,
    AVERAGE_BUFFER_LP100,
    AVERAGE_BUFFER_LAST
};
#define AVERAGE_BUFFERS_SIZE AVERAGE_BUFFER_LAST
typedef struct Average
{
    uint32_t sum;
    uint16_t average;
    uint16_t count;
    uint16_t sum_base;
}Average;
extern Average AVERAGE_buffers[];

uint16_t AVERAGE_addvalue(uint8_t bufferid, uint16_t value);
void AVERAGE_clear(uint8_t bufferid);
#endif