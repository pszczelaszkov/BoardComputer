import pytest
import unittest
import pandas
from helpers import load, floattofp, ModuleWrapper

# This test class should be launched first to check global definitions

m, ffi = load("testmodule")
def cast_void(ffi, variable):
    return ffi.cast("void*", ffi.addressof(variable))

class testInit(unittest.TestCase):
    def test_sensorsfeed(self):
        # Cant be 0(0 division issue at init)
        self.assertTrue(m.SENSORSFEED_injector_ccm)
        self.assertTrue(m.SENSORSFEED_speed_ticks_100m)

    def test_countersfeed(self):
        self.assertTrue(m.COUNTERSFEED_TICKSPERSECOND)

    def test_average(self):
        self.assertTrue(m.AVERAGE_BUFFERS_SIZE)

    def test_timer_cycle_timestamp_to_cs(self):
        self.assertTrue(m.TIMER_getCENTISECONDSPERSTEP())
        self.assertTrue(m.TIMER_getRTC_REGISTER_WEIGHT())
        self.assertEqual(12, m.TIMER_cycle_timestamp_to_cs(15))
        self.assertEqual(5, m.TIMER_cycle_timestamp_to_cs(6))
        self.assertEqual(0, m.TIMER_cycle_timestamp_to_cs(0))

    def test_input(self):
        self.assertTrue(m.INPUT_KEY_LAST)
        self.assertTrue(m.INPUT_KEYSTATUS_RELEASED == 0)
        self.assertTrue(m.INPUT_KEYSTATUS_PRESSED == 1)
        self.assertTrue(m.INPUT_KEYSTATUS_HOLD > m.INPUT_KEYSTATUS_PRESSED)
        self.assertTrue(m.INPUT_KEYSTATUS_CLICK > m.INPUT_KEYSTATUS_HOLD)

    def test_nextion(self):
        # EOT must be null-terminated triple 0xff
        eot = ffi.unpack(m.NEXTION_eot, 4)
        for byte in eot[0:3]:
            self.assertEqual(byte, 0xFF)
        self.assertEqual(eot[3], 0)

    #move?
    def test_uiboard_maindisplay_order(self):
        # Must have default component and be circular
        initial = m.UIBOARD_maindisplay_activecomponent
        desired = m.UIBOARD_maindisplay_components[0]
        self.assertEqual(initial, desired)
        temp = initial.nextComponent
        for i in range(m.UIBOARD_MD_LAST):
            if temp == initial:
                break
            temp = temp.nextComponent

        self.assertEqual(initial, temp)

    def test_input_common_bck_conformance(self):
        image = m.NEXTION_HIGHLIGHTTYPE_IMAGE
        bckcomponent = m.NEXTION_common_bckcomponent
        self.assertEqual(bckcomponent.value_default, 28)
        self.assertEqual(bckcomponent.value_selected, 29)
        name = ffi.unpack(bckcomponent.name, m.NEXTION_OBJNAME_LEN)
        self.assertEqual(name, b"bck")
        self.assertEqual(bckcomponent.highlighttype, image)

    def test_utils_atoi(self):
        sample = [b"2", b"0", b"0", b"0", b"0"]
        testvalue = ffi.new("char[]", sample)
        self.assertEqual(m.UTILS_atoi(testvalue), 20000)

    def test_utils_atoi_minus(self):
        sample = [b"-", b"2", b"0", b"0", b"0", b"0"]
        testvalue = ffi.new("char[]", sample)
        self.assertEqual(m.UTILS_atoi(testvalue), -20000)

    def test_utils_u16toa_zero(self):
        buffer = ffi.new("char[6]")
        testvalue = 0
        expectedstring = b"0\x00"
        m.u16toa(testvalue, buffer)
        self.assertEqual(ffi.unpack(buffer, 2), expectedstring)

    def test_utils_u16toa(self):
        buffer = ffi.new("char[6]")
        testvalue = 65355
        expectedstring = b"65355\x00"
        m.u16toa(testvalue, buffer)
        self.assertEqual(ffi.unpack(buffer, 6), expectedstring)

    def test_utils_i32toa_zero(self):
        buffer = ffi.new("char[11]")
        testvalue = 0
        expectedstring = b"0\x00"
        m.i32toa(testvalue, buffer)
        self.assertEqual(ffi.unpack(buffer, 2), expectedstring)

    def test_utils_i32toa_positive(self):
        buffer = ffi.new("char[11]")
        testvalue = 2147483647
        expectedstring = b"2147483647\x00"
        m.i32toa(testvalue, buffer)
        self.assertEqual(ffi.unpack(buffer, 11), expectedstring)
    
    def test_utils_i32toa_negative(self):
        buffer = ffi.new("char[12]")
        testvalue = -2147483647
        expectedstring = b"-2147483647\x00"
        m.i32toa(testvalue, buffer)
        self.assertEqual(ffi.unpack(buffer, 12), expectedstring)

    def test_utils_rightconcat(self):
        buffer = ffi.new("char[7]")

        for i in range(7):
            buffer[i] = b"\x00"
        testvalue = 100
        expectedstring = b"\x00\x00\x00100\x00"
        m.rightconcat_short(buffer, testvalue, 6)
        self.assertEqual(ffi.unpack(buffer, 7), expectedstring)

        for i in range(7):
            buffer[i] = b"\x00"
        expectedstring = b"100\x00\x00\x00\x00"
        m.rightconcat_short(buffer, testvalue, 3)
        self.assertEqual(ffi.unpack(buffer, 7), expectedstring)

    def test_utils_rightnconcat(self):
        buffer = ffi.new("char[7]")
        testvalue = 10
        expectedstring = b"\x00\x00\x00\x0010\x00"
        m.rightnconcat_short(buffer, testvalue, 6, 3)
        self.assertEqual(ffi.unpack(buffer, 7), expectedstring)

    def test_utils_fp16toa_zero(self):
        buffer = ffi.new("char[7]")
        testvalue = 0
        expectedstring = b"\x000.00\x00\x00"
        m.fp16toa(testvalue, buffer, 2, 2)
        self.assertEqual(ffi.unpack(buffer, 7), expectedstring)

    def test_utils_fp16toa_minus_half(self):
        buffer = ffi.new("char[7]")
        testvalue = -128
        expectedstring = b"-0.50\x00\x00"
        m.fp16toa(testvalue, buffer, 2, 2)
        self.assertEqual(ffi.unpack(buffer, 7), expectedstring)

    def test_utils_fp16toa_minus(self):
        buffer = ffi.new("char[7]")
        testvalue = floattofp(-5.8, 8)
        expectedstring = b"-5.80\x00\x00"
        m.fp16toa(testvalue, buffer, 2, 2)
        self.assertEqual(ffi.unpack(buffer, 7), expectedstring)

    def test_utils_fp16toa_half(self):
        buffer = ffi.new("char[7]")
        testvalue = 128
        expectedstring = b"\x000.50\x00\x00"
        m.fp16toa(testvalue, buffer, 2, 2)
        self.assertEqual(ffi.unpack(buffer, 7), expectedstring)

    def test_utils_fp16toa_threeofthousand(self):
        buffer = ffi.new("char[7]")
        testvalue = 1
        expectedstring = b"\x000.003\x00"
        m.fp16toa(testvalue, buffer, 2, 3)
        self.assertEqual(ffi.unpack(buffer, 7), expectedstring)

    def test_utils_fp16toa_maxfractionlength(self):
        buffer = ffi.new("char[7]")
        testvalue = 1
        expectedstring = b"\x000.0039"
        m.fp16toa(testvalue, buffer, 2, 10)
        self.assertEqual(ffi.unpack(buffer, 7), expectedstring)


class TestBasicUIBOARD:
    def test_uiboard_MDcomponents_cohesion(self):
        image = m.NEXTION_HIGHLIGHTTYPE_IMAGE
        model = [
            [11, 22],
            [12, 23],
            [13, 24],
            [14, 19],
            [15, 20],
            [16, 21],
        ]  # default,selected
        zipped = zip(ffi.unpack(m.UIBOARD_maindisplay_components, 6), model)
        i = 0
        for target, model in zipped:
            msg = "Failed @ " + str(i)
            component = target.executable_component.component
            assert component.value_default == model[0]
            assert component.value_selected == model[1]
            name = ffi.unpack(component.name, m.NEXTION_OBJNAME_LEN)
            assert name == b"mds"
            assert component.highlighttype == image
            i = i + 1

        assert m.UIBOARD_maindisplay_activecomponent == m.UIBOARD_maindisplay_components[0]

class TestBasicTimer:
    @pytest.mark.parametrize("centiseconds,result,expected_format_flag",[
        (0,"0:0:0:0",m.FORMATFLAG_NONE),
        (100,"0:0:0:100",m.FORMATFLAG_CENTISECONDS),
        (200,"0:0:1:0",m.FORMATFLAG_SECONDS),
        (200*60,"0:1:0:0",m.FORMATFLAG_MINUTES),
        (200*3600,"1:0:0:0",m.FORMATFLAG_HOURS),
        (200*86399,"23:59:59:0",m.FORMATFLAG_HOURS),
        (200*86399+100,"23:59:59:100",m.FORMATFLAG_HOURS),
        (200*86400,"0:0:0:0",m.FORMATFLAG_HOURS),
    ])
    def test_timer_increment(self,centiseconds,result,expected_format_flag):
        watch = ffi.new("TIMER_watch*")
        resultformatflag = m.FORMATFLAG_NONE
        while True:
            clipped_centiseconds = min(centiseconds,0xff)
            resultformatflag = resultformatflag | m.TIMER_increment(ffi.cast("void*",watch), clipped_centiseconds)
            centiseconds = centiseconds - clipped_centiseconds
            if(centiseconds == 0):
                break

        timer = watch.timer
        assert [timer.hours,timer.minutes,timer.seconds,timer.centiseconds] == [int(i) for i in result.split(':')]
        assert expected_format_flag == resultformatflag

    @pytest.mark.parametrize("centiseconds,result,expected_format_flag",[
        (0,"23:59:59:200",m.FORMATFLAG_NONE),
        (100,"23:59:59:100",m.FORMATFLAG_CENTISECONDS),
        (200,"23:59:59:0",m.FORMATFLAG_CENTISECONDS),
        (201,"23:59:58:199",m.FORMATFLAG_SECONDS),
        (200*60,"23:59:0:0",m.FORMATFLAG_SECONDS),
        (200*3600,"23:0:0:0",m.FORMATFLAG_MINUTES),
        (200*86399,"0:0:1:0",m.FORMATFLAG_HOURS),
        (200*86399+100,"0:0:0:100",m.FORMATFLAG_HOURS),
        (200*86400,"0:0:0:0",m.FORMATFLAG_HOURS),
    ])
    def test_timer_decrement(self,centiseconds,result,expected_format_flag):
        watch = ffi.new("TIMER_watch*")
        watch.timer=[23,59,59,200,0]
        resultformatflag = m.FORMATFLAG_NONE
        while True:
            clipped_centiseconds = min(centiseconds,0xff)
            resultformatflag = resultformatflag | m.TIMER_decrement(ffi.cast("void*",watch), clipped_centiseconds)
            centiseconds = centiseconds - clipped_centiseconds
            if(centiseconds == 0):
                break

        timer = watch.timer
        assert [timer.hours,timer.minutes,timer.seconds,timer.centiseconds] == [int(i) for i in result.split(':')]
        assert expected_format_flag == resultformatflag

    @pytest.mark.parametrize("timer_value,format_flag,expected_formated_str",
    [
      ([0,0,0,0],m.FORMATFLAG_NONE,b"  :  :  :  "),
      ([0,0,0,0],m.FORMATFLAG_CENTISECONDS,b"  :  :  :00"),
      ([0,0,0,0],m.FORMATFLAG_SECONDS,b"  :  :00:00"),
      ([0,0,0,0],m.FORMATFLAG_MINUTES,b"  :00:00:00"),
      ([0,0,0,0],m.FORMATFLAG_HOURS,b" 0:00:00:00"),
      ([5,5,5,5<<1],m.FORMATFLAG_HOURS,b" 5:05:05:05"),
      ([10,10,10,10<<1],m.FORMATFLAG_HOURS,b"10:10:10:10"),
      ([12,34,56,78<<1],m.FORMATFLAG_HOURS,b"12:34:56:78"),
    ])
    def test_timer_format(self,timer_value,format_flag,expected_formated_str):
        timer_formated = ffi.new("TIMER_FORMATED_t*")
        timer_formated.c_str = b"  :  :  :  "
        watch = ffi.new("TIMER_watch*")

        timer = watch.timer
        timer.hours, timer.minutes, timer.seconds, timer.centiseconds = timer_value

        m.TIMER_format(ffi.cast("void*",watch),ffi.cast("void*",timer_formated),format_flag)
        assert ffi.unpack(timer_formated.c_str,11) == expected_formated_str

class TestBasicConfig:
    def test_entries_correct_value_range(self):
        truth_table = []
        truth_table.insert(m.CONFIG_ENTRY_SYSTEM_FACTORY_RESET,(0, 1))
        truth_table.insert(m.CONFIG_ENTRY_SYSTEM_ALWAYS_ON,(0, 1))
        truth_table.insert(m.CONFIG_ENTRY_SYSTEM_DISPLAYBRIGHTNESS,(0, 100))
        truth_table.insert(m.CONFIG_ENTRY_SYSTEM_BEEP_ON_CLICK,(0, 1))
        truth_table.insert(m.CONFIG_ENTRY_SENSORS_SIGNAL_PER_100KM,(0, 9999))
        truth_table.insert(m.CONFIG_ENTRY_SENSORS_INJECTORS_CCM,(0, 9999))

        minvalue = ffi.new("int32_t*")
        maxvalue = ffi.new("int32_t*")

        result = []
        i = 0
        for v in truth_table:
            m.CONFIG_get_entry_min_max_values(i,minvalue,maxvalue)
            i = i+1
            result.append((int(minvalue[0]),int(maxvalue[0])))

        assert truth_table == result
