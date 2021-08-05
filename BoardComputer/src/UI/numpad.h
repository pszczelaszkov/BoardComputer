/*
 * numpad.h
 *
 * Created: 2021-06-07 21:45:00
 * Author : pszczelaszkov
 */
#ifndef UINUMPAD
#define UINUMPAD

#include "../nextion.h"
#define DISPLAYLENGTH 6
enum COMPONENT
{
    UINUMPAD_COMPONENT_B1,
    UINUMPAD_COMPONENT_B2,
    UINUMPAD_COMPONENT_B3,
    UINUMPAD_COMPONENT_B4,
    UINUMPAD_COMPONENT_B5,
    UINUMPAD_COMPONENT_B6,
    UINUMPAD_COMPONENT_B7,
    UINUMPAD_COMPONENT_B8,
    UINUMPAD_COMPONENT_B9,
    UINUMPAD_COMPONENT_B0,
    UINUMPAD_COMPONENT_MINUS,
    UINUMPAD_COMPONENT_DEL,
    UINUMPAD_COMPONENT_SEND,
};

void UINUMPAD_click_b1();
void UINUMPAD_click_b2();
void UINUMPAD_click_b3();
void UINUMPAD_click_b4();
void UINUMPAD_click_b5();
void UINUMPAD_click_b6();
void UINUMPAD_click_b7();
void UINUMPAD_click_b8();
void UINUMPAD_click_b9();
void UINUMPAD_click_b0();
void UINUMPAD_click_del();
void UINUMPAD_click_mns();
void UINUMPAD_click_snd();

extern NEXTION_Component UINUMPAD_components[];
void UINUMPAD_switch(int16_t* target);
void UINUMPAD_setup();
void UINUMPAD_update();
#ifndef __AVR__
char* UINUMPAD_getstringvalue();
#endif
#endif