/*
 * system.h
 *
 * Created: 2021-04-05 22:03:00
 * Author : pszczelaszkov
 */

#ifndef SYSTEM_H_
#define SYSTEM_H_
#include <inttypes.h>
#include "utils.h"

#define EVENT_TIMER_ISR ISR(TIMER2_COMPA_vect)

typedef enum SYSTEM_STATUS
{
    SYSTEM_STATUS_IDLE,
    SYSTEM_STATUS_GUIIDLE,
    SYSTEM_STATUS_HEADLESS, 
    SYSTEM_STATUS_OPERATIONAL
}SYSTEM_STATUS_t;

typedef enum SYSTEM_ALERT
{
    SYSTEM_ALERT_NOTIFICATION,
    SYSTEM_ALERT_WARNING,
    SYSTEM_ALERT_CRITICAL
}SYSTEM_ALERT_t;

void SYSTEM_raisealert(SYSTEM_ALERT_t alert);
extern volatile uint8_t SYSTEM_run;
extern volatile uint8_t SYSTEM_exec;
extern volatile uint8_t SYSTEM_event_timer;//Represent fraction of second in values from 0 to 7.
void SYSTEM_initialize();
void SYSTEM_update();
#ifndef __AVR__
EVENT_TIMER_ISR;
#endif
#endif