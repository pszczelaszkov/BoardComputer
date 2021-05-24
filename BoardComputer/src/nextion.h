/*
 * NEXTION.h
 *
 * Created: 2019-10-16 01:52:26
 *  Author: pszczelaszkov
 */ 


#ifndef NEXTION_H_
#define NEXTION_H_
#include <inttypes.h>
#include "utils.h"

#define NEXTION_SELECT_DECAY_TICKS 24
#define NEXTION_OBJNAME_LEN 3

enum NEXTION_PAGE
{
	NEXTION_PAGE_init = 0,
	NEXTION_PAGE_BOARD = 1,
	NEXTION_PAGE_BOARDCONFIG = 2
};
typedef enum NEXTION_COMPONENTSTATUS
{
	NEXTION_COMPONENTSTATUS_DEFAULT,
	NEXTION_COMPONENTSTATUS_SELECTED
	
}NEXTION_Componentstatus_t;

typedef enum NEXTION_COMPONENTTYPE
{
	NEXTION_COMPONENTTYPE_PIC,
	NEXTION_COMPONENTTYPE_TEXTFIELD,
	NEXTION_COMPONENTTYPE_TEXT,
	NEXTION_COMPONENTTYPE_SLIDER
}NEXTION_Componenttype_t;

typedef struct NEXTION_Component
{
	uint8_t picID_default;
	uint8_t picID_selected;
	const char* name;
	NEXTION_Componenttype_t type;
}NEXTION_Component;

typedef struct NEXTION_Executable_Component
{
	NEXTION_Component component;
	Callback execute;
}NEXTION_Executable_Component;

extern int8_t NEXTION_brightness;
extern uint8_t NEXTION_selection_counter;
extern char NEXTION_eot[];
extern Callback_32 NEXTION_requested_data_handler;

uint8_t NEXTION_send(char data[], uint8_t flush);
int8_t NEXTION_update();
int8_t NEXTION_switch_page(uint8_t page);

void NEXTION_request_brightness();
void NEXTION_set_brightness(uint8_t brightness);
void NEXTION_set_componentstatus(NEXTION_Component* component, NEXTION_Componentstatus_t status);
void NEXTION_update_select_decay();

#endif /* NEXTION_H_ */