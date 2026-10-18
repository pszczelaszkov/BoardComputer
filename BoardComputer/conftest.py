import ctypes
import os

import pytest


@pytest.hookimpl(trylast=True)
def pytest_sessionfinish(session, exitstatus):
    """Flush gcov counters from the already-loaded testmodule.so.

    CPython does not dlclose extension modules, so gcov's destructor never
    runs. RTLD_NOLOAD reuses the mapping helpers.load() already imported.
    """
    so_path = os.path.abspath("bin/testmodule.so")
    try:
        lib = ctypes.CDLL(so_path, mode=os.RTLD_NOLOAD)
    except OSError:
        return
    try:
        lib.__gcov_dump()
    except AttributeError:
        pass
