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
void UINUMPAD_setup();
TESTUSE void UINUMPAD_update();
TESTUSE void UINUMPAD_reset();
TESTUSE void UINUMPAD_handle_userinput(INPUT_Event* input_event);
#ifndef __AVR__
TESTUSE char* UINUMPAD_getstringvalue();
#endif
#endif