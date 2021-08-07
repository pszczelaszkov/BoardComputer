import unittest
from helpers import load
# This test class should be launched first to check global definitions





class testInit(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.bc, cls.ffi = load("main", "definitions.h")
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

    def test_timer(self):
        self.assertTrue(self.bc.TIMER_MILISECOND_WEIGHT)
        self.assertTrue(self.bc.TIMER_REGISTER_WEIGHT)
        self.bc.TCNT2 = 14
        miliseconds = self.bc.TIMER_counter_to_miliseconds()
        self.assertEqual(10, miliseconds)
        self.bc.TCNT2 = 0

    def test_input(self):
        self.assertTrue(self.bc.INPUT_KEY_LAST)
        self.assertTrue(self.bc.INPUT_KEYSTATUS_RELEASED == 0)
        self.assertTrue(self.bc.INPUT_KEYSTATUS_PRESSED == 1)
        self.assertTrue(self.bc.INPUT_KEYSTATUS_HOLD >
                        self.bc.INPUT_KEYSTATUS_PRESSED)
        self.assertTrue(self.bc.INPUT_KEYSTATUS_CLICK >
                        self.bc.INPUT_KEYSTATUS_HOLD)

    def test_nextion(self):
        # EOT must be null-terminated triple 0xff
        eot = self.ffi.unpack(self.bc.NEXTION_eot, 4)
        for byte in eot[0:3]:
            self.assertEqual(byte, 0xff)
        self.assertEqual(eot[3], 0)

    def test_uiboard_switch_maindisplay(self):
        # Must have default component and be circular
        initial = self.bc.UIBOARD_maindisplay_activecomponent
        desired = self.bc.UIBOARD_maindisplay_components[0]
        self.assertEqual(initial, desired)
        temp = initial.nextComponent
        for i in range(self.bc.UIBOARD_MD_LAST):
            if temp == initial:
                break
            temp = temp.nextComponent

        self.assertEqual(initial, temp)

    def test_uiboardconfig_components_cohesion(self):
        frontcolor = self.bc.NEXTION_HIGHLIGHTTYPE_FRONTCOLOR
        backcolor = self.bc.NEXTION_HIGHLIGHTTYPE_BACKCOLOR
        brightblue = 0x4DF
        brightbrown = 0xBC8D
        white = 0xffff
        model = [
            [white, brightblue, b"ipm", backcolor],
            [white, brightblue, b"ccm", backcolor],
            [white, brightblue, b"whh",backcolor],
            [white, brightblue, b"wmm",backcolor],
            [white, brightblue, b"wss", backcolor],
            [brightbrown, brightblue, b"dbs",frontcolor]
        ]#default,selected,name,highlighttype
        zipped = zip(self.ffi.unpack(self.bc.UIBOARDCONFIG_executable_components, self.bc.UIBOARDCONFIG_COMPONENT_LAST), model)
        i = 0
        for t, m in zipped:
            msg = "Failed @ "+str(i)
            t = self.ffi.cast("NEXTION_Component *", self.ffi.addressof(t))
            self.assertEqual(t.value_default, m[0],msg=msg)
            self.assertEqual(t.value_selected, m[1], msg=msg)
            name = self.ffi.unpack(t.name, self.bc.NEXTION_OBJNAME_LEN)
            self.assertEqual(name, m[2],msg=msg)
            self.assertEqual(t.highlighttype, m[3], msg=msg)
            i=i+1

    def test_uinumpad_components_conformance(self):
        backcolor = self.bc.NEXTION_HIGHLIGHTTYPE_BACKCOLOR
        model = [
            [0xFD88, 0x4DF, b"b01", backcolor],
            [0xFD88, 0x4DF, b"b02", backcolor],
            [0xFD88, 0x4DF, b"b03", backcolor],
            [0xFD88, 0x4DF, b"b04", backcolor],
            [0xFD88, 0x4DF, b"b05", backcolor],
            [0xFD88, 0x4DF, b"b06", backcolor],
            [0xFD88, 0x4DF, b"b07", backcolor],
            [0xFD88, 0x4DF, b"b08", backcolor],
            [0xFD88, 0x4DF, b"b09", backcolor],
            [0xFD88, 0x4DF, b"b00", backcolor],
            [0xFD88, 0x4DF, b"mns", backcolor],
            [0xFD88, 0x4DF, b"del", backcolor],
            [0xFD88, 0x4DF, b"snd", backcolor]

        ]#default,selected,name,highlighttype
        zipped = zip(self.ffi.unpack(self.bc.UINUMPAD_components, len(model)), model)
        i = 0
        for t, m in zipped:
            msg = "Failed @ "+str(i)
            self.assertEqual(t.value_default, m[0], msg=msg)
            self.assertEqual(t.value_selected, m[1], msg=msg)
            name = self.ffi.unpack(t.name, self.bc.NEXTION_OBJNAME_LEN)
            self.assertEqual(name, m[2], msg=msg)
            self.assertEqual(t.highlighttype, m[3], msg=msg)
            i=i+1

    def test_uiboard_components_cohesion(self):
        image = self.bc.NEXTION_HIGHLIGHTTYPE_IMAGE
        croppedimage = self.bc.NEXTION_HIGHLIGHTTYPE_CROPPEDIMAGE
        model = [
            [1, 25, b"wtd", croppedimage],
            [2, 17, b"wts", image],
            [3, 18, b"cfg", image]
        ]#default,selected,name,highlighttype
        zipped = zip(self.ffi.unpack(self.bc.UIBOARD_components, 2), model)
        i = 0
        for t, m in zipped:
            msg = "Failed @ "+str(i)
            self.assertEqual(t.value_default, m[0], msg=msg)
            self.assertEqual(t.value_selected, m[1], msg=msg)
            name = self.ffi.unpack(t.name, self.bc.NEXTION_OBJNAME_LEN)
            self.assertEqual(name, m[2], msg=msg)
            self.assertEqual(t.highlighttype, m[3], msg=msg)
            i=i+1

    def test_uiboard_MDcomponents_cohesion(self):
        image = self.bc.NEXTION_HIGHLIGHTTYPE_IMAGE
        model = [
            [11, 22],
            [12, 23],
            [13, 24],
            [14, 19],
            [15, 20],
            [16, 21]
        ]#default,selected
        zipped = zip(self.ffi.unpack(self.bc.UIBOARD_maindisplay_components, 6), model)
        i = 0
        for t, m in zipped:
            msg = "Failed @ "+str(i)
            component = t.executable_component.component
            self.assertEqual(component.value_default, m[0], msg=msg)
            self.assertEqual(component.value_selected, m[1], msg=msg)
            name = self.ffi.unpack(component.name, self.bc.NEXTION_OBJNAME_LEN)
            self.assertEqual(name, b"mds", msg=msg)
            self.assertEqual(component.highlighttype, image, msg=msg)
            i=i+1
   
        self.assertEqual(self.bc.UIBOARD_maindisplay_activecomponent,
                         self.bc.UIBOARD_maindisplay_components[0])

    def test_input_components_cohesion(self):
        model = [
            [self.bc.INPUT_COMPONENT_MAINDISPLAY, b"mds"],
            [self.bc.INPUT_COMPONENT_WATCH, b"wtd"],
            [self.bc.INPUT_COMPONENT_WATCHSEL, b"wts"],
            [self.bc.INPUT_COMPONENT_CONFIG, b"cfg"],
            [self.bc.INPUT_COMPONENT_CONFIGIPM, b"ipm"],
            [self.bc.INPUT_COMPONENT_CONFIGCCM, b"ccm"],
            [self.bc.INPUT_COMPONENT_CONFIGDBS, b"dbs"],
            [self.bc.INPUT_COMPONENT_CONFIGWHH, b"whh"],
            [self.bc.INPUT_COMPONENT_CONFIGWMM, b"wmm"],
            [self.bc.INPUT_COMPONENT_CONFIGWSS, b"wss"],
            [self.bc.INPUT_COMPONENT_CONFIGBCK, b"bck"],
            [self.bc.INPUT_COMPONENT_NUMPAD0, b"b00"],
            [self.bc.INPUT_COMPONENT_NUMPAD1, b"b01"],
            [self.bc.INPUT_COMPONENT_NUMPAD2, b"b02"],
            [self.bc.INPUT_COMPONENT_NUMPAD3, b"b03"],
            [self.bc.INPUT_COMPONENT_NUMPAD4, b"b04"],
            [self.bc.INPUT_COMPONENT_NUMPAD5, b"b05"],
            [self.bc.INPUT_COMPONENT_NUMPAD6, b"b06"],
            [self.bc.INPUT_COMPONENT_NUMPAD7, b"b07"],
            [self.bc.INPUT_COMPONENT_NUMPAD8, b"b08"],
            [self.bc.INPUT_COMPONENT_NUMPAD9, b"b09"],
            [self.bc.INPUT_COMPONENT_NUMPADMINUS, b"mns"],
            [self.bc.INPUT_COMPONENT_NUMPADDEL, b"del"],
            [self.bc.INPUT_COMPONENT_NUMPADSEND, b"snd"]
        ]

        nullptr = self.bc.INPUT_findcomponent(self.bc.INPUT_COMPONENT_NONE)
        self.assertFalse(nullptr)
        i = 0
        for sample in model:
            msg = "Failed @ "+str(i)
            component = self.bc.INPUT_findcomponent(sample[0])
            self.assertTrue(component, msg=msg)
            nextion_component = component.nextion_component
            self.assertTrue(nextion_component, msg=msg)
            name = nextion_component.name
            name = self.ffi.unpack(name, self.bc.NEXTION_OBJNAME_LEN)
            self.assertEqual(name, sample[1], msg=msg)
            i = i+1

        self.assertFalse(self.bc.INPUT_active_component)

    def test_input_common_bck_conformance(self):
        image = self.bc.NEXTION_HIGHLIGHTTYPE_IMAGE
        bckcomponent = self.bc.NEXTION_common_bckcomponent
        self.assertEqual(bckcomponent.value_default, 28)
        self.assertEqual(bckcomponent.value_selected, 29)
        name = self.ffi.unpack(bckcomponent.name, self.bc.NEXTION_OBJNAME_LEN)
        self.assertEqual(name, b'bck')
        self.assertEqual(bckcomponent.highlighttype, image)

    def test_utils_atoi(self):
        sample = [b'2', b'0', b'0', b'0', b'0']
        testvalue = self.ffi.new("char[]", sample)
        self.assertEqual(self.bc.UTILS_atoi(testvalue), 20000)

    def test_utils_atoi_minus(self):
        sample = [b'-', b'2', b'0', b'0', b'0', b'0']
        testvalue = self.ffi.new("char[]", sample)
        self.assertEqual(self.bc.UTILS_atoi(testvalue), -20000)


if __name__ == "main":
    unittest.main()
