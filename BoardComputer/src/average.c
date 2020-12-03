#include "average.h"


Average AVERAGE_buffers[AVERAGE_BUFFERS_SIZE];
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