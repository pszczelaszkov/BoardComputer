#include "UART.h"
#include "stdint.h"
#include "serial.h"
#include "system.h"
#include <string.h>
#include <stdatomic.h>

typedef struct
{
    uint8_t index;
    uint8_t length;
}uart_tx_msg_descriptor_t;

typedef struct
{
    uint8_t index;
    uint8_t length;
    SYSTEM_cycle_timestamp_t timestamp;
}uart_rx_msg_descriptor_t;

typedef enum
{
    UART_RX_IDLE,
    UART_RX_ACTIVE,
    UART_RX_DROPPING
}uart_rx_status_t;

typedef struct
{
    uint8_t descriptor_push_index;
    uint8_t descriptor_pop_index;
}uart_tx_state_t;

typedef struct
{
    uint8_t descriptor_push_index;
    uint8_t descriptor_pop_index;
    uint8_t buffer_write_index;
    uint8_t message_index;
    uint8_t message_end;
    uint8_t terminator_count;
    uart_rx_status_t status;
    uint8_t held_descriptor_index;
}uart_rx_state_t;

static uint8_t nextion_tx_buffer[UART_NEXTION_TX_BUFFER_SIZE];
static uart_tx_msg_descriptor_t nextion_tx_msg_descriptor[UART_NEXTION_TX_MSG_DESCRIPTORS_COUNT] = {0};
static uart_tx_state_t nextion_tx_state = {
    .descriptor_push_index = UART_NEXTION_TX_MSG_DESCRIPTORS_COUNT - 1
};

static uint8_t service_tx_buffer[UART_SERVICE_TX_BUFFER_SIZE];
static uart_tx_msg_descriptor_t service_tx_msg_descriptor[UART_SERVICE_TX_MSG_DESCRIPTORS_COUNT] = {0};
static uart_tx_state_t service_tx_state = {
    .descriptor_push_index = UART_SERVICE_TX_MSG_DESCRIPTORS_COUNT - 1
};

static uint8_t nextion_rx_buffer[NEXTION_RX_BUFFER_SIZE];
static uart_rx_msg_descriptor_t
    nextion_rx_msg_descriptor[UART_NEXTION_RX_MSG_DESCRIPTORS_COUNT] = {0};
static uart_rx_state_t nextion_rx_state = {
    .descriptor_push_index = UART_NEXTION_RX_MSG_DESCRIPTORS_COUNT - 1,
    .held_descriptor_index = UART_NEXTION_RX_MSG_DESCRIPTORS_COUNT
};

static uint8_t service_rx_buffer[SERVICE_RX_BUFFER_SIZE];
static uart_rx_msg_descriptor_t
    service_rx_msg_descriptor[UART_SERVICE_RX_MSG_DESCRIPTORS_COUNT] = {0};
static uart_rx_state_t service_rx_state = {
    .descriptor_push_index = UART_SERVICE_RX_MSG_DESCRIPTORS_COUNT - 1,
    .held_descriptor_index = UART_SERVICE_RX_MSG_DESCRIPTORS_COUNT
};

static const uint8_t nextion_tx_terminator[UART_NEXTION_TERMINATOR_LEN] = {0xFF, 0xFF, 0xFF};
static const uint8_t service_tx_terminator[UART_SERVICE_TERMINATOR_LEN] = {'\r'};

static inline uint8_t advance_tx_index(uint8_t index, uint8_t descriptors_count)
{
    return (index >= descriptors_count - 1) ? 0 : (uint8_t)(index + 1);
}

static inline uint8_t advance_rx_index(uint8_t index, uint8_t descriptors_count)
{
    return (index >= descriptors_count - 1) ? 0 : (uint8_t)(index + 1);
}

static void send_current_msg(UART_CHANNEL channel)
{
    uint8_t* buffer = 0x0;
    uart_tx_msg_descriptor_t* descriptors = 0x0;
    uart_tx_state_t* state = 0x0;
    volatile uint8_t* out_status = 0x0;

    switch(channel)
    {
        case UART_CHANNEL_NEXTION:
            buffer = nextion_tx_buffer;
            descriptors = nextion_tx_msg_descriptor;
            state = &nextion_tx_state;
            out_status = &SERIAL_NEXTION_OUT_status;
            break;
        case UART_CHANNEL_SERVICE:
            buffer = service_tx_buffer;
            descriptors = service_tx_msg_descriptor;
            state = &service_tx_state;
            out_status = &SERIAL_SERVICE_OUT_status;
            break;
        default:
            return;
    }

    if(SERIAL_OUT_STATUS_IDLE == *out_status)
    {
        uart_tx_msg_descriptor_t* current = &descriptors[state->descriptor_pop_index];
        /* Serial is idle, it won't kick send during IRQ's */
        uint8_t length = current->length;
        uint8_t* data = &buffer[current->index];
        if(length > 0)
        {
            SERIAL_send_msg(channel, data, length);
        }
    }
}

void UART_send_next_msg(UART_CHANNEL channel)
{
    uart_tx_msg_descriptor_t* descriptors = 0x0;
    uart_tx_state_t* state = 0x0;
    uint8_t descriptors_count = 0;
    volatile uint8_t* out_status = 0x0;

    switch(channel)
    {
        case UART_CHANNEL_NEXTION:
            descriptors = nextion_tx_msg_descriptor;
            state = &nextion_tx_state;
            descriptors_count = UART_NEXTION_TX_MSG_DESCRIPTORS_COUNT;
            out_status = &SERIAL_NEXTION_OUT_status;
            break;
        case UART_CHANNEL_SERVICE:
            descriptors = service_tx_msg_descriptor;
            state = &service_tx_state;
            descriptors_count = UART_SERVICE_TX_MSG_DESCRIPTORS_COUNT;
            out_status = &SERIAL_SERVICE_OUT_status;
            break;
        default:
            return;
    }

    SERIAL_BUFFER_OPERATIONS
    {
        *out_status = SERIAL_OUT_STATUS_IDLE;
        __atomic_signal_fence(memory_order_seq_cst);
        /* Mark current message as empty with zero length */
        descriptors[state->descriptor_pop_index].length = 0;
        state->descriptor_pop_index =
            advance_tx_index(state->descriptor_pop_index, descriptors_count);
        send_current_msg(channel);
    }
}

void UART_init()
{
    SERIAL_BUFFER_OPERATIONS
    {
        memset(nextion_tx_buffer, 0, UART_NEXTION_TX_BUFFER_SIZE);
        memset(nextion_tx_msg_descriptor, 0,
               sizeof(uart_tx_msg_descriptor_t) * UART_NEXTION_TX_MSG_DESCRIPTORS_COUNT);
        nextion_tx_state.descriptor_push_index = UART_NEXTION_TX_MSG_DESCRIPTORS_COUNT - 1;
        nextion_tx_state.descriptor_pop_index = 0;

        memset(service_tx_buffer, 0, UART_SERVICE_TX_BUFFER_SIZE);
        memset(service_tx_msg_descriptor, 0,
               sizeof(uart_tx_msg_descriptor_t) * UART_SERVICE_TX_MSG_DESCRIPTORS_COUNT);
        service_tx_state.descriptor_push_index = UART_SERVICE_TX_MSG_DESCRIPTORS_COUNT - 1;
        service_tx_state.descriptor_pop_index = 0;

        SERIAL_NEXTION_OUT_status = SERIAL_OUT_STATUS_IDLE;
        SERIAL_SERVICE_OUT_status = SERIAL_OUT_STATUS_IDLE;

        memset(nextion_rx_buffer, 0, NEXTION_RX_BUFFER_SIZE);
        memset(nextion_rx_msg_descriptor, 0,
               sizeof(uart_rx_msg_descriptor_t) * UART_NEXTION_RX_MSG_DESCRIPTORS_COUNT);
        memset(&nextion_rx_state, 0, sizeof(nextion_rx_state));
        nextion_rx_state.descriptor_push_index = UART_NEXTION_RX_MSG_DESCRIPTORS_COUNT - 1;
        nextion_rx_state.held_descriptor_index = UART_NEXTION_RX_MSG_DESCRIPTORS_COUNT;

        memset(service_rx_buffer, 0, SERVICE_RX_BUFFER_SIZE);
        memset(service_rx_msg_descriptor, 0,
               sizeof(uart_rx_msg_descriptor_t) * UART_SERVICE_RX_MSG_DESCRIPTORS_COUNT);
        memset(&service_rx_state, 0, sizeof(service_rx_state));
        service_rx_state.descriptor_push_index = UART_SERVICE_RX_MSG_DESCRIPTORS_COUNT - 1;
        service_rx_state.held_descriptor_index = UART_SERVICE_RX_MSG_DESCRIPTORS_COUNT;
    }
}

/*
    Puts message into buffer, returns 1 if successful, 0 if not.
    Appends channel terminator (Nextion 0xFF x3, service CR).
    If successful, message is ready to be sent and will be sent when serial is available.
    If not successful, message is dropped.
*/
uint8_t UART_write_message(UART_CHANNEL channel, const uint8_t* data, const uint8_t length)
{
    uint8_t* buffer = 0x0;
    uart_tx_msg_descriptor_t* descriptors = 0x0;
    uart_tx_state_t* state = 0x0;
    uint8_t descriptors_count = 0;
    uint8_t buffer_size = 0;
    const uint8_t* terminator = 0x0;
    uint8_t terminator_len = 0;
    uint8_t result = 0;

    switch(channel)
    {
        case UART_CHANNEL_NEXTION:
            buffer = nextion_tx_buffer;
            descriptors = nextion_tx_msg_descriptor;
            state = &nextion_tx_state;
            descriptors_count = UART_NEXTION_TX_MSG_DESCRIPTORS_COUNT;
            buffer_size = UART_NEXTION_TX_BUFFER_SIZE;
            terminator = nextion_tx_terminator;
            terminator_len = UART_NEXTION_TERMINATOR_LEN;
            break;
        case UART_CHANNEL_SERVICE:
            buffer = service_tx_buffer;
            descriptors = service_tx_msg_descriptor;
            state = &service_tx_state;
            descriptors_count = UART_SERVICE_TX_MSG_DESCRIPTORS_COUNT;
            buffer_size = UART_SERVICE_TX_BUFFER_SIZE;
            terminator = service_tx_terminator;
            terminator_len = UART_SERVICE_TERMINATOR_LEN;
            break;
        default:
            return 0;
    }

    const uint16_t framed_length = (uint16_t)length + terminator_len;
    if(framed_length > buffer_size)
    {
        return 0;
    }

    SERIAL_BUFFER_OPERATIONS
    {
        /* Peek next descriptor slot without committing push yet */
        const uint8_t next_push_index =
            advance_tx_index(state->descriptor_push_index, descriptors_count);
        uart_tx_msg_descriptor_t* new_descriptor = &descriptors[next_push_index];

        if(0 == new_descriptor->length)
        {
            const uart_tx_msg_descriptor_t* old_descriptor =
                &descriptors[state->descriptor_push_index];
            /*
                Then resolve where messages are read from to avoid overwriting.
                During IRQ's it may move forward and rollback, but it will not overtake write, or current read index.
            */
            const uint8_t buffer_read_index = descriptors[state->descriptor_pop_index].index;
            /* Write is only moved in this context, it won't change during IRQ's */
            const uint8_t buffer_write_index = old_descriptor->index + old_descriptor->length;

            /* Check if there is space for framed message. */
            const uint8_t space_ahead = buffer_size - buffer_write_index;
            const uint8_t space_behind = buffer_read_index;

            uint8_t index = 0;
            uint8_t have_space = 0;
            if(buffer_write_index >= buffer_read_index)
            {
                if(framed_length <= space_ahead)
                {
                    /* Buffer does not yet hit the end and there is space for message */
                    index = buffer_write_index;
                    have_space = 1;
                }
                else if(space_behind >= framed_length)
                {
                    /* No chance to place message in end of buffer, try to place it in front */
                    index = 0;
                    have_space = 1;
                }
            }
            else
            {
                /* Already wrapped: free space is between write cursor and read index */
                const uint8_t space_middle = buffer_read_index - buffer_write_index;
                if(framed_length <= space_middle)
                {
                    index = buffer_write_index;
                    have_space = 1;
                }
            }

            if(have_space)
            {
                /* Commit push only after space checks succeed */
                state->descriptor_push_index = next_push_index;
                new_descriptor->index = index;
                if(length > 0)
                {
                    memcpy(&buffer[index], data, length);
                }
                memcpy(&buffer[index + length], terminator, terminator_len);
                __atomic_signal_fence(memory_order_seq_cst);
                /* Length is written last, as it marks descriptor as ready to be sent*/
                new_descriptor->length = (uint8_t)framed_length;

                send_current_msg(channel);
                result = 1;
            }
        }
    }

    return result;
}

static uint8_t start_rx_message(uint8_t buffer_size,
                                uart_rx_msg_descriptor_t* descriptors,
                                uint8_t descriptors_count,
                                uart_rx_state_t* state)
{
    const uint8_t descriptor_index =
        advance_rx_index(state->descriptor_push_index, descriptors_count);
    if(0 != descriptors[descriptor_index].length)
    {
        /*No free descriptors to start new message.*/
        return 0;
    }

    uint8_t have_message = 0;
    uint8_t read_index = 0;
    /*Check if there is a held descriptor(message that is being processed).*/
    if(state->held_descriptor_index < descriptors_count)
    {
        read_index = descriptors[state->held_descriptor_index].index;
        have_message = 1;
    }
    else if(0 != descriptors[state->descriptor_pop_index].length)
    {
        read_index = descriptors[state->descriptor_pop_index].index;
        have_message = 1;
    }

    uint8_t message_index = 0;
    uint8_t message_end = 0;

    if(0 == have_message)
    {
        /*RX queue empty, full buffer available.*/
        state->buffer_write_index = 0;
        message_end = buffer_size;
    }
    else if(state->buffer_write_index >= read_index)
    {
        /*Normal case, reader follows writer, decide only based on space available.*/
        const uint8_t space_ahead = buffer_size - state->buffer_write_index;
        const uint8_t space_behind = read_index;
        if(space_ahead >= space_behind)
        {
            message_index = state->buffer_write_index;
            message_end = buffer_size;
        }
        else
        {
            message_end = space_behind;
        }
    }
    else
    {
        /*Already wrapped: free space is between write cursor and read index.*/
        message_index = state->buffer_write_index;
        message_end = read_index;
    }

    if(message_index == message_end)
    {
        /*No space to start new message*/
        return 0;
    }

    /*Start new message*/
    state->message_index = message_index;
    state->buffer_write_index = message_index;
    state->message_end = message_end;
    return 1;
}

static uint8_t rx_terminator_reached(UART_CHANNEL channel,
                                     uint8_t byte,
                                     uart_rx_state_t* state)
{
    uint8_t reached = 0;

    switch(channel)
    {
        case UART_CHANNEL_SERVICE:
            reached = ('\r' == byte);
            state->terminator_count = reached;
            break;
        case UART_CHANNEL_NEXTION:
            if(0xFF == byte)
            {
                if(state->terminator_count < UART_NEXTION_TERMINATOR_LEN)
                {
                    state->terminator_count++;
                }
            }
            else
            {
                state->terminator_count = 0;
            }
            reached = (UART_NEXTION_TERMINATOR_LEN == state->terminator_count);
            break;
        default:
            break;
    }

    return reached;
}

void UART_put_byte_to_RX_buffer(UART_CHANNEL channel, uint8_t byte)
{
    uint8_t* buffer = 0x0;
    uart_rx_msg_descriptor_t* descriptors = 0x0;
    uart_rx_state_t* state = 0x0;
    uint8_t descriptors_count = 0;
    uint8_t buffer_size = 0;
    uint8_t terminator_length = 0;

    switch(channel)
    {
        case UART_CHANNEL_NEXTION:
            buffer = nextion_rx_buffer;
            descriptors = nextion_rx_msg_descriptor;
            state = &nextion_rx_state;
            descriptors_count = UART_NEXTION_RX_MSG_DESCRIPTORS_COUNT;
            buffer_size = NEXTION_RX_BUFFER_SIZE;
            terminator_length = UART_NEXTION_TERMINATOR_LEN;
            break;
        case UART_CHANNEL_SERVICE:
            buffer = service_rx_buffer;
            descriptors = service_rx_msg_descriptor;
            state = &service_rx_state;
            descriptors_count = UART_SERVICE_RX_MSG_DESCRIPTORS_COUNT;
            buffer_size = SERVICE_RX_BUFFER_SIZE;
            terminator_length = UART_SERVICE_TERMINATOR_LEN;
            break;
        default:
            return;
    }

    SERIAL_BUFFER_OPERATIONS
    {
        if(UART_RX_IDLE == state->status)
        {
            if(start_rx_message(buffer_size, descriptors, descriptors_count, state))
            {
                /*New message started, set status to active.*/
                state->status = UART_RX_ACTIVE;
            }
            else
            {
                /*Either no space or descriptors to start new message, drop message.*/
                state->status = UART_RX_DROPPING;
            }
        }

        if(UART_RX_ACTIVE == state->status)
        {
            if(state->buffer_write_index < state->message_end)
            {
                /*Message data is being written.*/
                buffer[state->buffer_write_index] = byte;
                state->buffer_write_index++;
            }
            else
            {
                /*Buffer is full, drop message.*/
                state->status = UART_RX_DROPPING;
            }
        }

        if(rx_terminator_reached(channel, byte, state))
        {
            if(UART_RX_ACTIVE == state->status)
            {
                /*Message is complete, write to descriptor.*/
                const uint8_t framed_length =
                    state->buffer_write_index - state->message_index;
                const uint8_t payload_length = framed_length - terminator_length;
                state->buffer_write_index = state->message_index + payload_length;
                if(payload_length > 0)
                {
                    const uint8_t descriptor_index =
                        advance_rx_index(state->descriptor_push_index, descriptors_count);
                    uart_rx_msg_descriptor_t* descriptor = &descriptors[descriptor_index];

                    descriptor->index = state->message_index;
                    descriptor->timestamp = SYSTEM_get_cycle_timestamp();
                    state->descriptor_push_index = descriptor_index;
                    __atomic_signal_fence(memory_order_seq_cst);
                    descriptor->length = payload_length;
                }
            }
            else if(0 != state->message_end)
            {
                /* Dropped after a reservation, snap write back to the start of this attempt. */
                state->buffer_write_index = state->message_index;
            }

            state->message_end = 0;
            state->terminator_count = 0;
            state->status = UART_RX_IDLE;
        }
    }
}

uint8_t UART_get_next_message(UART_CHANNEL channel, uint8_t** message,
                              SYSTEM_cycle_timestamp_t* timestamp)
{
    uart_rx_msg_descriptor_t* descriptors = 0x0;
    uart_rx_state_t* state = 0x0;
    uint8_t* buffer = 0x0;
    uint8_t descriptors_count = 0;
    uint8_t result = 0;

    if(0x0 == message)
    {
        return 0;
    }
    *message = 0x0;

    switch(channel)
    {
        case UART_CHANNEL_NEXTION:
            buffer = nextion_rx_buffer;
            descriptors = nextion_rx_msg_descriptor;
            state = &nextion_rx_state;
            descriptors_count = UART_NEXTION_RX_MSG_DESCRIPTORS_COUNT;
            break;
        case UART_CHANNEL_SERVICE:
            buffer = service_rx_buffer;
            descriptors = service_rx_msg_descriptor;
            state = &service_rx_state;
            descriptors_count = UART_SERVICE_RX_MSG_DESCRIPTORS_COUNT;
            break;
        default:
            return 0;
    }

    SERIAL_BUFFER_OPERATIONS
    {
        if(state->held_descriptor_index < descriptors_count)
        {
            descriptors[state->held_descriptor_index].length = 0;
            state->held_descriptor_index = descriptors_count;
        }

        uart_rx_msg_descriptor_t* descriptor =
            &descriptors[state->descriptor_pop_index];
        result = descriptor->length;
        if(result > 0)
        {
            *message = &buffer[descriptor->index];
            if(0x0 != timestamp)
            {
                *timestamp = descriptor->timestamp;
            }
            state->held_descriptor_index = state->descriptor_pop_index;
            state->descriptor_pop_index =
                advance_rx_index(state->descriptor_pop_index, descriptors_count);
        }
    }

    return result;
}
