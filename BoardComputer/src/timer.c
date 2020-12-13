#include "timer.h"


const uint8_t TIMER_REGISTER_WEIGHT = 200;//0.78125(7.8125ms) 8 FP
const uint8_t TIMER_MILISECOND_WEIGHT = 12<<1|1;//12.5(125ms) 7+1 FP
TIMER_watch* TIMER_active_watch;
TIMER_watch TIMER_watches[2];  
char TIMER_formated[12] = " 0:00:00:00";//h:mm:ss:ms

uint8_t TIMER_counter_to_miliseconds()
{
    uint16_t counter = TIMER_REGISTER;
    return (uint8_t)((counter * TIMER_REGISTER_WEIGHT) >> 8); 
}

void TIMER_format(TIMER_watch* timer, uint8_t format_flag)
{
    const char ASCIIDOUBLEZERO[] = "00";
    const char ASCIIDOUBLESPACE[] = "  ";
    
    switch(format_flag)
    {
        case FORMATFLAG_HOURS:
            memcpy(TIMER_formated,ASCIIDOUBLESPACE,2);
            rightconcat_short(TIMER_formated,timer->timer.hours,2);
        case FORMATFLAG_MINUTES:
            memcpy(&TIMER_formated[3],ASCIIDOUBLEZERO,2);
            rightconcat_short(&TIMER_formated[3],timer->timer.minutes,2);
        case FORMATFLAG_SECONDS:
            memcpy(&TIMER_formated[6],ASCIIDOUBLEZERO,2);
            rightconcat_short(&TIMER_formated[6],timer->timer.seconds,2);
        case FORMATFLAG_MILISECONDS:
            if(TIMER_active_watch != &TIMER_watches[TIMERTYPE_WATCH])
            {
                uint8_t miliseconds = timer->timer.miliseconds>>1;
                memcpy(&TIMER_formated[9],ASCIIDOUBLEZERO,2);
                rightconcat_short(&TIMER_formated[9],miliseconds,2);
            }
        break;
    }
}


uint8_t TIMER_increment(TIMER_watch* watch)
{
    uint8_t format_flag = 0;
    
    watch->timer.miliseconds += TIMER_MILISECOND_WEIGHT;
    format_flag = FORMATFLAG_MILISECONDS;
    if(watch->timer.miliseconds >= 200)//FP 7+1
    {
        watch->timer.miliseconds -= 200;
        watch->timer.seconds++;
        format_flag = FORMATFLAG_SECONDS;  
        if(watch->timer.seconds == 60)
        {                
            watch->timer.minutes++;
            format_flag = FORMATFLAG_MINUTES;
            watch->timer.seconds = 0;
            if(watch->timer.minutes == 60)
            {
                watch->timer.hours++;
                format_flag = FORMATFLAG_HOURS;
                watch->timer.minutes = 0;
                if(watch->timer.hours == 24)
                    watch->timer.hours = 0;
            }
        }
    }
    return format_flag; 
}

void TIMER_watch_toggle(TIMER_watch* watch)
{
    if(watch->timer.watchstatus == TIMER_WATCHSTATUS_COUNTING)
    {
        watch->timer.watchstatus = TIMER_WATCHSTATUS_STOP;
        //in fact thats the only moment when counter could be ahead of event timer.
        watch->timer.miliseconds += TIMER_counter_to_miliseconds() << 1;
    }
    else
        watch->timer.watchstatus = TIMER_WATCHSTATUS_COUNTING;
}

void TIMER_watch_zero(TIMER_watch* watch)
{
    watch->timer.watchstatus = TIMER_WATCHSTATUS_ZERO;
    memset(&watch->timer,0x0,sizeof(watch->timer));
    if(TIMER_active_watch == watch)
        TIMER_format(watch, FORMATFLAG_HOURS);
}

void TIMER_next_watch()
{
    TIMER_active_watch = TIMER_active_watch->next_watch;
    TIMER_format(TIMER_active_watch, FORMATFLAG_HOURS);
}

void TIMER_update()
{
    TIMER_watch* watch;
    uint8_t format_flag = 0;
    for(uint8_t i = 0;i < TIMERTYPE_LAST;i++)
    {
        watch = &TIMER_watches[i];
        if(watch->timer.watchstatus == TIMER_WATCHSTATUS_COUNTING)
        {
            if(TIMER_active_watch == watch)
                format_flag = TIMER_increment(watch);
            else
                TIMER_increment(watch);
            
        }
    }
    TIMER_format(TIMER_active_watch, format_flag);
}

void TIMER_initialize()
{
    #ifdef __AVR__
    //Event Timer
    OCR2A = 15;// 1/8 seconds
    ASSR = (1 << AS2);// async
    TCCR2A = (1 << WGM21);// Clear on match
    TCCR2B = (3 << CS21);// 256 prescaler
    TIMSK2 = (1 << OCIE2A);// Enable IRQ
    #endif

    TIMER_watches[TIMERTYPE_WATCH].next_watch = &TIMER_watches[TIMERTYPE_STOPWATCH];
    TIMER_watches[TIMERTYPE_STOPWATCH].next_watch = &TIMER_watches[TIMERTYPE_WATCH];
    TIMER_active_watch = &TIMER_watches[TIMERTYPE_WATCH];
    TIMER_active_watch->timer.watchstatus = TIMER_WATCHSTATUS_COUNTING;
}