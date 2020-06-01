//Definitions only for testing purposes
int main();

#define SCHEDULER_LOW_PRIORITY_QUEUE_SIZE ...
typedef int8_t (*Fptr)();
typedef struct LowPriorityTask
{
    uint8_t fid;
	struct LowPriorityTask* nextTask;

}LowPriorityTask;
enum SCHEDULER_callbacks{
    core_cb,
	USART_register_cb,
	LAST_cb
};
extern Fptr SCHEDULER_fregister[];
extern LowPriorityTask SCHEDULER_low_priority_tasks[];
extern LowPriorityTask* SCHEDULER_final_task;
extern LowPriorityTask* SCHEDULER_active_task;

extern int8_t USART_eot_counter;
extern uint8_t USART_RX_buffer_index;
extern uint8_t USART_TX_buffer_index;
