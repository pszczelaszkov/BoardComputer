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
#define BRIGHTBLUE 0x4DF
#define BRIGHTBROWN 0xBC8D
#define PASTELORANGE 0xFD88
#define WHITE 0XFFFF

typedef enum NEXTION_PAGEID
{
	NEXTION_PAGEID_INIT = 0,
	NEXTION_PAGEID_BOARD = 1,
	NEXTION_PAGEID_BOARDCONFIG = 2,
	NEXTION_PAGEID_NUMPAD = 3
}NEXTION_PageID_t;
typedef enum NEXTION_COMPONENTSTATUS
{
	NEXTION_COMPONENTSTATUS_DEFAULT,
	NEXTION_COMPONENTSTATUS_SELECTED,
	NEXTION_COMPONENTSTATUS_WARNING
	
}NEXTION_Componentstatus_t;

typedef enum NEXTION_HIGHLIGHTTYPE
{
	NEXTION_HIGHLIGHTTYPE_IMAGE,
	NEXTION_HIGHLIGHTTYPE_IMAGE2,
	NEXTION_HIGHLIGHTTYPE_CROPPEDIMAGE,
	NEXTION_HIGHLIGHTTYPE_BACKCOLOR,
	NEXTION_HIGHLIGHTTYPE_FRONTCOLOR
}NEXTION_Highlighttype_t;

typedef struct NEXTION_Component
{
	const char* name;
	uint16_t value_default;
	uint16_t value_selected;
	NEXTION_Highlighttype_t highlighttype;
}NEXTION_Component;

typedef struct NEXTION_Executable_Component
{
	NEXTION_Component component;
	Callback execute;
}NEXTION_Executable_Component;

extern uint8_t NEXTION_brightness;
extern uint8_t NEXTION_selection_counter;
extern char NEXTION_eot[];
extern Callback_32 NEXTION_requested_data_handler;
extern NEXTION_Component NEXTION_common_bckcomponent;
uint8_t NEXTION_send(char data[], uint8_t flush);
int8_t NEXTION_update();
int8_t NEXTION_switch_page(NEXTION_PageID_t pageID, uint8_t push_to_history);
uint8_t NEXTION_add_brightness(uint8_t value, uint8_t autoreload);

void NEXTION_request_brightness();
void NEXTION_set_brightness(uint8_t brightness);
void NEXTION_set_previous_page();
void NEXTION_set_componentstatus(NEXTION_Component* component, NEXTION_Componentstatus_t status);
void NEXTION_update_select_decay();


#endif /* NEXTION_H_ */