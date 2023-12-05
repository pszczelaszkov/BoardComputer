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


#define TIMER_REGISTER TCNT2

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
        uint8_t watchstatus;
    }timer; 
}TIMER_watch;

TESTUSE extern enum TIMER_TIMERTYPE TIMER_active_timertype;
TESTUSE extern char TIMER_formated[];

TESTUSE void TIMER_active_watch_toggle();
TESTUSE void TIMER_clear_active_watch();
TESTUSE void TIMER_next_watch();
TESTUSE void TIMER_update();
TESTUSE uint8_t TIMER_set_watch(enum TIMER_TIMERTYPE type);
TESTUSE TIMER_watch* TIMER_get_watch(enum TIMER_TIMERTYPE type);
void TIMER_initialize();

#endif