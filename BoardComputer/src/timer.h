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

TESTUSE enum TIMER_FORMATFLAG
{
    TIMER_FORMATFLAG_NONE = 0,
    TIMER_FORMATFLAG_CENTISECONDS = 1,
    TIMER_FORMATFLAG_SECONDS = 3,
    TIMER_FORMATFLAG_MINUTES = 7,
    TIMER_FORMATFLAG_HOURS = 15,
};

TESTUSE enum TIMER_WATCHTYPE
{
    TIMER_WATCHTYPE_WATCH,
    TIMER_WATCHTYPE_STOPWATCH,
    TIMER_WATCHTYPE_LAST
};

TESTUSE enum TIMER_WATCHSTATUS
{
    TIMER_WATCHSTATUS_ZERO,
    TIMER_WATCHSTATUS_COUNTING,
    TIMER_WATCHSTATUS_STOP,
    TIMER_WATCHSTATUS_SETUP
};

TESTUSE typedef union TIMER_watch_formated
{
    struct
    {
        char hh[3];
        char mm[3];
        char ss[3];
        char cs[3];
    }segments;
    char c_str[12];
}TIMER_watch_formated;

TESTUSE typedef struct TIMER_watch
{    
    struct
    {
        uint8_t hours;
        uint8_t minutes;
        uint8_t seconds;
        uint8_t centiseconds;
        enum TIMER_WATCHSTATUS watchstatus;
    }timer; 
}TIMER_watch;
/*Unit representing 1/100 of second, should be handled as FPint 7+1 bit*/
TESTUSE typedef uint8_t TIMER_centisecond_t;
TESTUSE extern enum TIMER_WATCHTYPE TIMER_active_watchtype;
TESTUSE extern TIMER_watch_formated TIMER_active_watch_formated;
/*
Convert cycle timestamp to centiseconds.
*/
TESTUSE uint8_t TIMER_cycle_timestamp_to_cs(SYSTEM_cycle_timestamp_t timestamp);

TESTUSE void TIMER_userinput_handle_watch(INPUT_Event* input_event);
TESTUSE void TIMER_active_watch_toggle();
TESTUSE void TIMER_clear_active_watch();
TESTUSE void TIMER_next_watch();
TESTUSE void TIMER_update();
TESTUSE void TIMER_format(TIMER_watch* timer, TIMER_watch_formated* formated, enum TIMER_FORMATFLAG format_flag);
TESTUSE uint8_t TIMER_set_watch(enum TIMER_WATCHTYPE watchtype);
TESTUSE TIMER_watch* TIMER_get_watch(enum TIMER_WATCHTYPE watchtype);
void TIMER_initialize();

#endif