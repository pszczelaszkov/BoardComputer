/*
 * timer.h
 *
 * Created: 2020-10-29 20:28:00
 * Author : pszczelaszkov
 */

#ifndef __TIMER__
#define __TIMER__
#include <stdlib.h>
#include <string.h>
#include "utils.h"


#define TIMER_REGISTER TCNT2
typedef struct TIMER_watch
{    
    struct
    {
        uint8_t hours;
        uint8_t minutes;
        uint8_t seconds;
        uint8_t miliseconds;//2digit
        uint8_t watchstatus;
    }timer; 
    struct TIMER_watch* next_watch;
}TIMER_watch;

enum TIMER_FORMATFLAG
{
    FORMATFLAG_HOURS = 1,
    FORMATFLAG_MINUTES = 4,
    FORMATFLAG_SECONDS = 8,
    FORMATFLAG_MILISECONDS = 16
};

enum TIMER_STOPWATCHSTATUS
{
    TIMER_WATCHSTATUS_ZERO,
    TIMER_WATCHSTATUS_COUNTING,
    TIMER_WATCHSTATUS_STOP,
};

enum TIMER_TIMERTYPE
{
    TIMERTYPE_WATCH,
    TIMERTYPE_STOPWATCH,
    TIMERTYPE_LAST
};

enum TIMER_FORMATED
{
    TIMER_FORMATEDHH = 0,
    TIMER_FORMATEDMM = 3,
    TIMER_FORMATEDSS = 6,
    TIMER_FORMATEDMS = 9
};

extern const uint8_t TIMER_REGISTER_WEIGHT;
extern const uint8_t TIMER_MILISECOND_WEIGHT;
extern TIMER_watch* TIMER_active_watch;
extern TIMER_watch TIMER_watches[];  
extern char TIMER_formated[];

uint8_t TIMER_counter_to_miliseconds();
uint8_t TIMER_increment(TIMER_watch* watch);
void TIMER_format(TIMER_watch* timer, uint8_t format_flag);
void TIMER_watch_toggle();
void TIMER_watch_zero();
void TIMER_next_watch();
void TIMER_update();
void TIMER_initialize();

#endif