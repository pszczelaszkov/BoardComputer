#include "timer.h"

TESTSTATICVAR(static const uint8_t RTC_REGISTER_WEIGHT) = 200;//0.78125(7.8125ms) 8 FP
TESTSTATICVAR(static const uint8_t CENTISECONDSPERSTEP) = 12<<1|1;//12.5(125ms) 7+1 FP
TESTSTATICVAR(static TIMER_watch* active_watch);
static TIMER_watch watches[TIMER_TIMERTYPE_LAST];

TIMER_FORMATED_t TIMER_active_watch_formated = {.c_str=" 0:00:00:00"};//h:mm:ss:ms
enum TIMER_TIMERTYPE TIMER_active_timertype;

TESTUSE static uint8_t TESTADDPREFIX(increment)(TIMER_watch* watch, TIMER_centisecond_t centiseconds)
{
    const TIMER_centisecond_t full_second = 100 << 1;//200
    TIMER_centisecond_t clipped_centiseconds = 0;
    uint8_t format_flag = FORMATFLAG_NONE;
    while(centiseconds || clipped_centiseconds)
    {
        format_flag |= FORMATFLAG_CENTISECONDS;
        clipped_centiseconds = MIN(full_second - watch->timer.centiseconds,centiseconds); 
        watch->timer.centiseconds += clipped_centiseconds;
        centiseconds -= clipped_centiseconds;
        clipped_centiseconds = 0;
        if(full_second == watch->timer.centiseconds)
        {
            watch->timer.centiseconds = 0;
            watch->timer.seconds++;
            format_flag |= FORMATFLAG_SECONDS;  
            if(watch->timer.seconds == 60)
            {                
                watch->timer.minutes++;
                format_flag |= FORMATFLAG_MINUTES;
                watch->timer.seconds = 0;
                if(watch->timer.minutes == 60)
                {
                    watch->timer.hours++;
                    format_flag |= FORMATFLAG_HOURS;
                    watch->timer.minutes = 0;
                    if(watch->timer.hours == 24)
                        watch->timer.hours = 0;
                }
            }
        }
    }
    return format_flag; 
}

TESTUSE static uint8_t TESTADDPREFIX(decrement)(TIMER_watch* watch, TIMER_centisecond_t centiseconds)
{
    const TIMER_centisecond_t full_second = 100 << 1;//200
    TIMER_centisecond_t clipped_centiseconds = 0;
    uint8_t format_flag = FORMATFLAG_NONE;

    while(centiseconds || clipped_centiseconds)
    {
        format_flag |= FORMATFLAG_CENTISECONDS;
        clipped_centiseconds = MIN(watch->timer.centiseconds+1,centiseconds); 
        centiseconds -= clipped_centiseconds;
        watch->timer.centiseconds -= clipped_centiseconds;
        clipped_centiseconds = 0;
        if(0xff == watch->timer.centiseconds)
        {
            format_flag |= FORMATFLAG_SECONDS; 
            watch->timer.centiseconds = full_second-1;
            watch->timer.seconds--;
            if(0xff == watch->timer.seconds)
            {  
                format_flag |= FORMATFLAG_MINUTES;              
                watch->timer.minutes--;
                watch->timer.seconds = 59;
                if(0xff == watch->timer.minutes)
                {
                    format_flag |= FORMATFLAG_HOURS;                    
                    watch->timer.hours--;
                    watch->timer.minutes = 59;
                    if(0xff == watch->timer.hours)
                        watch->timer.hours = 23;
                }
            }
        }
    }

    return format_flag;
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

/*Format input timer to string based formated string*/
void TIMER_format(TIMER_watch* timer, TIMER_FORMATED_t* formated, enum TIMER_FORMATFLAG format_flag)
{
    switch(format_flag)
    {
        case FORMATFLAG_HOURS:
            *formated->segments.hh = ' ';
            rightconcat_short(formated->segments.hh,timer->timer.hours,2);
        case FORMATFLAG_MINUTES:
            *formated->segments.mm = '0';
            rightconcat_short(formated->segments.mm,timer->timer.minutes,2);
        case FORMATFLAG_SECONDS:
            *formated->segments.ss = '0';
            rightconcat_short(formated->segments.ss,timer->timer.seconds,2);
        case FORMATFLAG_CENTISECONDS:
            *formated->segments.cs = '0';
            rightconcat_short(formated->segments.cs,timer->timer.centiseconds>>1,2);
        break;
    }
}

/*Toggle status of currently active watch*/
void TIMER_active_watch_toggle(TIMER_centisecond_t time_offset)
{
    if(active_watch->timer.watchstatus == TIMER_TIMERSTATUS_COUNTING)
    {
        active_watch->timer.watchstatus = TIMER_TIMERSTATUS_STOP;
    }
    else
        active_watch->timer.watchstatus = TIMER_TIMERSTATUS_COUNTING;
}

void TIMER_clear_active_watch()
{
    memset(&active_watch->timer,0x0,sizeof(active_watch->timer));
    active_watch->timer.watchstatus = TIMER_TIMERSTATUS_ZERO;
    TIMER_format(active_watch, &TIMER_active_watch_formated, FORMATFLAG_HOURS);
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
        TIMER_format(active_watch, &TIMER_active_watch_formated, FORMATFLAG_HOURS);
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
    return (uint16_t)(timestamp * RTC_REGISTER_WEIGHT + 128) >> 8;
}

void TIMER_userinput_handle_watch(INPUT_Event* input_event)
{
    if(INPUT_KEY_ENTER == input_event->key)
    {
        if(TIMER_TIMERTYPE_STOPWATCH== TIMER_active_timertype)
        {
            if(INPUT_KEYSTATUS_HOLD == input_event->keystatus)
            {
                TIMER_clear_active_watch();
            }
            else if(INPUT_KEYSTATUS_CLICK == input_event->keystatus)
            {

                SYSTEM_cycle_timestamp_t current_timestamp = SYSTEM_get_cycle_timestamp();
                SYSTEM_cycle_timestamp_t event_timestamp = input_event->timestamp;
                SYSTEM_cycle_timestamp_t delta_timestamp = current_timestamp-event_timestamp;

                if(delta_timestamp >= current_timestamp)
                    delta_timestamp -= (0xff-SYSTEM_fullcycle_rtc_steps+1);
                
                if(TIMER_TIMERSTATUS_STOP == active_watch->timer.watchstatus || TIMER_TIMERSTATUS_ZERO == active_watch->timer.watchstatus)
                {
                    increment(active_watch,TIMER_cycle_timestamp_to_cs(delta_timestamp));
                    start_watch(active_watch);
                }
                else if(TIMER_TIMERSTATUS_COUNTING == active_watch->timer.watchstatus)
                {
                    stop_watch(active_watch);                    
                    decrement(active_watch,TIMER_cycle_timestamp_to_cs(delta_timestamp));
                }
            }
        }
    }
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
    TIMER_format(active_watch, &TIMER_active_watch_formated, format_flag);
}

void TIMER_initialize()
{
    switch_watchtype(TIMER_TIMERTYPE_WATCH);
    watches[TIMER_TIMERTYPE_WATCH].timer.watchstatus = TIMER_TIMERSTATUS_COUNTING;
}