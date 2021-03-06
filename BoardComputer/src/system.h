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

extern volatile uint8_t SYSTEM_run;
extern volatile uint8_t SYSTEM_exec;
extern volatile uint8_t SYSTEM_event_timer;//Represent fraction of second in values from 0 to 7.
void SYSTEM_initialize();
#ifndef __AVR__
EVENT_TIMER_ISR;
#endif
#endif