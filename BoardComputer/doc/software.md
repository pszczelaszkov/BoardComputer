# Software

[It's not bug it's a feature](known-issues.md)

### Simplified system design
```mermaid
flowchart TD
    init["ENTRY_ROUTINE: initialize modules"]
    sleep[SYSTEMINTERFACE_sleep]
    postIrq["post_irq_core: process UART messages"]
    timerIrq["Timer IRQ: set SYSTEM_exec"]
    highPrio["high_prio_core: UART, input, timer"]
    core["core: counters, sensors, Nextion, system"]

    init --> sleep
    sleep --> postIrq
    postIrq -->|"SYSTEM_exec is false"| sleep
    postIrq -->|"SYSTEM_exec is true"| highPrio
    timerIrq --> highPrio
    highPrio --> core
    core --> sleep

    subgraph uartRx [UART receive]
        rxIrq[USART RX ISR]
        rxQueue["UART_put_byte_to_RX_buffer: queue frame and timestamp"]
        dispatch[process_UART_messages]
        input["INPUT_userinput for Nextion touch"]
        handlers[Nextion message handlers]
        rxIrq --> rxQueue
        rxQueue --> dispatch
        dispatch --> input
        dispatch --> handlers
    end

    postIrq --> dispatch
    highPrio --> dispatch

    subgraph uartTx [UART transmit]
        producer[Nextion producer]
        enqueue[UART_write_message]
        serialSend[SERIAL_send_msg]
        txIrq["USART TX ISR: continue bytes"]
        producer --> enqueue
        enqueue --> serialSend
        serialSend --> txIrq
    end
```

The asynchronous Timer2 beat runs at 8 Hz. Other IRQs may wake the CPU and let
`post_irq_core` process queued UART frames, but only the timer IRQ schedules the
high-priority and regular core cycle. UART RX handlers queue complete frames;
application handlers run outside the RX ISR and receive the frame timestamp.

Hardware WDT is 1 s, enabled at boot and refreshed from `SYSTEM_update` each 8 Hz beat; BOD (4.3 V) is a fuse setting, not firmware.
