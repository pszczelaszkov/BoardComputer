from cffi import FFI
from cffi import recompiler

#It wraps main c code with python stuff.
with open('src/main.c') as source, open('src/definitions.h') as definitions:
	# pass source code to CFFI
	ffibuilder = FFI()
	ffibuilder.cdef(definitions.read())
	recompiler.make_c_source(ffibuilder, "src.main_", source.read(), "src/main_.c")
