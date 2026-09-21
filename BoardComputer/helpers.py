import cffi
import importlib



class ModuleWrapper:
    def __init__(self, module):
        self.module = module
        self.snapshot = {}

    def __enter__(self):
        self.create_snapshot()
        return self.module

    def __exit__(self, exc_type, exc_value, exc_traceback):
        self.load_snapshot()

    def create_snapshot(self):
        for key in self.module.__dict__.keys():
            value = eval(f"self.module.{key}")
            if isinstance(value, cffi.FFI().CData):
                try:
                    size = eval(f"self.module.{key.upper()}_SIZE")
                    # cast cdata to byte array
                    self.snapshot[key] = [value[i] for i in range(size)]
                except AttributeError:
                    print(f"Missing {key.upper()}_SIZE for {key}")
            else:
                self.snapshot[key] = value

    def load_snapshot(self):
        for key, value in self.snapshot.items():
            if isinstance(value, int):
                try:
                    exec(f"self.module.{key}=value")
                except AttributeError:  # Probably const
                    pass
            elif isinstance(value, list):
                for i, data in enumerate(value):
                    exec(f"self.module.{key}[{i}]=data")


def load(filename):
    # import and return resulting module
    ffi = cffi.FFI()
    module = importlib.import_module("bin." + filename)
    with open("test/generatedDefinitions/definitions.h") as definitions_file:
        ffi.cdef(definitions_file.read())
    return module.lib, ffi


def exec_cycle(module,display_alive=True):
    if display_alive is True:
        module.NEXTION_handler_sendme(module.NEXTION_get_pageid())
    module.core()


def max6675_response(module, test_response):
    for i in range(2):
        module.SPDR0 = (test_response & 0xFF00) >> 8  # MSB first
        test_response = test_response << 8
        module.SPI0_STC_vect()
        yield


def uart_write_message(module, ffi, channel, payload):
    """Enqueue a TX payload via UART_write_message (terminator appended in C)."""
    if len(payload) == 0:
        return module.UART_write_message(channel, ffi.NULL, 0)
    data = ffi.new("uint8_t[]", list(payload))
    return module.UART_write_message(channel, data, len(payload))


def uart_consume_message(module, channel):
    """Simulate TX-complete: free current descriptor and kick the next."""
    module.UART_send_next_msg(channel)


def uart_send_nextion(module, ffi, payload):
    """Enqueue a Nextion TX payload via UART_write_message."""
    return uart_write_message(module, ffi, module.UART_CHANNEL_NEXTION, payload)


def uart_consume_nextion(module):
    """Simulate Nextion TX-complete: free current descriptor and kick the next."""
    uart_consume_message(module, module.UART_CHANNEL_NEXTION)


def write_usart(module, header, message):
    usart_eot = int.to_bytes(module.USART_EOT, 1, byteorder="little")
    usart_eot = usart_eot * module.USART_EOT_COUNT
    if header is not None:
        usart_header = header.to_bytes(1, byteorder="little")
        message = usart_header + bytearray(message) + usart_eot
    else:
        message = bytearray(message) + usart_eot
    for byte in message:
        module.serial_service_in = byte
        module.USART_read_service_byte()


def floattofp(value, fractionalsize):
    weight = 2**fractionalsize
    return int(round(value * weight))


def fptofloat(value, fractionalsize):
    weight = 2**fractionalsize
    return value / weight


def _drain_uart_channel(m, ffi, out_status, message_ptr, message_length, channel):
    """Drain in-flight UART TX messages for one channel into a bytearray."""
    chunks = bytearray()
    busy = m.SERIAL_OUT_STATUS_BUSY
    while getattr(m, out_status) == busy and getattr(m, message_length) > 0:
        ptr = getattr(m, message_ptr)
        length = getattr(m, message_length)
        chunks.extend(ffi.unpack(ptr, length))
        m.UART_send_next_msg(channel)
    return bytes(chunks)


def read_nextion_output(m, ffi):
    result = {}
    raw = _drain_uart_channel(
        m,
        ffi,
        "SERIAL_NEXTION_OUT_status",
        "nextion_tx_message",
        "nextion_tx_message_length",
        m.UART_CHANNEL_NEXTION,
    )
    output = raw.split(b"\xff\xff\xff")
    for message in output[:-1]:
        if not message:
            continue
        try:
            msg_decoded = message.decode()
        except UnicodeDecodeError:
            continue
        if msg_decoded.startswith("page "):
            variable, value = msg_decoded.split(" ", 1)
        elif '=' in msg_decoded:
            variable, value = msg_decoded.split("=", 1)
        else:
            variable = msg_decoded
            value = ""
        result[variable] = value

    print(f"NEXTION RAW: {output}->PARSED: {result}")
    return result


def read_service_output(m, ffi):
    """Drain service UART TX; return payloads with trailing CR stripped."""
    raw = _drain_uart_channel(
        m,
        ffi,
        "SERIAL_SERVICE_OUT_status",
        "service_tx_message",
        "service_tx_message_length",
        m.UART_CHANNEL_SERVICE,
    )
    messages = []
    start = 0
    for i, byte in enumerate(raw):
        if byte == ord("\r"):
            messages.append(raw[start:i])
            start = i + 1
    return messages


"""
    Generate signal
    @return pairs of rising and falling edges
"""


def generate_signal(first_interval, second_interval, iterations):
    interval = first_interval + second_interval
    primary_edges = range(0, interval * iterations, interval)
    secondary_edges = range(first_interval, interval * iterations, interval)
    return zip(primary_edges, secondary_edges)
