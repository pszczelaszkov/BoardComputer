#include "board.h"
#include "../sensorsfeed.h"
#include "timer.h"
#include "../USART.h"
#include "../system.h"

typedef enum INPUTCOMPONENTID
{
	INPUTCOMPONENT_NONE = 0,
	INPUTCOMPONENT_MAINDISPLAY = 1,
	INPUTCOMPONENT_WATCH = 4,
	INPUTCOMPONENT_WATCHSEL = 5,
	INPUTCOMPONENT_CONFIG = 6,
}InputComponentID_t;

TESTUSE typedef enum VISUALALERTSEVERITY
{
	VISUALALERT_SEVERITY_NOTIFICATION,
	VISUALALERT_SEVERITY_WARNING,
	VISUALALERT_SEVERITY_BADVALUE
}Visualalertseverity_t;

TESTUSE typedef enum VISUALALERTID
{
	VISUALALERTID_MAINDISPLAY,
	VISUALALERTID_EGT,
	VISUALALERTID_MAP,
	VISUALALERTID_FRP,
	VISUALALERTID_INTAKE,
	VISUALALERTID_OUTSIDE,
	VISUALALERTID_OIL,
	VISUALALERTID_LAST
}Visualalertid_t;

TESTUSE typedef struct Visualalert
{
	const char* objname;
	uint8_t temppattern;
	uint8_t pattern;
	uint16_t color;
}Visualalert;

static const int16_t fuelmanifold_threshold = 2 << 8;//2 Bar
static const uint16_t MD_MAX_VALUE = 0x63e7;//99.9

static uint8_t critical_raised;
TESTSTATICVAR(static uint8_t raise_critical);

static void renderer_md_lph();
static void renderer_md_lp100();
static void renderer_md_lp100_avg();
static void renderer_md_speed_avg();
static void renderer_md_inj_t();
static void renderer_md_range();
TESTUSE static Visualalert TESTADDPREFIX(visualalerts)[];
TESTUSE static void TESTADDPREFIX(update_EGT)();
TESTUSE static void TESTADDPREFIX(update_watch)();
TESTUSE static void TESTADDPREFIX(raisevisualalert)(Visualalertid_t alertid, Visualalertseverity_t severity);
TESTUSE static void TESTADDPREFIX(update_visual_alert)();
TESTUSE static void TESTADDPREFIX(update_sensorgroup_bottom)();
TESTUSE static void TESTADDPREFIX(update_sensorgroup_pressure)();


//Object needs to have "pco" variable, as its used to visualize alert.
static Visualalert visualalerts[] = 
{
	[VISUALALERTID_MAINDISPLAY] = {
		.objname = "mdv"
	},
	[VISUALALERTID_EGT] = {
		.objname = "egt"
	},
	[VISUALALERTID_MAP] = {
		.objname = "map"
	},
	[VISUALALERTID_FRP] = {
		.objname = "frp"
	},
	[VISUALALERTID_OIL] = {
		.objname = "oil"
	},
	[VISUALALERTID_INTAKE] = {
		.objname = "int" 
	},
	[VISUALALERTID_OUTSIDE] = {
		.objname = "out"
	}
};

NEXTION_Component UIBOARD_components[] = {
	[UIBOARD_COMPONENT_WATCH]=
	{
		.highlighttype = NEXTION_HIGHLIGHTTYPE_CROPPEDIMAGE,
		.value_default = 1,
		.value_selected = 25,
		.name = "wtd"
	},
	[UIBOARD_COMPONENT_WATCHSEL]=
	{
		.highlighttype = NEXTION_HIGHLIGHTTYPE_IMAGE,
		.value_default = 2,
		.value_selected = 17,
		.name = "wts"
	},
	[UIBOARD_COMPONENT_CONFIG]=
	{
		.highlighttype = NEXTION_HIGHLIGHTTYPE_IMAGE,
		.value_default = 3,
		.value_selected = 18,
		.name = "cfg"
	}
};

UIBOARD_MDComponent* UIBOARD_maindisplay_activecomponent = UIBOARD_MD_INITIAL_COMPONENT;
UIBOARD_MDComponent UIBOARD_maindisplay_components[] = {
	[UIBOARD_MD_LPH]=
	{
		.executable_component = 
		{
			.component = 			
			{
				.value_default = 11,
				.value_selected = 22,
				.name = "mds",
				.highlighttype = NEXTION_HIGHLIGHTTYPE_IMAGE
			},
			.execute = renderer_md_lph,
		},
		.nextComponent = &UIBOARD_maindisplay_components[UIBOARD_MD_LP100_AVG]
	},
	[UIBOARD_MD_LP100]=
	{
		.executable_component = 
		{
			.component = 			
			{
				.value_default = 12,
				.value_selected = 23,
				.name = "mds",
				.highlighttype = NEXTION_HIGHLIGHTTYPE_IMAGE
			},
			.execute = renderer_md_lp100
		},
		.nextComponent = &UIBOARD_maindisplay_components[UIBOARD_MD_LP100_AVG]
	},
	[UIBOARD_MD_LP100_AVG]=
	{
		.executable_component = 
		{
			.component = 		
			{
				.value_default = 13,
				.value_selected = 24,
				.name = "mds",
				.highlighttype = NEXTION_HIGHLIGHTTYPE_IMAGE
			},
			.execute = renderer_md_lp100_avg
		},
		.nextComponent = &UIBOARD_maindisplay_components[UIBOARD_MD_SPEED_AVG]
	},
	[UIBOARD_MD_SPEED_AVG]=
	{
		.executable_component = 
		{
			.component =  			
			{
				.value_default = 14,
				.value_selected = 19,
				.name = "mds",
				.highlighttype = NEXTION_HIGHLIGHTTYPE_IMAGE
			},
			.execute = renderer_md_speed_avg
		},
		.nextComponent = &UIBOARD_maindisplay_components[UIBOARD_MD_INJ_T]
	},
	[UIBOARD_MD_INJ_T]=
	{
		.executable_component = 
		{
			.component = 			
			{
				.value_default = 15,
				.value_selected = 20,
				.name = "mds",
				.highlighttype = NEXTION_HIGHLIGHTTYPE_IMAGE
			},
			.execute = renderer_md_inj_t
		},
		.nextComponent = &UIBOARD_maindisplay_components[UIBOARD_MD_RANGE]
	},
	[UIBOARD_MD_RANGE]=
	{
		.executable_component = 
		{
			.component = 			
			{
				.value_default = 16,
				.value_selected = 21,
				.name = "mds",
				.highlighttype = NEXTION_HIGHLIGHTTYPE_IMAGE
			},
			.execute = renderer_md_range
		},
		.nextComponent = &UIBOARD_maindisplay_components[UIBOARD_MD_LPH]
	}
};

/*
Raises visual alert for specified alertid.
Alert lives 8 system cycles.Its safe to double rise.
It may raise corresponding system alert.
@param alertid Selects Visualalert instance.
@param severity Severity sets predefined periodicity and color.
*/
static void raisevisualalert(Visualalertid_t alertid, Visualalertseverity_t severity)
{
	Visualalert* alert = &visualalerts[alertid];
	uint8_t pattern;
	uint16_t color;
	switch(severity)
	{
		case VISUALALERT_SEVERITY_BADVALUE:
			pattern = 0xff;
			color = CRIMSONRED;
		break;
		case VISUALALERT_SEVERITY_NOTIFICATION:
			pattern = 0xcc;
			color = BRIGHTBLUE;
			SYSTEM_raisealert(SYSTEM_ALERT_NOTIFICATION);
		break;
		case VISUALALERT_SEVERITY_WARNING:
			pattern = 0xaa;
			color = SAFETYYELLOW;
			SYSTEM_raisealert(SYSTEM_ALERT_WARNING);
		break;
	}
	alert->color = color;
	alert->pattern = pattern;
}
/*Display liters per hour*/
static void renderer_md_lph()
{	NEXTION_INSTRUCTION_BUFFER_BLOCK(6)
	NEXTION_instruction_compose("mdv","txt",instruction);
	NEXTION_quote_payloadbuffer(payload,payload_length);

	uint16_t speed = SENSORSFEED_feed[SENSORSFEED_FEEDID_SPEED];
	uint16_t lph = SENSORSFEED_feed[SENSORSFEED_FEEDID_LPH];

	if(speed)
		UIBOARD_maindisplay_activecomponent = &UIBOARD_maindisplay_components[UIBOARD_MD_LP100];

	lph = MIN(lph,MD_MAX_VALUE);
	fp16toa(lph,&buffer[9],2,1);
	NEXTION_send(buffer, USART_HOLD);
}
/*Display realtime liters per 100km, if not in motion fallback to liters per hour*/
static void renderer_md_lp100()
{	
	NEXTION_INSTRUCTION_BUFFER_BLOCK(6)
	NEXTION_instruction_compose("mdv","txt",instruction);
	NEXTION_quote_payloadbuffer(payload,payload_length);

	uint16_t lp100 = SENSORSFEED_feed[SENSORSFEED_FEEDID_LP100];
	uint16_t speed = SENSORSFEED_feed[SENSORSFEED_FEEDID_SPEED];

	if(!speed)
	{
		UIBOARD_maindisplay_activecomponent = &UIBOARD_maindisplay_components[UIBOARD_MD_LPH];
		return;
	}
	lp100 = MIN(lp100,MD_MAX_VALUE);
	fp16toa(lp100,&buffer[9],2,1);
	NEXTION_send(buffer, USART_HOLD);
}
/*Display average liters per 100km*/
static void renderer_md_lp100_avg()
{
	NEXTION_INSTRUCTION_BUFFER_BLOCK(6)
	NEXTION_instruction_compose("mdv","txt",instruction);
	NEXTION_quote_payloadbuffer(payload,payload_length);
	uint16_t lp100 = SENSORSFEED_feed[SENSORSFEED_FEEDID_LP100_AVG];

	lp100 = MIN(lp100,MD_MAX_VALUE);
	fp16toa(lp100,&buffer[9],2,1);
	NEXTION_send(buffer, USART_HOLD);
}

/*Display average speed*/
static void renderer_md_speed_avg()
{
	NEXTION_INSTRUCTION_BUFFER_BLOCK(6)
	NEXTION_instruction_compose("mdv","txt",instruction);
	NEXTION_quote_payloadbuffer(payload,payload_length);
	uint16_t speed = SENSORSFEED_feed[SENSORSFEED_FEEDID_SPEED_AVG] >> 8;
	rightconcat_short(&payload[1], speed, 4);
	NEXTION_send(buffer, USART_HOLD);
}


/*Display current injector open time */
static void renderer_md_inj_t()
{
	NEXTION_INSTRUCTION_BUFFER_BLOCK(6)
	NEXTION_instruction_compose("mdv","txt",instruction);
	NEXTION_quote_payloadbuffer(payload,payload_length);
	uint16_t fuel_time = SENSORSFEED_feed[SENSORSFEED_FEEDID_INJT];

	fuel_time = MIN(fuel_time,MD_MAX_VALUE);
	fp16toa(fuel_time,&buffer[9],2,1);
	NEXTION_send(buffer, USART_HOLD);
}



static void renderer_md_range()
{
	NEXTION_INSTRUCTION_BUFFER_BLOCK(6)
	NEXTION_instruction_compose("mdv","txt",instruction);
	NEXTION_quote_payloadbuffer(payload,payload_length);
	uint8_t tank = SENSORSFEED_feed[SENSORSFEED_FEEDID_TANK];
	uint8_t lp100 = SENSORSFEED_feed[SENSORSFEED_FEEDID_LP100_AVG] >> 8;
	uint16_t range = 0;

	if(lp100)
		range = tank*100/lp100;

	range = MIN(range,9999);
	rightconcat_short(&payload[1], range, 4);
	NEXTION_send(buffer, USART_HOLD);
}

static void update_EGT()
{
	NEXTION_INSTRUCTION_BUFFER_BLOCK(6)
	NEXTION_instruction_compose("egt","txt",instruction);
	NEXTION_quote_payloadbuffer(payload,payload_length);
	uint8_t alert = 0;
	switch(SENSORSFEED_EGT_status)
	{
		case SENSORSFEED_EGT_STATUS_UNKN:
			memcpy(&payload[1],"----", 4);
			alert = 1;
		break;
		case SENSORSFEED_EGT_STATUS_OPEN:
			memcpy(&payload[1],"open", 4);
			alert = 1;
		break;
		case SENSORSFEED_EGT_STATUS_VALUE:
			rightconcat_short(&payload[1],SENSORSFEED_feed[SENSORSFEED_FEEDID_EGT], 4);
	}
	NEXTION_send(buffer,USART_HOLD);
	if(alert)
	{
		raise_critical = 1;
		raisevisualalert(VISUALALERTID_EGT,VISUALALERT_SEVERITY_BADVALUE);
	}
}

static void update_sensorgroup_bottom()
{
	NEXTION_INSTRUCTION_BUFFER_BLOCK(5)
	NEXTION_instruction_compose("out","txt",instruction);
	NEXTION_quote_payloadbuffer(payload,payload_length);

	const uint8_t num_of_digits = 3;
	int16_t out_temp = SENSORSFEED_feed[SENSORSFEED_FEEDID_OUTTEMP];
	int16_t int_temp = SENSORSFEED_feed[SENSORSFEED_FEEDID_INTAKETEMP];
	int16_t oil_temp = SENSORSFEED_feed[SENSORSFEED_FEEDID_OILTEMP];

	if(out_temp)
	{
		rightconcat_short(&payload[1],out_temp, num_of_digits);
	}
	else
	{
		raisevisualalert(VISUALALERTID_OUTSIDE,VISUALALERT_SEVERITY_BADVALUE);
		memset(&payload[1], '-', num_of_digits);
		raise_critical = 1;
	}
	NEXTION_send(buffer,USART_HOLD);
	memset(&payload[1],' ',num_of_digits);

	memcpy(buffer,"int",3);
	if(int_temp)
	{
		rightconcat_short(&payload[1],int_temp,num_of_digits);
	}
	else
	{
		raisevisualalert(VISUALALERTID_INTAKE,VISUALALERT_SEVERITY_BADVALUE);
		memset(&payload[1], '-', num_of_digits);
		raise_critical = 1;
	}
	NEXTION_send(buffer,USART_HOLD);
	memset(&payload[1],' ', num_of_digits);

	memcpy(buffer,"oil", 3);
	if(oil_temp)
	{
		rightconcat_short(&payload[1],oil_temp, num_of_digits);
	}
	else
	{
		raisevisualalert(VISUALALERTID_OIL,VISUALALERT_SEVERITY_BADVALUE);
		memset(&payload[1], '-', num_of_digits);
		raise_critical = 1;
	}
	NEXTION_send(buffer,USART_HOLD);
}

static void update_sensorgroup_pressure()
{
	NEXTION_INSTRUCTION_BUFFER_BLOCK(7)
	NEXTION_instruction_compose("map","txt",instruction);
	NEXTION_quote_payloadbuffer(payload,payload_length);

	int16_t manifoldpressure = SENSORSFEED_feed[SENSORSFEED_FEEDID_MAP];
	int16_t fuelrailpressure = SENSORSFEED_feed[SENSORSFEED_FEEDID_FRP];

	if(manifoldpressure)
	{
		fp16toa(manifoldpressure,&payload[1],2,2);
	}
	else
	{
		raisevisualalert(VISUALALERTID_MAP,VISUALALERT_SEVERITY_BADVALUE);
		memset(&payload[1], '-', 5);
		raise_critical = 1;
	}
	NEXTION_send(buffer,USART_HOLD);
	memset(&payload[1],' ',payload_length-2);
	memcpy(buffer,"frp",3);
	///
	if(fuelrailpressure)
	{
		fp16toa(fuelrailpressure,&payload[1],2,2);
	}
	else
	{
		raisevisualalert(VISUALALERTID_FRP,VISUALALERT_SEVERITY_BADVALUE);
		memset(&payload[1], '-', 5);
		raise_critical = 1;
	}
	NEXTION_send(buffer,USART_HOLD);
	memset(payload,' ',payload_length);
	NEXTION_instruction_compose("fmd","var",instruction);
	///
	int16_t deltapressure = MIN(MAX(0, fuelrailpressure - manifoldpressure - fuelmanifold_threshold),0x100);
	//Delta has resolution of 1Bar, only fraction part is used
	//Multiply by 100 to shift 2 fraction positions into integer part
	//Then unpack value with a 8 times shift
	deltapressure = deltapressure * 100 >> 8;
	itoa(deltapressure, payload, 10);+
	
	NEXTION_send(buffer,USART_HOLD);
}

static void update_visual_alert()
{
	NEXTION_INSTRUCTION_BUFFER_BLOCK(5)
	for(uint8_t i = 0; i < VISUALALERTID_LAST;i++)
	{
		Visualalert* alert = &visualalerts[i];
		if(!alert->temppattern)
		{
			alert->temppattern = alert->pattern;
			alert->pattern = 0;
		}
		if(alert->temppattern)
		{
			uint16_t color = DEFAULTCOLOR;
			uint8_t patternmatch = alert->temppattern & 0x01;
			alert->temppattern >>= 1;
			if(patternmatch)
			{
					color = alert->color;
			}
			NEXTION_instruction_compose(alert->objname,"pco",instruction);
			uitoa(color,payload);
			NEXTION_send(buffer,USART_HOLD);
		}
	}
}

static void update_watch()
{
	NEXTION_INSTRUCTION_BUFFER_BLOCK(10)
	NEXTION_instruction_compose("wtd","txt",instruction);
	NEXTION_quote_payloadbuffer(payload,payload_length);
	if(TIMER_active_timertype == TIMER_TIMERTYPE_WATCH)
		memcpy(&payload[3],TIMER_formated,5);
	else
		memcpy(&payload[1],&TIMER_formated[3],8);

	NEXTION_send(buffer,USART_HOLD);
}

static void switch_page_to_config()
{
	NEXTION_switch_page(NEXTION_PAGEID_BOARDCONFIG, 1);
}

static void switch_maindisplay()
{
	UIBOARD_maindisplay_activecomponent = UIBOARD_maindisplay_activecomponent->nextComponent;
}
/*---STATIC END---*/


void UIBOARD_setup()
{
	//INPUT_active_component = INPUT_findcomponent(INPUT_COMPONENT_MAINDISPLAY);
	SENSORSFEED_update();
}

void UIBOARD_callback_config()
{
	NEXTION_switch_page(NEXTION_PAGEID_BOARDCONFIG, 1);
}

void UIBOARD_handle_userinput(INPUT_Event* input_event)
{
	static uint8_t input_order_it = 0;
	static const InputComponentID_t input_order[] = {
		INPUTCOMPONENT_MAINDISPLAY,
		INPUTCOMPONENT_WATCHSEL,
		INPUTCOMPONENT_CONFIG,
		INPUTCOMPONENT_WATCH,
	};

	Callback on_press = NULL;
	Callback on_hold = NULL;
	Callback on_click = NULL;
	NEXTION_Component* component = NULL;
	InputComponentID_t componentID = input_event->componentID;

	if(input_event->key == INPUT_KEY_DOWN && input_event->keystatus == INPUT_KEYSTATUS_CLICK){
		input_order_it++;
		if(input_order_it >= sizeof(input_order)/sizeof(InputComponentID_t)){
			input_order_it = 0;
		}
	}

	if(INPUT_COMPONENT_NONE == componentID)
	{
		componentID = input_order[input_order_it];
	}
	
	switch(componentID)
	{
		case INPUTCOMPONENT_MAINDISPLAY:
			component = (NEXTION_Component*)UIBOARD_maindisplay_activecomponent;
			on_click = switch_maindisplay;
		break;
		case INPUTCOMPONENT_WATCHSEL:
			component = &UIBOARD_components[UIBOARD_COMPONENT_WATCHSEL];
			on_hold = TIMER_next_watch;
		break;
		case INPUTCOMPONENT_WATCH:
			component = &UIBOARD_components[UIBOARD_COMPONENT_WATCH];
			TIMER_userinput_handle_watch(input_event);
		break;
		case INPUTCOMPONENT_CONFIG:
			component = &UIBOARD_components[UIBOARD_COMPONENT_CONFIG];
			on_click = switch_page_to_config;
		break;
	}

	NEXTION_set_component_select_status(component, NEXTION_COMPONENTSELECTSTATUS_SELECTED);

	if(input_event->key == INPUT_KEY_ENTER)
	{
		if(input_event->keystatus == INPUT_KEYSTATUS_CLICK)
			on_click();
		else if(input_event->keystatus == INPUT_KEYSTATUS_HOLD)
			on_hold();
		else if(input_event->keystatus == INPUT_KEYSTATUS_PRESSED)
			on_press();
	}
}

void UIBOARD_update()
{	
	uint8_t timer = SYSTEM_event_timer;
	switch(timer)
	{
		case 0:
			raise_critical = 0;
		case 4:
			update_sensorgroup_bottom();
			update_EGT();
		break;
		case 2:
		case 5:
			update_sensorgroup_pressure();
			((NEXTION_Executable_Component*)UIBOARD_maindisplay_activecomponent)->execute();
		break;
	}
	update_watch();
	update_visual_alert();
	if(raise_critical)
	{
		if(!critical_raised)
		{
			SYSTEM_raisealert(SYSTEM_ALERT_CRITICAL);
			critical_raised = 1;
		}
	}
	else
		critical_raised = 0;
}