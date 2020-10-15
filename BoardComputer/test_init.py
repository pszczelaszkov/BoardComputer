import unittest
import cffi
from helpers import load
# This test class should be launched first to check global definitions





class testInit(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.bc = load("main")
        cls.ffi = cffi.FFI()
        cls.nullptr = cls.ffi.NULL

    def test_USART(self):
        # Test buffers and counters are at 0
        self.assertFalse(self.bc.USART_RX_buffer_index)
        self.assertFalse(self.bc.USART_eot_counter)

    def test_sensorsfeed(self):
        # Cant be 0(0 division issue at init)
        self.assertTrue(self.bc.SENSORSFEED_injector_ccm)
        self.assertTrue(self.bc.SENSORSFEED_speed_ticks_100m)

    def test_countersfeed(self):
        self.assertTrue(self.bc.COUNTERSFEED_TICKSPERSECOND)

    def test_average(self):
        self.assertTrue(self.bc.AVERAGE_BUFFERS_SIZE)


if __name__ == "main":
    unittest.main()
