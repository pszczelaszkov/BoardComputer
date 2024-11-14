/*
 * numpad.h
 *
 * Created: 2021-06-07 21:45:00
 * Author : pszczelaszkov
 */
#ifndef UINUMPAD
#define UINUMPAD

#include "../nextion.h"
#include "../input.h"

#define DISPLAYLENGTH 6

TESTUSE extern NEXTION_Component UINUMPAD_components[];
TESTUSE void UINUMPAD_switch(int16_t* returnvalue);
void UINUMPAD_setup();
void UINUMPAD_update();
void UINUMPAD_handle_userinput(INPUT_Event* input_event);
#ifndef __AVR__
TESTUSE char* UINUMPAD_getstringvalue();
#endif
#endif