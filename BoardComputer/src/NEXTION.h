/*
 * NEXTION.h
 *
 * Created: 2019-10-16 01:52:26
 *  Author: pszczelaszkov
 */ 


#ifndef NEXTION_H_
#define NEXTION_H_
#include "inttypes.h"
#include "utils.h"

#define NEXTION_SELECT_DECAY_TICKS 24
#define NEXTION_OBJNAME_LEN 3
enum NEXTION_MD
{
	NEXTION_MD_LPH,
	NEXTION_MD_LP100,
	NEXTION_MD_LP100_AVG,
	NEXTION_MD_SPEED_AVG,
	NEXTION_MD_INJ_T,
	NEXTION_MD_RANGE,
	NEXTION_MD_LAST
};
typedef enum NEXTION_COMPONENTSTATUS
{
	NEXTION_COMPONENTSTATUS_DEFAULT,
	NEXTION_COMPONENTSTATUS_SELECTED
	
}NEXTION_Componentstatus_t;

enum NEXTION_COMPONENT
{
	NEXTION_COMPONENT_WATCH,
	NEXTION_COMPONENT_WATCHSEL
};

typedef enum NEXTION_COMPONENTTYPE
{
	NEXTION_COMPONENTTYPE_PIC,
	NEXTION_COMPONENTTYPE_TEXT
}NEXTION_Componenttype_t;

#define NEXTION_MAINDISPLAY_RENDERERS_SIZE NEXTION_MD_LAST
#define NEXTION_MD_INITIAL_COMPONENT &NEXTION_maindisplay_renderers[0]
typedef struct NEXTION_Component
{
	uint8_t picID_default;
	uint8_t picID_selected;
	const char* name;
	NEXTION_Componenttype_t type;
}NEXTION_Component;
typedef struct NEXTION_MDComponent
{
	NEXTION_Component parent;

	Callback render;
	struct NEXTION_MDComponent* nextRenderer;
}NEXTION_MDComponent;

extern uint8_t NEXTION_selection_counter;
extern char NEXTION_eot[];
extern NEXTION_Component NEXTION_components[];
extern NEXTION_MDComponent NEXTION_maindisplay_renderers[];
extern NEXTION_MDComponent* NEXTION_maindisplay_renderer;


uint8_t NEXTION_send(char data[], uint8_t flush);
int8_t NEXTION_update();
int8_t NEXTION_switch_page(uint8_t page);
void NEXTION_renderer_md_lph();
void NEXTION_renderer_md_lp100();
void NEXTION_renderer_md_lp100_avg();
void NEXTION_renderer_md_speed_avg();
void NEXTION_renderer_md_inj_t();
void NEXTION_renderer_md_range();
void NEXTION_set_componentstatus(NEXTION_Component* component, NEXTION_Componentstatus_t status);
void NEXTION_switch_maindisplay();
void NEXTION_update_EGT();
void NEXTION_update_ADC();
void NEXTION_update_watch();
void NEXTION_update_select_decay();
void NEXTION_initialize();
#endif /* NEXTION_H_ */