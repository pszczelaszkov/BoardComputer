/*
 * board.h
 *
 * Created: 2021-04-03 21:53:00
 * Author : pszczelaszkov
 */

#ifndef UIBOARD_H_
#define UIBOARD_H_
#include <inttypes.h>
#include "../nextion.h"

TESTUSE enum UIBOARD_MD
{
	UIBOARD_MD_LPH,
	UIBOARD_MD_LP100,
	UIBOARD_MD_LP100_AVG,
	UIBOARD_MD_SPEED_AVG,
	UIBOARD_MD_INJ_T,
	UIBOARD_MD_RANGE,
	UIBOARD_MD_LAST
};

enum UIBOARD_COMPONENT
{
	UIBOARD_COMPONENT_WATCH,
	UIBOARD_COMPONENT_WATCHSEL,
	UIBOARD_COMPONENT_CONFIG
};

#define UIBOARD_MD_INITIAL_COMPONENT &UIBOARD_maindisplay_components[0]

TESTUSE typedef struct UIBOARD_MDComponent
{
	NEXTION_Executable_Component executable_component;
	struct UIBOARD_MDComponent* nextComponent;
}UIBOARD_MDComponent;

TESTUSE extern NEXTION_Component UIBOARD_components[];
TESTUSE extern UIBOARD_MDComponent UIBOARD_maindisplay_components[];
TESTUSE extern UIBOARD_MDComponent* UIBOARD_maindisplay_activecomponent;

void UIBOARD_renderer_md_lph();
void UIBOARD_renderer_md_lp100();
void UIBOARD_renderer_md_lp100_avg();
void UIBOARD_renderer_md_speed_avg();
void UIBOARD_renderer_md_inj_t();
void UIBOARD_renderer_md_range();
void UIBOARD_callback_config();
TESTUSE void UIBOARD_switch_maindisplay();
TESTUSE void UIBOARD_update_EGT();
void UIBOARD_update_ADC();
void UIBOARD_update_watch();
void UIBOARD_update();
void UIBOARD_setup();

#endif /* UIBOARD_H_ */
