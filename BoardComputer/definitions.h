//Definitions only for testing purposes
extern volatile uint8_t run, exec;
int main();
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
extern uint8_t UDR,UDRRX;
void USART_RXC_vect();
void USART_TXC_vect();
void USART_register();

#define SENSORSFEED_READY ...
#define SENSORSFEED_ADC_CHANNELS ...
extern uint8_t SENSORSFEED_status;
extern uint8_t ADMUX;
extern uint8_t ADC;
extern uint16_t SENSORSFEED_injector_ccm;
extern uint16_t SENSORSFEED_fuelmodifier;
void ADC_vect();

#define COUNTERSFEED_TICKSPERSECOND ...

extern char NEXTION_eot[4];

extern const int16_t PROGRAMDATA_NTC_2200_INVERTED[];