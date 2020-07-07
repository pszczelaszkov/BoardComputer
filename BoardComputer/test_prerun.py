import unittest
import cffi
import random
from test_init import load

# This test class should be launched second to check one-cycle run.


class testInit(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.bc = load("main")
        cls.ffi = cffi.FFI()
        cls.nullptr = cls.ffi.NULL
        cls.bc.run = False
        cls.bc.main()
        random.seed()

    def test_scheduler(self):
        # Test queue is circular
        size = self.bc.SCHEDULER_LOW_PRIORITY_QUEUE_SIZE
        tasks = self.ffi.unpack(self.bc.SCHEDULER_low_priority_tasks, size)
        last = tasks[-1]
        nextptr = self.ffi.cast("void*", last.nextTask)
        for task in tasks:
            # Are tasks fid's properly initialized?
            self.assertEqual(task.fid, self.bc.LAST_cb)
            fptr = self.ffi.cast("void*", cffi.FFI().addressof(task))
            self.assertEqual(nextptr, fptr)
            nextptr = self.ffi.cast("void*", task.nextTask)

    def test_nextion(self):
        # EOT must be null-terminated triple 0xff
        eot = self.ffi.unpack(self.bc.NEXTION_eot, 4)
        for byte in eot[0:3]:
            self.assertEqual(byte, 0xff)
        self.assertEqual(eot[3], 0)

    def test_sensorsfeed(self):
        # Divide by 0 issue
        self.assertTrue(self.bc.SENSORSFEED_fuelmodifier)
        # After init, status should be ready
        self.assertEqual(self.bc.SENSORSFEED_status, self.bc.SENSORSFEED_READY)

    def test_USART(self):
        usart_eot = int.to_bytes(self.bc.USART_EOT,
                                 1,
                                 byteorder="little")
        usart_eot = usart_eot * self.bc.USART_EOT_COUNT
        usart_header = 0x01.to_bytes(1, byteorder="little")
        # Buffer must be empty at this point
        self.assertFalse(self.bc.USART_TX_message_length)
        # Test if USART is prepared for TX/RX
        self.assertTrue(self.bc.USART_TX_buffer_index,
                        self.bc.USART_TX_BUFFER_SIZE)
        self.assertLess(self.bc.USART_RX_buffer_index,
                        self.bc.USART_RX_BUFFER_SIZE)
        # Test raw funcionality
        message = usart_header + b"PING" + usart_eot
        for byte in message:
            self.bc.UDRRX = byte
            self.bc.USART_RXC_vect()
        # At this point usart_register should parse it
        self.bc.USART_register()
        response = bytearray()
        while self.bc.UDR != 0xff:
            response.append(self.bc.UDR)
            self.bc.USART_TXC_vect()
        self.assertEqual(response, b"PONG")
        self.bc.USART_TX_clear()

    def test_countersfeed_fuel(self):
        # Tricky one, timer overflows at uint16
        # Normal situation (previous value lower)
        fuelindex = self.bc.COUNTERSFEED_fuelps_index
        injectorinput = self.bc.COUNTERSFEED_injector_input
        self.bc.TCNT1 = 10000
        self.bc.PINA = injector_input  # Injector rising
        self.bc.PCINT0_vect()  # Simulate IRQ, should dump timestamp at rising
        self.bc.TCNT1 = 60000
        self.bc.PINA = injector_input   # All falling
        self.bc.PCINT0_vect()  # Simulate IRQ, should calc at falling
        self.bc.COUNTERSFEED_event_update()  # Move to front buffer
        self.assertEqual(self.bc.COUNTERSFEED_feed[fuelindex][0], 50000)
        # Overflow situation (previous value higher)
        self.bc.TCNT1 = 60000
        self.bc.PINA = injector_input
        self.bc.PCINT0_vect()
        self.bc.TCNT1 = 10000
        self.bc.PINA = injector_input
        self.bc.PCINT0_vect()
        for i in range(7):
            self.bc.COUNTERSFEED_event_update()  # Move again rolling back to 0
        self.assertEqual(self.bc.COUNTERSFEED_feed[fuelindex][0], 15535)
        self.assertFalse(self.bc.COUNTERSFEED_event_timer)

if __name__ == "main":
    unittest.main()
