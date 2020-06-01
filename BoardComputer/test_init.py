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


class testInit(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.bc = load("main")
        cls.ffi = cffi.FFI()
        cls.nullptr = cls.ffi.NULL
        cls.bc.main()

    def test_scheduler(self):
        # Test queue is circular
        size = self.bc.SCHEDULER_LOW_PRIORITY_QUEUE_SIZE
        tasks = self.ffi.unpack(self.bc.SCHEDULER_low_priority_tasks, size)
        last = tasks[-1]
        nextptr = self.ffi.cast("void*", last.nextTask)
        for task in tasks:
            fptr = self.ffi.cast("void*", cffi.FFI().addressof(task))
            self.assertEqual(nextptr, fptr)
            nextptr = self.ffi.cast("void*", task.nextTask)
