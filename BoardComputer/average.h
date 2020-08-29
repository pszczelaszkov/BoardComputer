/*
 * average.h
 *
 * Created: 2020-08-18 02:15:00
 *  Author: pszczelaszkov
 */ 


#ifndef AVERAGE_H
#define AVERAGE_H
/*
    Somekind of circularbuffer that accepts 2^16 of 16bit integers as max capacity and holds their average value.
*/
enum
{
    AVERAGE_BUFFER_SPEED,
    AVERAGE_BUFFER_LP100,
    AVERAGE_BUFFER_LAST
};
#define AVERAGE_BUFFERS_COUNT AVERAGE_BUFFER_LAST
typedef struct Average
{
    uint32_t sum;
    uint16_t average;
    uint16_t count;
    uint16_t sum_base;
}Average;
Average AVERAGE_buffers[AVERAGE_BUFFERS_COUNT];

uint16_t AVERAGE_addvalue(uint8_t bufferid, uint16_t value)
{
    Average* buf = &AVERAGE_buffers[bufferid];
    if(buf->sum_base)
    {
        if(!buf->count)
        {
            buf->count = 0xffff;
            if(buf->sum_base != buf->average)
                buf->sum_base = buf->average;
            buf->sum = buf->sum_base;
            buf->sum = buf->sum << 16;
        }
        else
        {
            buf->count--;
            buf->sum += (value - buf->sum_base);
        }
        buf->average = buf->sum >> 16;
    }
    else
    {
        buf->count++;
        buf->sum += value;
        buf->average = buf->sum / buf->count;
        if(buf->count == 0xffff)//buffer is full, switch to cyclic buffering
        {
            buf->count = 0;
            buf->sum_base = buf->average;
        }    
    }
    return buf->average;
}

void AVERAGE_clear(uint8_t bufferid)
{
    Average* buf = &AVERAGE_buffers[bufferid];
    buf->sum = 0;
    buf->average = 0;
    buf->count = 0;
    buf->sum_base = 0;
}

#endif