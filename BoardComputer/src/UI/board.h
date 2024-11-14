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
#include "../input.h"

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

void UIBOARD_callback_config();
void UIBOARD_handle_userinput(INPUT_Event* input_event);
void UIBOARD_update();
void UIBOARD_setup();

#endif /* UIBOARD_H_ */
