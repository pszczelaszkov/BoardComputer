#include "config.h"
#include <config.h>
#include "nextion.h"
#include "utils.h"
#include "UI/numpad.h"

typedef enum INPUTCOMPONENTID
{
	INPUTCOMPONENT_NONE = 0,
    INPUTCOMPONENT_PRV = 5,
    INPUTCOMPONENT_NXT = 6,
    INPUTCOMPONENT_DEC = 7,
    INPUTCOMPONENT_INC = 8,
    INPUTCOMPONENT_BCK = 9,
    INPUTCOMPONENT_PAD = 10,
    INPUTCOMPONENT_LAST,
}InputComponentID_t;

typedef enum DIRECTION
{
    DIRECTION_BACKWARD = 0,
    DIRECTION_FORWARD = 1,
}Direction_t;

static char objname[NEXTION_OBJNAME_LEN+1];
static InputComponentID_t inputcomponent_it;
static uint8_t configvariable_it;
static CONFIG_maxdata_t configvariable_value;
static NEXTION_Component generic_button_component = 
{
    .highlighttype = NEXTION_HIGHLIGHTTYPE_BACKCOLOR,
    .value_default = BACKGROUNDGRAY,
    .value_selected = BRIGHTBLUE,
    .name = (char*)&objname
};
static uint8_t send_configpointer_to_nextion();
static uint8_t send_configvalue_to_nextion();
static uint8_t check_configvalue_is_valid();
static void save_configvariable();

static void switch_configvariable(Direction_t direction);
static void modify_configvariable_value(Direction_t direction);
static void incomingdata_handler(int32_t data);

static void save_configvariable()
{
    if(check_configvalue_is_valid())
    {
        CONFIG_modify_entry(&SYSTEM_config, configvariable_it, &configvariable_value);
    }
}

static uint8_t send_configpointer_to_nextion()
{
    NEXTION_INSTRUCTION_BUFFER_BLOCK(3)
    CONFIG_maxdata_t min, max;
    CONFIG_get_entry_min_max_values(configvariable_it,&min,&max);
    NEXTION_instruction_compose("ptr","val",instruction);
    u16toa(configvariable_it, payload);
    NEXTION_send(buffer,0);

    NEXTION_instruction_compose("min","val",instruction);
    u16toa(min, payload);
    NEXTION_send(buffer,0);

    NEXTION_instruction_compose("max","val",instruction);
    u16toa(max, payload);
    NEXTION_send(buffer,0);

    NEXTION_send("rfp.en=1",0);
}

static uint8_t send_configvalue_to_nextion()
{
    NEXTION_INSTRUCTION_BUFFER_BLOCK(11)
    /*
        For now it's 32bit based 1x32 Value Lower Half
        If future 64 bit support is needed it needs to be adjusted to support vhh.
    */
    NEXTION_instruction_compose("vlh","val",instruction);
    i32toa(configvariable_value, payload);
    return NEXTION_send(buffer,0);
}

/*
    Checks if current config value is valid,
    if not then status will be send to nextion and value will be auto adjusted
*/
static uint8_t check_configvalue_is_valid()
{   
    
    CONFIG_ENTRY_VALIDATOR_RESULT validator_result = CONFIG_validate_entry(configvariable_it, configvariable_value);
    if(CONFIG_ENTRY_VERDICT_PASS == validator_result.verdict)
    {
        return 1;
    }
    else
    {
        NEXTION_INSTRUCTION_BUFFER_BLOCK(3)
        NEXTION_instruction_compose("res","val",instruction);
        i16toa(validator_result.verdict, payload);
        NEXTION_send(buffer,0);

        configvariable_value = validator_result.value;
        return 0;
    }
}

static void switch_configvariable(Direction_t direction)
{
    switch(direction)
    {
        case DIRECTION_FORWARD:
            if(CONFIG_ENTRY_LAST == ++configvariable_it)
            {
                configvariable_it = 0;
            }
        break;
        case DIRECTION_BACKWARD:
            if(0 == configvariable_it--)
            {
               configvariable_it = CONFIG_ENTRY_LAST-1; 
            }
        break;
    }

    CONFIG_read_entry(&SYSTEM_config, configvariable_it, &configvariable_value);

    if(send_configvalue_to_nextion())
        send_configpointer_to_nextion();
}

static void modify_configvariable_value(Direction_t direction)
{
    CONFIG_maxdata_t min, max;
    CONFIG_get_entry_min_max_values(configvariable_it, &min, &max);
    switch(direction)
    {
        case DIRECTION_FORWARD:
            if(configvariable_value < max)
                configvariable_value++;
        break;
        case DIRECTION_BACKWARD:
            if(min < configvariable_value)
                configvariable_value--;
        break;
    }
    send_configvalue_to_nextion();
}
/*Handler of incoming data, I.e slider value*/
static void incomingdata_handler(int32_t data)
{
    configvariable_value = data;
}

void UICONFIG_handle_userinput(INPUT_Event* input_event)
{
    const InputComponentID_t inputcomponent_it_current = inputcomponent_it;
    INPUT_Key_t key = input_event->key;
    InputComponentID_t componentID = input_event->componentID;
    INPUT_Keystatus_t keystatus = input_event->keystatus;

    if(INPUT_KEYSTATUS_CLICK == keystatus)
    {
        if(INPUT_KEY_DOWN == key)
        { 
            if(INPUTCOMPONENT_LAST == inputcomponent_it || INPUTCOMPONENT_NONE == inputcomponent_it)
            {
                inputcomponent_it = INPUTCOMPONENT_PRV;
            }
            else
            {
                inputcomponent_it++;
            }
        }

        if(INPUTCOMPONENT_NONE == componentID)
        {
            /* Physical key without assigned component. */
            componentID = inputcomponent_it;
        }
        else
        {
            /* 
                Touch event brings own assigned component.
                Move focus to that element.
            */
            inputcomponent_it = componentID;
        }

        if(INPUT_KEY_ENTER == key)
        {
            switch(componentID)
            {
                case INPUTCOMPONENT_PRV:
                case INPUTCOMPONENT_NXT:
                    save_configvariable();
                    switch_configvariable((Direction_t)componentID - INPUTCOMPONENT_PRV);
                break;
                case INPUTCOMPONENT_DEC:
                case INPUTCOMPONENT_INC:
                    modify_configvariable_value((Direction_t)componentID - INPUTCOMPONENT_DEC);
                break;
                case INPUTCOMPONENT_BCK:
                    save_configvariable();
                    if(1 == SYSTEM_config.SYSTEM_FACTORY_RESET)
                    {
                        SYSTEM_config.CONFIG_VERSION = 0;
                        SYSTEM_run = 0; // reset device.
                    }
                    CONFIG_saveconfig(&SYSTEM_config);
                    NEXTION_set_previous_page();
                    return;
                break;
                case INPUTCOMPONENT_PAD:
                    UINUMPAD_switch(&configvariable_value);
                    return;
                break;
                default:
            }
        }

        if(inputcomponent_it_current != componentID)
        {
            /*
                InputComponent has been switched.
                Active component needs to be cleared when next component selection will happen and
                before changes to placeholder component will be made.
            */
            NEXTION_clear_selected_component();
        }

        //Resolve generic input component name.
        switch(componentID)
        {
            case INPUTCOMPONENT_PRV:
                memcpy(&objname,"prv",sizeof(objname));
            break;
            case INPUTCOMPONENT_NXT:
                memcpy(&objname,"nxt",sizeof(objname));
            break;
            case INPUTCOMPONENT_DEC:
                memcpy(&objname,"dec",sizeof(objname));
            break;
            case INPUTCOMPONENT_INC:
                memcpy(&objname,"inc",sizeof(objname));
            break;
            case INPUTCOMPONENT_BCK:
                memcpy(&objname,"bck",sizeof(objname));
            break;
            case INPUTCOMPONENT_PAD:
                memcpy(&objname,"pad",sizeof(objname));
            break;
            default:
        }

        //Always select, at least for refreshing counter.
        NEXTION_set_component_select_status(&generic_button_component, NEXTION_COMPONENTSELECTSTATUS_SELECTED);
    }
}

void UICONFIG_setup()
{
    /*
        If pad was last component used on config page, and config is active again
        in normal circumstances it means that page is activated after going back from pad.
        Continue operation instead of reset.
    */
    if(INPUTCOMPONENT_PAD == inputcomponent_it)
    {        
        check_configvalue_is_valid();
        send_configvalue_to_nextion();
        send_configpointer_to_nextion();
    }
    else
    {
        inputcomponent_it = 0;
        configvariable_it = 0;

        CONFIG_read_entry(&SYSTEM_config, configvariable_it, &configvariable_value);

        if(send_configvalue_to_nextion())
            send_configpointer_to_nextion();


    }
    NEXTION_incomingdata_handler = incomingdata_handler;
}

void UICONFIG_update()
{
}