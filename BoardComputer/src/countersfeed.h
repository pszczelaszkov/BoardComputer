/*
 * countersfeed.h
 *
 * Created: 2020-06-07 23:12:00
 *  Author: pszczelaszkov
 */
#ifndef COUNTERSFEED_H
#define COUNTERSFEED_H

#include "system_interface.h"

#define FRONTBUFFER 0
#define BACKBUFFER 1

TESTUSE enum COUNTERSFEED_FEEDID
{
    COUNTERSFEED_FEEDID_LPH,
    COUNTERSFEED_FEEDID_INJT_MS,
    COUNTERSFEED_FEEDID_SPEED_KPH,
    COUNTERSFEED_FEEDID_LP100,
    COUNTERSFEED_FEEDID_LP100_AVG,
    COUNTERSFEED_FEEDID_SPEED_AVG,
    COUNTERSFEED_FEEDID_LAST
};

TESTUSE extern volatile uint16_t COUNTERSFEED_feed[];
TESTUSE void COUNTERSFEED_initialize();
TESTUSE void COUNTERSFEED_update();
TESTUSE void COUNTERSFEED_count_fuelusage(uint16_t amount);
TESTUSE void COUNTERSFEED_count_speed(uint16_t amount);
#endif
