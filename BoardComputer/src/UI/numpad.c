#include "numpad.h"
#include "../nextion.h"
#include "../USART.h"

typedef enum INPUTCOMPONENTID
{
	INPUTCOMPONENT_NONE = 0,
	INPUTCOMPONENT_NUMPAD1 = 1,
	INPUTCOMPONENT_NUMPAD2 = 2,
	INPUTCOMPONENT_NUMPAD3 = 3,
	INPUTCOMPONENT_NUMPAD4 = 4,
	INPUTCOMPONENT_NUMPAD5 = 5,
	INPUTCOMPONENT_NUMPAD6 = 6,
	INPUTCOMPONENT_NUMPAD7 = 7,
	INPUTCOMPONENT_NUMPAD8 = 8,
	INPUTCOMPONENT_NUMPAD9 = 9,
	INPUTCOMPONENT_NUMPAD0 = 10,
	INPUTCOMPONENT_NUMPADMINUS = 11,
	INPUTCOMPONENT_NUMPADDEL = 12,
	INPUTCOMPONENT_NUMPADSEND = 13,
    INPUTCOMPONENT_LAST = 14,
}InputComponentID_t;

static const uint8_t max_length = UINUMPAD_DISPLAYLENGTH;
static const uint8_t cursor = UINUMPAD_DISPLAYLENGTH - 1;
static char objname[NEXTION_OBJNAME_LEN+1];
static char stringvalue[UINUMPAD_DISPLAYLENGTH+1];
static uint8_t current_length;
static uint8_t input_order_it = 0;
static int16_t* returnvalue_ptr;

/*
    For optimization purpose numpad holds memory for one component which works as placeholder.
    Selecting and Deselecting uses the same object with changed values. 
*/
static NEXTION_Component generic_button_component = 
{
    .highlighttype = NEXTION_HIGHLIGHTTYPE_BACKCOLOR,
    .value_default = PASTELORANGE,
    .value_selected = BRIGHTBLUE,
    .name = (char*)&objname
};

static void append(const char character)
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

static void delete()
{
    if(current_length)
    {
        uint8_t stringbeginning = cursor - current_length;
        memmove(&stringvalue[stringbeginning+1],&stringvalue[stringbeginning],current_length);
        current_length--;
    }
}

static void toggle_sign()
{
    if(current_length > 0)
    {
        if(stringvalue[0] == ' ')
            stringvalue[0] = '-';
        else
            stringvalue[0] = ' ';
    }    
}

static void send()
{
    *returnvalue_ptr = UTILS_atoi(stringvalue);
    UINUMPAD_reset();
    NEXTION_set_previous_page();
}

/*The only safe way to trigger numpad*/
void UINUMPAD_switch(int16_t* returnvalue)
{
    returnvalue_ptr = returnvalue;
    if(returnvalue_ptr)
        NEXTION_switch_page(NEXTION_PAGEID_NUMPAD, 1);
}

void UINUMPAD_handle_userinput(INPUT_Event* input_event)
{
    INPUT_Key_t key = input_event->key;
    /*
        Active component needs to be cleared before changes to placeholder component will be made.
    */
    NEXTION_clear_active_component();

    if(input_event->keystatus == INPUT_KEYSTATUS_CLICK)
    {
        /*
            Input component could be delivered from outside i.e touch event.
            When input is from physical buttons internal iterator is used.
        */
        InputComponentID_t componentID = input_event->componentID;
        if(INPUTCOMPONENT_NONE == componentID)
        {
            componentID = (InputComponentID_t)input_order_it;
        }

        switch(componentID)
        {
            case INPUTCOMPONENT_NUMPADSEND:
                if(INPUT_KEY_ENTER == key){
                    send();
                }
                memcpy(&objname,"snd",sizeof(objname));
            break;
            case INPUTCOMPONENT_NUMPADDEL:
                if(INPUT_KEY_ENTER == key){
                    delete();
                }
                memcpy(&objname,"del",sizeof(objname));
            break;
            case INPUTCOMPONENT_NUMPADMINUS:
                if(INPUT_KEY_ENTER == key){
                    toggle_sign();
                }
                memcpy(&objname,"mns",sizeof(objname));
            break;
            default:
                if(INPUTCOMPONENT_NONE < componentID)
                {
                    uint8_t digit = (INPUTCOMPONENT_NUMPAD0 == componentID)? 0 : (uint8_t)componentID;
                    //Contraption to build "b0n" string, where n is button number
                    memcpy(&objname,"b0 ",sizeof(objname));
                    digit = '0' + digit;
                    objname[2] = (char)digit;
                    if(INPUT_KEY_ENTER == key){
                        if(!(INPUTCOMPONENT_NUMPAD0 == componentID && current_length == 0)){
                            append(digit);
                        }
                    }
                }
        }
        if(key == INPUT_KEY_DOWN)
        {
            input_order_it++;
            if(INPUTCOMPONENT_LAST == input_order_it)
            {
                input_order_it = 1;
            }
        }
        /*
            Proceed only when componentID is resolved, in other case trash may be send to Nextion
        */
        if(INPUT_COMPONENT_NONE != componentID)
        {
            NEXTION_set_component_select_status(&generic_button_component, NEXTION_COMPONENTSELECTSTATUS_SELECTED);
        }
    }
}

void UINUMPAD_reset()
{
    memset(stringvalue,' ',max_length);
    current_length = 0;
    returnvalue_ptr = NULL;
    input_order_it = 0;
}

void UINUMPAD_setup()
{
    char buffer[UINUMPAD_DISPLAYLENGTH+1];
    int16_t returnvalue_value = *returnvalue_ptr;
    input_order_it = 0;

    uint8_t buffer_length;
    if(0 > returnvalue_value)
    {
        returnvalue_value*=-1;
        stringvalue[0] = '-';
    }
    itoa(returnvalue_value,buffer,10);
    buffer_length = strlen(buffer);

    memcpy(&stringvalue[max_length-buffer_length],buffer,buffer_length);
}

void UINUMPAD_update()
{
    NEXTION_INSTRUCTION_BUFFER_BLOCK(max_length+2)
    NEXTION_instruction_compose("dsp","txt",instruction);
	NEXTION_quote_payloadbuffer(payload,payload_length);
    memcpy(payload+1, stringvalue, max_length);
    NEXTION_send(buffer, USART_HOLD);
}

#ifndef __AVR__
char* UINUMPAD_getstringvalue() {return stringvalue;}
#endif