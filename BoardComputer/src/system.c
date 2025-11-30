#include "system.h"
#include "countersfeed.h"
#include "nextion.h"
#include "system_interface.h"

#define RTC_REGISTER TCNT2

CONFIG_Config SYSTEM_config;
volatile SYSTEM_STATUS SYSTEM_status;
volatile uint8_t SYSTEM_run = 1;
volatile uint8_t SYSTEM_exec;
volatile uint8_t SYSTEM_event_timer;//Represent fraction of second in values from 0 to 7.

const uint16_t SYSTEM_VERSION = 0x01;
const SYSTEM_cycle_timestamp_t SYSTEM_fullcycle_rtc_steps = 128;
static const uint8_t BEEP_DURATION = 4;
static const uint16_t MINIMAL_COMPAT_SYSTEMVERSION = 0x01;
static uint8_t shortbeep_counter;
static SYSTEM_ALERT_SEVERITY resolve_alert_severity(SYSTEM_ALERT alert);
static ALERT_PATTERN resolve_severity_pattern(SYSTEM_ALERT_SEVERITY severity);

static SYSTEM_Alert_t active_alert;

static ALERT_PATTERN resolve_severity_pattern(SYSTEM_ALERT_SEVERITY severity)
{
    ALERT_PATTERN pattern;
    switch(severity)
    {
        case SYSTEM_ALERT_SEVERITY_NOTIFICATION:
            pattern = 0xf;
        break;
        case SYSTEM_ALERT_SEVERITY_WARNING:
            pattern = 0xAA;
        break;
        case SYSTEM_ALERT_SEVERITY_CRITICAL:
            pattern = 0xf0f;
        break;
    }
    return pattern;
}

SYSTEM_ALERT_SEVERITY SYSTEM_resolve_alert_severity(SYSTEM_ALERT alert)
{
    SYSTEM_ALERT_SEVERITY severity;
    if(SYSTEM_ALERT_NOTIFICATIONS_END < alert)
    {
        if(SYSTEM_ALERT_WARNINGS_END < alert)
        {
            severity = SYSTEM_ALERT_SEVERITY_CRITICAL;
        }
        else
        {
            severity = SYSTEM_ALERT_SEVERITY_WARNING;
        }
    }
    else
    {
        severity = SYSTEM_ALERT_SEVERITY_NOTIFICATION;
    }

    return severity;
}

SYSTEM_cycle_timestamp_t SYSTEM_get_cycle_timestamp()
{
    SYSTEM_cycle_timestamp_t result = SYSTEM_event_timer * 0xf + RTC_REGISTER;
    return result;
}

SYSTEM_Alert_t SYSTEM_get_active_alert()
{
    return active_alert;
}

void SYSTEM_raisealert(SYSTEM_ALERT alert)
{
    SYSTEM_ALERT_SEVERITY severity = SYSTEM_resolve_alert_severity(alert);
    if(active_alert.severity <= severity)
    {
        active_alert.alert = alert;
        active_alert.severity = severity;
        active_alert.pattern = resolve_severity_pattern(severity);
        NEXTION_send_activealert();
    }
}

void SYSTEM_resetalert()
{
    active_alert.severity = 0;
    active_alert.pattern = 0;
    active_alert.alert = SYSTEM_ALERT_NO_ALERT;
}

void SYSTEM_initialize()
{
    SYSTEMINTERFACE_initialize_IO();
    CONFIG_loadconfig(&SYSTEM_config);
    /*
        Check Config version compatibility.
        Config version must be between Minimal compatible and current system version.
    */
    uint8_t config_version = SYSTEM_config.CONFIG_VERSION;
    if(MINIMAL_COMPAT_SYSTEMVERSION > config_version || SYSTEM_VERSION < config_version)
    {
        /*
            Version mismatch, format config and reload it.
        */
        CONFIG_factory_default_reset();
        CONFIG_loadconfig(&SYSTEM_config);
    }
    if(0 < CONFIG_sanitize_config(&SYSTEM_config))
    {
        /*
            If config contains errors, save corrected and raise warning.
        */
        CONFIG_saveconfig(&SYSTEM_config);
    }

    if(SYSTEM_config.SYSTEM_ALWAYS_ON)
    {
        SYSTEM_status = SYSTEM_STATUS_OPERATIONAL;
    }
    else
    {
        SYSTEM_status = SYSTEM_STATUS_IDLE;
    }

    SYSTEMINTERFACE_start_system_clock();
}

void SYSTEM_trigger_short_beep()
{
    shortbeep_counter = BEEP_DURATION;
}

void SYSTEM_update()
{
    if(!SYSTEM_config.SYSTEM_ALWAYS_ON)
    {        
        if(SYSTEM_STATUS_IDLE == SYSTEM_status && SYSTEMINTERFACE_is_board_enabled())
        {
            /*
                Setting board enabled reinitializes UI
                From now on UI system should restart display and wait for welcome packet.
            */
            NEXTION_initialize();
            SYSTEM_status = SYSTEM_STATUS_OPERATIONAL;
        }
        else if(SYSTEM_STATUS_OPERATIONAL == SYSTEM_status && !SYSTEMINTERFACE_is_board_enabled())
        {
            /*
                Set system to idle, display should have sleep procedure handlet by itself
                Display will sleep after no serial activity.
            */
            SYSTEM_status = SYSTEM_STATUS_IDLE;
        }
    }

    //Alarm and beep
    if(SYSTEM_STATUS_OPERATIONAL == SYSTEM_status)
    {    
        uint8_t beep_or_not_to_beep = 0;
        //Alert Control
        if(active_alert.pattern)
        { 
            if(active_alert.pattern & 0x01)
                beep_or_not_to_beep = 1;

            active_alert.pattern >>= 1;
            //Clear alert
            if(!active_alert.pattern)
                SYSTEM_resetalert();
        }
        //We can also beep when short beep is triggered.
        if(shortbeep_counter)
        {
            shortbeep_counter--;
            beep_or_not_to_beep = 1;
        }
        
        if(beep_or_not_to_beep)
        {
            SYSTEMINTERFACE_beeper_on();
        }
        else
        {
            SYSTEMINTERFACE_beeper_off();
        }
    }
}

EVENT_TIMER_ISR
{	
    SYSTEM_event_timer++;	
    SYSTEM_exec = 1;
    switch(SYSTEM_event_timer)
    {
        case 7:
			COUNTERSFEED_feed[COUNTERSFEED_FEEDID_FUELPS] = COUNTERSFEED_feed[COUNTERSFEED_FEEDID_FUEL];
            COUNTERSFEED_feed[COUNTERSFEED_FEEDID_FUEL] = 0;
            SYSTEM_event_timer = 0;
			return;
        break;
    }
}