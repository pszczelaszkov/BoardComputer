/*
 * timer.h
 *
 * Created: 2020-10-29 20:28:00
 * Author : pszczelaszkov
 */
#ifndef __TIMER__
#define __TIMER__
#include <string.h>
#include "utils.h"
#include "system.h"
#include "input.h"

enum TIMER_FORMATFLAG
{
    FORMATFLAG_HOURS = 1,
    FORMATFLAG_MINUTES = 4,
    FORMATFLAG_SECONDS = 8,
    FORMATFLAG_MILISECONDS = 16
};

TESTUSE enum TIMER_TIMERTYPE
{
    TIMER_TIMERTYPE_WATCH,
    TIMER_TIMERTYPE_STOPWATCH,
    TIMER_TIMERTYPE_LAST
};

enum TIMER_FORMATED
{
    TIMER_FORMATEDHH = 0,
    TIMER_FORMATEDMM = 3,
    TIMER_FORMATEDSS = 6,
    TIMER_FORMATEDMS = 9
};

TESTUSE enum TIMER_TIMERSTATUS
{
    TIMER_TIMERSTATUS_ZERO,
    TIMER_TIMERSTATUS_COUNTING,
    TIMER_TIMERSTATUS_STOP,
};

TESTUSE typedef struct TIMER_watch
{    
    struct
    {
        uint8_t hours;
        uint8_t minutes;
        uint8_t seconds;
        uint8_t miliseconds;//2digit
        enum TIMER_TIMERSTATUS watchstatus;
    }timer; 
}TIMER_watch;
/*Unit representing 1/100 of second, should be handled as FPint 7+1 bit*/
TESTUSE typedef uint8_t TIMER_centisecond_t;
TESTUSE extern enum TIMER_TIMERTYPE TIMER_active_timertype;
TESTUSE extern char TIMER_formated[];

/*
Convert cycle timestamp to centiseconds.
*/
uint8_t TIMER_cycle_timestamp_to_cs(SYSTEM_cycle_timestamp_t timestamp);

void TIMER_handle_userinput_stopwatch(INPUT_Event* input_event);
void TIMER_userinput_handle_watch(INPUT_Event* input_event);
TESTUSE void TIMER_active_watch_toggle(TIMER_centisecond_t time_offset);
TESTUSE void TIMER_clear_active_watch();
TESTUSE void TIMER_next_watch();
TESTUSE void TIMER_update();
TESTUSE uint8_t TIMER_set_watch(enum TIMER_TIMERTYPE type);
TESTUSE TIMER_watch* TIMER_get_watch(enum TIMER_TIMERTYPE type);
void TIMER_initialize();

#endif