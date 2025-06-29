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
        m.test()

    @pytest.fixture(autouse=True)
    def clear(self):
        m.NEXTION_clear_active_component()
        m.USART_TX_clear()

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
        assert ffi.unpack(
            m.USART_TX_buffer, 14
        ) == f'mdv.txt="{expectedstring}"'.encode("utf-8")

    @pytest.mark.parametrize(
        "status,expected",
        [
            (m.SENSORSFEED_EGT_STATUS_UNKN, "----"),
            (m.SENSORSFEED_EGT_STATUS_OPEN, "open"),
            (m.SENSORSFEED_EGT_STATUS_VALUE, "1234"),
        ],
    )
    def test_UIEGT_output(self, status, expected):
        m.SENSORSFEED_feed[m.SENSORSFEED_FEEDID_EGT] = 1234
        m.SENSORSFEED_EGT_status = status
        m.UIBOARD_update_EGT()
        assert ffi.unpack(m.USART_TX_buffer, 14) == f'egt.txt="{expected}"'.encode(
            "utf-8"
        )

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

    def test_uiboard_modify_display_brightness(self):
        STEP = 5
        m.NEXTION_set_brightness(0)
        assert m.NEXTION_brightness == 0
        for i in range(int(100 / STEP)):
            m.USART_TX_clear()
            m.UIBOARDCONFIG_modify_dbs()
        assert read_nextion_output(m, ffi)["dim"] == "100"

        for i in range(8):
            m.USART_TX_clear()
            m.UIBOARDCONFIG_modify_dbs()
        assert not read_nextion_output(m, ffi)

        m.UIBOARDCONFIG_modify_dbs()
        assert read_nextion_output(m, ffi)["dim"] == "0"

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
        assert int(output["fmd.var"]) == deltapressure * 100 >> 8

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
        m.test()

    @pytest.fixture(autouse=True)
    def clear(self):
        m.NEXTION_clear_active_component()
        m.UINUMPAD_reset()
        m.USART_TX_clear()

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
        target = ffi.new("int16_t*", testvalue)
        m.UINUMPAD_switch(target)
        stringvalue = ffi.unpack(m.UINUMPAD_getstringvalue(), length).decode("ascii")
        m.NEXTION_set_previous_page()
        assert stringvalue == expectedvalue

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
        target = ffi.new("int16_t*", inputvalue)
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
        target = ffi.new("int16_t*")
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
        value_default = m.PASTELORANGE
        value_selected = m.BRIGHTBLUE
        for selected in range(0,15):
            inputevent = ffi.new("INPUT_Event*")
            inputevent.key = m.INPUT_KEY_DOWN
            inputevent.keystatus = m.INPUT_KEYSTATUS_CLICK
            m.UINUMPAD_handle_userinput(cast_void(inputevent))
            output = read_nextion_output(m, ffi)
            expected_values = truth_table[selected]
            assert len(expected_values) == len(output)
            print(selected)
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
