/*
 * numpad.h
 *
 * Created: 2021-06-07 21:45:00
 * Author : pszczelaszkov
 */
#ifndef UINUMPAD
#define UINUMPAD

#include <config.h>
#include "nextion.h"
#include "input.h"

#define UINUMPAD_DISPLAYLENGTH 6

TESTUSE void UINUMPAD_switch(CONFIG_maxdata_t* returnvalue);
TESTUSE void UINUMPAD_reset();
TESTUSE void UINUMPAD_page_control(NEXTION_page_control_t pagecontrol, void* data);
TESTUSE char* UINUMPAD_getstringvalue();

#endif