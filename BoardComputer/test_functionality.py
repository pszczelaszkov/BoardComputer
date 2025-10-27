import cffi
import random
import pytest
import config
from helpers import (
    write_usart,
    read_usart,
    load,
    max6675_response,
    parse_nextion,
    read_nextion_output,
    ModuleWrapper,
    generate_signal,
    exec_cycle,
)

# This test class tests direct functionality of subsystems.


nextion_data = {"val": {}, "pic": {}, "txt": {}}


def cast_void(ffi, variable):
    return ffi.cast("void*", variable)


m, ffi = load("testmodule")
session = ModuleWrapper(m)

class TestParent:
    @classmethod
    def setup_class(cls):
        m.SYSTEM_run = False
        #Set system to always on to ignore board checking enable status, so we can set manualy.
        m.CONFIG_factory_default_reset()
        m.test()
        session.create_snapshot()

    @pytest.fixture(autouse=True)
    def snapshot_control(self):
        session.load_snapshot()
        yield

class TestPreRun(TestParent):

    def test_system_defaultstate(self):
        assert m.SYSTEM_status == m.SYSTEM_STATUS_OPERATIONAL

    @pytest.mark.parametrize("testvalue", [100, 0])
    def test_nextion_set_brightness(self, testvalue):
        m.NEXTION_set_brightness(testvalue)
        output = read_nextion_output(m, ffi)
        assert int(output["dim"]) == testvalue

    @pytest.mark.parametrize(
        "watchtype,formatedresult,ticks",
        [
            (m.TIMER_TIMERTYPE_STOPWATCH,b" 0:00:00:62",5,),
            (m.TIMER_TIMERTYPE_WATCH, b" 0:00:00:62",5,),
            (m.TIMER_TIMERTYPE_STOPWATCH, b"23:59:59:00", 86399 * 8),
            (m.TIMER_TIMERTYPE_WATCH, b"23:59:59:00", 86399 * 8),
            (m.TIMER_TIMERTYPE_STOPWATCH, b" 0:00:00:00", 86400 * 8),
            (m.TIMER_TIMERTYPE_WATCH, b" 0:00:00:00", 86400 * 8),
        ],
    )
    def test_watches_update(self, watchtype, formatedresult, ticks):
        def formated_time():
            time = ffi.unpack(m.TIMER_active_watch_formated.c_str, 11)
            return time

        m.TIMER_set_watch(watchtype)
        m.TIMER_clear_active_watch()
        m.TIMER_active_watch_toggle(0)
        for i in range(ticks):
            m.TIMER_update()
        assert formated_time() == formatedresult

        m.TIMER_clear_active_watch()
        m.TIMER_active_watch_toggle(0)

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

        m.TIMER_active_watch_toggle(0)
        check_other_notaffected(watch)
        assert watch.timer.watchstatus == m.TIMER_TIMERSTATUS_COUNTING

        m.TIMER_active_watch_toggle(0)
        check_other_notaffected(watch)
        assert watch.timer.watchstatus == m.TIMER_TIMERSTATUS_STOP

    def test_nextion_selection(self):
        selectionvalue = 100
        objname_length = m.NEXTION_OBJNAME_LEN
        status_selected = m.NEXTION_COMPONENTSELECTSTATUS_SELECTED

        name = ffi.new("const char[" + str(objname_length) + "]", b"tst")
        component = ffi.new("NEXTION_Component*")
        component.value_selected = selectionvalue
        component.name = name
        component.highlighttype = m.NEXTION_HIGHLIGHTTYPE_IMAGE
        component = ffi.cast("void*", component)
        m.NEXTION_set_component_select_status(component, status_selected)
        m.USART_flush()
        m.NEXTION_clear_selected_component()

        parse_nextion(m, read_usart(m), nextion_data)
        assert int(nextion_data["pic"]["tst"]) == selectionvalue

    def test_nextion_selection_decay(self):
        defaultvalue = 50
        objname_length = m.NEXTION_OBJNAME_LEN
        status_selected = m.NEXTION_COMPONENTSELECTSTATUS_SELECTED
        decay_ticks = m.NEXTION_SELECT_DECAY_TICKS

        name = ffi.new("const char[" + str(objname_length) + "]", b"tst")
        component = ffi.new("NEXTION_Component*")
        component.value_default = defaultvalue
        component.name = name
        component.highlighttype = m.NEXTION_HIGHLIGHTTYPE_IMAGE
        component = ffi.cast("void*", component)
        m.NEXTION_set_component_select_status(component, status_selected)

        for i in range(decay_ticks):
            m.NEXTION_update_select_decay()
        m.USART_flush()

        m.NEXTION_clear_selected_component()
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
            (m.SYSTEM_ALERT_NOTIFICATIONS_END, 0xF),
            (m.SYSTEM_ALERT_WARNINGS_END, 0xAA),
            (m.SYSTEM_ALERT_WARNINGS_END+1, 0xF0F),
        ],
    )
    def test_system_alert(self, alert, expected_pattern):
        PATTERN_LEN = 16
        result_pattern = 0
        m.SYSTEM_resetalert()
        m.SYSTEM_raisealert(alert)
        for i in range(PATTERN_LEN):
            m.SYSTEM_update()
            pinvalue = (m.PORTD & (1 << 7)) >> 7
            result_pattern = (pinvalue << i) | result_pattern
        assert result_pattern == expected_pattern

    @pytest.mark.parametrize(
        "alert",
        [
            (m.SYSTEM_ALERT_NOTIFICATIONS_END),
            (m.SYSTEM_ALERT_WARNINGS_END),
            (m.SYSTEM_ALERT_WARNINGS_END+1),
        ],
    )
    def test_SYSTEM_ALERT_SEVERITY_idle(self, alert):
        #At idle no alarm should be raised

        PATTERN_LEN = 16
        expected_pattern = 0x0
        result_pattern = 0
        m.SYSTEM_status = m.SYSTEM_STATUS_IDLE
        m.SYSTEM_resetalert()
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
        assert m.USART_RX_buffer_index == 0
        assert m.USART_TX_buffer_index == m.USART_TX_BUFFER_SIZE
        assert m.USART_eot_counter == 3

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

class TestConfig(TestParent):
    CONFIG_SIZE = ffi.sizeof("CONFIG_Config")

    def test_save_and_load(self):
        TEST_PATTERN = 0xCE
        config_sample = ffi.new("uint8_t[]", init=[TEST_PATTERN]*TestConfig.CONFIG_SIZE)
        m.CONFIG_saveconfig(cast_void(ffi,config_sample))
        loaded_config = ffi.new("uint8_t[]", init=[0]*TestConfig.CONFIG_SIZE)
        m.CONFIG_loadconfig(cast_void(ffi,loaded_config))
        assert(ffi.unpack(config_sample,TestConfig.CONFIG_SIZE) == ffi.unpack(loaded_config,TestConfig.CONFIG_SIZE))

    @pytest.mark.parametrize("entry,expected_min,expected_max",[
        (i,*config.ENTRY_VALIDATORS[
            config.config[i]["validator"]
            ])
         for i in range(m.CONFIG_ENTRY_LAST)
    ])
    def test_entries_has_correct_validation_min_max(self, entry, expected_min, expected_max):
        too_small_value = expected_min - 1
        too_big_value = expected_max + 1
        #Too small
        validator_result = m.CONFIG_validate_entry(entry, too_small_value)
        assert validator_result.value == expected_min
        assert validator_result.verdict == m.CONFIG_ENTRY_VERDICT_TOO_SMALL
        #Too big
        validator_result = m.CONFIG_validate_entry(entry, too_big_value)
        assert validator_result.value == expected_max
        assert validator_result.verdict == m.CONFIG_ENTRY_VERDICT_TOO_BIG
        #OK
        validator_result = m.CONFIG_validate_entry(entry, expected_min)
        assert validator_result.verdict == m.CONFIG_ENTRY_VERDICT_PASS
        validator_result = m.CONFIG_validate_entry(entry, expected_max)
        assert validator_result.verdict == m.CONFIG_ENTRY_VERDICT_PASS

    def test_entries_has_correct_validation_invalid(self):
        validator_result = m.CONFIG_validate_entry(m.CONFIG_ENTRY_LAST, 0)
        assert validator_result.verdict == m.CONFIG_ENTRY_VERDICT_INVALID_ENTRY

    @pytest.mark.parametrize("testval",[
        (0),
        (-1),
        (125),
        (-125),
        (-32000),
        (32000),
        (0x7f7f7f7f)
    ])
    def test_modify_entry_modifies_local_config(self,testval):
        def test_config_variable(var_name,var_size):
            #New config per each config entry
            c_testval = ffi.new("CONFIG_maxdata_t*", testval)
            test_config_left = ffi.new("CONFIG_Config*")
            test_config_right = cast_void(ffi,ffi.new("CONFIG_Config*"))

            lcpy = locals().copy()
            #execute code changing left value by variable and right side by modifying function
            overflow_detected = False
            try:
                exec(f"test_config_left.{var_name} = {testval}")
            except OverflowError:
                overflow_detected = True
            
            exec(f"modified_bytes = m.CONFIG_modify_entry(test_config_right, m.CONFIG_ENTRY_{var_name}, c_testval)", globals(), lcpy)
            assert lcpy["modified_bytes"] == var_size
            if not overflow_detected:
                '''
                    Compare if those 2 configs represents the same content only when var overflow was not detected,
                    when overflow was detected signess may be lost anyway.
                '''
                unpacked_raw_lconfig = ffi.unpack(ffi.cast("uint8_t*",test_config_left), TestConfig.CONFIG_SIZE)
                unpacked_raw_rconfig = ffi.unpack(ffi.cast("uint8_t*",test_config_right), TestConfig.CONFIG_SIZE)
                assert unpacked_raw_lconfig == unpacked_raw_rconfig

        for entry in config.config:
            test_config_variable(f'{entry["category"]}_{entry["name"]}', entry["size"])

    @pytest.mark.parametrize("testval",[
        (0),
        (-1),
        (125),
        (-125),
        (-32000),
        (32000),
        (0x7f7f7f7f)
    ])
    def test_modify_entry_modifies_persistent_config(self,testval):
        def test_config_variable(var_name,var_size):
            c_testval = ffi.new("CONFIG_maxdata_t*", testval)
            test_config_left = ffi.new("CONFIG_Config*")
            test_config_right = ffi.new("CONFIG_Config*")
            #write clean config
            m.PERSISTENT_MEMORY_write(0, cast_void(ffi,test_config_right), TestConfig.CONFIG_SIZE)

            lcpy = locals().copy()
            #execute code changing left value by variable and right side by modifying function
            overflow_detected = False
            try:
                exec(f"test_config_left.{var_name} = {testval}")
            except OverflowError:
                overflow_detected = True
            exec(f"modified_bytes = m.CONFIG_modify_entry(ffi.NULL, m.CONFIG_ENTRY_{var_name}, c_testval)", globals(), lcpy)
            assert lcpy["modified_bytes"] == var_size

            if not overflow_detected:
                '''
                    Compare if those 2 configs represents the same content only when var overflow was not detected,
                    when overflow was detected signess may be lost anyway.
                '''
                #read modified config
                m.PERSISTENT_MEMORY_read(0, cast_void(ffi,test_config_right), TestConfig.CONFIG_SIZE)

                #compare if those 2 configs represents the same content
                unpacked_raw_lconfig = ffi.unpack(ffi.cast("uint8_t*",test_config_left),TestConfig.CONFIG_SIZE)
                unpacked_raw_rconfig = ffi.unpack(ffi.cast("uint8_t*",test_config_right),TestConfig.CONFIG_SIZE)
                assert unpacked_raw_lconfig == unpacked_raw_rconfig

        for entry in config.config:
            test_config_variable(f'{entry["category"]}_{entry["name"]}', entry["size"])
    
    @pytest.mark.parametrize("testval",[
        (0),
        (-1),
        (125),
        (-125),
        (-32000),
        (32000),
        (0x7f7f7f7f)
    ])
    def test_read_config_entry_local(self,testval):
        def test_config_variable(var_name,var_size):

            c_testval = ffi.new("CONFIG_maxdata_t*")
            test_config = ffi.new("CONFIG_Config*")

            lcpy = locals().copy()
            overflow_detected = False
            try:
                exec(f"test_config.{var_name} = {testval}")
            except OverflowError:
                overflow_detected = True
            #execute code changing left value by variable and right side by modifying function
            exec(f"read_bytes = m.CONFIG_read_entry(cast_void(ffi,test_config), m.CONFIG_ENTRY_{var_name}, c_testval)", globals(), lcpy)
            assert lcpy["read_bytes"] == var_size
            
            if not overflow_detected:
                assert (c_testval[0] & testval) == c_testval[0] # Trickery to compare omiting sign interpretation

        for entry in config.config:
            test_config_variable(f'{entry["category"]}_{entry["name"]}', entry["size"])

    @pytest.mark.parametrize("testval",[
        (0),
        (-1),
        (125),
        (-125),
        (-32000),
        (32000),
        (0x7f7f7f7f)
    ])
    def test_read_config_entry_persistent(self,testval):
        def test_config_variable(var_name,var_size):

            c_testval = ffi.new("CONFIG_maxdata_t*")
            test_config = ffi.new("CONFIG_Config*")

            lcpy = locals().copy()
            overflow_detected = False
            try:
                exec(f"test_config.{var_name} = {testval}")
            except OverflowError:
                overflow_detected = True
            #put config into persistent memory
            m.PERSISTENT_MEMORY_write(0, cast_void(ffi,test_config), TestConfig.CONFIG_SIZE)
            lcpy = locals().copy()
            #execute code reading variable from persistent memory(config == null)
            exec(f"read_bytes = m.CONFIG_read_entry(ffi.NULL, m.CONFIG_ENTRY_{var_name}, c_testval)", globals(), lcpy)
            assert lcpy["read_bytes"] == var_size
            if not overflow_detected:
                assert (c_testval[0] & testval) == c_testval[0] # Trickery to compare omiting sign interpretation

        for entry in config.config:
            test_config_variable(f'{entry["category"]}_{entry["name"]}', entry["size"])

    @pytest.mark.parametrize(
        "entry,defaultvalue",
        [
            (i, config.config[i]["default"]) for i in range(m.CONFIG_ENTRY_LAST)
        ]
    )
    def test_factory_reset(self,entry,defaultvalue):
        readval = ffi.cast("CONFIG_maxdata_t*",ffi.new("CONFIG_maxdata_t*"))
        m.CONFIG_factory_default_reset()
        m.CONFIG_read_entry(ffi.NULL, entry, readval)
        assert defaultvalue == readval[0]

    @pytest.mark.parametrize("entry,minval,maxval",[
        (i,*config.ENTRY_VALIDATORS[
            config.config[i]["validator"]
            ])
         for i in range(m.CONFIG_ENTRY_LAST)
    ])
    def test_sanitize_config(self,entry,minval,maxval):
        too_small_value = ffi.new("CONFIG_maxdata_t*",minval-1)
        too_big_value = ffi.new("CONFIG_maxdata_t*",maxval+1)
        readval = ffi.new("CONFIG_maxdata_t*")

        SYSTEM_config_ptr = ffi.addressof(m.SYSTEM_config)
        #Check value sanitized when too small
        m.CONFIG_modify_entry(SYSTEM_config_ptr, entry, too_small_value)
        assert 1 == m.CONFIG_sanitize_config(SYSTEM_config_ptr)
        m.CONFIG_read_entry(SYSTEM_config_ptr, entry, readval)
        assert readval[0] in (minval,maxval) # Due to signedness of variable it may overflow so check any max val

        #Check value sanitized when too big
        m.CONFIG_modify_entry(SYSTEM_config_ptr, entry, too_big_value)
        assert 1 == m.CONFIG_sanitize_config(SYSTEM_config_ptr)
        m.CONFIG_read_entry(SYSTEM_config_ptr, entry, readval)
        assert readval[0] in (minval,maxval) # Due to signedness of variable it may overflow so check any max val

