import unittest
import cffi
import random
from helpers import write_usart, read_usart, load
# This test class should be launched second to check one-cycle run
# i.e tests direct functionality of subsystems.

def cast_void(ffi, variable):
    return ffi.cast("void*", cffi.FFI().addressof(variable))

class testPreRun(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.bc = load("main")
        cls.ffi = cffi.FFI()
        cls.nullptr = cls.ffi.NULL
        cls.bc.SYSTEM_run = False
        cls.bc.main()
        random.seed()

    def test_scheduler(self):
        # Test if fregister is fully initialized
        fregister = self.ffi.unpack(self.bc.SCHEDULER_fregister,
                                    self.bc.SCHEDULER_CALLBACK_LAST)
        for fptr in fregister:
            fptr = self.ffi.cast("void*", fptr)
            self.assertNotEqual(fptr, self.nullptr)
        # Test queue is circular
        size = self.bc.SCHEDULER_LOW_PRIORITY_QUEUE_SIZE
        tasks = self.ffi.unpack(self.bc.SCHEDULER_low_priority_tasks, size)
        last = tasks[-1]
        nextptr = self.ffi.cast("void*", last.nextTask)
        for task in tasks:
            # Are tasks fid's properly initialized?
            self.assertEqual(task.fid, self.bc.SCHEDULER_CALLBACK_LAST)
            fptr = self.ffi.cast("void*", cffi.FFI().addressof(task))
            self.assertEqual(nextptr, fptr)
            nextptr = self.ffi.cast("void*", task.nextTask)

    def test_nextion(self):
        # EOT must be null-terminated triple 0xff
        eot = self.ffi.unpack(self.bc.NEXTION_eot, 4)
        for byte in eot[0:3]:
            self.assertEqual(byte, 0xff)
        self.assertEqual(eot[3], 0)
        self.assertEqual(self.bc.NEXTION_maindisplay_renderer,
                         self.bc.NEXTION_maindisplay_renderers[0])

    def test_nextion_switch_maindisplay(self):
        # Must have default renderer and be circular
        initial = self.bc.NEXTION_maindisplay_renderer
        desired = self.bc.NEXTION_maindisplay_renderers[0]
        self.assertEqual(initial, desired)
        temp = initial.nextRenderer
        for i in range(self.bc.NEXTION_MD_LAST):
            if temp == initial:
                break
            temp = temp.nextRenderer

        self.assertEqual(initial, temp)

    def test_sensorsfeed(self):
        # Divide by 0 issue
        self.assertTrue(self.bc.SENSORSFEED_fuelmodifier)
        # After init, status should be ready
        self.assertEqual(self.bc.ADCMULTIPLEXER, 0)

    def test_USART(self):
        write_usart(self.bc, 0x01, b"PING")
        response = read_usart(self.bc)
        self.assertEqual(response[:4], b"PONG")
        self.bc.USART_TX_clear()

    def test_countersfeed_fuel(self):
        # Tricky one, timer overflows at uint16
        # Normal situation (previous value lower)
        fuelindex = self.bc.COUNTERSFEED_FEEDID_FUELPS
        injtindex = self.bc.COUNTERSFEED_FEEDID_INJT
        injector_input = self.bc.COUNTERSFEED_INPUT_INJECTOR
        self.bc.TCNT1 = 10000
        self.bc.PINB = injector_input  # Injector rising
        self.bc.PCINT0_vect()  # Simulate IRQ, should dump timestamp at rising
        self.bc.TCNT1 = 60000
        self.bc.PINB = self.bc.PINB ^ injector_input   # Injector falling back
        self.bc.PCINT0_vect()  # Simulate IRQ, should calc at falling
        self.bc.COUNTERSFEED_pushfeed(self.bc.COUNTERSFEED_FEEDID_FUELPS)
        self.assertEqual(self.bc.COUNTERSFEED_feed[fuelindex][0], 50000)
        self.assertEqual(self.bc.COUNTERSFEED_feed[injtindex][0], 50000)
        # Overflow situation (previous value higher)
        self.bc.TCNT1 = 60000
        self.bc.PINB = injector_input
        self.bc.PCINT0_vect()
        self.bc.TCNT1 = 10000
        self.bc.PINB = self.bc.PINB ^ injector_input
        self.bc.PCINT0_vect()
        self.bc.COUNTERSFEED_pushfeed(self.bc.COUNTERSFEED_FEEDID_FUELPS)
        self.assertEqual(self.bc.COUNTERSFEED_feed[fuelindex][0], 15535)
        self.assertEqual(self.bc.COUNTERSFEED_feed[injtindex][0], 15535)

    def test_average(self):
        for i in range(2):
            # Random is quite uniform, after all average will be near ~(0xffff/2)
            # But it is able to detect mismatch.
            localsum = 0
            for i in range(1, 2**16):
                testint = random.randint(0, 0xffff)
                localsum = localsum + testint
                average = self.bc.AVERAGE_addvalue(0, testint)
                self.assertEqual(average, localsum//i)
            # At this point average sum is full
            # Now it should switch to shifting instead of dividing
            sum_base = self.bc.AVERAGE_addvalue(0, random.randint(0, 0xffff))
            localsum = sum_base * 2**16
            for i in range(1, 2**16):
                testint = random.randint(0, 0xffff)
                localsum = localsum + (testint - sum_base)
                average = self.bc.AVERAGE_addvalue(0, testint)
                self.assertEqual(average, localsum//2**16)

            sum_base = self.bc.AVERAGE_addvalue(0, random.randint(0, 0xffff))
            localsum = sum_base * 2**16
            sum = self.bc.AVERAGE_buffers[0].sum
            self.assertEqual(sum, localsum)
            self.bc.AVERAGE_clear(0)

    def test_egt(self):
        self.assertTrue(self.bc.DDRB & self.bc.BIT0)  # CS
        self.assertTrue(self.bc.DDRB & self.bc.BIT4)  # SS
        self.assertTrue(self.bc.DDRB & self.bc.BIT7)  # SCK

    def test_timer(self):
        watch = self.bc.TIMER_watches[self.bc.TIMERTYPE_WATCH]
        watch = cffi.FFI().addressof(watch)
        self.assertEqual(watch.timer.watchstatus,
                         self.bc.TIMER_WATCHSTATUS_COUNTING)
        self.bc.TIMER_watch_toggle(watch)
        self.assertEqual(watch.timer.watchstatus,
                         self.bc.TIMER_WATCHSTATUS_STOP)
        stopwatch = self.bc.TIMER_watches[self.bc.TIMERTYPE_STOPWATCH]
        stopwatch = cffi.FFI().addressof(stopwatch)
        stopwatch.timer.miliseconds = 50
        stopwatch.timer.seconds = 2
        stopwatch.timer.minutes = 1
        stopwatch.timer.hours = 40
        self.bc.TIMER_watch_zero(stopwatch)
        self.assertEqual(stopwatch.timer.hours, 0)
        self.assertEqual(stopwatch.timer.miliseconds, 0)
        self.assertTrue(stopwatch.next_watch)
        self.assertTrue(self.bc.TIMER_active_watch)

    def test_input(self):
        keystatus = self.bc.INPUT_keystatus
        enter = self.bc.INPUT_KEY_ENTER
        down = self.bc.INPUT_KEY_DOWN
        released = self.bc.INPUT_KEYSTATUS_RELEASED
        pressed = self.bc.INPUT_KEYSTATUS_PRESSED
        hold = self.bc.INPUT_KEYSTATUS_HOLD
        click = self.bc.INPUT_KEYSTATUS_CLICK
        self.bc.INPUT_userinput(released, enter)
        self.assertEqual(keystatus[enter], released)

        self.bc.INPUT_userinput(pressed, enter)
        self.assertEqual(keystatus[enter], pressed)

        self.bc.INPUT_userinput(released, down)
        self.assertEqual(keystatus[enter], pressed)

        for i in range(8):
            self.bc.INPUT_update()
        self.assertEqual(keystatus[enter], hold)
        self.assertEqual(keystatus[down], released)

        self.bc.INPUT_userinput(released, enter)
        self.assertEqual(keystatus[enter], click)

if __name__ == "main":
    unittest.main()
