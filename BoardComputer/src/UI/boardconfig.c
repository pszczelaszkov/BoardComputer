#include "boardconfig.h"
#include "../nextion.h"
#include "../USART.h"
#include "../utils.h"
#include "../sensorsfeed.h"
#include "../timer.h"

static uint8_t update_dbs = 1;

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
            .type = NEXTION_COMPONENTTYPE_TEXTFIELD,
            .picID_default = 26,
            .picID_selected = 25
        },
        .execute = display_ipm
    },
    [UIBOARDCONFIG_COMPONENT_CCM] = 
    {
        .component = 
        {
            .name = ccm_str,
            .type = NEXTION_COMPONENTTYPE_TEXTFIELD,
            .picID_default = 26,
            .picID_selected = 25
        },
        .execute = display_ccm
    },
    [UIBOARDCONFIG_COMPONENT_WHH] = 
    {
        .component = 
        {
            .name = whh_str,
            .type = NEXTION_COMPONENTTYPE_TEXTFIELD,
            .picID_default = 26,
            .picID_selected = 25
        },
        .execute = display_whh
    },
    [UIBOARDCONFIG_COMPONENT_WMM] = 
    {
        .component = 
        {
            .name = wmm_str,
            .type = NEXTION_COMPONENTTYPE_TEXTFIELD,
            .picID_default = 26,
            .picID_selected = 25
        },
        .execute = display_wmm
    },
    [UIBOARDCONFIG_COMPONENT_WSS] = 
    {
        .component = 
        {
            .name = wss_str,
            .type = NEXTION_COMPONENTTYPE_TEXTFIELD,
            .picID_default = 26,
            .picID_selected = 25
        },
        .execute = display_wss
    },
    [UIBOARDCONFIG_COMPONENT_DBS] = 
    {
        .component = 
        {
            .name = dbs_str,
            .type = NEXTION_COMPONENTTYPE_SLIDER,
            .picID_default = 26,
            .picID_selected = 25
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
    memcpy(&buffer[8],TIMER_formated,2);

    if(buffer[8] == ' ')//double digit guard
        buffer[8] = '0';

	NEXTION_send(buffer, USART_HOLD);
}

void display_wmm()
{
    char buffer[] = "wmm.val=  ";
    memcpy(&buffer[8],&TIMER_formated[TIMER_FORMATEDMM],2);
	NEXTION_send(buffer, USART_HOLD);
}

void display_wss()
{
    char buffer[] = "wss.val=  ";
    memcpy(&buffer[8],&TIMER_formated[TIMER_FORMATEDSS],2);
	NEXTION_send(buffer, USART_HOLD);
}

void display_dbs()
{
    if(!update_dbs)
        return;

    update_dbs = 0;
    char buffer[] = "dbs.val=   ";
    itoa(NEXTION_brightness,&buffer[8],10);
	NEXTION_send(buffer, USART_HOLD);
}

void UIBOARDCONFIG_set_brightness(uint8_t brightness)
{
    NEXTION_set_brightness(brightness);
    update_dbs = 1;
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


