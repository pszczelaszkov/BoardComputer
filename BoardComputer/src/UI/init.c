#include"init.h"
#include<nextion.h>
#include<system.h>

const int8_t WAIT_FOREVER = -1;
const int8_t WAIT_1S = 8;
static int8_t wait_time;

void UIINIT_setup()
{
	SYSTEM_ALERT_SEVERITY severity = SYSTEM_resolve_alert_severity(SYSTEM_get_active_alert().alert);
	NEXTION_send_activealert();
	switch(severity)
	{
		case SYSTEM_ALERT_SEVERITY_WARNING:
			wait_time = WAIT_1S*2;
		break;
		case SYSTEM_ALERT_SEVERITY_CRITICAL:
			wait_time = WAIT_FOREVER;
		break;
		default:
			wait_time = WAIT_1S;
	}
}

void UIINIT_update()
{
	if(WAIT_FOREVER != wait_time)
	{
		if(0 < wait_time)
		{
			wait_time--;
		}
		else
		{
			NEXTION_switch_page(NEXTION_PAGEID_BOARD,0);
		}
	}
}
