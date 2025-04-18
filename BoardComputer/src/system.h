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
TESTUSE typedef uint8_t SYSTEM_cycle_timestamp_t;
extern const SYSTEM_cycle_timestamp_t SYSTEM_fullcycle_rtc_steps;//System timestamp at the end of full cycle

TESTUSE typedef enum SYSTEM_STATUS
{
    SYSTEM_STATUS_IDLE,
    SYSTEM_STATUS_OPERATIONAL
}SYSTEM_STATUS_t;

TESTUSE typedef enum SYSTEM_ALERT
{
    SYSTEM_ALERT_NOTIFICATION,
    SYSTEM_ALERT_WARNING,
    SYSTEM_ALERT_CRITICAL
}SYSTEM_ALERT_t;

/*
Returns current cycle timestamp.
Cycle is one second long, unit is RTC timer step.
*/
SYSTEM_cycle_timestamp_t SYSTEM_get_cycle_timestamp();

TESTUSE void SYSTEM_raisealert(SYSTEM_ALERT_t alert);
TESTUSE extern volatile SYSTEM_STATUS_t SYSTEM_status;
TESTUSE extern volatile uint8_t SYSTEM_run;
TESTUSE extern volatile uint8_t SYSTEM_exec;
TESTUSE extern volatile uint8_t SYSTEM_event_timer;//Represent fraction of second in values from 0 to 7.
void SYSTEM_initialize();
TESTUSE void SYSTEM_update();
#ifndef __AVR__
EVENT_TIMER_ISR;
#endif
#endif