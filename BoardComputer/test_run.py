import unittest
import cffi
import time
from math import ceil
from threading import Thread
from test_init import load
from helpers import write_usart, read_usart, parse_nextion, exec_cycle, click

nextion_data = {"val": {}, "pic": {}}
ADC = [10, 2, 3, 4, 5, 6, 7, 8]

# This test class should be launched last to perform runtime tests
# i.e tests can affect whole runtime.

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
        cls.bc.run = False
        cls.bc.main()
        cls.bc.prestart_routine()
        #clear usart from initial stuff
        read_usart(cls.bc)
        #cls.bcThread = Thread(target=cls.bc.main, daemon=True)
        #cls.bcThread.start()

    def test_analog(self):
        parse_nextion(self.bc, read_usart(self.bc), nextion_data)
        # ADC range -1, last is TANK input, its handled by maindisplay
        for i in range(self.bc.SENSORSFEED_ADC_CHANNELS):
            ADC_channel = self.bc.ADMUX & 0x0f
            self.bc.ADC = ADC[ADC_channel]
            self.bc.ADC_vect()

        exec_cycle(self.bc)
        parse_nextion(self.bc, read_usart(self.bc), nextion_data)
        for i in range(self.bc.SENSORSFEED_ADC_CHANNELS):
            if i != self.bc.SENSORSFEED_FEEDID_TANK:  # Tank is part of MD
                self.assertEqual(int(nextion_data["val"]["a"+str(i)]),
                                 self.bc.PROGRAMDATA_NTC_2200_INVERTED[ADC[i]])

    def test_nextion_maindisplay(self):
        # Cuz of chain of events, test needs to dive into countersfeed
        maindisplay_id = self.bc.NEXTION_COMPONENT_MAINDISPLAY
        TANK_ID = self.bc.SENSORSFEED_FEEDID_TANK
        SPEED_ID = self.bc.COUNTERSFEED_FEEDID_SPEED
        FUEL_ID = self.bc.COUNTERSFEED_FEEDID_FUELPS
        SPEED_AVG_ID = self.bc.SENSORSFEED_FEEDID_SPEED_AVG
        LP100_AVG_ID = self.bc.SENSORSFEED_FEEDID_LP100_AVG
        INJT_ID = self.bc.COUNTERSFEED_FEEDID_INJT
        ticks_100m = 540
        speed_modifier = ticks_100m/360
        # Fuelps is raw tickcount per injection, however speed is in kph.
        packets = [
            {"fuelps": 2700, "ccm": 700, "speed": 0, "assert": " 0.9"},
            {"fuelps": 4400, "ccm": 1000, "speed": 1, "assert": " 2.0"},
            #Speed added, should switch to liters per 100/km
            {"fuelps": 5000, "ccm": 1000, "speed": 100, "assert": " 2.3"},
            #Speed 0 should trigger zero div checkup and wont update value.
            {"fuelps": 5000, "ccm": 1000, "speed": 0, "assert": " 2.3"},
            #It should lead to lph mode as a result
            {"fuelps": 4400, "ccm": 1000, "speed": 0, "assert": " 2.0"},
            {"fuelps": 0, "ccm": 1, "speed": 0, "assert": " 0.0"},
            {"fuelps": 20000, "ccm": 1500, "speed": 0, "assert": "14.3"},
            # Hit limiter
            {"fuelps": 0xffff, "ccm": 14000, "speed": 0, "assert": "99.9"}
        ]
        # At Speed 0 there are liters per hour as current consumption
        for packet in packets:
            self.bc.SENSORSFEED_injector_ccm = packet["ccm"]
            self.bc.SENSORSFEED_speed_ticks_100m = ticks_100m
            self.bc.SENSORSFEED_initialize()  # recalc
            self.bc.COUNTERSFEED_feed[FUEL_ID][0] = packet["fuelps"]
            raw_speed = packet["speed"]*speed_modifier
            self.bc.COUNTERSFEED_feed[SPEED_ID][0] = int(raw_speed)
            self.bc.SENSORSFEED_update_fuel()
            self.bc.SENSORSFEED_update_speed()
            self.bc.NEXTION_maindisplay_renderer.render()
            self.bc.AVERAGE_clear(self.bc.AVERAGE_BUFFER_LP100)
            self.bc.AVERAGE_clear(self.bc.AVERAGE_BUFFER_SPEED)
            parse_nextion(self.bc, read_usart(self.bc),
                          nextion_data)
            self.assertEqual(nextion_data['val']['md'], packet["assert"])

        # Fast checkup of averages, only format is tested
        # Average itself is tested in prerun
        # LP100_AVG
        self.bc.NEXTION_switch_maindisplay()
        read_usart(self.bc)
        self.bc.SENSORSFEED_feed[LP100_AVG_ID] = 12 << 8
        self.bc.NEXTION_maindisplay_renderer.render()
        parse_nextion(self.bc, read_usart(self.bc),
                      nextion_data)
        self.assertEqual(nextion_data['val']['md'], "12.0")
        # SPEED_AVG
        self.bc.NEXTION_switch_maindisplay()
        read_usart(self.bc)
        self.bc.SENSORSFEED_feed[SPEED_AVG_ID] = (90 << 8)
        self.bc.NEXTION_maindisplay_renderer.render()
        parse_nextion(self.bc, read_usart(self.bc),
                      nextion_data)
        self.assertEqual(nextion_data['val']['md'], " 90")
        self.bc.SENSORSFEED_feed[SPEED_AVG_ID] = 0
        # INJ_T
        self.bc.NEXTION_switch_maindisplay()
        read_usart(self.bc)
        self.bc.COUNTERSFEED_feed[INJT_ID][0] = 150
        self.bc.NEXTION_maindisplay_renderer.render()
        parse_nextion(self.bc, read_usart(self.bc),
                      nextion_data)
        self.assertEqual(nextion_data['val']['md'], " 1.1")
        # RANGE
        self.bc.NEXTION_switch_maindisplay()
        read_usart(self.bc)
        self.bc.SENSORSFEED_feed[TANK_ID] = 70
        self.bc.SENSORSFEED_feed[LP100_AVG_ID] = 12 << 8
        self.bc.NEXTION_maindisplay_renderer.render()
        parse_nextion(self.bc, read_usart(self.bc),
                      nextion_data)
        self.assertEqual(nextion_data['val']['md'], " 583")

if __name__ == "main":
    unittest.main()
