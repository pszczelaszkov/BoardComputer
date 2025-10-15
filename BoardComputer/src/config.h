/*
 * config.h
 *
 * Created: 2025-04-13 00:39:00
 * Author : pszczelaszkov
 */
#include<stdint.h>
#include"programdata.h"

#ifndef CONFIG_H_
#define CONFIG_H_

TESTUSE typedef int32_t CONFIG_maxdata_t;
TESTUSE extern const int32_t CONFIG_maxvalue;
TESTUSE extern const int32_t CONFIG_minvalue;

TESTUSE typedef struct CONFIG_Config
{
    uint8_t SYSTEM_FACTORY_RESET;
    uint8_t SYSTEM_ALWAYS_ON;
    uint8_t SYSTEM_BEEP_ON_CLICK;
    uint8_t SYSTEM_DISPLAYBRIGHTNESS;
    int16_t SENSORS_SIGNAL_PER_100KM;
    int16_t SENSORS_INJECTORS_CCM;
    uint16_t CONFIG_VERSION;
}CONFIG_Config;

/*
    Keep values consecutive and zero-based — do not assign manually.
    Required for proper indexing and use of _LAST.
*/
TESTUSE typedef enum CONFIG_ENTRY
{
    CONFIG_ENTRY_SYSTEM_FACTORY_RESET,
    CONFIG_ENTRY_SYSTEM_ALWAYS_ON,
    CONFIG_ENTRY_SYSTEM_BEEP_ON_CLICK,
    CONFIG_ENTRY_SYSTEM_DISPLAYBRIGHTNESS,
    CONFIG_ENTRY_SENSORS_SIGNAL_PER_100KM,
    CONFIG_ENTRY_SENSORS_INJECTORS_CCM,
    CONFIG_ENTRY_LAST
}CONFIG_Entry;

TESTUSE typedef enum CONFIG_ENTRY_VALIDATOR_VERDICT
{
    CONFIG_ENTRY_VERDICT_TOO_SMALL = -1,
    CONFIG_ENTRY_VERDICT_PASS = 0,
    CONFIG_ENTRY_VERDICT_TOO_BIG = 1,
    CONFIG_ENTRY_VERDICT_INVALID_ENTRY = 2
}CONFIG_ENTRY_VERDICT;

TESTUSE typedef struct CONFIG_ENTRY_VALIDATOR_RESULT
{   
    CONFIG_ENTRY_VERDICT verdict;
    CONFIG_maxdata_t value; /* Border value which triggered faulty verdict */
}CONFIG_ENTRY_VALIDATOR_RESULT;

/* To match PERSISTENT MEMORY, CODE and UI interface */
extern const uint16_t CONFIG_VERSION;

/*
    Modify config using entry as key.
    If config is provided(!NULL) then change will be made directly in it rather than in persistent memory.
    Returns real number of written bytes.
*/
TESTUSE uint8_t CONFIG_modify_entry(CONFIG_Config* config, CONFIG_Entry entry, CONFIG_maxdata_t* value);

/*
    Read config using entry as key.
    If config is provided(!NULL) then read will be made directly from it rather than from persistent memory.
    Returns real number of read bytes.
*/
TESTUSE uint8_t CONFIG_read_entry(CONFIG_Config* config, CONFIG_Entry entry, CONFIG_maxdata_t* dst);

/*
    Get entrys allowed values. 
*/
TESTUSE void CONFIG_get_entry_min_max_values(CONFIG_Entry entry, CONFIG_maxdata_t* min, CONFIG_maxdata_t* max);

/*
    Validate value against entry requirements.
*/
TESTUSE CONFIG_ENTRY_VALIDATOR_RESULT CONFIG_validate_entry(CONFIG_Entry entry, CONFIG_maxdata_t value);

/*
    Loads config structure from persistent memory
*/
TESTUSE void CONFIG_loadconfig(CONFIG_Config* config);

/*
    Save config structure to persistent memory
*/
TESTUSE void CONFIG_saveconfig(CONFIG_Config* config);

/*
    Resets config in persistent memory to default values. 
*/
uint8_t CONFIG_factory_default_reset();
#endif