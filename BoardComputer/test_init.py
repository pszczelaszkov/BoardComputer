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

    def test_uiboard_components_cohesion(self):
        image = self.bc.NEXTION_HIGHLIGHTTYPE_IMAGE
        croppedimage = self.bc.NEXTION_HIGHLIGHTTYPE_CROPPEDIMAGE
        model = [
            [1, 25, b"wtd", croppedimage],
            [2, 17, b"wts", image]
        ]
        zipped = zip(self.ffi.unpack(self.bc.UIBOARD_components, 2), model)
        for t, m in zipped:
            self.assertEqual(t.value_default, m[0])
            self.assertEqual(t.value_selected, m[1])
            name = self.ffi.unpack(t.name, self.bc.NEXTION_OBJNAME_LEN)
            self.assertEqual(name, m[2])
            self.assertEqual(t.highlighttype, m[3])

    def test_uiboard_MDcomponents_cohesion(self):
        image = self.bc.NEXTION_HIGHLIGHTTYPE_IMAGE
        model = [
            [11, 22],
            [12, 23],
            [13, 24],
            [14, 19],
            [15, 20],
            [16, 21]
        ]
        zipped = zip(self.ffi.unpack(self.bc.UIBOARD_maindisplay_components, 6), model)
        for t, m in zipped:
            component = t.executable_component.component
            self.assertEqual(component.value_default, m[0])
            self.assertEqual(component.value_selected, m[1])
            name = self.ffi.unpack(component.name, self.bc.NEXTION_OBJNAME_LEN)
            self.assertEqual(name, b"mds")
            self.assertEqual(component.highlighttype, image)

        self.assertEqual(self.bc.UIBOARD_maindisplay_activecomponent,
                         self.bc.UIBOARD_maindisplay_components[0])

    def test_input_components_cohesion(self):
        model = [
            [self.bc.INPUT_COMPONENT_MAINDISPLAY, b"mds"],
            [self.bc.INPUT_COMPONENT_WATCH, b"wtd"],
            [self.bc.INPUT_COMPONENT_WATCHSEL, b"wts"],
            [self.bc.INPUT_COMPONENT_CONFIGIPM, b"ipm"],
            [self.bc.INPUT_COMPONENT_CONFIGCCM, b"ccm"],
            [self.bc.INPUT_COMPONENT_CONFIGDBS, b"dbs"],
            [self.bc.INPUT_COMPONENT_CONFIGWHH, b"whh"],
            [self.bc.INPUT_COMPONENT_CONFIGWMM, b"wmm"],
            [self.bc.INPUT_COMPONENT_CONFIGWSS, b"wss"],
        ]

        nullptr = self.bc.INPUT_findcomponent(self.bc.INPUT_COMPONENT_NONE)
        self.assertFalse(nullptr)
        for sample in model:
            component = self.bc.INPUT_findcomponent(sample[0])
            self.assertTrue(component)
            name = component.nextion_component.name
            name = self.ffi.unpack(name, self.bc.NEXTION_OBJNAME_LEN)
            self.assertEqual(name, sample[1])

        self.assertTrue(self.bc.INPUT_active_component)


if __name__ == "main":
    unittest.main()
