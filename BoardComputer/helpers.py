import os
import re
import cffi
import importlib
import glob


def load(filename):
    # load source code
    with open('src/' + filename + '.c') as source, open('src/definitions.h') as definitions:
        # pass source code to CFFI
        ffibuilder = cffi.FFI()
        ffibuilder.cdef(definitions.read())
        ffibuilder.set_source("src." + filename + '_', source.read())
        ffibuilder.compile()

        # import and return resulting module
        module = importlib.import_module('src.' + filename + '_')
        fileList = glob.glob('src/' + filename +'_*')
        for filePath in fileList:
            try:
                os.remove(filePath)
            except:
                print('Error while deleting file : ', filePath)
        return module.lib


def exec_cycle(module):
    module.exec = True
    module.core()
    #while module.exec is True:
     #   pass


def read_usart(module):
    response = bytearray()
    while module.USART_TX_message_length:
        response.append(module.UDR)
        module.USART_TXC_vect()

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
        module.USART_RXC_vect()
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
        if re.fullmatch("[a-z]+[0-9]*\\.pic+=\\S+", message):
            # picture id e.g var.pic=24
            variable = message.split('.')[0]
            value = message.split("=")[1]
            nextion_values["pic"][variable] = value
