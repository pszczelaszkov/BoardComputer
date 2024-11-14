#include "timer.h"

TESTSTATICVAR(static const uint8_t RTC_REGISTER_WEIGHT) = 200;//0.78125(7.8125ms) 8 FP
TESTSTATICVAR(static const uint8_t CENTISECONDSPERSTEP) = 12<<1|1;//12.5(125ms) 7+1 FP
TESTSTATICVAR(static TIMER_watch* active_watch);
static TIMER_watch watches[TIMER_TIMERTYPE_LAST];

char TIMER_formated[12] = " 0:00:00:00";//h:mm:ss:ms
enum TIMER_TIMERTYPE TIMER_active_timertype;

TESTUSE static uint8_t TESTADDPREFIX(increment)(TIMER_watch* watch, TIMER_centisecond_t centiseconds)
{
    const TIMER_centisecond_t full_second = 100 << 1;//200
    TIMER_centisecond_t clipped_centiseconds;
    uint8_t format_flag;
    
    do{
        clipped_centiseconds = MIN(full_second - watch->timer.miliseconds,centiseconds); 
        watch->timer.miliseconds += clipped_centiseconds;
        centiseconds -= clipped_centiseconds;
        format_flag = FORMATFLAG_MILISECONDS;
        if(100 << 1 == watch->timer.miliseconds && centiseconds)
        {
            watch->timer.miliseconds = 0;
            centiseconds--;
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
    }while(centiseconds);
    return format_flag; 
}

TESTUSE static uint8_t TESTADDPREFIX(decrement)(TIMER_watch* watch, TIMER_centisecond_t centiseconds)
{
    const TIMER_centisecond_t full_second = 100 << 1;//200
    TIMER_centisecond_t clipped_centiseconds;
    uint8_t format_flag;
    do{
        clipped_centiseconds = MIN(watch->timer.miliseconds,centiseconds); 
        watch->timer.miliseconds -= clipped_centiseconds;
        centiseconds -= clipped_centiseconds;
        format_flag = FORMATFLAG_MILISECONDS;
        if(0 == watch->timer.miliseconds && centiseconds)
        {
            watch->timer.miliseconds = 99 << 1;
            centiseconds--;
            watch->timer.seconds--;
            format_flag = FORMATFLAG_SECONDS;  
            if(0xff == watch->timer.seconds)
            {                
                watch->timer.minutes--;
                format_flag = FORMATFLAG_MINUTES;
                watch->timer.seconds = 0;
                if(0xff == watch->timer.minutes)
                {
                    watch->timer.hours--;
                    format_flag = FORMATFLAG_HOURS;
                    watch->timer.minutes = 0;
                    if(0xff == watch->timer.hours)
                        watch->timer.hours = 23;
                }
            }
        }
    }while(centiseconds);
    return format_flag;
}

TESTUSE static void TESTADDPREFIX(format)(TIMER_watch* timer, uint8_t format_flag)
{
    const char ASCIIDOUBLEZERO[] = "00";
    const char ASCIIDOUBLESPACE[] = "  ";
    
    switch(format_flag)
    {
        case FORMATFLAG_HOURS:
            memcpy(TIMER_formated,ASCIIDOUBLESPACE,2);
            rightconcat_short(TIMER_formated,timer->timer.hours,2);
        case FORMATFLAG_MINUTES:
            memcpy(&TIMER_formated[TIMER_FORMATEDMM],ASCIIDOUBLEZERO,2);
            rightconcat_short(&TIMER_formated[TIMER_FORMATEDMM],timer->timer.minutes,2);
        case FORMATFLAG_SECONDS:
            memcpy(&TIMER_formated[TIMER_FORMATEDSS],ASCIIDOUBLEZERO,2);
            rightconcat_short(&TIMER_formated[TIMER_FORMATEDSS],timer->timer.seconds,2);
        case FORMATFLAG_MILISECONDS:
            if(active_watch != &watches[TIMER_TIMERTYPE_WATCH])
            {
                uint8_t miliseconds = timer->timer.miliseconds>>1;
                memcpy(&TIMER_formated[TIMER_FORMATEDMS],ASCIIDOUBLEZERO,2);
                rightconcat_short(&TIMER_formated[TIMER_FORMATEDMS],miliseconds,2);
            }
        break;
    }
}

static void start_watch(TIMER_watch* watch)
{
    watch->timer.watchstatus = TIMER_TIMERSTATUS_COUNTING;
}

static void stop_watch(TIMER_watch* watch)
{
    watch->timer.watchstatus = TIMER_TIMERSTATUS_STOP;
}

static void switch_watchtype(enum TIMER_TIMERTYPE type)
{
    active_watch = &watches[type];
    TIMER_active_timertype = type;
}

/*Toggle status of currently active watch*/
void TIMER_active_watch_toggle(TIMER_centisecond_t time_offset)
{
    if(active_watch->timer.watchstatus == TIMER_TIMERSTATUS_COUNTING)
    {
        active_watch->timer.watchstatus = TIMER_TIMERSTATUS_STOP;
        //in fact thats the only moment when counter could be ahead of event timer.
        //active_watch->timer.miliseconds += counter_to_miliseconds() << 1;
    }
    else
        active_watch->timer.watchstatus = TIMER_TIMERSTATUS_COUNTING;
}

void TIMER_clear_active_watch()
{
    active_watch->timer.watchstatus = TIMER_TIMERSTATUS_ZERO;
    memset(&active_watch->timer,0x0,sizeof(active_watch->timer));
    format(active_watch, FORMATFLAG_HOURS);
}

void TIMER_next_watch()
{   
    uint8_t timertype = TIMER_active_timertype + 1;
    if(timertype == TIMER_TIMERTYPE_LAST)
        timertype = 0;
    TIMER_set_watch(timertype);
}

uint8_t TIMER_set_watch(enum TIMER_TIMERTYPE type)
{
    if(type < TIMER_TIMERTYPE_LAST)
    {
        switch_watchtype(type);
        format(active_watch, FORMATFLAG_HOURS);
        return 1;
    }
    return 0;
}

TIMER_watch* TIMER_get_watch(enum TIMER_TIMERTYPE type)
{ 
    TIMER_watch* watchptr = 0x0;
    if(type < TIMER_TIMERTYPE_LAST)
    {
        watchptr = &watches[type];
    }
    return watchptr;
}

uint8_t TIMER_cycle_timestamp_to_cs(SYSTEM_cycle_timestamp_t timestamp)
{
    return (uint16_t)(timestamp * RTC_REGISTER_WEIGHT) >> 8;
}

void TIMER_userinput_handle_watch(INPUT_Event* input_event)
{
    if(input_event->key == INPUT_KEY_ENTER)
    {
        if(TIMER_active_timertype == TIMER_TIMERTYPE_STOPWATCH)
        {
            if(input_event->keystatus == INPUT_KEYSTATUS_HOLD)
            {
                TIMER_clear_active_watch;
            }
            else if(input_event->keystatus == INPUT_KEYSTATUS_PRESSED)
            {

                SYSTEM_cycle_timestamp_t current_timestamp = SYSTEM_get_cycle_timestamp();
                SYSTEM_cycle_timestamp_t event_timestamp = input_event->timestamp;
                SYSTEM_cycle_timestamp_t delta_timestamp = current_timestamp-event_timestamp;

                if(delta_timestamp >= current_timestamp)
                    delta_timestamp -= (0xff-SYSTEM_fullcycle_rtc_steps+1);
                
                if(active_watch->timer.watchstatus == TIMER_TIMERSTATUS_STOP)
                {
                    increment(active_watch,TIMER_cycle_timestamp_to_cs(delta_timestamp));
                    start_watch(active_watch);
                }
                else if(active_watch->timer.watchstatus == TIMER_TIMERSTATUS_COUNTING)
                {
                    stop_watch(active_watch);                    
                    decrement(active_watch,TIMER_cycle_timestamp_to_cs(delta_timestamp));
                }
            }
        }
    }
}

void TIMER_handle_userinput_stopwatch(INPUT_Event* input_event)
{

}

void TIMER_update()
{
    TIMER_watch* watch;
    uint8_t format_flag = 0;
    for(uint8_t i = 0;i < TIMER_TIMERTYPE_LAST;i++)
    {
        watch = &watches[i];
        if(watch->timer.watchstatus == TIMER_TIMERSTATUS_COUNTING)
        {
            if(active_watch == watch)
                format_flag = increment(watch,CENTISECONDSPERSTEP);
            else
                increment(watch,CENTISECONDSPERSTEP);
        }
    }
    format(active_watch, format_flag);
}

void TIMER_initialize()
{
    switch_watchtype(TIMER_TIMERTYPE_WATCH);
    watches[TIMER_TIMERTYPE_WATCH].timer.watchstatus = TIMER_TIMERSTATUS_COUNTING;
}