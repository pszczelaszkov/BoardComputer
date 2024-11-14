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
	INPUTCOMPONENT_NUMPADSEND = 14,
    INPUTCOMPONENT_LAST = 15,
}InputComponentID_t;

static const uint8_t max_length = DISPLAYLENGTH;
static const uint8_t cursor = DISPLAYLENGTH - 1;

static char stringvalue[DISPLAYLENGTH+1];
static uint8_t current_length;
static uint16_t* returnvalue_ptr;

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

void send()
{
    if(current_length > 0)
        *returnvalue_ptr = UTILS_atoi(stringvalue);
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
    static uint8_t input_order_it = 0;
    static NEXTION_Component generic_button_component = 
    {
        .highlighttype = NEXTION_HIGHLIGHTTYPE_BACKCOLOR,
		.value_default = PASTELORANGE,
		.value_selected = BRIGHTBLUE,
    };//Placeholder for highlighting on nextion side.
    char* name[NEXTION_OBJNAME_LEN+1];

    if(input_event->keystatus == INPUT_KEYSTATUS_CLICK)
    {
        if(input_event->key == INPUT_KEY_ENTER)
        {
            InputComponentID_t componentID = input_event->componentID;
            if(INPUTCOMPONENT_NONE == componentID)
            {
                componentID = (InputComponentID_t)input_order_it;
            }

            switch(componentID)
            {
                case INPUTCOMPONENT_NUMPADSEND:
                    send();
                break;
                case INPUTCOMPONENT_NUMPADDEL:
                    delete();
                    *name = "del";
                break;
                case INPUTCOMPONENT_NUMPADMINUS:
                    toggle_sign();
                    *name = "mns";
                break;
                default:
                    if(INPUTCOMPONENT_NONE < componentID && INPUTCOMPONENT_NUMPAD0 > componentID || (INPUTCOMPONENT_NUMPAD0 == componentID && current_length > 0))
                    {
                        uint8_t digit = (INPUTCOMPONENT_NUMPAD0 == componentID)? 0 : (uint8_t)componentID;
                        //Contraption to build "b0n" string, where n is button number
                        *name = "b0 ";
                        digit = '0' + digit;
                        (*name)[2] = digit;
                        append(digit);
                    } 
            }
        }
        else if(input_event->key == INPUT_KEY_DOWN)
        {
            input_order_it++;
            if(INPUTCOMPONENT_LAST == input_order_it)
            {
                input_order_it = 0;
            }
        }
        /*
        For optimization purpose there's only one component in numpad
        if previous one is active it will have the same ptr.
        Manual clear is needed to ensure new component will be accepted. 
        */
        NEXTION_clear_active_component();
        generic_button_component.name = *name;
        NEXTION_set_component_select_status(&generic_button_component, NEXTION_COMPONENTSELECTSTATUS_SELECTED);
    }
}

void UINUMPAD_setup()
{
    char buffer[7];
    uint8_t buffer_length;
    int16_t returnvalue_value = *returnvalue_ptr;
    uint16_t returnvalue_absvalue = (returnvalue_value > 0 ? returnvalue_value:returnvalue_value*-1);

    itoa(returnvalue_absvalue,buffer,10);
    buffer_length = strlen(buffer);
    if(returnvalue_absvalue)
        current_length = buffer_length;
    else
        current_length = 0;
    
    memset(stringvalue,' ',max_length);
    memcpy(&stringvalue[max_length-buffer_length],buffer,buffer_length);
    if(returnvalue_value < 0)
        toggle_sign();

}

void UINUMPAD_update()
{
    char buffer[11+DISPLAYLENGTH] = "dsp.txt=\"      \"";
    memcpy(&buffer[9], stringvalue, DISPLAYLENGTH);
    NEXTION_send(buffer, USART_HOLD);
}

#ifndef __AVR__
char* UINUMPAD_getstringvalue() {return stringvalue;}
#endif