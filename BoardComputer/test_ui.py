import pytest
from helpers import load, ModuleWrapper, read_nextion_output, fptofloat, floattofp

m, ffi = load("testmodule")
session = ModuleWrapper(m)

def cast_void(variable):
    return ffi.cast("void*", variable)

class TestBoardUI:
    @classmethod
    def setup_class(cls):
        m.SYSTEM_run = False
        m.CONFIG_factory_default_reset()
        m.test()

    @pytest.fixture(autouse=True)
    def clear(self):
        m.NEXTION_clear_selected_component()
        m.USART_TX_clear()
        m.NEXTION_handler_ready(m.NEXTION_VERSION)

    @pytest.mark.parametrize(
        "component,inputdata,expectedstring",
        [
            (
                m.UIBOARD_MD_LPH,
                {m.SENSORSFEED_FEEDID_LPH: 100 << 8},
                "99.9",
            ),
            (
                m.UIBOARD_MD_LPH,
                {m.SENSORSFEED_FEEDID_LPH: 10 << 8},
                "10.0",
            ),
            (
                m.UIBOARD_MD_LPH,
                {m.SENSORSFEED_FEEDID_LPH: 1 << 8},
                " 1.0",
            ),
            (
                m.UIBOARD_MD_LPH,
                {m.SENSORSFEED_FEEDID_LPH: 0},
                " 0.0",
            ),
            (
                m.UIBOARD_MD_LPH,
                {m.SENSORSFEED_FEEDID_LPH: 129},
                " 0.5",
            ),
            (
                m.UIBOARD_MD_LPH,
                {m.SENSORSFEED_FEEDID_LPH: 255},
                " 0.9",
            ),
            (
                m.UIBOARD_MD_LP100,
                {
                    m.SENSORSFEED_FEEDID_LP100: 100 << 8,
                    m.SENSORSFEED_FEEDID_SPEED: True,
                },
                "99.9",
            ),
            (
                m.UIBOARD_MD_LP100,
                {
                    m.SENSORSFEED_FEEDID_LP100: 10 << 8,
                    m.SENSORSFEED_FEEDID_SPEED: True,
                },
                "10.0",
            ),
            (
                m.UIBOARD_MD_LP100,
                {
                    m.SENSORSFEED_FEEDID_LP100: 1 << 8,
                    m.SENSORSFEED_FEEDID_SPEED: True,
                },
                " 1.0",
            ),
            (
                m.UIBOARD_MD_LP100,
                {
                    m.SENSORSFEED_FEEDID_LP100: 0,
                    m.SENSORSFEED_FEEDID_SPEED: True,
                },
                " 0.0",
            ),
            (
                m.UIBOARD_MD_LP100,
                {
                    m.SENSORSFEED_FEEDID_LP100: 129,
                    m.SENSORSFEED_FEEDID_SPEED: True,
                },
                " 0.5",
            ),
            (
                m.UIBOARD_MD_LP100,
                {
                    m.SENSORSFEED_FEEDID_LP100: 255,
                    m.SENSORSFEED_FEEDID_SPEED: True,
                },
                " 0.9",
            ),
            (
                m.UIBOARD_MD_LP100_AVG,
                {m.SENSORSFEED_FEEDID_LP100_AVG: 100 << 8},
                "99.9",
            ),
            (
                m.UIBOARD_MD_LP100_AVG,
                {m.SENSORSFEED_FEEDID_LP100_AVG: 10 << 8},
                "10.0",
            ),
            (
                m.UIBOARD_MD_LP100_AVG,
                {m.SENSORSFEED_FEEDID_LP100_AVG: 1 << 8},
                " 1.0",
            ),
            (
                m.UIBOARD_MD_LP100_AVG,
                {m.SENSORSFEED_FEEDID_LP100_AVG: 0},
                " 0.0",
            ),
            (
                m.UIBOARD_MD_LP100_AVG,
                {m.SENSORSFEED_FEEDID_LP100_AVG: 129},
                " 0.5",
            ),
            (
                m.UIBOARD_MD_LP100_AVG,
                {m.SENSORSFEED_FEEDID_LP100_AVG: 255},
                " 0.9",
            ),
            (
                m.UIBOARD_MD_SPEED_AVG,
                {m.SENSORSFEED_FEEDID_SPEED_AVG: 255 << 8},
                " 255",
            ),
            (
                m.UIBOARD_MD_SPEED_AVG,
                {m.SENSORSFEED_FEEDID_SPEED_AVG: 10 << 8},
                "  10",
            ),
            (
                m.UIBOARD_MD_SPEED_AVG,
                {m.SENSORSFEED_FEEDID_SPEED_AVG: 1 << 8},
                "   1",
            ),
            (
                m.UIBOARD_MD_SPEED_AVG,
                {m.SENSORSFEED_FEEDID_SPEED_AVG: 0},
                "   0",
            ),
            (
                m.UIBOARD_MD_SPEED_AVG,
                {m.SENSORSFEED_FEEDID_SPEED_AVG: 255},
                "   0",
            ),
            (m.UIBOARD_MD_INJ_T, {m.SENSORSFEED_FEEDID_INJT: 101 << 8}, "99.9"),
            (m.UIBOARD_MD_INJ_T, {m.SENSORSFEED_FEEDID_INJT: 10 << 8}, "10.0"),
            (m.UIBOARD_MD_INJ_T, {m.SENSORSFEED_FEEDID_INJT: 129}, " 0.5"),
            (m.UIBOARD_MD_INJ_T, {m.SENSORSFEED_FEEDID_INJT: 255}, " 0.9"),
            (
                m.UIBOARD_MD_RANGE,
                {
                    m.SENSORSFEED_FEEDID_LP100_AVG: 50 << 8,
                    m.SENSORSFEED_FEEDID_TANK: 50,
                },
                " 100",
            ),
            (
                m.UIBOARD_MD_RANGE,
                {
                    m.SENSORSFEED_FEEDID_LP100_AVG: 1 << 8,
                    m.SENSORSFEED_FEEDID_TANK: 100,
                },
                "9999",
            ),
            (
                m.UIBOARD_MD_RANGE,
                {
                    m.SENSORSFEED_FEEDID_LP100_AVG: 0,
                    m.SENSORSFEED_FEEDID_TANK: 50,
                },
                "   0",
            ),
            (
                m.UIBOARD_MD_RANGE,
                {
                    m.SENSORSFEED_FEEDID_LP100_AVG: 10,
                    m.SENSORSFEED_FEEDID_TANK: 0,
                },
                "   0",
            ),
        ],
    )
    def test_maindisplay(self, component, inputdata, expectedstring):
        component = m.UIBOARD_maindisplay_components[component]
        for key, value in inputdata.items():
            m.SENSORSFEED_feed[key] = value
        component.executable_component.execute()
        output = read_nextion_output(m,ffi)
        assert output["mdv.txt"] == f'"{expectedstring}"'


    @pytest.mark.parametrize(
        "status,expectedstring",
        [
            (m.SENSORSFEED_EGT_STATUS_UNKN, "----"),
            (m.SENSORSFEED_EGT_STATUS_OPEN, "open"),
            (m.SENSORSFEED_EGT_STATUS_VALUE, "1234"),
        ],
    )
    def test_UIEGT_output(self, status, expectedstring):
        m.SENSORSFEED_feed[m.SENSORSFEED_FEEDID_EGT] = 1234
        m.SENSORSFEED_EGT_status = status
        m.UIBOARD_update_EGT()
        output = read_nextion_output(m,ffi)
        assert output["egt.txt"] == f'"{expectedstring}"'

    @pytest.mark.parametrize(
        "status,expected",
        [
            (m.SENSORSFEED_EGT_STATUS_UNKN, True),
            (m.SENSORSFEED_EGT_STATUS_OPEN, True),
            (m.SENSORSFEED_EGT_STATUS_VALUE, False),
        ],
    )
    def test_UIEGT_raisealert(self, status, expected):
        m.SENSORSFEED_EGT_status = status
        m.UIBOARD_update_EGT()
        assert m.UIBOARD_getraise_critical() == expected
        m.UIBOARD_setraise_critical(False)

    @pytest.mark.parametrize(
        "oiltemp,intaketemp,outtemp",
        [
            (0, 0, 0),
            (0, 0, 10),
            (1, 0, 100),
            (0, 10, 0),
            (0, 10, 10),
            (0, 10, 100),
            (0, 100, 0),
            (0, 100, 10),
            (0, 100, 100),
            (10, 0, 0),
            (10, 0, 10),
            (10, 0, 100),
            (10, 10, 0),
            (10, 10, 10),
            (10, 10, 100),
            (10, 100, 0),
            (10, 100, 1),
            (10, 100, 100),
            (100, 0, 0),
            (100, 0, 10),
            (100, 0, 100),
            (100, 10, 0),
            (100, 1, 10),
            (100, 10, 100),
            (100, 100, 0),
            (100, 100, 10),
            (100, 100, 100),
        ],
    )
    def test_uiboard_sensorgroup_bottom(self, oiltemp, intaketemp, outtemp):
        m.SENSORSFEED_feed[m.SENSORSFEED_FEEDID_OILTEMP] = oiltemp
        m.SENSORSFEED_feed[m.SENSORSFEED_FEEDID_INTAKETEMP] = intaketemp
        m.SENSORSFEED_feed[m.SENSORSFEED_FEEDID_OUTTEMP] = outtemp

        m.UIBOARD_update_sensorgroup_bottom()
        output = read_nextion_output(m, ffi)
        assert output["out.txt"] == (
            f'"{str(outtemp).rjust(3)}"' if outtemp else '"---"'
        )
        assert output["int.txt"] == (
            f'"{str(intaketemp).rjust(3)}"' if intaketemp else '"---"'
        )
        assert output["oil.txt"] == (
            f'"{str(oiltemp).rjust(3)}"' if oiltemp else '"---"'
        )

    @pytest.mark.parametrize(
        "map, frp",
        [
            (0, 0),
            (0, floattofp(2.5, 8)),
            (floattofp(2.5, 8), 0),
            (floattofp(2, 8), 0),
            (0, floattofp(2, 8)),
            (0, floattofp(2.999, 8)),
            (floattofp(2.999, 8), 0),
            (0, floattofp(99, 8)),
            (floattofp(-0.5, 8), 0),
        ],
    )
    def test_uiboard_sensorgroup_pressure(self, map, frp):
        threshold = 2 << 8
        m.SENSORSFEED_feed[m.SENSORSFEED_FEEDID_MAP] = map
        m.SENSORSFEED_feed[m.SENSORSFEED_FEEDID_FRP] = frp
        deltapressure = min(max(0, frp - map - threshold), 0x100)

        m.UIBOARD_update_sensorgroup_pressure()
        output = read_nextion_output(m, ffi)
        assert output["map.txt"] == (
            f'"{f"{fptofloat(map,8):.2f}".rjust(5)}"' if map else '"-----"'
        )
        assert output["frp.txt"] == (
            f'"{f"{fptofloat(frp,8):.2f}".rjust(5)}"' if frp else '"-----"'
        )
        print(deltapressure)
        print(deltapressure * 100 >> 8)
        assert int(output["fmd.val"]) == deltapressure * 100 >> 8

    @pytest.mark.parametrize(
        "watchtype,expectedstring",
        [
            (m.TIMER_TIMERTYPE_WATCH, "  12:34 "),
            (m.TIMER_TIMERTYPE_STOPWATCH, "34:56:78"),
        ],
    )
    def test_uiboard_watch(self, watchtype, expectedstring):
        fullwatchstring = b"12:34:56:78"
        m.TIMER_set_watch(watchtype)
        m.TIMER_active_watch_formated.c_str = fullwatchstring
        m.UIBOARD_update_watch()
        assert read_nextion_output(m, ffi)["wtd.txt"] == f'"{expectedstring}"'

    @pytest.mark.parametrize(
        "alert_severity,pattern,color",
        [
            (m.VISUALALERT_SEVERITY_NOTIFICATION, 0xCC, m.BRIGHTBLUE),
            (m.VISUALALERT_SEVERITY_BADVALUE, 0xFF, m.CRIMSONRED),
            (m.VISUALALERT_SEVERITY_WARNING, 0xAA, m.SAFETYYELLOW),
        ],
    )
    def test_uiboard_visualalert_raise_and_decay_all(
        self, alert_severity, pattern, color
    ):
        for i in range(m.VISUALALERTID_LAST):
            m.UIBOARD_raisevisualalert(i, alert_severity)
        for i in range(m.VISUALALERTID_LAST):
            assert m.UIBOARD_visualalerts[i].color == color
            assert m.UIBOARD_visualalerts[i].pattern == pattern
        for _ in range(8):
            m.UIBOARD_update_visual_alert()

        assert not m.UIBOARD_visualalerts[i].pattern

    @pytest.mark.parametrize(
        "alert_severity,expected_pattern",
        [
            (m.VISUALALERT_SEVERITY_NOTIFICATION, 0xCC),
            (m.VISUALALERT_SEVERITY_BADVALUE, 0xFF),
            (m.VISUALALERT_SEVERITY_WARNING, 0xAA),
        ],
    )
    def test_uiboard_visualalert_patternshift_single(
        self, alert_severity, expected_pattern
    ):
        m.UIBOARD_raisevisualalert(0, alert_severity)
        result_pattern = 0
        for i in range(8):
            m.UIBOARD_update_visual_alert()
            value = int(read_nextion_output(m, ffi).popitem()[1])

            if value != m.DEFAULTCOLOR:
                result_pattern = result_pattern | (1 << i)

        assert result_pattern == expected_pattern


class TestNumpadUI:

    INPUTCOMPONENT_NUMPAD1 = 1
    INPUTCOMPONENT_NUMPAD2 = 2
    INPUTCOMPONENT_NUMPAD3 = 3
    INPUTCOMPONENT_NUMPAD4 = 4
    INPUTCOMPONENT_NUMPAD5 = 5
    INPUTCOMPONENT_NUMPAD6 = 6
    INPUTCOMPONENT_NUMPAD7 = 7
    INPUTCOMPONENT_NUMPAD8 = 8
    INPUTCOMPONENT_NUMPAD9 = 9
    INPUTCOMPONENT_NUMPAD0 = 10
    INPUTCOMPONENT_NUMPADMINUS = 11
    INPUTCOMPONENT_NUMPADDEL = 12
    INPUTCOMPONENT_NUMPADSEND = 13

    @classmethod
    def setup_class(cls):
        cls.nullptr = ffi.NULL
        m.SYSTEM_run = False
        m.CONFIG_factory_default_reset()
        m.test()

    @pytest.fixture(autouse=True)
    def clear(self):
        m.NEXTION_clear_selected_component()
        m.UINUMPAD_reset()
        m.USART_TX_clear()
        m.NEXTION_handler_ready(m.NEXTION_VERSION)

    @pytest.mark.parametrize(
        "testvalue,expectedvalue",
        [
            (0, "     0"),
            (100, "   100"),
            (-100, "-  100"),
        ],
    )
    def test_setup(self, testvalue, expectedvalue):
        length = m.UINUMPAD_DISPLAYLENGTH
        target = ffi.new("int32_t*", testvalue)
        m.UINUMPAD_switch(target)
        stringvalue = ffi.unpack(m.UINUMPAD_getstringvalue(), length).decode("ascii")
        m.NEXTION_set_previous_page()
        assert stringvalue == expectedvalue

    def test_setup_nulltarget_wont_explode(self):
        length = m.UINUMPAD_DISPLAYLENGTH
        m.NEXTION_switch_page(m.NEXTION_PAGEID_NUMPAD,0)
        assert ffi.unpack(m.UINUMPAD_getstringvalue(),length) == b'!'*length

        inputevent = ffi.new("INPUT_Event*")
        inputevent.componentID = self.INPUTCOMPONENT_NUMPADSEND
        inputevent.key = m.INPUT_KEY_ENTER
        inputevent.keystatus = m.INPUT_KEYSTATUS_CLICK

        m.UINUMPAD_handle_userinput(cast_void(inputevent))

    @pytest.mark.parametrize(
        "inputseq,expectedvalue",
        [
            (([INPUTCOMPONENT_NUMPADMINUS]),"      "),
            ((INPUTCOMPONENT_NUMPAD0,INPUTCOMPONENT_NUMPADMINUS),"      "),
            ((INPUTCOMPONENT_NUMPAD0,INPUTCOMPONENT_NUMPAD0),"      "),
            ((INPUTCOMPONENT_NUMPAD1,INPUTCOMPONENT_NUMPAD2),"    12"),
            ((INPUTCOMPONENT_NUMPAD1,INPUTCOMPONENT_NUMPAD2,INPUTCOMPONENT_NUMPAD3,INPUTCOMPONENT_NUMPAD4,INPUTCOMPONENT_NUMPAD5), " 12345"),
            ((INPUTCOMPONENT_NUMPAD6,INPUTCOMPONENT_NUMPAD7,INPUTCOMPONENT_NUMPAD8,INPUTCOMPONENT_NUMPAD9,INPUTCOMPONENT_NUMPAD0), " 67890"),
            ((INPUTCOMPONENT_NUMPAD1,INPUTCOMPONENT_NUMPAD2,INPUTCOMPONENT_NUMPAD3,INPUTCOMPONENT_NUMPAD4,INPUTCOMPONENT_NUMPAD5,INPUTCOMPONENT_NUMPAD6), " 12345"),
            ((INPUTCOMPONENT_NUMPAD1,INPUTCOMPONENT_NUMPAD2,INPUTCOMPONENT_NUMPAD3,INPUTCOMPONENT_NUMPAD4,INPUTCOMPONENT_NUMPAD5,INPUTCOMPONENT_NUMPADMINUS), "-12345"),
            ((INPUTCOMPONENT_NUMPAD1,INPUTCOMPONENT_NUMPAD2,INPUTCOMPONENT_NUMPAD3,INPUTCOMPONENT_NUMPAD4,INPUTCOMPONENT_NUMPAD5,INPUTCOMPONENT_NUMPADMINUS,INPUTCOMPONENT_NUMPADMINUS), " 12345"),
            ((INPUTCOMPONENT_NUMPAD1,INPUTCOMPONENT_NUMPADDEL), "      "),
            ((INPUTCOMPONENT_NUMPAD1,INPUTCOMPONENT_NUMPADDEL,INPUTCOMPONENT_NUMPADDEL), "      "),
            ((INPUTCOMPONENT_NUMPAD1,INPUTCOMPONENT_NUMPADDEL,INPUTCOMPONENT_NUMPAD2), "     2"),
            ((INPUTCOMPONENT_NUMPAD1,INPUTCOMPONENT_NUMPAD2,INPUTCOMPONENT_NUMPAD3,INPUTCOMPONENT_NUMPAD4,INPUTCOMPONENT_NUMPAD5,INPUTCOMPONENT_NUMPADDEL,INPUTCOMPONENT_NUMPADDEL), "   123"),
        ],
    )
    def test_inputhandler(self, inputseq, expectedvalue):
        length = m.UINUMPAD_DISPLAYLENGTH
        for input_componentID in inputseq:
            inputevent = ffi.new("INPUT_Event*")
            inputevent.componentID = input_componentID
            inputevent.key = m.INPUT_KEY_ENTER
            inputevent.keystatus = m.INPUT_KEYSTATUS_CLICK
            m.UINUMPAD_handle_userinput(cast_void(inputevent))

        stringvalue = ffi.unpack(m.UINUMPAD_getstringvalue(), length).decode("ascii")
        assert stringvalue == expectedvalue

    @pytest.mark.parametrize("inputvalue",[0,100,-100,1000,-1000,10000,-10000,-32768,32767])
    def test_switch_send_check_targetuntouched(self, inputvalue):
        target = ffi.new("int32_t*", inputvalue)
        inputevent = ffi.new("INPUT_Event*")
        inputevent.componentID = self.INPUTCOMPONENT_NUMPADSEND
        inputevent.key = m.INPUT_KEY_ENTER
        inputevent.keystatus = m.INPUT_KEYSTATUS_CLICK
                
        m.UINUMPAD_switch(target)
        target[0] = 0x7FFF
        m.UINUMPAD_handle_userinput(cast_void(inputevent))

        target = int(ffi.cast("int16_t", target[0]))
        assert target == inputvalue

    @pytest.mark.parametrize("stringvalue, expectedvalue",[
        (b"      0",0),(b"      1",1),(b"     10",10),(b"    100",100),
        (b"   1000",1000),(b" 10000",10000),(b"-10000",-10000)
    ])
    def test_switch_modify_send_check_targetmodified(self, stringvalue, expectedvalue):
        target = ffi.new("int32_t*")
        m.UINUMPAD_switch(target)
        inputevent = ffi.new("INPUT_Event*")
        c_stringvalue = ffi.new("char[7]",stringvalue)
        inputevent.componentID = self.INPUTCOMPONENT_NUMPADSEND
        inputevent.key = m.INPUT_KEY_ENTER
        inputevent.keystatus = m.INPUT_KEYSTATUS_CLICK
        ffi.memmove(m.UINUMPAD_getstringvalue(),c_stringvalue,m.UINUMPAD_DISPLAYLENGTH+1)
        m.UINUMPAD_handle_userinput(cast_void(inputevent))

        target = int(ffi.cast("int16_t", target[0]))
        assert target == expectedvalue

    def test_nextion_select_with_down_key(self):
        truth_table = [
            {},
            { "b01.bco" : m.BRIGHTBLUE},
            { "b01.bco" : m.PASTELORANGE, "b02.bco" : m.BRIGHTBLUE},
            { "b02.bco" : m.PASTELORANGE, "b03.bco" : m.BRIGHTBLUE},
            { "b03.bco" : m.PASTELORANGE, "b04.bco" : m.BRIGHTBLUE},
            { "b04.bco" : m.PASTELORANGE, "b05.bco" : m.BRIGHTBLUE},
            { "b05.bco" : m.PASTELORANGE, "b06.bco" : m.BRIGHTBLUE},
            { "b06.bco" : m.PASTELORANGE, "b07.bco" : m.BRIGHTBLUE},
            { "b07.bco" : m.PASTELORANGE, "b08.bco" : m.BRIGHTBLUE},
            { "b08.bco" : m.PASTELORANGE, "b09.bco" : m.BRIGHTBLUE},
            { "b09.bco" : m.PASTELORANGE, "b00.bco" : m.BRIGHTBLUE},
            { "b00.bco" : m.PASTELORANGE, "mns.bco" : m.BRIGHTBLUE},
            { "mns.bco" : m.PASTELORANGE, "del.bco" : m.BRIGHTBLUE},
            { "del.bco" : m.PASTELORANGE, "snd.bco" : m.BRIGHTBLUE},
            { "snd.bco" : m.PASTELORANGE, "b01.bco" : m.BRIGHTBLUE},
        ]
        for selected in range(0,15):
            inputevent = ffi.new("INPUT_Event*")
            inputevent.key = m.INPUT_KEY_DOWN
            inputevent.keystatus = m.INPUT_KEYSTATUS_CLICK
            m.UINUMPAD_handle_userinput(cast_void(inputevent))
            output = read_nextion_output(m, ffi)
            expected_values = truth_table[selected]
            assert len(expected_values) == len(output)
            for k, v in expected_values.items():
                assert int(output[k]) == v

    def test_nextion_delivered_componentID_dont_touch_iterator(self):
        touch_event = ffi.new("INPUT_Event*")
        touch_event.key = m.INPUT_KEY_ENTER
        touch_event.keystatus = m.INPUT_KEYSTATUS_CLICK
        touch_event.componentID = 5

        key_event = ffi.new("INPUT_Event*")
        key_event.key = m.INPUT_KEY_DOWN
        key_event.keystatus = m.INPUT_KEYSTATUS_CLICK

        m.UINUMPAD_handle_userinput(cast_void(touch_event))
        m.UINUMPAD_handle_userinput(cast_void(key_event))
        m.UINUMPAD_handle_userinput(cast_void(key_event))

        output = read_nextion_output(m, ffi)
        assert int(output["b05.bco"]) == m.PASTELORANGE
        assert int(output["b01.bco"]) == m.BRIGHTBLUE

    def test_nextion_output_stringvaluecontent(self):
        stringvalue = b"-99999"
        expectedstring = '"-99999"'
        c_stringvalue = ffi.new("char[7]",stringvalue)
        ffi.memmove(m.UINUMPAD_getstringvalue(), c_stringvalue, m.UINUMPAD_DISPLAYLENGTH+1)
        m.UINUMPAD_update()
        output = read_nextion_output(m, ffi)
        assert output["dsp.txt"] == expectedstring

    @pytest.mark.parametrize("keystatus",[
        m.INPUT_KEYSTATUS_RELEASED,
        m.INPUT_KEYSTATUS_PRESSED,
        m.INPUT_KEYSTATUS_HOLD,
    ])
    def test_nextion_unused_keystatus_dont_select(self, keystatus):
        touch_event = ffi.new("INPUT_Event*")
        touch_event.key = m.INPUT_KEY_ENTER
        touch_event.keystatus = keystatus
        touch_event.componentID = 5
        m.UINUMPAD_handle_userinput(cast_void(touch_event))
        assert 0 == len(read_nextion_output(m, ffi))

class TestConfigUI:
    INPUTCOMPONENT_PRV = 6
    INPUTCOMPONENT_NXT = 7
    INPUTCOMPONENT_DEC = 8
    INPUTCOMPONENT_INC = 9
    INPUTCOMPONENT_BCK = 10
    INPUTCOMPONENT_PAD = 11

    @classmethod
    def setup_class(cls):
        m.SYSTEM_run = False
        m.CONFIG_factory_default_reset()
        m.test()

    @pytest.fixture(autouse=True)
    def clear(self):
        m.NEXTION_clear_selected_component()
        m.UICONFIG_setup()
        m.USART_TX_clear()
        m.NEXTION_handler_ready(m.NEXTION_VERSION)

    def test_setup(self):
        m.SYSTEM_config.SYSTEM_FACTORY_RESET = 1
        m.UICONFIG_setup()
        output = read_nextion_output(m,ffi)
        assert int(output["ptr.val"]) == 0 # Set pointer to variable
        assert int(output["rfp.en"]) == 1 # Enable refresh pointer resolving
        assert int(output["vlh.val"]) == 1 # Value Lower Half expected SYSTEM_FACTORY_RESET value
        m.SYSTEM_config.SYSTEM_FACTORY_RESET = 0
    
    def test_nextion_select_with_down_key(self):
        truth_table = [
            { "prv.bco" : m.BRIGHTBLUE},
            { "prv.bco" : m.BACKGROUNDGRAY, "nxt.bco" : m.BRIGHTBLUE},
            { "nxt.bco" : m.BACKGROUNDGRAY, "dec.bco" : m.BRIGHTBLUE},
            { "dec.bco" : m.BACKGROUNDGRAY, "inc.bco" : m.BRIGHTBLUE},
            { "inc.bco" : m.BACKGROUNDGRAY, "bck.bco" : m.BRIGHTBLUE},
            { "bck.bco" : m.BACKGROUNDGRAY, "pad.bco" : m.BRIGHTBLUE},
            { "pad.bco" : m.BACKGROUNDGRAY, "prv.bco" : m.BRIGHTBLUE},
        ]
        for selected in range(0,6):
            inputevent = ffi.new("INPUT_Event*")
            inputevent.key = m.INPUT_KEY_DOWN
            inputevent.keystatus = m.INPUT_KEYSTATUS_CLICK
            m.UICONFIG_handle_userinput(cast_void(inputevent))
            output = read_nextion_output(m, ffi)
            expected_values = truth_table[selected]
            assert len(expected_values) == len(output)
            for k, v in expected_values.items():
                assert int(output[k]) == v

    def test_nextion_select_variable(self):
        CONFIGVARIABLES_COUNT = m.CONFIG_ENTRY_LAST
        MAX_ENTRY = CONFIGVARIABLES_COUNT-1

        inputevent = ffi.new("INPUT_Event*")
        inputevent.key = m.INPUT_KEY_ENTER
        inputevent.keystatus = m.INPUT_KEYSTATUS_CLICK

        #FORWARD
        inputevent.componentID = self.INPUTCOMPONENT_NXT
        for i in [n for n in range(1,CONFIGVARIABLES_COUNT)] + [0]:
            m.UICONFIG_handle_userinput(cast_void(inputevent))
            output = read_nextion_output(m, ffi)
            assert int(output["ptr.val"]) == i # Set pointer to variable
            assert int(output["rfp.en"]) == 1 # Enable refresh pointer resolving
            assert "vlh.val" in output # Set value of value lower half
        
        #BACKWARD
        inputevent.componentID = self.INPUTCOMPONENT_PRV
        for i in ([MAX_ENTRY] + [n for n in range(0,CONFIGVARIABLES_COUNT)])[::-1]:
            m.UICONFIG_handle_userinput(cast_void(inputevent))
            output = read_nextion_output(m, ffi)
            assert int(output["ptr.val"]) == i # Set pointer to variable
            assert int(output["rfp.en"]) == 1 # Enable refresh pointer resolving
            assert "vlh.val" in output # Set value of value lower half

    def test_nextion_inc_value(self):
        m.SYSTEM_config.SYSTEM_FACTORY_RESET = 0
        touch_event = ffi.new("INPUT_Event*")
        touch_event.key = m.INPUT_KEY_ENTER
        touch_event.keystatus = m.INPUT_KEYSTATUS_CLICK
        touch_event.componentID = self.INPUTCOMPONENT_INC
        m.UICONFIG_handle_userinput(cast_void(touch_event))

        output = read_nextion_output(m, ffi)
        assert int(output["vlh.val"]) == 1

        m.UICONFIG_handle_userinput(cast_void(touch_event))
        output = read_nextion_output(m, ffi)
        assert int(output["vlh.val"]) == 1
        assert 0 == m.SYSTEM_config.SYSTEM_FACTORY_RESET

    def test_nextion_dec_value(self):
        m.SYSTEM_config.SYSTEM_FACTORY_RESET = 1
        touch_event = ffi.new("INPUT_Event*")
        touch_event.key = m.INPUT_KEY_ENTER
        touch_event.keystatus = m.INPUT_KEYSTATUS_CLICK
        touch_event.componentID = self.INPUTCOMPONENT_DEC
        m.UICONFIG_handle_userinput(cast_void(touch_event))

        output = read_nextion_output(m, ffi)
        assert int(output["vlh.val"]) == 0

        m.UICONFIG_handle_userinput(cast_void(touch_event))
        output = read_nextion_output(m, ffi)
        assert int(output["vlh.val"]) == 0

        assert 1 == m.SYSTEM_config.SYSTEM_FACTORY_RESET

    def test_nextion_apply_value_after_selecting_next_variable(self):
        m.SYSTEM_config.SYSTEM_FACTORY_RESET = 0
        touch_event = ffi.new("INPUT_Event*")
        touch_event.key = m.INPUT_KEY_ENTER
        touch_event.keystatus = m.INPUT_KEYSTATUS_CLICK
        touch_event.componentID = self.INPUTCOMPONENT_INC
        m.UICONFIG_handle_userinput(cast_void(touch_event))

        output = read_nextion_output(m, ffi)
        assert int(output["vlh.val"]) == 1

        touch_event.key = m.INPUT_KEY_ENTER
        touch_event.keystatus = m.INPUT_KEYSTATUS_CLICK
        touch_event.componentID = self.INPUTCOMPONENT_NXT
        m.UICONFIG_handle_userinput(cast_void(touch_event))

        assert 1 == m.SYSTEM_config.SYSTEM_FACTORY_RESET

    def test_apply_correct_slider_value_after_selecting_next_variable(self):
        m.SYSTEM_config.SYSTEM_FACTORY_RESET = 0
        m.NEXTION_incomingdata_handler(1)
        touch_event = ffi.new("INPUT_Event*")
        touch_event.key = m.INPUT_KEY_ENTER
        touch_event.keystatus = m.INPUT_KEYSTATUS_CLICK
        touch_event.componentID = self.INPUTCOMPONENT_NXT
        m.UICONFIG_handle_userinput(cast_void(touch_event))
        assert 1 == m.SYSTEM_config.SYSTEM_FACTORY_RESET

    @pytest.mark.parametrize("badvalue, expectedresult",[
        (-250,m.CONFIG_ENTRY_VERDICT_TOO_SMALL),
        (250,m.CONFIG_ENTRY_VERDICT_TOO_BIG)
    ])
    def test_dont_apply_bad_slider_valuesmall_after_selecting_next_variable(self, badvalue, expectedresult):
        m.SYSTEM_config.SYSTEM_FACTORY_RESET = 0
        m.NEXTION_incomingdata_handler(badvalue)
        touch_event = ffi.new("INPUT_Event*")
        touch_event.key = m.INPUT_KEY_ENTER
        touch_event.keystatus = m.INPUT_KEYSTATUS_CLICK
        touch_event.componentID = self.INPUTCOMPONENT_NXT
        m.UICONFIG_handle_userinput(cast_void(touch_event))
        assert 0 == m.SYSTEM_config.SYSTEM_FACTORY_RESET
        
        output = read_nextion_output(m, ffi)
        assert int(output["res.val"]) == expectedresult

    def test_apply_correct_numpad_value(self):
        #Set config page with API, so page history will work correctly
        m.SYSTEM_config.SYSTEM_FACTORY_RESET = 0
        m.NEXTION_switch_page(m.NEXTION_PAGEID_BOARDCONFIG,0)
        touch_event = ffi.new("INPUT_Event*")
        touch_event.key = m.INPUT_KEY_ENTER
        touch_event.keystatus = m.INPUT_KEYSTATUS_CLICK
        touch_event.componentID = self.INPUTCOMPONENT_PAD
        m.UICONFIG_handle_userinput(cast_void(touch_event))
        output = read_nextion_output(m, ffi)
        assert int(output["page"]) == m.NEXTION_PAGEID_NUMPAD
        #Page switched to pad
        touch_event.componentID = TestNumpadUI.INPUTCOMPONENT_NUMPAD1
        m.UINUMPAD_handle_userinput(cast_void(touch_event))
        touch_event.componentID = TestNumpadUI.INPUTCOMPONENT_NUMPADSEND
        m.UINUMPAD_handle_userinput(cast_void(touch_event))
        output = read_nextion_output(m, ffi)
        #Page should be back to config
        assert int(output["page"]) == m.NEXTION_PAGEID_BOARDCONFIG
        #Config variables should be refreshed
        assert int(output["ptr.val"]) == 0
        assert int(output["rfp.en"]) == 1 # Enable refresh pointer resolving
        assert int(output["vlh.val"]) == 1 # Set value of value lower half, in this case 1 as entered in pad


    def test_apply_bad_positive_numpad_value(self):
        #Set config page with API, so page history will work correctly
        m.SYSTEM_config.SYSTEM_FACTORY_RESET = 0
        m.NEXTION_switch_page(m.NEXTION_PAGEID_BOARDCONFIG,0)
        touch_event = ffi.new("INPUT_Event*")
        touch_event.key = m.INPUT_KEY_ENTER
        touch_event.keystatus = m.INPUT_KEYSTATUS_CLICK
        touch_event.componentID = self.INPUTCOMPONENT_PAD
        m.UICONFIG_handle_userinput(cast_void(touch_event))
        output = read_nextion_output(m, ffi)
        assert int(output["page"]) == m.NEXTION_PAGEID_NUMPAD
        #Page switched to pad
        touch_event.componentID = TestNumpadUI.INPUTCOMPONENT_NUMPAD2
        m.UINUMPAD_handle_userinput(cast_void(touch_event))
        touch_event.componentID = TestNumpadUI.INPUTCOMPONENT_NUMPADSEND
        m.UINUMPAD_handle_userinput(cast_void(touch_event))
        output = read_nextion_output(m, ffi)
        #Page should be back to config
        assert int(output["page"]) == m.NEXTION_PAGEID_BOARDCONFIG
        #Config variables should be refreshed
        assert int(output["ptr.val"]) == 0
        assert int(output["rfp.en"]) == 1 # Enable refresh pointer resolving
        # Set value of value lower half, in this case 1 as value entered in pad is invalid and 1 is biggest possible.
        assert int(output["vlh.val"]) == 1
        assert int(output["res.val"]) == m.CONFIG_ENTRY_VERDICT_TOO_BIG

    def test_apply_bad_negative_numpad_value(self):
        #Set config page with API, so page history will work correctly
        m.NEXTION_switch_page(m.NEXTION_PAGEID_BOARDCONFIG,0)
        touch_event = ffi.new("INPUT_Event*")
        touch_event.key = m.INPUT_KEY_ENTER
        touch_event.keystatus = m.INPUT_KEYSTATUS_CLICK
        touch_event.componentID = self.INPUTCOMPONENT_PAD
        m.UICONFIG_handle_userinput(cast_void(touch_event))
        output = read_nextion_output(m, ffi)
        assert int(output["page"]) == m.NEXTION_PAGEID_NUMPAD
        #Page switched to pad
        touch_event.componentID = TestNumpadUI.INPUTCOMPONENT_NUMPAD1
        m.UINUMPAD_handle_userinput(cast_void(touch_event))
        touch_event.componentID = TestNumpadUI.INPUTCOMPONENT_NUMPADMINUS
        m.UINUMPAD_handle_userinput(cast_void(touch_event))
        touch_event.componentID = TestNumpadUI.INPUTCOMPONENT_NUMPADSEND
        m.UINUMPAD_handle_userinput(cast_void(touch_event))
        output = read_nextion_output(m, ffi)
        #Page should be back to config
        assert int(output["page"]) == m.NEXTION_PAGEID_BOARDCONFIG
        #Config variables should be refreshed
        assert int(output["ptr.val"]) == 0
        assert int(output["rfp.en"]) == 1 # Enable refresh pointer resolving
        # Set value of value lower half, in this case 0 as value entered in pad is invalid and 0 is smallest possible.
        assert int(output["vlh.val"]) == 0
        assert int(output["res.val"]) == m.CONFIG_ENTRY_VERDICT_TOO_SMALL

    def test_back_sets_previous_page_without_pad_usage(self):
        #Create history trace
        m.NEXTION_switch_page(m.NEXTION_PAGEID_BOARD,1)
        m.NEXTION_switch_page(m.NEXTION_PAGEID_BOARDCONFIG,1)
        read_nextion_output(m, ffi)

        touch_event = ffi.new("INPUT_Event*")
        touch_event.key = m.INPUT_KEY_ENTER
        touch_event.keystatus = m.INPUT_KEYSTATUS_CLICK
        touch_event.componentID = self.INPUTCOMPONENT_BCK
        m.UICONFIG_handle_userinput(cast_void(touch_event))

        output = read_nextion_output(m, ffi)
        #Page should be back to Board
        assert int(output["page"]) == m.NEXTION_PAGEID_BOARD

    def test_back_sets_previous_page_with_pad_usage(self):
        #Create history trace
        m.NEXTION_switch_page(m.NEXTION_PAGEID_BOARD,1)
        m.NEXTION_switch_page(m.NEXTION_PAGEID_BOARDCONFIG,1)
        read_nextion_output(m, ffi)

        touch_event = ffi.new("INPUT_Event*")
        touch_event.key = m.INPUT_KEY_ENTER
        touch_event.keystatus = m.INPUT_KEYSTATUS_CLICK

        #bring pad
        touch_event.componentID = self.INPUTCOMPONENT_PAD
        m.UICONFIG_handle_userinput(cast_void(touch_event))
        #send value to close pad
        touch_event.componentID = TestNumpadUI.INPUTCOMPONENT_NUMPADSEND
        m.UINUMPAD_handle_userinput(cast_void(touch_event))
        #go back from config
        touch_event.componentID = self.INPUTCOMPONENT_BCK
        m.UICONFIG_handle_userinput(cast_void(touch_event))

        output = read_nextion_output(m, ffi)
        #Page should be back to Board
        assert int(output["page"]) == m.NEXTION_PAGEID_BOARD

    def test_back_triggers_factory_reset_when_set(self):
        m.SYSTEM_run = True
        m.SYSTEM_config.SYSTEM_FACTORY_RESET = 1
        TESTDATA = ffi.cast("CONFIG_maxdata_t*",ffi.new("uint32_t*", 0xFF))
        for i in range(m.CONFIG_ENTRY_LAST):
            m.CONFIG_modify_entry(ffi.NULL, i , TESTDATA)
        
        touch_event = ffi.new("INPUT_Event*")
        touch_event.key = m.INPUT_KEY_ENTER
        touch_event.keystatus = m.INPUT_KEYSTATUS_CLICK
        touch_event.componentID = self.INPUTCOMPONENT_BCK
        m.UICONFIG_handle_userinput(cast_void(touch_event))


        m.CONFIG_read_entry(ffi.NULL, m.CONFIG_ENTRY_SYSTEM_FACTORY_RESET, TESTDATA)
        assert 0 == TESTDATA[0]

        # Check if device restart is triggered
        assert False == m.SYSTEM_run

    def test_back_saves_values_to_persistent_memory(self):
        TESTDATA = ffi.cast("CONFIG_maxdata_t*",ffi.new("uint32_t*", 0xFF))
        for i in range(m.CONFIG_ENTRY_LAST):
            m.CONFIG_modify_entry(ffi.addressof(m.SYSTEM_config), i , TESTDATA)
        
        touch_event = ffi.new("INPUT_Event*")
        touch_event.key = m.INPUT_KEY_ENTER
        touch_event.keystatus = m.INPUT_KEYSTATUS_CLICK
        touch_event.componentID = self.INPUTCOMPONENT_BCK
        m.UICONFIG_handle_userinput(cast_void(touch_event))

        for i in range(m.CONFIG_ENTRY_LAST):
            m.CONFIG_read_entry(ffi.NULL, i , TESTDATA)
            assert 0xFF == TESTDATA[0]
