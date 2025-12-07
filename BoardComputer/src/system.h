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
#include "config.h"

#define EVENT_TIMER_ISR ISR(TIMER2_COMPA_vect)
TESTUSE typedef uint8_t SYSTEM_cycle_timestamp_t;
TESTUSE typedef uint16_t ALERT_PATTERN;

extern const SYSTEM_cycle_timestamp_t SYSTEM_fullcycle_rtc_steps;//System timestamp at the end of full cycle
TESTUSE extern const uint16_t SYSTEM_VERSION;
TESTUSE typedef enum SYSTEM_STATUS
{
    SYSTEM_STATUS_IDLE,
    SYSTEM_STATUS_OPERATIONAL,
}SYSTEM_STATUS;

TESTUSE typedef enum SYSTEM_ALERT_SEVERITY
{
    SYSTEM_ALERT_SEVERITY_NOTIFICATION,
    SYSTEM_ALERT_SEVERITY_WARNING,
    SYSTEM_ALERT_SEVERITY_CRITICAL
}SYSTEM_ALERT_SEVERITY;

TESTUSE typedef enum SYSTEM_ALERT
{
    SYSTEM_ALERT_NO_ALERT = 0,
    SYSTEM_ALERT_NOTIFICATIONS_END= 32,
    SYSTEM_ALERT_CONFIG_RESET = 33,/* Config was factory reseted */
    SYSTEM_ALERT_CONFIG_INCORRECT = 34/* Config validation found issue */,
    SYSTEM_ALERT_NEXTION_TIMEOUT = 35,/* Nextion Display does not respond in time*/
    SYSTEM_ALERT_WARNINGS_END = 64,
    SYSTEM_ALERT_UI_INCOMPATIBLE = 65,/* Display and Board UI versions mismatch */

}SYSTEM_ALERT;

TESTUSE typedef struct SYSTEM_Alert
{
    SYSTEM_ALERT alert;
    SYSTEM_ALERT_SEVERITY severity;
    ALERT_PATTERN pattern;
}SYSTEM_Alert_t;
/*
Returns current cycle timestamp.
Cycle is one second long, unit is RTC timer step.
*/
extern SYSTEM_cycle_timestamp_t SYSTEM_get_cycle_timestamp();
extern void SYSTEM_trigger_short_beep();
extern SYSTEM_ALERT_SEVERITY SYSTEM_resolve_alert_severity(SYSTEM_ALERT alert);
TESTUSE void SYSTEM_raisealert(SYSTEM_ALERT alert);
TESTUSE SYSTEM_Alert_t SYSTEM_get_active_alert();
TESTUSE void SYSTEM_resetalert();
TESTUSE extern CONFIG_Config SYSTEM_config;
TESTUSE extern volatile SYSTEM_STATUS SYSTEM_status;
TESTUSE extern volatile uint8_t SYSTEM_run;
TESTUSE extern volatile uint8_t SYSTEM_exec;
TESTUSE extern volatile uint8_t SYSTEM_event_timer;//Represent fraction of second in values from 0 to 7.
TESTUSE extern void SYSTEM_initialize();
TESTUSE void SYSTEM_update();
#ifndef __AVR__
EVENT_TIMER_ISR;
#endif
#endif