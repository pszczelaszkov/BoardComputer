//Definitions only for testing purposes
extern volatile uint8_t SYSTEM_run, SYSTEM_exec, SYSTEM_event_timer, PINA, PINB, DDRB;
volatile extern uint16_t TCNT1,TCNT2;
typedef void (*Callback)();
void test();
void core();
void prestart_routine();
void PCINT0_vect();
void SPI0_STC_vect();
void TIMER2_COMPA_vect();
//#define SCHEDULER_LOW_PRIORITY_QUEUE_SIZE ...
#define BIT0 1
#define BIT1 2
#define BIT2 4
#define BIT3 8
#define BIT4 16
#define BIT5 32
#define BIT6 64
#define BIT7 128
/*typedef void (*Fptr)();
typedef struct LowPriorityTask
{
    uint8_t fid;
	struct LowPriorityTask* nextTask;

}LowPriorityTask;
enum SCHEDULER_CALLBACK
{
	SCHEDULER_CALLBACK_USART_REGISTER,
	SCHEDULER_CALLBACK_LAST
};
#define SCHEDULER_FREGISTER_SIZE ...
extern Fptr SCHEDULER_fregister[];
extern LowPriorityTask SCHEDULER_low_priority_tasks[];
extern LowPriorityTask* SCHEDULER_final_task;
extern LowPriorityTask* SCHEDULER_active_task;
void SCHEDULER_checkLowPriorityTasks();
*/
#define USART_EOT ...
#define USART_EOT_COUNT ...
#define USART_TX_BUFFER_SIZE ...
#define USART_RX_BUFFER_SIZE ...
extern int8_t USART_eot_counter;
extern uint8_t USART_TX_message_length;
extern uint8_t USART_RX_buffer_index;
extern uint8_t USART_TX_buffer_index;
extern uint8_t UDR2,UDRRX;
void USART2_RX_vect();
void USART2_TX_vect();
void USART_register();
void USART_TX_clear();
void USART_flush();

#define ADCMULTIPLEXER ...
#define SENSORSFEED_ADC_CHANNELS ...
enum SENSORSFEED_FEEDID
{
	SENSORSFEED_FEEDID_TANK,
	SENSORSFEED_FEEDID_EGT,
	SENSORSFEED_FEEDID_LPH,//fp 8bit
	SENSORSFEED_FEEDID_LP100,//fp 8bit
	SENSORSFEED_FEEDID_LP100_AVG,//fp 8bit
	SENSORSFEED_FEEDID_SPEED,//fp 8bit
	SENSORSFEED_FEEDID_SPEED_AVG,//fp 8bit
	SENSORSFEED_FEEDID_LAST
};
extern uint8_t ADMUX;
extern uint8_t ADC;
extern uint16_t SENSORSFEED_injector_ccm;
extern uint16_t SENSORSFEED_speed_ticks_100m;
extern uint16_t SENSORSFEED_fuelmodifier;
extern uint16_t SENSORSFEED_feed[];
void SENSORSFEED_initialize();
void SENSORSFEED_update_fuel();
void SENSORSFEED_update_speed();
void ADC_vect();

#define COUNTERSFEED_TICKSPERSECOND ...
enum COUNTERSFEED_INPUT
{
    COUNTERSFEED_INPUT_INJECTOR = 1,
    COUNTERSFEED_INPUT_SPEED = 2
};
enum COUNTERSFEED_FEEDID
{
    COUNTERSFEED_FEEDID_FUELPS,
    COUNTERSFEED_FEEDID_INJT,
    COUNTERSFEED_FEEDID_SPEED,
    COUNTERSFEED_FEEDID_LAST
};
extern uint16_t COUNTERSFEED_feed[][2];
inline void COUNTERSFEED_pushfeed(uint8_t index);

#define NEXTION_SELECT_DECAY_TICKS ...
#define NEXTION_OBJNAME_LEN ...
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

typedef enum NEXTION_COMPONENTTYPE
{
	NEXTION_COMPONENTTYPE_PIC,
	NEXTION_COMPONENTTYPE_TEXT
}NEXTION_Componenttype_t;

typedef struct NEXTION_Component
{
	uint8_t picID_default;
	uint8_t picID_selected;
	const char* name;
	NEXTION_Componenttype_t type;
}NEXTION_Component;
typedef struct NEXTION_MDComponent
{
	uint8_t picID_default;
	uint8_t picID_selected;
	const char* name;
	NEXTION_Componenttype_t type;

	Callback render;
	struct NEXTION_MDComponent* nextRenderer;
}NEXTION_MDComponent;

void NEXTION_switch_maindisplay();
extern char NEXTION_eot[];
extern NEXTION_Component NEXTION_components[];
extern NEXTION_MDComponent NEXTION_maindisplay_renderers[];
extern NEXTION_MDComponent* NEXTION_maindisplay_renderer;

enum
{
    AVERAGE_BUFFER_SPEED,
    AVERAGE_BUFFER_LP100,
    AVERAGE_BUFFER_LAST
};
#define AVERAGE_BUFFERS_SIZE ...
typedef struct Average
{
    uint32_t sum;
    uint16_t average;
    uint16_t count;
    uint16_t sum_base;
}Average;
extern Average AVERAGE_buffers[];
uint16_t AVERAGE_addvalue(uint8_t bufferid, uint16_t value);
void AVERAGE_clear(uint8_t bufferid);

extern enum SENSORSFEED_EGT_STATUS
{
	SENSORSFEED_EGT_STATUS_UNKN,
	SENSORSFEED_EGT_STATUS_OPEN,
	SENSORSFEED_EGT_STATUS_VALUE
}SENSORSFEED_EGT_status;

extern enum SENSORSFEED_EGT_TRANSMISSION_STATUS
{
	SENSORSFEED_EGT_TRANSMISSION_READY,
	SENSORSFEED_EGT_TRANSMISSION_HALF,
	SENSORSFEED_EGT_TRANSMISSION_FULL
}SENSORSFEED_EGT_transmission_status;

void SENSORSFEED_update_EGT();
void NEXTION_update_EGT();
void NEXTION_update_select_decay();
extern uint8_t NEXTION_selection_counter;
extern uint8_t SPDR0;
extern uint16_t SENSORSFEED_max6675_data;

#define TIMER_REGISTER ...
extern const uint8_t TIMER_REGISTER_WEIGHT;
extern const uint8_t TIMER_MILISECOND_WEIGHT;
typedef struct TIMER_watch
{    
    struct
    {
        uint8_t hours;
        uint8_t minutes;
        uint8_t seconds;
        uint8_t miliseconds;//2digit
        uint8_t watchstatus;
    }timer; 
    struct TIMER_watch* next_watch;
}TIMER_watch;

enum TIMER_STOPWATCHSTATUS
{
    TIMER_WATCHSTATUS_ZERO,
    TIMER_WATCHSTATUS_COUNTING,
    TIMER_WATCHSTATUS_STOP
};

enum TIMER_TIMERTYPE
{
    TIMERTYPE_WATCH,
    TIMERTYPE_STOPWATCH
};

uint8_t TIMER_counter_to_miliseconds();
void TIMER_update();
void TIMER_watch_zero(TIMER_watch* timer);
void TIMER_watch_toggle(TIMER_watch* timer);
void TIMER_next_watch();

extern TIMER_watch* TIMER_active_watch;
extern TIMER_watch TIMER_watches[2];
extern char TIMER_formated[12];

typedef enum INPUT_COMPONENTID
{
	INPUT_COMPONENT_NONE = 0,
	INPUT_COMPONENT_MAINDISPLAY = 2,
	INPUT_COMPONENT_WATCH = 5,
	INPUT_COMPONENT_WATCHSEL = 6
}INPUT_ComponentID_t;

typedef enum INPUT_KEYSTATUS
{
	INPUT_KEYSTATUS_RELEASED = 0,
	INPUT_KEYSTATUS_PRESSED,
	//8 system cycles to detect as HOLD 
	INPUT_KEYSTATUS_HOLD = 9,
	INPUT_KEYSTATUS_CLICK
}INPUT_Keystatus_t;

typedef enum INPUT_KEY
{
	INPUT_KEY_ENTER,
	INPUT_KEY_DOWN,
	INPUT_KEY_LAST
}INPUT_Key_t;

typedef struct INPUT_Component
{
	INPUT_ComponentID_t componentID;
	Callback on_click;
	Callback on_hold;
	NEXTION_Component* nextion_component;
}INPUT_Component;

extern uint8_t INPUT_active_page;
extern INPUT_Keystatus_t INPUT_keystatus[INPUT_KEY_LAST];
extern INPUT_Component INPUT_components[];
extern INPUT_Component* INPUT_active_component;

void INPUT_switch_maindisplay();
void INPUT_userinput(INPUT_Keystatus_t keystatus, INPUT_Key_t key, INPUT_ComponentID_t componentID);
INPUT_Component* INPUT_findcomponent(uint8_t componentID);
void INPUT_update();

extern const int16_t PROGRAMDATA_NTC_2200_INVERTED[];