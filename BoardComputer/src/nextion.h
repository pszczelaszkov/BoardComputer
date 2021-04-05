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
//wskaznik na aktualne componenty potrzebny?
typedef enum NEXTION_COMPONENTSTATUS
{
	NEXTION_COMPONENTSTATUS_DEFAULT,
	NEXTION_COMPONENTSTATUS_SELECTED
	
}NEXTION_Componentstatus_t;

typedef enum NEXTION_COMPONENTTYPE
{
	NEXTION_COMPONENTTYPE_PIC,
	NEXTION_COMPONENTTYPE_TEXT
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

//tutaj musi byc exec component?
extern uint8_t NEXTION_selection_counter;
extern char NEXTION_eot[];

uint8_t NEXTION_send(char data[], uint8_t flush);
int8_t NEXTION_update();
int8_t NEXTION_switch_page(uint8_t page);
void NEXTION_set_componentstatus(NEXTION_Component* component, NEXTION_Componentstatus_t status);
void NEXTION_update_select_decay();

#endif /* NEXTION_H_ */