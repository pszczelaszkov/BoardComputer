/*
 * sensorsfeed.h
 *
 * Created: 2019-12-23 00:27:41
 *  Author: pszczelaszkov
 */ 

#ifndef SENSORSFEED_H_
#define SENSORSFEED_H_
#include "bitwise.h"
#include "utils.h"
#include "programdata.h"
#include "adc.h"
#include "egt.h"

extern const uint16_t SENSORSFEED_ADC_BAD_VALUE;

TESTUSE extern enum SENSORSFEED_EGT_STATUS
{
	SENSORSFEED_EGT_STATUS_UNKN,
	SENSORSFEED_EGT_STATUS_OPEN,
	SENSORSFEED_EGT_STATUS_VALUE
}SENSORSFEED_EGT_status;

TESTUSE enum SENSORSFEED_FEEDID
{
	SENSORSFEED_FEEDID_OILTEMP,
	SENSORSFEED_FEEDID_INTAKETEMP,
	SENSORSFEED_FEEDID_OUTTEMP,
	SENSORSFEED_FEEDID_MAP,
	SENSORSFEED_FEEDID_FRP,
	SENSORSFEED_FEEDID_TANK,
	SENSORSFEED_FEEDID_EGT,
	SENSORSFEED_FEEDID_LAST
};

TESTUSE typedef uint16_t FP16_t;// Fixedpoint 8+8

/* Main feed can be used as FP 8+8 or plain 16 bit integer, depends on sensor implementation*/
TESTUSE extern FP16_t SENSORSFEED_feed[];

TESTUSE void SENSORSFEED_update();
TESTUSE void SENSORSFEED_initialize();
/* Push value from ADC register into internal feed. */
TESTUSE void SENSORSFEED_push_adc_value();
#endif /* SENSORSFEED_H_ */
