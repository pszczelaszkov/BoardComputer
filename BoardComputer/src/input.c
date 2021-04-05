#include "input.h"
#include "UI/board.h"
#include "bitwise.h"

//Map all possible input components
INPUT_Component INPUT_components[] = {
	{
		.componentID = INPUT_COMPONENT_MAINDISPLAY,
		.nextcomponentID = INPUT_COMPONENT_WATCH,
		.on_click = INPUT_switch_maindisplay,
		.nextion_component = (NEXTION_Component*)UIBOARD_MD_INITIAL_COMPONENT
	},
	{
		.componentID = INPUT_COMPONENT_WATCH,
		.nextcomponentID = INPUT_COMPONENT_WATCHSEL,
		.on_hold = TIMER_watch_zero,
		.nextion_component = &UIBOARD_components[UIBOARD_COMPONENT_WATCH]
	},
	{
		.componentID = INPUT_COMPONENT_WATCHSEL,
		.nextcomponentID = INPUT_COMPONENT_MAINDISPLAY,
		.on_click = TIMER_next_watch,
		.nextion_component = &UIBOARD_components[UIBOARD_COMPONENT_WATCHSEL]
	}
};

static const uint8_t components_count = sizeof(INPUT_components)/sizeof(INPUT_Component);
static uint8_t activity_counter;
static uint8_t pending_componentID;
INPUT_Keystatus_t INPUT_keystatus[INPUT_KEY_LAST];
INPUT_Component* INPUT_active_component = &INPUT_components[0];
uint8_t INPUT_active_page;

void INPUT_switch_maindisplay()
{
	UIBOARD_switch_maindisplay();
	INPUT_active_component->nextion_component = (NEXTION_Component*)UIBOARD_maindisplay_activecomponent;
	NEXTION_set_componentstatus(INPUT_active_component->nextion_component, NEXTION_COMPONENTSTATUS_SELECTED);
};

//Called from ISR, keep fit
void INPUT_userinput(INPUT_Keystatus_t keystatus, INPUT_Key_t key, INPUT_ComponentID_t componentID)
{	
	if(componentID)
		pending_componentID = componentID;
	else
		componentID = INPUT_active_component->componentID;

	if(keystatus == INPUT_KEYSTATUS_PRESSED)
	{
		if(componentID == INPUT_COMPONENT_WATCH && key == INPUT_KEY_ENTER)
		{	
			if(TIMER_active_watch == &TIMER_watches[TIMERTYPE_STOPWATCH])
				TIMER_watch_toggle(TIMER_active_watch);
		}
	}
	else
	{
		if(INPUT_keystatus[key] > INPUT_KEYSTATUS_RELEASED && INPUT_keystatus[key] < INPUT_KEYSTATUS_HOLD)
			keystatus = INPUT_KEYSTATUS_CLICK;
	}
	INPUT_keystatus[key] = keystatus;
}

INPUT_Component* INPUT_findcomponent(uint8_t componentID)
{
	uint8_t left = 0;
	uint8_t right = components_count - 1;
    uint8_t middle;

    while(right - left > 1 ) 
    { 
        middle = left + (right-left)/2; 
  
        if(INPUT_components[middle].componentID <= componentID ) 
            left = middle; 
        else
            right = middle; 
    } 
  
	
    if(INPUT_components[left].componentID == componentID) 
        return &INPUT_components[left]; 
    if(INPUT_components[right].componentID == componentID) 
        return &INPUT_components[right]; 
    else
        return 0; 
}


void INPUT_update()
{
	for(uint8_t i = 0; i < INPUT_KEY_LAST; i++)
	{	
		uint8_t status = INPUT_keystatus[i];
		if(status > INPUT_KEYSTATUS_RELEASED && status < INPUT_KEYSTATUS_HOLD)
		{
			INPUT_keystatus[i]++;
		}
	}

	if(INPUT_keystatus[INPUT_KEY_ENTER] || INPUT_keystatus[INPUT_KEY_DOWN] == INPUT_KEYSTATUS_CLICK)
	{
		INPUT_Component* nextcomponent = getnextcomponent();
		if(nextcomponent)
		{
			//Try to deselect current component if active
			NEXTION_set_componentstatus(INPUT_active_component->nextion_component, NEXTION_COMPONENTSTATUS_DEFAULT);
			INPUT_active_component = nextcomponent;
		}
		NEXTION_set_componentstatus(INPUT_active_component->nextion_component, NEXTION_COMPONENTSTATUS_SELECTED);
	}

	switch(INPUT_keystatus[INPUT_KEY_ENTER])
	{	
		Callback callback;
		case INPUT_KEYSTATUS_CLICK:
			callback = INPUT_active_component->on_click;
			if(callback)
				callback();
			INPUT_keystatus[INPUT_KEY_ENTER] = INPUT_KEYSTATUS_RELEASED;
		break;
		case INPUT_KEYSTATUS_HOLD:
			callback = INPUT_active_component->on_hold;
			if(callback)
				callback();
		break;
	}
}

void INPUT_initialize()
{	
	#ifdef __AVR__
	CLEAR(PORTD,BIT2|BIT3);
	CLEAR(DDRD,BIT2|BIT3);
	EICRA = (1 << ISC00) | (1 << ISC10);//Any logical change
	EIMSK = 3;//Enable INT0&1
	#endif
}

INPUT_Component* getnextcomponent()
{
	if(INPUT_keystatus[INPUT_KEY_DOWN] == INPUT_KEYSTATUS_CLICK)
	{
		if(NEXTION_selection_counter)
			pending_componentID = INPUT_active_component->nextcomponentID;

		INPUT_keystatus[INPUT_KEY_DOWN] = INPUT_KEYSTATUS_RELEASED;
	}
	if(pending_componentID)
	{
		INPUT_Component* component = INPUT_findcomponent(pending_componentID);
		pending_componentID = 0;
		if(component && component != INPUT_active_component)
		{
			return component;
		}
	}
	return 0;
}

ISR(INT0_vect)
{
	uint8_t keystatus = !READ(PIND,BIT2);
	INPUT_userinput((INPUT_Keystatus_t)keystatus,INPUT_KEY_ENTER,INPUT_COMPONENT_NONE);
}

ISR(INT1_vect)
{
	uint8_t keystatus = !READ(PIND,BIT3);
	INPUT_userinput((INPUT_Keystatus_t)keystatus,INPUT_KEY_DOWN,INPUT_COMPONENT_NONE);
}