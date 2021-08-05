#include "numpad.h"
#include "../nextion.h"

static const uint8_t max_length = DISPLAYLENGTH;
static const uint8_t cursor = DISPLAYLENGTH - 1;

static const char str_b00[NEXTION_OBJNAME_LEN] = "b00";
static const char str_b01[NEXTION_OBJNAME_LEN] = "b01";
static const char str_b02[NEXTION_OBJNAME_LEN] = "b02";
static const char str_b03[NEXTION_OBJNAME_LEN] = "b03";
static const char str_b04[NEXTION_OBJNAME_LEN] = "b04";
static const char str_b05[NEXTION_OBJNAME_LEN] = "b05";
static const char str_b06[NEXTION_OBJNAME_LEN] = "b06";
static const char str_b07[NEXTION_OBJNAME_LEN] = "b07";
static const char str_b08[NEXTION_OBJNAME_LEN] = "b08";
static const char str_b09[NEXTION_OBJNAME_LEN] = "b09";
static const char str_snd[NEXTION_OBJNAME_LEN] = "snd";
static const char str_mns[NEXTION_OBJNAME_LEN] = "mns";
static const char str_del[NEXTION_OBJNAME_LEN] = "del";

static char stringvalue[DISPLAYLENGTH+1];
static uint8_t current_length;
static uint16_t* target_ptr;

NEXTION_Component UINUMPAD_components[] = 
{
    [UINUMPAD_COMPONENT_B0] = 
    {
        .highlighttype = NEXTION_HIGHLIGHTTYPE_IMAGE,
		.value_default = PASTELORANGE,
		.value_selected = BRIGHTBLUE,
		.name = str_b00
    },
    [UINUMPAD_COMPONENT_B1] = 
    {
        .highlighttype = NEXTION_HIGHLIGHTTYPE_IMAGE,
		.value_default = PASTELORANGE,
		.value_selected = BRIGHTBLUE,
		.name = str_b01
    },
    [UINUMPAD_COMPONENT_B2] = 
    {
        .highlighttype = NEXTION_HIGHLIGHTTYPE_IMAGE,
		.value_default = PASTELORANGE,
		.value_selected = BRIGHTBLUE,
		.name = str_b02
    },
    [UINUMPAD_COMPONENT_B3] = 
    {
        .highlighttype = NEXTION_HIGHLIGHTTYPE_IMAGE,
		.value_default = PASTELORANGE,
		.value_selected = BRIGHTBLUE,
		.name = str_b03
    },
    [UINUMPAD_COMPONENT_B4] = 
    {
        .highlighttype = NEXTION_HIGHLIGHTTYPE_IMAGE,
		.value_default = PASTELORANGE,
		.value_selected = BRIGHTBLUE,
		.name = str_b04
    },
    [UINUMPAD_COMPONENT_B5] = 
    {
        .highlighttype = NEXTION_HIGHLIGHTTYPE_IMAGE,
		.value_default = PASTELORANGE,
		.value_selected = BRIGHTBLUE,
		.name = str_b05
    },
    [UINUMPAD_COMPONENT_B6] = 
    {
        .highlighttype = NEXTION_HIGHLIGHTTYPE_IMAGE,
		.value_default = PASTELORANGE,
		.value_selected = BRIGHTBLUE,
		.name = str_b06
    },
    [UINUMPAD_COMPONENT_B7] = 
    {
        .highlighttype = NEXTION_HIGHLIGHTTYPE_IMAGE,
		.value_default = PASTELORANGE,
		.value_selected = BRIGHTBLUE,
		.name = str_b07
    },
    [UINUMPAD_COMPONENT_B8] = 
    {
        .highlighttype = NEXTION_HIGHLIGHTTYPE_IMAGE,
		.value_default = PASTELORANGE,
		.value_selected = BRIGHTBLUE,
		.name = str_b08
    },
    [UINUMPAD_COMPONENT_B9] = 
    {
        .highlighttype = NEXTION_HIGHLIGHTTYPE_IMAGE,
		.value_default = PASTELORANGE,
		.value_selected = BRIGHTBLUE,
		.name = str_b09
    },
    [UINUMPAD_COMPONENT_MINUS] = 
    {
        .highlighttype = NEXTION_HIGHLIGHTTYPE_IMAGE,
		.value_default = PASTELORANGE,
		.value_selected = BRIGHTBLUE,
		.name = str_mns
    },
    [UINUMPAD_COMPONENT_DEL] = 
    {
        .highlighttype = NEXTION_HIGHLIGHTTYPE_IMAGE,
		.value_default = PASTELORANGE,
		.value_selected = BRIGHTBLUE,
		.name = str_del
    },
    [UINUMPAD_COMPONENT_SEND] = 
    {
        .highlighttype = NEXTION_HIGHLIGHTTYPE_IMAGE,
		.value_default = PASTELORANGE,
		.value_selected = BRIGHTBLUE,
		.name = str_snd
    }
};

void append(const char character)
{
    //Sign occupies first index thus -1 for available length
    if(current_length < max_length-1)
    {
        uint8_t stringbeginning = cursor - current_length + 1;
        memmove(&stringvalue[stringbeginning-1],&stringvalue[stringbeginning],current_length);
        stringvalue[cursor] = character;
        current_length++;
    }
}

void delete()
{
    if(current_length)
    {
        uint8_t stringbeginning = cursor - current_length;
        memmove(&stringvalue[stringbeginning+1],&stringvalue[stringbeginning],current_length);
        current_length--;
    }
}

void toggle_sign()
{
    if(current_length > 0)
    {
        if(stringvalue[0] == ' ')
            stringvalue[0] = '-';
        else
            stringvalue[0] = ' ';
    }    
}
void UINUMPAD_click_b0()
{
    if(current_length > 0)
        append('0');
}
void UINUMPAD_click_b1()
{
    append('1');
}
void UINUMPAD_click_b2()
{
    append('2');
}
void UINUMPAD_click_b3()
{
    append('3');
}
void UINUMPAD_click_b4()
{
    append('4');
}
void UINUMPAD_click_b5()
{
    append('5');
}
void UINUMPAD_click_b6()
{
    append('6');
}
void UINUMPAD_click_b7()
{
    append('7');
}
void UINUMPAD_click_b8()
{
    append('8');
}
void UINUMPAD_click_b9()
{
    append('9');
}
void UINUMPAD_click_del()
{
    delete();
}
void UINUMPAD_click_mns()
{
    toggle_sign();
}

void UINUMPAD_click_snd()
{
    if(current_length > 0)
        *target_ptr = UTILS_atoi(stringvalue);
}

/*The only safe way to trigger numpad*/
void UINUMPAD_switch(int16_t* target)
{
    target_ptr = target;
    if(target_ptr)
        NEXTION_switch_page(NEXTION_PAGEID_NUMPAD);
}

void UINUMPAD_setup()
{
    char buffer[7];
    uint8_t buffer_length;
    int16_t target_value = *target_ptr;
    uint16_t target_absvalue = (target_value > 0 ? target_value:target_value*-1);

    itoa(target_absvalue,buffer,10);
    buffer_length = strlen(buffer);
    if(target_absvalue)
        current_length = buffer_length;
    else
        current_length = 0;
    
    memset(stringvalue,' ',max_length);
    memcpy(&stringvalue[max_length-buffer_length],buffer,buffer_length);
    if(target_value < 0)
        toggle_sign();
}

void UINUMPAD_update()
{}

#ifndef __AVR__
char* UINUMPAD_getstringvalue() {return stringvalue;}
#endif