#include "boardconfig.h"
#include "../nextion.h"
#include "../USART.h"
#include "../utils.h"
#include "../sensorsfeed.h"
#include "../timer.h"
#include "../input.h"
#include "numpad.h"

static const char ipm_str[NEXTION_OBJNAME_LEN] = "ipm";
static const char ccm_str[NEXTION_OBJNAME_LEN] = "ccm";
static const char whh_str[NEXTION_OBJNAME_LEN] = "whh";
static const char wmm_str[NEXTION_OBJNAME_LEN] = "wmm";
static const char wss_str[NEXTION_OBJNAME_LEN] = "wss";
static const char dbs_str[NEXTION_OBJNAME_LEN] = "dbs";

static void display_ipm();
static void display_ccm();
static void display_whh();
static void display_wmm();
static void display_wss();
static void display_dbs();

NEXTION_Executable_Component UIBOARDCONFIG_executable_components[] = 
{   
    [UIBOARDCONFIG_COMPONENT_IPM] = 
    {
        .component = 
        {
            .name = ipm_str,
            .highlighttype = NEXTION_HIGHLIGHTTYPE_BACKCOLOR,
            .value_default = WHITE,
            .value_selected = BRIGHTBLUE
        },
        .execute = display_ipm
    },
    [UIBOARDCONFIG_COMPONENT_CCM] = 
    {
        .component = 
        {
            .name = ccm_str,
            .highlighttype = NEXTION_HIGHLIGHTTYPE_BACKCOLOR,
            .value_default = WHITE,
            .value_selected = BRIGHTBLUE
        },
        .execute = display_ccm
    },
    [UIBOARDCONFIG_COMPONENT_WHH] = 
    {
        .component = 
        {
            .name = whh_str,
            .highlighttype = NEXTION_HIGHLIGHTTYPE_BACKCOLOR,
            .value_default = WHITE,
            .value_selected = BRIGHTBLUE
        },
        .execute = display_whh
    },
    [UIBOARDCONFIG_COMPONENT_WMM] = 
    {
        .component = 
        {
            .name = wmm_str,
            .highlighttype = NEXTION_HIGHLIGHTTYPE_BACKCOLOR,
            .value_default = WHITE,
            .value_selected = BRIGHTBLUE
        },
        .execute = display_wmm
    },
    [UIBOARDCONFIG_COMPONENT_WSS] = 
    {
        .component = 
        {
            .name = wss_str,
            .highlighttype = NEXTION_HIGHLIGHTTYPE_BACKCOLOR,
            .value_default = WHITE,
            .value_selected = BRIGHTBLUE
        },
        .execute = display_wss
    },
    [UIBOARDCONFIG_COMPONENT_DBS] = 
    {
        .component = 
        {
            .name = dbs_str,
            .highlighttype = NEXTION_HIGHLIGHTTYPE_FRONTCOLOR,
            .value_default = BRIGHTBROWN,
            .value_selected = BRIGHTBLUE
        },
        .execute = display_dbs
    }
};

void display_ipm()
{
    char buffer[] = "ipm.val=      ";
    itoa(SENSORSFEED_speed_ticks_100m,&buffer[8],10);
	NEXTION_send(buffer, USART_HOLD);
}

void display_ccm()
{
    char buffer[] = "ccm.val=      ";
    itoa(SENSORSFEED_injector_ccm,&buffer[8],10);
	NEXTION_send(buffer, USART_HOLD);
}

void display_whh()
{
    char buffer[] = "whh.val=  ";

	itoa((&TIMER_watches[TIMERTYPE_WATCH])->timer.hours,&buffer[8],10);
	NEXTION_send(buffer, USART_HOLD);
}

void display_wmm()
{
    char buffer[] = "wmm.val=  ";
	itoa((&TIMER_watches[TIMERTYPE_WATCH])->timer.minutes,&buffer[8],10);
	NEXTION_send(buffer, USART_HOLD);
}

void display_wss()
{
    char buffer[] = "wss.val=  ";
	itoa((&TIMER_watches[TIMERTYPE_WATCH])->timer.seconds,&buffer[8],10);
	NEXTION_send(buffer, USART_HOLD);
}

void display_dbs()
{
    static uint8_t dbsvalue;
    if(dbsvalue == NEXTION_brightness)
        return;
    
    char buffer[] = "dbs.val=   ";
    itoa(NEXTION_brightness,&buffer[8],10);
	if(NEXTION_send(buffer, USART_HOLD))
        dbsvalue = NEXTION_brightness;
}

void UIBOARDCONFIG_modify_dbs()
{
    const uint8_t step = 5;
    static uint8_t autoreload_counter;
    if(autoreload_counter)
    {
        if(NEXTION_add_brightness(step,0))
            autoreload_counter--;
    }
    else
    {
        NEXTION_add_brightness(step,1);
        autoreload_counter = 8;//full cycle
    }
}

void UIBOARDCONFIG_modify_ccm()
{
    
}

void UIBOARDCONFIG_modify_ipm()
{
    UINUMPAD_switch(&SENSORSFEED_speed_ticks_100m);
}

void UIBOARDCONFIG_setup()
{
    INPUT_active_component = INPUT_findcomponent(INPUT_COMPONENT_CONFIGBCK);
}

void UIBOARDCONFIG_update()
{
    for(uint8_t i = 0; i < UIBOARDCONFIG_COMPONENT_LAST; i++)
    {
        Callback callback = UIBOARDCONFIG_executable_components[i].execute;
        if(callback)
            callback();
    }
};


