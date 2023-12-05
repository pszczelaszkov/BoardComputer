import re
import cffi
import importlib


class ModuleWrapper():
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
                    #cast cdata to byte array
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
    module = importlib.import_module('bin.' + filename)
    with open('test/generatedDefinitions/definitions.h') as definitions_file:
        ffi.cdef(definitions_file.read())
    return module.lib, ffi


def exec_cycle(module):
    module.SYSTEM_exec = True
    module.core()


def max6675_response(module, test_response):
    for i in range(2):
        module.SPDR0 = (test_response & 0xff00) >> 8  # MSB first
        test_response = test_response << 8
        module.SPI0_STC_vect()
        yield


def read_usart(module):
    response = bytearray()
    while module.USART_TX_message_length:
        response.append(module.UDR2)
        module.USART2_TX_vect()

    return response


def click(module, id):
    write_usart(module, 0x65, [0, id, 1])
    return read_usart(module)


def write_usart(module, header, message):
    usart_eot = int.to_bytes(module.USART_EOT,
                             1,
                             byteorder="little")
    usart_eot = usart_eot * module.USART_EOT_COUNT
    if(header is not None):
        usart_header = header.to_bytes(1, byteorder="little")
        message = usart_header + bytearray(message) + usart_eot
    else:
        message = bytearray(message) + usart_eot
    for byte in message:
        module.UDRRX = byte
        module.USART2_RX_vect()


def parse_nextion(module, stream, nextion_values):
    for message in stream.split(b'\xff\xff\xff'):
        message = message.decode(encoding="ASCII")
        if re.fullmatch("[a-z]+[0-9]*\\.val+=\\s*\\S+", message):
            # variable value e.g var.val=24
            variable = message.split('.')[0]
            value = message.split("=")[1]
            nextion_values["val"][variable] = value
        if re.fullmatch("[a-z]+[0-9]*\\.txt+=\"\\s*\\S+\"", message):
            # String variable value e.g var.txt=24
            variable = message.split('.')[0]
            value = message.split("=")[1]
            nextion_values["txt"][variable] = value[1:-1]
        if re.fullmatch("[a-z]+[0-9]*\\.pic+=\\S+", message):
            # picture id e.g var.pic=24
            variable = message.split('.')[0]
            value = message.split("=")[1]
            nextion_values["pic"][variable] = value

'''
    Generate signal
    @return pairs of rising and falling edges
'''
def generate_signal(first_interval, second_interval, iterations):
    interval = first_interval + second_interval
    primary_edges = range(0,interval*iterations,interval)
    secondary_edges = range(first_interval,interval*iterations,interval)
    return zip(primary_edges, secondary_edges)