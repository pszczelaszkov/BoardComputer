#include<config.h>
#include<stddef.h>
#include"persistent_memory.h"
#include"program_memory.h"


typedef enum ENTRY_VALIDATOR
{
    ENTRY_VALIDATOR_NONE,
    ENTRY_VALIDATOR_BOOLEAN, /* True/False validator */
    ENTRY_VALIDATOR_ENUM_2,
    ENTRY_VALIDATOR_ENUM_3,
    ENTRY_VALIDATOR_ENUM_4,
    ENTRY_VALIDATOR_ENUM_5,
    ENTRY_VALIDATOR_ENUM_6,
    ENTRY_VALIDATOR_ENUM_7,
    ENTRY_VALIDATOR_ENUM_8,
    ENTRY_VALIDATOR_ENUM_9,
    ENTRY_VALIDATOR_POSITIVE, /* Check if maxvalue >= value > 0 */
    ENTRY_VALIDATOR_NEGATIVE, /* Check if minvalue <= value < 0 */
    ENTRY_VALIDATOR_POSITIVE_4DIGIT, /* Check if 9999 >= value > 0 */
    ENTRY_VALIDATOR_NEGATIVE_4DIGIT, /* Check if -9999 <= value < 0 */
    ENTRY_VALIDATOR_PERCENT, /* Check if 0 <= value <= 100 */
}ENTRY_VALIDATOR;

typedef enum ENTRYINFO_SIZE
{
    ENTRYINFO_SIZE_8,
    ENTRYINFO_SIZE_16,
    ENTRYINFO_SIZE_32,
    ENTRYINFO_SIZE_64,
}ENTRYINFO_SIZE;

typedef struct ENTRYINFO
{
    uint8_t memory_offset;
    ENTRYINFO_SIZE size:2;
    ENTRY_VALIDATOR validator:6;
}Entryinfo;

#define SIZEOF_CONFIGFIELD(_field) sizeof(((CONFIG_Config *)0)->_field)
#define GET_ENTRY_SIZE(_field) (ENTRYINFO_SIZE)(SIZEOF_CONFIGFIELD(_field) < ENTRYINFO_SIZE_64 ? SIZEOF_CONFIGFIELD(_field) / 2 : ENTRYINFO_SIZE_64)
#define CONFIG_ENTRY(_field, _validator) \
    [CONFIG_ENTRY_##_field] = { \
        .memory_offset = (uint8_t)offsetof(CONFIG_Config, _field), \
        .size = GET_ENTRY_SIZE(_field), \
        .validator = _validator \
    }

const uint16_t CONFIG_VERSION = 0x1;
const int32_t CONFIG_maxvalue = 32768;
const int32_t CONFIG_minvalue = -32767;

static const Entryinfo entryinfo[CONFIG_ENTRY_LAST] PROGMEM = {
    CONFIG_ENTRY(SYSTEM_ALWAYS_ON, ENTRY_VALIDATOR_BOOLEAN),
    CONFIG_ENTRY(SYSTEM_DISPLAYBRIGHTNESS, ENTRY_VALIDATOR_PERCENT),
    CONFIG_ENTRY(SYSTEM_BEEP_ON_CLICK, ENTRY_VALIDATOR_BOOLEAN),
    CONFIG_ENTRY(SENSORS_SIGNAL_PER_100KM, ENTRY_VALIDATOR_POSITIVE_4DIGIT),
    CONFIG_ENTRY(SENSORS_INJECTORS_CCM, ENTRY_VALIDATOR_POSITIVE_4DIGIT),
};

static const uint32_t ADDRESS_IN_PERSISTENT_MEMORY = 0x0;

void CONFIG_get_entry_min_max_values(CONFIG_Entry entry, CONFIG_maxdata_t* min, CONFIG_maxdata_t* max)
{
    *min = 0;
    *max = 0;

    ENTRY_VALIDATOR validator = entryinfo[entry].validator;
    if (entry < CONFIG_ENTRY_LAST) {
        switch(validator)
        {
            case ENTRY_VALIDATOR_BOOLEAN:
            case ENTRY_VALIDATOR_ENUM_2:
            case ENTRY_VALIDATOR_ENUM_3:
            case ENTRY_VALIDATOR_ENUM_4:
            case ENTRY_VALIDATOR_ENUM_5:
            case ENTRY_VALIDATOR_ENUM_6:
            case ENTRY_VALIDATOR_ENUM_7:
            case ENTRY_VALIDATOR_ENUM_8:
            case ENTRY_VALIDATOR_ENUM_9:
                *max = validator;
            break;
            case ENTRY_VALIDATOR_PERCENT:
                *max = 100;
            break;
            case ENTRY_VALIDATOR_POSITIVE:
                *max = CONFIG_maxvalue;
            break;
            case ENTRY_VALIDATOR_NEGATIVE:
                *min = CONFIG_minvalue;
            break;
            case ENTRY_VALIDATOR_POSITIVE_4DIGIT:
                *max = 9999;
            break;
            case ENTRY_VALIDATOR_NEGATIVE_4DIGIT:
                *min = -9999;
            break;
        }
    }
}

CONFIG_ENTRY_VALIDATOR_RESULT CONFIG_validate_entry(CONFIG_Entry entry, CONFIG_maxdata_t value)
{
    CONFIG_ENTRY_VALIDATOR_RESULT result = {
        .verdict = CONFIG_ENTRY_VERDICT_PASS,
        .value = 0
    };

    CONFIG_maxdata_t min = 0;
    CONFIG_maxdata_t max = 0;

    if (entry >= CONFIG_ENTRY_LAST) {
        result.verdict = CONFIG_ENTRY_VERDICT_INVALID_ENTRY;
        return result;
    }

    CONFIG_get_entry_min_max_values(entry,&min,&max);
    if(max < value)
    {
        result.verdict = CONFIG_ENTRY_VERDICT_TOO_BIG;
        result.value = max;
    }
    else if(min > value)
    {
        result.verdict = CONFIG_ENTRY_VERDICT_TOO_SMALL;
        result.value = min;
    }

    return result;
}


void CONFIG_loadconfig(CONFIG_Config* config)
{
    PERSISTENT_MEMORY_read(ADDRESS_IN_PERSISTENT_MEMORY, config, (uint16_t)sizeof(CONFIG_Config));
}


void CONFIG_saveconfig(CONFIG_Config* config)
{
    PERSISTENT_MEMORY_write(ADDRESS_IN_PERSISTENT_MEMORY, config, (uint16_t)sizeof(CONFIG_Config));
}

uint8_t CONFIG_modify_entry(CONFIG_Config* config, CONFIG_Entry entry, CONFIG_maxdata_t* value)
{
    Entryinfo info;
    PROGRAM_MEMORY_read(&entryinfo[entry],&info,sizeof(Entryinfo));
    uint8_t size = 1 << info.size;
    uint8_t offset = info.memory_offset;

    if(config)
    {
        memcpy(&((uint8_t*)config)[offset], value, size);
    }
    else
    {
        PERSISTENT_MEMORY_write(ADDRESS_IN_PERSISTENT_MEMORY+offset, value, size);
    }

    return size;
}

uint8_t CONFIG_read_entry(CONFIG_Config* config, CONFIG_Entry entry, CONFIG_maxdata_t* value)
{
    Entryinfo info;
    PROGRAM_MEMORY_read(&entryinfo[entry],&info,sizeof(Entryinfo));
    uint8_t size = 1 << info.size;
    uint8_t offset = info.memory_offset;

    if(config)
    {
        memcpy(value, &((uint8_t*)config)[offset], size);
    }
    else
    {
        PERSISTENT_MEMORY_read(ADDRESS_IN_PERSISTENT_MEMORY+offset, value, size);
    }

    return size;
}
