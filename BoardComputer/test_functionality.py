import cffi
import random
import pytest
from helpers import (
    write_usart,
    read_usart,
    load,
    max6675_response,
    parse_nextion,
    ModuleWrapper,
    generate_signal,
    exec_cycle,
)

# This test class tests direct functionality of subsystems.


nextion_data = {"val": {}, "pic": {}, "txt": {}}


def cast_void(ffi, variable):
    return ffi.cast("void*", cffi.FFI().addressof(variable))


m, ffi = load("testmodule")
session = ModuleWrapper(m)


class TestPreRun:
    @classmethod
    def setup_class(cls):
        m.SYSTEM_run = False
        m.test()
        session.create_snapshot()

    @pytest.fixture(autouse=True)
    def snapshot_control(self):
        session.load_snapshot()
        yield

    @pytest.mark.parametrize("testvalue", [100, 0])
    def test_nextion_set_brightness(self, testvalue):
        m.NEXTION_set_brightness(testvalue)
        assert m.NEXTION_brightness == testvalue

    @pytest.mark.parametrize(
        "watchtype,formatedresult,ticks",
        [
            (
                m.TIMER_TIMERTYPE_STOPWATCH,
                b" 0:00:00:62",
                5,
            ),  # miliseconds should work on stopwatch
            (
                m.TIMER_TIMERTYPE_WATCH,
                b" 0:00:00:00",
                5,
            ),  # miliseconds should be ignored on watch
            (m.TIMER_TIMERTYPE_WATCH, b" 0:00:30:00", 30 * 8),  # 30sec
            (m.TIMER_TIMERTYPE_WATCH, b" 0:01:00:00", 60 * 8),  # 1 min
            (m.TIMER_TIMERTYPE_WATCH, b" 1:00:00:00", 3600 * 8),  #  1 h
            (m.TIMER_TIMERTYPE_WATCH, b"23:59:59:00", 86399 * 8),  # Almost day
            (m.TIMER_TIMERTYPE_WATCH, b" 0:00:00:00", 86400 * 8),  # Day cycle
        ],
    )
    def test_watches_update(self, watchtype, formatedresult, ticks):
        def formated_time():
            time = ffi.unpack(m.TIMER_formated, 11)
            return time

        m.TIMER_set_watch(watchtype)
        m.TIMER_clear_active_watch()
        m.TIMER_active_watch_toggle()
        for i in range(ticks):
            m.TIMER_update()
        assert formated_time() == formatedresult

        m.TIMER_clear_active_watch()
        m.TIMER_active_watch_toggle()

    def test_watch_order(self):
        order = [
            m.TIMER_TIMERTYPE_WATCH,
            m.TIMER_TIMERTYPE_STOPWATCH,
            m.TIMER_TIMERTYPE_WATCH,
        ]

        for watchtype in order:
            active = m.TIMER_getactive_watch()
            expected = m.TIMER_get_watch(watchtype)
            assert active == expected
            m.TIMER_next_watch()

    @pytest.mark.parametrize(
        "targettype", [(m.TIMER_TIMERTYPE_WATCH), (m.TIMER_TIMERTYPE_STOPWATCH)]
    )
    def test_activewatch_toggle_while_other_not_affected(self, targettype):
        def check_other_notaffected(target):
            for watchtype in range(m.TIMER_TIMERTYPE_LAST):
                watch = m.TIMER_get_watch(watchtype)
                if watch != target:
                    assert watch.timer.watchstatus == m.TIMER_TIMERSTATUS_ZERO

        for watchtype in range(m.TIMER_TIMERTYPE_LAST):
            m.TIMER_set_watch(watchtype)
            m.TIMER_clear_active_watch()

        m.TIMER_set_watch(targettype)
        watch = m.TIMER_get_watch(targettype)
        assert watch.timer.watchstatus == m.TIMER_TIMERSTATUS_ZERO

        m.TIMER_active_watch_toggle()
        check_other_notaffected(watch)
        assert watch.timer.watchstatus == m.TIMER_TIMERSTATUS_COUNTING

        m.TIMER_active_watch_toggle()
        check_other_notaffected(watch)
        assert watch.timer.watchstatus == m.TIMER_TIMERSTATUS_STOP

    def test_nextion_selection(self):
        selectionvalue = 100
        objname_length = m.NEXTION_OBJNAME_LEN
        status_selected = m.NEXTION_COMPONENTSTATUS_SELECTED

        name = ffi.new("const char[" + str(objname_length) + "]", b"tst")
        component = ffi.new("NEXTION_Component*")
        component.value_selected = selectionvalue
        component.name = name
        component.highlighttype = m.NEXTION_HIGHLIGHTTYPE_IMAGE
        component = ffi.cast("void*", component)
        m.NEXTION_set_componentstatus(component, status_selected)
        m.USART_flush()

        parse_nextion(m, read_usart(m), nextion_data)
        assert int(nextion_data["pic"]["tst"]) == selectionvalue

    def test_nextion_selection_decay(self):
        defaultvalue = 50
        objname_length = m.NEXTION_OBJNAME_LEN
        status_selected = m.NEXTION_COMPONENTSTATUS_SELECTED
        decay_ticks = m.NEXTION_SELECT_DECAY_TICKS

        name = ffi.new("const char[" + str(objname_length) + "]", b"tst")
        component = ffi.new("NEXTION_Component*")
        component.value_default = defaultvalue
        component.name = name
        component.highlighttype = m.NEXTION_HIGHLIGHTTYPE_IMAGE
        component = ffi.cast("void*", component)
        m.NEXTION_set_componentstatus(component, status_selected)

        for i in range(decay_ticks):
            m.NEXTION_update_select_decay()
        m.USART_flush()

        parse_nextion(m, read_usart(m), nextion_data)
        assert int(nextion_data["pic"]["tst"]) == defaultvalue

    def test_egt_pin_configuration(self):
        assert m.DDRB & m.BIT0  # CS
        assert m.DDRB & m.BIT4  # SS
        assert m.DDRB & m.BIT7  # SCK

    @pytest.mark.parametrize(
        "msg,status",
        [
            (0xFFFF, m.SENSORSFEED_EGT_STATUS_UNKN),
            (0xFFFD, m.SENSORSFEED_EGT_STATUS_OPEN),
            (0x7720, m.SENSORSFEED_EGT_STATUS_VALUE),
        ],
    )
    def test_MAX6675_status_readout(self, msg, status):
        response = max6675_response(m, msg)
        next(response)
        assert (
            m.SENSORSFEED_EGT_transmission_status == m.SENSORSFEED_EGT_TRANSMISSION_HALF
        )
        next(response)
        assert (
            m.SENSORSFEED_EGT_transmission_status
            == m.SENSORSFEED_EGT_TRANSMISSION_READY
        )
        m.SENSORSFEED_update_EGT()
        assert m.SENSORSFEED_EGT_status == status

    @pytest.mark.parametrize(
        "msg,value",
        [
            (0x0, 0),
            (0xC80, 100),
            (0x7FE0, 1023),
            (0x7FE8, 1023),
        ],
    )
    def test_MAX6675_value_readout(self, msg, value):
        response = max6675_response(m, msg)
        next(response)
        next(response)
        m.SENSORSFEED_update_EGT()
        assert m.SENSORSFEED_feed[m.SENSORSFEED_FEEDID_EGT] == value

    @pytest.mark.parametrize(
        "alert,expected_pattern",
        [
            (m.SYSTEM_ALERT_NOTIFICATION, 0xF),
            (m.SYSTEM_ALERT_WARNING, 0xAA),
            (m.SYSTEM_ALERT_CRITICAL, 0xF0F),
        ],
    )
    def test_system_alert(self, alert, expected_pattern):
        PATTERN_LEN = 16
        result_pattern = 0
        m.SYSTEM_raisealert(alert)
        for i in range(PATTERN_LEN):
            m.SYSTEM_update()
            pinvalue = (m.PORTD & (1 << 7)) >> 7
            result_pattern = (pinvalue << i) | result_pattern
        assert result_pattern == expected_pattern

    @pytest.mark.parametrize(
        "alert",
        [
            (m.SYSTEM_ALERT_NOTIFICATION),
            (m.SYSTEM_ALERT_WARNING),
            (m.SYSTEM_ALERT_CRITICAL),
        ],
    )
    def test_system_alert_idle(self, alert):
        PATTERN_LEN = 16
        expected_pattern = 0x0
        result_pattern = 0
        m.SYSTEM_status = m.SYSTEM_STATUS_IDLE
        m.SYSTEM_raisealert(alert)
        for i in range(PATTERN_LEN):
            m.SYSTEM_update()
            pinvalue = (m.PORTD & (1 << 7)) >> 7
            result_pattern = (pinvalue << i) | result_pattern

        # CLEAN
        m.SYSTEM_status = m.SYSTEM_STATUS_OPERATIONAL
        for i in range(PATTERN_LEN):
            m.SYSTEM_update()

        assert result_pattern == expected_pattern

    def test_input_keystatus(self):
        keystatus = m.INPUT_keystatus
        enter = m.INPUT_KEY_ENTER
        down = m.INPUT_KEY_DOWN
        released = m.INPUT_KEYSTATUS_RELEASED
        pressed = m.INPUT_KEYSTATUS_PRESSED
        hold = m.INPUT_KEYSTATUS_HOLD
        click = m.INPUT_KEYSTATUS_CLICK
        nonecomponent = m.INPUT_COMPONENT_NONE

        m.INPUT_userinput(released, enter, nonecomponent)
        assert keystatus[enter] == released

        m.INPUT_userinput(pressed, enter, nonecomponent)
        assert keystatus[enter] == pressed

        m.INPUT_userinput(released, down, nonecomponent)
        assert keystatus[enter] == pressed
        assert keystatus[down] == released

        for _ in range(hold):
            m.INPUT_update()

        assert keystatus[enter] == hold
        assert keystatus[down] == released

        m.INPUT_userinput(released, enter, nonecomponent)
        assert keystatus[enter] == released

        m.INPUT_userinput(pressed, enter, nonecomponent)
        m.INPUT_userinput(released, enter, nonecomponent)
        assert keystatus[enter] == click

    def test_average(self):
        for i in range(2):
            # Random is quite uniform, after all average will be near ~(0xffff/2)
            # But it is able to detect mismatch.
            localsum = 0
            for i in range(1, 2**16):
                testint = random.randint(0, 0xFFFF)
                localsum = localsum + testint
                average = m.AVERAGE_addvalue(0, testint)
                assert average == localsum // i
            # At this point average sum is full
            # Now it should switch to shifting instead of dividing
            sum_base = m.AVERAGE_addvalue(0, random.randint(0, 0xFFFF))
            localsum = sum_base * 2**16
            for i in range(1, 2**16):
                testint = random.randint(0, 0xFFFF)
                localsum = localsum + (testint - sum_base)
                average = m.AVERAGE_addvalue(0, testint)
                assert average == localsum // 2**16

            sum_base = m.AVERAGE_addvalue(0, random.randint(0, 0xFFFF))
            localsum = sum_base * 2**16
            sum = m.AVERAGE_buffers[0].sum
            assert sum == localsum
            m.AVERAGE_clear(0)

    @pytest.mark.parametrize(
        "highlevel_time,lowlevel_time",
        [
            (1, 5),
            (5, 1),
            (100, 200),
            (1000, 100),
            (20000, 20000),
            (30000, 40000),
            (65535, 200),
            (200, 65535),
        ],
    )
    def test_countersfeed_fuel_signal(self, highlevel_time, lowlevel_time):
        SAMPLES = 1000
        IRQ = m.PCINT0_vect
        fuelindex = m.COUNTERSFEED_FEEDID_FUELPS
        injtindex = m.COUNTERSFEED_FEEDID_INJT
        injector_input = m.COUNTERSFEED_INPUT_INJECTOR
        m.COUNTERSFEED_feed[fuelindex] = 0
        m.COUNTERSFEED_feed[injtindex] = 0
        for rising_edge, falling_edge in generate_signal(
            highlevel_time, lowlevel_time, SAMPLES
        ):
            m.TCNT1 = rising_edge & 0xFFFF
            m.PINB = injector_input
            IRQ()
            m.TCNT1 = falling_edge & 0xFFFF
            m.PINB = m.PINB ^ injector_input
            IRQ()
        assert m.COUNTERSFEED_feed[injtindex] == highlevel_time
        assert m.COUNTERSFEED_feed[fuelindex] == 0

    @pytest.mark.parametrize(
        "highlevel_time,lowlevel_time",
        [
            (1, 5),
            (5, 1),
            (100, 200),
            (655, 655),
        ],
    )
    def test_countersfeed_fuel(self, highlevel_time, lowlevel_time):
        SAMPLES = 100
        IRQ = m.PCINT0_vect
        fuelindex = m.COUNTERSFEED_FEEDID_FUEL
        injector_input = m.COUNTERSFEED_INPUT_INJECTOR
        total_time = highlevel_time * SAMPLES
        m.COUNTERSFEED_feed[fuelindex] = 0
        for rising_edge, falling_edge in generate_signal(
            highlevel_time, lowlevel_time, SAMPLES
        ):
            m.TCNT1 = rising_edge & 0xFFFF
            m.PINB = injector_input
            IRQ()
            m.TCNT1 = falling_edge & 0xFFFF
            m.PINB = m.PINB ^ injector_input
            IRQ()
        assert m.COUNTERSFEED_feed[fuelindex] == total_time

    def test_sensorsfeed(self):
        # Divide by 0 issue
        assert m.SENSORSFEED_fuelmodifier

    @pytest.mark.parametrize(
        "adcinput", [0xFE, 0xA5, 0x45, 0x67, 0x50, 0xFF, 0x00, 0xAA]
    )
    def test_analog(self, adcinput):
        # Magic numbers, totally random
        for i in range(m.SENSORSFEED_ADC_CHANNELS):
            ADC_channel = m.ADMUX & 0x0F
            m.ADC = adcinput
            m.ADC_vect()
            assert adcinput == m.SENSORSFEED_feed[ADC_channel]

    def test_analog_full(self):
        ADC_channel = m.ADMUX & 0x0F
        testvalue = 0x3FF
        m.ADC = testvalue
        m.ADC_vect()
        assert 0 == m.SENSORSFEED_feed[ADC_channel]

    def test_USART(self):
        write_usart(m, 0x01, b"PING")
        response = read_usart(m)
        assert response[:4] == b"PONG"
        m.USART_TX_clear()

    def test_USART_passthrough_mode(self):
        write_usart(m, None, b"DRAKJHSUYDGBNCJHGJKSHBDN")
        # Manualy check two opposite registers
        m.UDRRX = 0xFF
        m.USART2_RX_vect()
        assert m.UDR0 == 0xFF
        m.UDRRX = 0xFE
        m.USART0_RX_vect()
        assert m.UDR2 == 0xFE
        # Loop for watchdog
        for i in range(9):
            exec_cycle(m)
        # Check if USART is not copying RX
        m.UDRRX = 0xFE
        m.USART0_RX_vect()
        assert m.UDR2 != 0xFE
