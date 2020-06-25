import unittest
import cffi
import re
import time
from collections import deque
from threading import Thread
from test_prerun import load

nextion_values = {}
ADC = [10, 2, 3, 4, 5, 6, 7, 8]


def exec_cycle(module):
    module.exec = True
    while module.exec is True:
        pass


def read_nextion_msg(module):
    # Nextion message ends with triple 0xff
    eot_count = 0
    pending = deque()
    while eot_count < 3:
        if not module.USART_TX_message_length:
            return -1

        if module.UDR != 0xff:
            if eot_count:
                eot_count = 0
                for i in range(len(pending)):
                    yield pending.popleft()
            else:
                yield module.UDR
        else:
            pending.append(module.UDR)
            eot_count = eot_count + 1
        module.USART_TXC_vect()


def usart_receive_nextion(module):
    buffer = bytearray()
    for byte in read_nextion_msg(module):
        if byte != -1:
            buffer.append(byte)
    message = buffer.decode(encoding="ASCII")
    if re.fullmatch("[a-z]+[0-9]*\\.val+=\\S+", message):
        # variable value e.g var.val=24
        variable = message.split('.')[0]
        value = message.split("=")[1]
        nextion_values[variable] = value


class testRun(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.bc = load("main")
        cls.ffi = cffi.FFI()
        cls.nullptr = cls.ffi.NULL
        cls.usart_eot = int.to_bytes(cls.bc.USART_EOT,
                                     1,
                                     byteorder="little")
        cls.usart_eot = cls.usart_eot * cls.bc.USART_EOT_COUNT
        cls.bcThread = Thread(target=cls.bc.main, daemon=True)
        cls.bcThread.start()

    def setUp(self):#cleanup usart
        usart_receive_nextion(self.bc)

    def test_analog(self):
        usart_receive_nextion(self.bc)

        while self.bc.SENSORSFEED_status != self.bc.SENSORSFEED_READY:# to pewnie trzeba bedzie zmienic przy countersfeed
            ADC_channel = self.bc.ADMUX & 0x0f
            self.bc.ADC = ADC[ADC_channel]
            self.bc.ADC_vect()

        exec_cycle(self.bc)
        usart_receive_nextion(self.bc)
        for i in range(self.bc.SENSORSFEED_ADC_CHANNELS):
            self.assertEqual(int(nextion_values["a"+str(i)]),
                             self.bc.PROGRAMDATA_NTC_2200_INVERTED[ADC[i]])


if __name__ == "main":
    unittest.main()
