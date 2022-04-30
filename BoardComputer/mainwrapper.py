from cffi import FFI
from cffi import recompiler

#It wraps main c code with python stuff.
with open('test/src/main.c') as source, open('test/generatedDefinitions/definitions.h') as definitions:
	# pass source code to CFFI
	ffibuilder = FFI()
	ffibuilder.cdef(definitions.read())
	recompiler.make_c_source(ffibuilder, "test.testmodule", source.read(), "test/src/main.c")
