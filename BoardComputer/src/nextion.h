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
#define SAFETYYELLOW 0xEE80
#define CRIMSONRED 0xD800

#define WHITE 0XFFFF
#define DEFAULTCOLOR WHITE

static const char str_val[NEXTION_OBJNAME_LEN] = "val";
static const char str_txt[NEXTION_OBJNAME_LEN] = "txt";

#define NEXTION_INSTRUCTION_BUFFER_BLOCK(PAYLOAD_LENGTH)\
	const uint8_t instruction_length = NEXTION_OBJNAME_LEN + 5;\
	const uint8_t payload_length = PAYLOAD_LENGTH;\
	char buffer[instruction_length + payload_length + 1];\
	char* instruction = buffer;\
	char* payload = &buffer[instruction_length];\
	buffer[instruction_length + payload_length] = 0x0;\
	memset(payload,' ',payload_length);

TESTUSE typedef enum NEXTION_PAGEID
{
	NEXTION_PAGEID_INIT = 0,
	NEXTION_PAGEID_BOARD = 1,
	NEXTION_PAGEID_BOARDCONFIG = 2,
	NEXTION_PAGEID_NUMPAD = 3
}NEXTION_PageID_t;
TESTUSE typedef enum NEXTION_COMPONENTSELECTSTATUS
{
	NEXTION_COMPONENTSELECTSTATUS_DEFAULT,
	NEXTION_COMPONENTSELECTSTATUS_SELECTED,
}NEXTION_Component_select_status_t;

TESTUSE typedef enum NEXTION_HIGHLIGHTTYPE
{
	NEXTION_HIGHLIGHTTYPE_IMAGE,
	NEXTION_HIGHLIGHTTYPE_IMAGE2,
	NEXTION_HIGHLIGHTTYPE_CROPPEDIMAGE,
	NEXTION_HIGHLIGHTTYPE_BACKCOLOR,
	NEXTION_HIGHLIGHTTYPE_FRONTCOLOR
}NEXTION_Highlighttype_t;


TESTUSE typedef struct NEXTION_Component
{
	const char* name;
	const uint16_t value_default;
	const uint16_t value_selected;
	const NEXTION_Highlighttype_t highlighttype;
}NEXTION_Component;

TESTUSE typedef struct NEXTION_Executable_Component
{
	NEXTION_Component component;
	Callback execute;
}NEXTION_Executable_Component;

TESTUSE extern uint8_t NEXTION_brightness;
TESTUSE extern NEXTION_Component NEXTION_common_bckcomponent;
TESTUSE extern char NEXTION_eot[];
extern uint8_t NEXTION_selection_counter;
extern Callback_32 NEXTION_handler_requested_data;

void NEXTION_handler_ready();
void NEXTION_handler_sendme(uint8_t pageid);
uint8_t NEXTION_send(char data[], uint8_t flush);
int8_t NEXTION_update();
uint8_t NEXTION_add_brightness(uint8_t value, uint8_t autoreload);
TESTUSE int8_t NEXTION_switch_page(NEXTION_PageID_t pageID, uint8_t push_to_history);

//Place quotes at the beginning and the end of payload buffer for text variables.
inline void NEXTION_quote_payloadbuffer(char* payload,uint8_t payload_length)
{
	payload[0] = '"';
	payload[payload_length-1] = '"';
}

TESTUSE void NEXTION_set_brightness(uint8_t brightness);
TESTUSE void NEXTION_set_component_select_status(NEXTION_Component* component, NEXTION_Component_select_status_t status);
TESTUSE void NEXTION_clear_active_component();
TESTUSE void NEXTION_set_previous_page();
void NEXTION_request_brightness();
void NEXTION_reset();
void NEXTION_instruction_compose(const char* objname, const char* varname, char* instruction);

#endif /* NEXTION_H_ */