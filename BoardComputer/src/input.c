#include "input.h"


INPUT_Component INPUT_components[] = {
	{
		.componentID = INPUT_COMPONENT_MAINDISPLAY,
		.on_click = INPUT_switch_maindisplay,
		.nextion_component = (NEXTION_Component*)NEXTION_MD_INITIAL_COMPONENT
	},
	{
		.componentID = INPUT_COMPONENT_WATCH,
		.on_click = TIMER_watch_toggle,
		.on_hold = TIMER_watch_zero,
		.nextion_component = &NEXTION_components[NEXTION_COMPONENT_WATCH]
	},
	{
		.componentID = INPUT_COMPONENT_WATCHSEL,
		.on_click = TIMER_next_watch,
		.nextion_component = &NEXTION_components[NEXTION_COMPONENT_WATCHSEL]
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
	NEXTION_switch_maindisplay();
	INPUT_active_component->nextion_component = (NEXTION_Component*)NEXTION_maindisplay_renderer;
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
		if(componentID == INPUT_COMPONENT_WATCH)
		{	
			if(TIMER_active_watch == &TIMER_watches[TIMERTYPE_STOPWATCH] && key == INPUT_KEY_ENTER)
				TIMER_watch_toggle(TIMER_active_watch);
		}
	}
	else
	{
		if(INPUT_keystatus[key] > INPUT_KEYSTATUS_RELEASED)
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

INPUT_Component* getnextcomponent()
{
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

void INPUT_update()
{	
	for(uint8_t i = 0; i < INPUT_KEY_LAST; i++)
	{	
		uint8_t status = INPUT_keystatus[i];
		if(status > INPUT_KEYSTATUS_RELEASED && status < INPUT_KEYSTATUS_HOLD)
			INPUT_keystatus[i]++;
	}

	INPUT_Component* nextcomponent = getnextcomponent();
	if(nextcomponent)
	{
		NEXTION_set_componentstatus((NEXTION_Component*)INPUT_active_component->nextion_component, NEXTION_COMPONENTSTATUS_DEFAULT);
		INPUT_active_component = nextcomponent;
	}
	switch(INPUT_keystatus[INPUT_KEY_ENTER])
	{	
		Callback callback;
		case INPUT_KEYSTATUS_CLICK:
			callback = INPUT_active_component->on_click;
			if(callback)
				callback();
			INPUT_keystatus[INPUT_KEY_ENTER] = INPUT_KEYSTATUS_RELEASED;
			NEXTION_set_componentstatus(INPUT_active_component->nextion_component, NEXTION_COMPONENTSTATUS_SELECTED);
		break;
		case INPUT_KEYSTATUS_HOLD:
			callback = INPUT_active_component->on_hold;
			if(callback)
				callback();
			NEXTION_set_componentstatus(INPUT_active_component->nextion_component, NEXTION_COMPONENTSTATUS_SELECTED);
		break;

	}
}