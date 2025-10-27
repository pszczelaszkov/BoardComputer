#include "input.h"
#include "UI/board.h"
#include "UI/numpad.h"
#include "bitwise.h"

static uint8_t pending_componentID;
INPUT_Keystatus_t INPUT_keystatus[INPUT_KEY_LAST];
INPUT_Userinput_Handler INPUT_userinput_handler = NULL;

static volatile INPUT_Event input_event;

//Called from ISR, keep fit
void INPUT_userinput(INPUT_Keystatus_t keystatus, INPUT_Key_t key, INPUT_ComponentID_t componentID)
{	
	if(keystatus == INPUT_KEYSTATUS_PRESSED)
	{
	}
	else//if keystatus released
	{
		if(INPUT_keystatus[key] > INPUT_KEYSTATUS_RELEASED && INPUT_keystatus[key] < INPUT_KEYSTATUS_HOLD)
		{
			keystatus = INPUT_KEYSTATUS_CLICK;
			if(SYSTEM_config.SYSTEM_BEEP_ON_CLICK)
			{
				SYSTEM_trigger_short_beep();
			}
		}
	}

	INPUT_keystatus[key] = keystatus;
	if(!input_event.next_handler)
	{
		input_event.timestamp = SYSTEM_get_cycle_timestamp();
		input_event.componentID = componentID;
		input_event.key = key;
		input_event.keystatus = keystatus;
		input_event.next_handler = INPUT_userinput_handler;
	}
}

void INPUT_update()
{
	INPUT_Userinput_Handler handler = (&input_event)->next_handler;

	while(handler)
	{
		handler((INPUT_Event*)&input_event);
		if(handler == (&input_event)->next_handler)
		{
			(&input_event)->next_handler = NULL;
		}
		else
		{
			handler = (&input_event)->next_handler;
		}
		
	}

	for(uint8_t i = 0; i < INPUT_KEY_LAST; i++)
	{	
		uint8_t status = INPUT_keystatus[i];
		if(status > INPUT_KEYSTATUS_RELEASED && status < INPUT_KEYSTATUS_HOLD){
			INPUT_keystatus[i]++;
		}
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