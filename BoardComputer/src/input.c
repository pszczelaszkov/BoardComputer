#include "input.h"


INPUT_Component INPUT_components[] = {
	{
		.componentID = INPUT_COMPONENT_MAINDISPLAY,
		.on_click = &INPUT_switch_maindisplay,
		//.nextion_component = NEXTION_maindisplay_renderer
	}
};

INPUT_Keystatus_t INPUT_keystatus[INPUT_KEY_LAST];
INPUT_Component* INPUT_active_component;
uint8_t INPUT_active_page;

void INPUT_switch_maindisplay(){};
//Called from ISR, keep fit
void INPUT_userinput(INPUT_Keystatus_t keystatus, INPUT_Key_t key)
{
	if(keystatus == INPUT_KEYSTATUS_PRESSED)
	{
		if(INPUT_active_component->componentID == INPUT_COMPONENT_WATCH)
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

void INPUT_update()
{
	for(uint8_t i = 0; i < INPUT_KEY_LAST; i++)
	{	
		uint8_t status = INPUT_keystatus[i];
		if(status > INPUT_KEYSTATUS_RELEASED && status < INPUT_KEYSTATUS_HOLD)
			INPUT_keystatus[i]++;
	}
	//click na DOWN zmienia active na nastepny z listy X
	//potrzebne resettery w maindisplayu
	//jak ma wygladac marker?
	//struct elementow typu input?
	/*switch(INPUT_active_component)
	{
		case INPUT_COMPONENT_MAINDISPLAY:
			if(INPUT_keystatus[INPUT_KEY_ENTER] == INPUT_KEYSTATUS_CLICK)
			{
				NEXTION_switch_maindisplay();
				INPUT_keystatus[INPUT_KEY_ENTER] = INPUT_KEYSTATUS_RELEASED;
			}
		break;
		default:
		break;
	}*/
}