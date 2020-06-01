import cffi
import importlib
import unittest


def load(filename):
    # load source code
    with open(filename + '.c') as source, open("definitions.h") as definitions:
        # pass source code to CFFI
        ffibuilder = cffi.FFI()
        ffibuilder.cdef(definitions.read())
        ffibuilder.set_source(filename + '_', source.read())
        ffibuilder.compile()

        # import and return resulting module
        module = importlib.import_module(filename + '_')
        return module.lib


class testPreRun(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.bc = load("main")
        cls.ffi = cffi.FFI()
        cls.nullptr = cls.ffi.NULL

    def test_scheduler(self):
        # Test if fregister is fully initialized
        fregister = self.ffi.unpack(self.bc.SCHEDULER_fregister,
                                    self.bc.LAST_cb)
        for fptr in fregister:
            fptr = self.ffi.cast("void*", fptr)
            self.assertNotEqual(fptr, self.nullptr)

    def test_USART(self):
        # Test buffers and counters are at 0
        self.assertFalse(self.bc.USART_RX_buffer_index)
        self.assertFalse(self.bc.USART_TX_buffer_index)
        self.assertFalse(self.bc.USART_eot_counter)


if __name__ == "main":
    unittest.main()
