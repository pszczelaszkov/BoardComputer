import unittest
import cffi
from test_prerun import load


class testInit(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.bc = load("main")
        cls.ffi = cffi.FFI()
        cls.nullptr = cls.ffi.NULL
        cls.bc.run = False
        cls.bc.main()

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

if __name__ == "main":
    unittest.main()