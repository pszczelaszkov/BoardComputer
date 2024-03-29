import pytest
from helpers import load, ModuleWrapper, read_nextion_output, fptofloat, floattofp

m, ffi = load("testmodule")
session = ModuleWrapper(m)


class TestBoardUI:
    @classmethod
    def setup_class(cls):
        m.SYSTEM_run = False
        m.test()
        session.create_snapshot()

    @pytest.fixture(autouse=True)
    def snapshot_control(self):
        session.load_snapshot()
        yield

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
        m.USART_TX_clear()

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
        m.USART_TX_clear()

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
        m.TIMER_formated = fullwatchstring
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
    @classmethod
    def setup_class(cls):
        cls.nullptr = ffi.NULL
        m.SYSTEM_run = False
        m.test()

    @pytest.mark.parametrize(
        "testvalue,expectedvalue",
        [
            (100, "   100"),
            (-100, "-  100"),
        ],
    )
    def test_uinumpad_setup(self, testvalue, expectedvalue):
        length = m.DISPLAYLENGTH
        target = ffi.new("int16_t*", testvalue)
        m.UINUMPAD_switch(target)
        stringvalue = ffi.unpack(m.UINUMPAD_getstringvalue(), length).decode("ascii")
        assert stringvalue == expectedvalue

    @pytest.mark.parametrize(
        "testvalue,expectedvalue",
        [
            (0, "     0"),
            (5, "-    5"),
        ],
    )
    def test_uinumpad_minus(self, testvalue, expectedvalue):
        length = m.DISPLAYLENGTH
        target = ffi.new("int16_t*", testvalue)
        m.UINUMPAD_switch(target)
        m.UINUMPAD_click_mns()
        stringvalue = ffi.unpack(m.UINUMPAD_getstringvalue(), length).decode("ascii")
        assert stringvalue == expectedvalue

    def test_uinumpad_append(self):
        length = m.DISPLAYLENGTH
        target = ffi.new("int16_t*", 0)
        m.UINUMPAD_switch(target)
        m.UINUMPAD_click_b1()
        m.UINUMPAD_click_b2()
        m.UINUMPAD_click_b3()
        m.UINUMPAD_click_b4()
        stringvalue = ffi.unpack(m.UINUMPAD_getstringvalue(), length).decode("ascii")
        expectedvalue = "".join([" " for i in range(length)])[0:-4] + str(1234)
        assert stringvalue == expectedvalue

    def test_uinumpad_delete(self):
        testvalue = 12345
        length = m.DISPLAYLENGTH
        target = ffi.new("int16_t*", testvalue)
        m.UINUMPAD_switch(target)
        m.UINUMPAD_click_del()
        m.UINUMPAD_click_del()
        stringvalue = ffi.unpack(m.UINUMPAD_getstringvalue(), length).decode("ascii")
        expectedvalue = "".join([" " for i in range(length)])[0:-3] + str(123)
        assert stringvalue == expectedvalue

    def test_uinumpad_send(self):
        target = ffi.new("int16_t*", 100)
        m.UINUMPAD_switch(target)
        m.UINUMPAD_click_b0()
        m.UINUMPAD_click_b0()
        m.UINUMPAD_click_snd()
        target = int(ffi.cast("int16_t", target[0]))
        assert target == 10000
