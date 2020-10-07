//Definitions only for testing purposes
extern volatile uint8_t run, exec, PINA, PINB;
volatile extern uint16_t TCNT1;
int main();
void core();
void prestart_routine();
void PCINT0_vect();
#define SCHEDULER_LOW_PRIORITY_QUEUE_SIZE ...
typedef void (*Fptr)();
typedef struct LowPriorityTask
{
    uint8_t fid;
	struct LowPriorityTask* nextTask;

}LowPriorityTask;
enum SCHEDULER_callbacks{
	USART_register_cb,
	LAST_cb
};
extern Fptr SCHEDULER_fregister[];
extern LowPriorityTask SCHEDULER_low_priority_tasks[];
extern LowPriorityTask* SCHEDULER_final_task;
extern LowPriorityTask* SCHEDULER_active_task;
void SCHEDULER_checkLowPriorityTasks();

#define USART_EOT ...
#define USART_EOT_COUNT ...
#define USART_TX_BUFFER_SIZE ...
#define USART_RX_BUFFER_SIZE ...
extern int8_t USART_eot_counter;
extern uint8_t USART_TX_message_length;
extern uint8_t USART_RX_buffer_index;
extern uint8_t USART_TX_buffer_index;
extern uint8_t UDR0,UDRRX;
void USART0_RX_vect();
void USART0_TX_vect();
void USART_register();
void USART_TX_clear();

#define SENSORSFEED_READY ...
#define ADCMULTIPLEXER ...
#define SENSORSFEED_ADC_CHANNELS ...
enum SENSORSFEED_feedid
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
enum COUNTERSFEED_counterinputs
{
    COUNTERSFEED_injector_input = 1,
    COUNTERSFEED_speed_input = 2
};
enum COUNTERSFEED_feed_indexes
{
    COUNTERSFEED_FEEDID_FUELPS,
    COUNTERSFEED_FEEDID_INJT,
    COUNTERSFEED_FEEDID_SPEED,
    COUNTERSFEED_FEEDID_LAST
};
void COUNTERSFEED_event_update();
extern uint16_t COUNTERSFEED_feed[][2]; 
extern uint8_t COUNTERSFEED_event_timer;

typedef void (*RenderingCallback)();
typedef struct MainDisplayRenderer
{
	RenderingCallback render;
	struct MainDisplayRenderer* nextRenderer;
	uint8_t picID;
}MainDisplayRenderer;

enum NextionMainDisplayModes
{
	NEXTION_MD_LPH,
	NEXTION_MD_LP100,
	NEXTION_MD_LP100_AVG,
	NEXTION_MD_SPEED_AVG,
	NEXTION_MD_INJ_T,
	NEXTION_MD_RANGE,
	NEXTION_MD_LAST
};
#define NEXTION_COMPONENT_MAINDISPLAY ...

int8_t NEXTION_switch_maindisplay();
extern char NEXTION_eot[4];
extern MainDisplayRenderer NEXTION_maindisplay_renderers[];
extern MainDisplayRenderer* NEXTION_maindisplay_renderer;

enum
{
    AVERAGE_BUFFER_SPEED,
    AVERAGE_BUFFER_LP100,
    AVERAGE_BUFFER_LAST
};
#define AVERAGE_BUFFERS_COUNT ...
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

extern const int16_t PROGRAMDATA_NTC_2200_INVERTED[];