/*
 * boardconfig.h
 *
 * Created: 2021-04-10 01:24:00
 * Author : pszczelaszkov
 */
#ifndef UIBOARDCONFIG_H_
#define UIBOARDCONFIG_H_
#include "../nextion.h"

enum UIBOARDCONFIG_COMPONENT
{
	UIBOARDCONFIG_COMPONENT_IPM,
	UIBOARDCONFIG_COMPONENT_CCM,
    UIBOARDCONFIG_COMPONENT_WHH,
    UIBOARDCONFIG_COMPONENT_WMM,
    UIBOARDCONFIG_COMPONENT_WSS,
    UIBOARDCONFIG_COMPONENT_DBS,
    UIBOARDCONFIG_COMPONENT_LAST
};

extern NEXTION_Executable_Component UIBOARDCONFIG_executable_components[];
void UIBOARDCONFIG_update();
void UIBOARDCONFIG_set_brightness(uint8_t brightness);
#endif