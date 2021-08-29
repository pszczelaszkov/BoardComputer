import os
import re
import cffi
import importlib
import glob


def load(filename,definitions):
    # import and return resulting module
    module = importlib.import_module('bin.' + filename + '_')
    glob.glob('bin/' + filename + '_*')
    with open('src/'+ definitions) as definitions_file:
        ffi = cffi.FFI()
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


def write_usart(module, header, message, force_register=True):
    usart_eot = int.to_bytes(module.USART_EOT,
                             1,
                             byteorder="little")
    usart_eot = usart_eot * module.USART_EOT_COUNT
    usart_header = header.to_bytes(1, byteorder="little")
    # Test raw funcionality
    message = usart_header + bytearray(message) + usart_eot
    for byte in message:
        module.UDRRX = byte
        module.USART2_RX_vect()
    # At this point usart_register should parse it
    if force_register:
        module.USART_register()


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
