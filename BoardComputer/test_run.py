import unittest
from math import ceil
from threading import Thread
from helpers import write_usart, read_usart, max6675_response, parse_nextion, exec_cycle, click, load

nextion_data = {"val": {}, "pic": {}, "txt": {}}

# This test class should be launched last to perform runtime tests
# i.e tests can affect whole runtime.


class testRun(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.bc, cls.ffi = load("main", "definitions.h")
        cls.nullptr = cls.ffi.NULL
        cls.usart_eot = int.to_bytes(cls.bc.USART_EOT,
                                     1,
                                     byteorder="little")
        cls.usart_eot = cls.usart_eot * cls.bc.USART_EOT_COUNT
        cls.bc.SYSTEM_run = False
        cls.bc.test()
        cls.bc.SYSTEM_status = cls.bc.SYSTEM_STATUS_OPERATIONAL
        write_usart(cls.bc, 0x88, b"")  # ping

    def setUp(self):
        exec_cycle(self.bc)
        self.bc.NEXTION_switch_page(self.bc.NEXTION_PAGEID_BOARD, 0)
        parse_nextion(self.bc, read_usart(self.bc), nextion_data)
        return super().setUp()

    def test_uiboard_maindisplay(self):
        # Cuz of chain of events, test needs to dive into countersfeed
        #Needs rework
        firstcomponent = self.bc.UIBOARD_maindisplay_components[0]
        TANK_ID = self.bc.SENSORSFEED_FEEDID_TANK
        SPEED_ID = self.bc.COUNTERSFEED_FEEDID_SPEED
        FUEL_ID = self.bc.COUNTERSFEED_FEEDID_FUELPS
        SPEED_AVG_ID = self.bc.SENSORSFEED_FEEDID_SPEED_AVG
        LP100_AVG_ID = self.bc.SENSORSFEED_FEEDID_LP100_AVG
        INJT_ID = self.bc.COUNTERSFEED_FEEDID_INJT
        ticks_100m = 540
        speed_modifier = ticks_100m/360

        def execute_activecomponent():
            active_component = self.bc.UIBOARD_maindisplay_activecomponent
            active_component.executable_component.execute()

        self.bc.UIBOARD_maindisplay_activecomponent = self.ffi.addressof(firstcomponent)
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
            execute_activecomponent()
            self.bc.USART_flush()
            self.bc.AVERAGE_clear(self.bc.AVERAGE_BUFFER_LP100)
            self.bc.AVERAGE_clear(self.bc.AVERAGE_BUFFER_SPEED)
            parse_nextion(self.bc, read_usart(self.bc),
                          nextion_data)
            self.assertEqual(nextion_data['txt']['mdv'], packet["assert"])

        # Fast checkup of averages, only format is tested
        # Average itself is tested in prerun
        # LP100_AVG
        self.bc.UIBOARD_switch_maindisplay()
        read_usart(self.bc)
        self.bc.SENSORSFEED_feed[LP100_AVG_ID] = 12 << 8
        execute_activecomponent()
        self.bc.USART_flush()
        parse_nextion(self.bc, read_usart(self.bc),
                      nextion_data)
        self.assertEqual(nextion_data['txt']['mdv'], "12.0")
        # SPEED_AVG
        self.bc.UIBOARD_switch_maindisplay()
        read_usart(self.bc)
        self.bc.SENSORSFEED_feed[SPEED_AVG_ID] = (90 << 8)
        execute_activecomponent()
        self.bc.USART_flush()
        parse_nextion(self.bc, read_usart(self.bc),
                      nextion_data)
        self.assertEqual(nextion_data['txt']['mdv'], " 90")
        self.bc.SENSORSFEED_feed[SPEED_AVG_ID] = 0
        # INJ_T
        self.bc.UIBOARD_switch_maindisplay()
        read_usart(self.bc)
        self.bc.COUNTERSFEED_feed[INJT_ID][0] = 150
        self.bc.UIBOARD_maindisplay_activecomponent.executable_component.execute()
        self.bc.USART_flush()
        parse_nextion(self.bc, read_usart(self.bc),
                      nextion_data)
        self.assertEqual(nextion_data['txt']['mdv'], " 1.1")
        # RANGE
        self.bc.UIBOARD_switch_maindisplay()
        read_usart(self.bc)
        self.bc.SENSORSFEED_feed[TANK_ID] = 70
        self.bc.SENSORSFEED_feed[LP100_AVG_ID] = 12 << 8
        execute_activecomponent()
        self.bc.USART_flush()
        parse_nextion(self.bc, read_usart(self.bc),
                      nextion_data)
        self.assertEqual(nextion_data['txt']['mdv'], " 583")

    def test_uiboard_modify_dbs(self):
        step = 5
        self.bc.NEXTION_set_brightness(0)
        self.assertEqual(self.bc.NEXTION_brightness, 0)
        for i in range(int(100/step)):
            self.bc.UIBOARDCONFIG_modify_dbs()
            exec_cycle(self.bc)
            read_usart(self.bc)

        self.assertEqual(self.bc.NEXTION_brightness, 100)
        for i in range(8):
            self.bc.UIBOARDCONFIG_modify_dbs()
            exec_cycle(self.bc)
            read_usart(self.bc)
            self.assertEqual(self.bc.NEXTION_brightness, 100)

        self.bc.UIBOARDCONFIG_modify_dbs()
        self.assertEqual(self.bc.NEXTION_brightness, 0)

    def test_nextion_set_brightness(self):
        self.bc.NEXTION_set_brightness(100)
        self.assertEqual(self.bc.NEXTION_brightness, 100)
        self.bc.NEXTION_set_brightness(0)
        self.assertEqual(self.bc.NEXTION_brightness, 0)

    def test_USART_passthrough_mode(self):
        write_usart(self.bc, None, b"DRAKJHSUYDGBNCJHGJKSHBDN")
        # Manualy check two opposite registers
        self.bc.UDRRX = 0xff
        self.bc.USART2_RX_vect()
        self.assertEqual(self.bc.UDR0, 0xff)
        self.bc.UDRRX = 0xfe
        self.bc.USART0_RX_vect()
        self.assertEqual(self.bc.UDR2, 0xfe)
        # Loop for watchdog
        for i in range(9):
            exec_cycle(self.bc)
        # Check if USART is not copying RX
        self.bc.UDRRX = 0xfe
        self.bc.USART0_RX_vect()
        self.assertNotEqual(self.bc.UDR2, 0xfe)

    def test_EGT(self):
        packets = [
            {"test": 0xffff, "result": self.bc.SENSORSFEED_EGT_STATUS_UNKN,
             "display": "----"},
            {"test": 0xfffd, "result": self.bc.SENSORSFEED_EGT_STATUS_OPEN,
             "display": "open"},
            {"test": 0x7720, "result": self.bc.SENSORSFEED_EGT_STATUS_VALUE,
             "display": " 953"}
        ]
        for test_packet in packets:
            response = max6675_response(self.bc, test_packet["test"])
            self.bc.SYSTEM_event_timer = 5
            next(response)
            self.assertEqual(self.bc.SENSORSFEED_EGT_transmission_status,
                            self.bc.SENSORSFEED_EGT_TRANSMISSION_HALF)
            next(response)
            self.assertEqual(self.bc.SENSORSFEED_EGT_transmission_status,
                            self.bc.SENSORSFEED_EGT_TRANSMISSION_READY)
            self.bc.SENSORSFEED_update_EGT()
            self.assertEqual(self.bc.SENSORSFEED_EGT_status,
                             test_packet["result"])
            self.bc.UIBOARD_update_EGT()
            self.bc.USART_flush()
            parse_nextion(self.bc, read_usart(self.bc), nextion_data)
            self.assertEqual(nextion_data["txt"]["egt"],
                             test_packet["display"])

        # Last test should bring value to sensors feed (bits 14 to 5 inclusive)
        self.assertEqual(self.bc.SENSORSFEED_feed[self.bc.SENSORSFEED_FEEDID_EGT],
                         0x3b9)

        self.bc.SYSTEM_event_timer = 0

    def test_timer(self):
        def formated_time(self):
            time = self.ffi.unpack(self.bc.TIMER_formated, 11)
            return time

        self.bc.TCNT2 = 14
        self.bc.TIMER_watch_zero()
        self.bc.TIMER_watch_toggle()
        #  Day Cycle
        for i in range(3601*8):
            self.bc.TIMER_update()
        self.assertEqual(formated_time(self), b" 1:00:01:00")

        for i in range(82799*8):
            self.bc.TIMER_update()
        self.assertEqual(formated_time(self), b" 0:00:00:00")
        #  Stopwatch
        self.bc.TIMER_next_watch()
        for i in range(5):
            self.bc.TIMER_update()
        # its not in counting state yet
        self.assertEqual(formated_time(self), b" 0:00:00:00")
        self.bc.TIMER_watch_toggle()
        # now is
        for i in range(5):
            self.bc.TIMER_update()
        self.assertEqual(formated_time(self), b" 0:00:00:62")
        # but watch should be counting too
        for i in range(5):
            self.bc.TIMER_update()

        self.bc.TIMER_next_watch()
        self.assertEqual(formated_time(self)[0:-3], b" 0:00:01")
        #  it is, now once again to stopwatch
        self.bc.TIMER_next_watch()
        self.assertEqual(formated_time(self), b" 0:00:01:25")
        #  nothing changed, zero stopwatch
        self.bc.TIMER_watch_zero()
        self.assertEqual(formated_time(self), b" 0:00:00:00")
        #  stopwatch at zero, check if watch is not affected
        self.bc.TIMER_next_watch()
        #  miliseconds are not used in watch(wont be updated)
        self.assertEqual(formated_time(self)[:-3], b" 0:00:01")


    def test_nextion_selection(self):
        selectionvalue = 100
        objname_length = self.bc.NEXTION_OBJNAME_LEN
        status_selected = self.bc.NEXTION_COMPONENTSTATUS_SELECTED
        decay_ticks = self.bc.NEXTION_SELECT_DECAY_TICKS

        name = self.ffi.new('const char['+str(objname_length)+']', b'tst')
        component = self.ffi.new("NEXTION_Component*")
        component.value_selected = selectionvalue
        component.name = name
        component.highlighttype = self.bc.NEXTION_HIGHLIGHTTYPE_IMAGE
        component = self.ffi.cast("void*", component)
        self.bc.NEXTION_set_componentstatus(component, status_selected)
        self.bc.USART_flush()

        parse_nextion(self.bc, read_usart(self.bc), nextion_data)
        self.assertEqual(int(nextion_data['pic']['tst']), selectionvalue)

    def test_nextion_selection_decay(self):
        defaultvalue = 50
        objname_length = self.bc.NEXTION_OBJNAME_LEN
        status_selected = self.bc.NEXTION_COMPONENTSTATUS_SELECTED
        decay_ticks = self.bc.NEXTION_SELECT_DECAY_TICKS

        name = self.ffi.new('const char['+str(objname_length)+']', b'tst')
        component = self.ffi.new("NEXTION_Component*")
        component.value_default = defaultvalue
        component.name = name
        component.highlighttype = self.bc.NEXTION_HIGHLIGHTTYPE_IMAGE
        component = self.ffi.cast("void*", component)
        self.bc.NEXTION_set_componentstatus(component, status_selected)

        for i in range(decay_ticks):
            self.bc.NEXTION_update_select_decay()
        self.bc.USART_flush()

        parse_nextion(self.bc, read_usart(self.bc), nextion_data)
        self.assertEqual(int(nextion_data['pic']['tst']), defaultvalue)

    def test_uinumpad_setup(self):
        testvalue = 100
        length = self.bc.DISPLAYLENGTH
        target = self.ffi.new("int16_t*", testvalue)
        self.bc.UINUMPAD_switch(target)
        stringvalue = self.ffi.unpack(
            self.bc.UINUMPAD_getstringvalue(), length).decode("ascii")
        expectedvalue = "   100"
        self.assertEqual(stringvalue, expectedvalue)

    def test_uinumpad_setup_minus(self):
        testvalue = -100
        length = self.bc.DISPLAYLENGTH
        target = self.ffi.new("int16_t*", testvalue)
        self.bc.UINUMPAD_switch(target)
        stringvalue = self.ffi.unpack(
            self.bc.UINUMPAD_getstringvalue(), length).decode("ascii")
        expectedvalue = "-  100"
        self.assertEqual(stringvalue, expectedvalue)

    def test_uinumpad_append(self):
        length = self.bc.DISPLAYLENGTH
        target = self.ffi.new("int16_t*", 0)
        self.bc.UINUMPAD_switch(target)
        self.bc.UINUMPAD_click_b1()
        self.bc.UINUMPAD_click_b2()
        self.bc.UINUMPAD_click_b3()
        self.bc.UINUMPAD_click_b4()
        stringvalue = self.ffi.unpack(
            self.bc.UINUMPAD_getstringvalue(), length).decode("ascii")
        expectedvalue = ''.join(
            [' ' for i in range(length)])[0:-4] + str(1234)
        self.assertEqual(stringvalue, expectedvalue)

    def test_uinumpad_delete(self):
        testvalue = 12345
        length = self.bc.DISPLAYLENGTH
        target = self.ffi.new("int16_t*", testvalue)
        self.bc.UINUMPAD_switch(target)
        self.bc.UINUMPAD_click_del()
        self.bc.UINUMPAD_click_del()
        stringvalue = self.ffi.unpack(
            self.bc.UINUMPAD_getstringvalue(), length).decode("ascii")
        expectedvalue = ''.join(
            [' ' for i in range(length)])[0:-3] + str(123)
        self.assertEqual(stringvalue, expectedvalue)

    def test_uinumpad_minus(self):
        length = self.bc.DISPLAYLENGTH
        target = self.ffi.new("int16_t*", 0)
        self.bc.UINUMPAD_switch(target)
        self.bc.UINUMPAD_click_mns()
        stringvalue = self.ffi.unpack(
            self.bc.UINUMPAD_getstringvalue(), length).decode("ascii")
        expectedvalue = "     0"
        self.assertEqual(stringvalue, expectedvalue)
        self.bc.UINUMPAD_click_b5()
        self.bc.UINUMPAD_click_mns()
        stringvalue = self.ffi.unpack(
            self.bc.UINUMPAD_getstringvalue(), length).decode("ascii")
        expectedvalue = "-    5"
        self.assertEqual(stringvalue, expectedvalue)

    def test_uinumpad_send(self):
        target = self.ffi.new("int16_t*", 100)
        self.bc.UINUMPAD_switch(target)
        self.bc.UINUMPAD_click_b0()
        self.bc.UINUMPAD_click_b0()
        self.bc.UINUMPAD_click_snd()
        target = int(self.ffi.cast("int16_t", target[0]))
        self.assertEqual(target, 10000)


if __name__ == "main":
    unittest.main()
