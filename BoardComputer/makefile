PYTHON_VER := $(shell python3 -c "import sys; print(f'{sys.version_info.major}.{sys.version_info.minor}')")
IDIR = src/
TDIR = test/src/

TESTDEFINES = -DTESTUSE=
TESTFLAGS = -O1 -fpic -g -D__DEBUG__ $(TESTDEFINES)
AVRFLAGS = -O1 -mmcu=atmega324pb $(TESTDEFINES) -fshort-enums

X86_TEST_IDIR = $(IDIR)Interface/x86_Test/
HW_1_AVR_IDIR = $(IDIR)Interface/HW_1_AVR/
x86_test: clean
	cp -R $(IDIR) $(TDIR)
	python3 testsuite/fileswrapper.py $(TDIR) test/generatedDefinitions
	@echo "Using Python${PYTHON_VER}"
	gcc $(TESTFLAGS) -I$(IDIR) -I$(X86_TEST_IDIR) -c $(TDIR)USART.c -o bin/USART_.o
	gcc $(TESTFLAGS) -I$(IDIR) -I$(X86_TEST_IDIR) -c $(TDIR)input.c -o bin/input_.o
	gcc $(TESTFLAGS) -I$(IDIR) -I$(X86_TEST_IDIR) -c $(TDIR)nextion.c -o bin/nextion_.o
	gcc $(TESTFLAGS) -I$(IDIR) -I$(X86_TEST_IDIR) -c $(TDIR)UI/board.c -o bin/uiboard_.o
	gcc $(TESTFLAGS) -I$(IDIR) -I$(X86_TEST_IDIR) -c $(TDIR)UI/numpad.c -o bin/uinumpad_.o
	gcc $(TESTFLAGS) -I$(IDIR) -I$(X86_TEST_IDIR) -c $(TDIR)UI/config.c -o bin/uiconfig_.o
	gcc $(TESTFLAGS) -I$(IDIR) -I$(X86_TEST_IDIR) -c $(TDIR)timer.c -o bin/timer_.o
	gcc $(TESTFLAGS) -I$(IDIR) -I$(X86_TEST_IDIR) -c $(TDIR)sensorsfeed.c -o bin/sensorsfeed_.o
	gcc $(TESTFLAGS) -I$(IDIR) -I$(X86_TEST_IDIR) -c $(TDIR)countersfeed.c -o bin/countersfeed_.o
	gcc $(TESTFLAGS) -I$(IDIR) -I$(X86_TEST_IDIR) -c $(TDIR)utils.c -o bin/utils_.o
	gcc $(TESTFLAGS) -I$(IDIR) -I$(X86_TEST_IDIR) -c $(TDIR)programdata.c -o bin/programdata_.o
	gcc $(TESTFLAGS) -I$(IDIR) -I$(X86_TEST_IDIR) -c $(TDIR)average.c -o bin/average_.o
	gcc $(TESTFLAGS) -I$(IDIR) -I$(X86_TEST_IDIR) -c $(TDIR)system.c -o bin/system_.o
	gcc $(TESTFLAGS) -I$(IDIR) -I$(X86_TEST_IDIR) -c $(TDIR)config.c -o bin/config_.o
	gcc $(TESTFLAGS) -I$(IDIR) -I$(X86_TEST_IDIR) -c $(X86_TEST_IDIR)persistent_memory.c -o bin/persistent_memory_.o
	gcc $(TESTFLAGS) -I$(IDIR) -I$(X86_TEST_IDIR) -c $(TDIR)main.c -I/usr/include/python${PYTHON_VER} -o bin/main_.o
	gcc -shared bin/*_.o -lpython${PYTHON_VER} -o bin/testmodule.so

HW_1:
	avr-gcc $(AVRFLAGS) -I$(IDIR) -I$(HW_1_AVR_IDIR) -c src/main.c -o bin/main.o
	avr-gcc $(AVRFLAGS) -I$(IDIR) -I$(HW_1_AVR_IDIR) -c src/input.c -o bin/input.o
	avr-gcc $(AVRFLAGS) -I$(IDIR) -I$(HW_1_AVR_IDIR) -c src/USART.c -o bin/USART.o
	avr-gcc $(AVRFLAGS) -I$(IDIR) -I$(HW_1_AVR_IDIR) -c src/sensorsfeed.c -o bin/sensorsfeed.o
	avr-gcc $(AVRFLAGS) -I$(IDIR) -I$(HW_1_AVR_IDIR) -c src/countersfeed.c -o bin/countersfeed.o
	avr-gcc $(AVRFLAGS) -I$(IDIR) -I$(HW_1_AVR_IDIR) -c src/nextion.c -o bin/nextion.o
	avr-gcc $(AVRFLAGS) -I$(IDIR) -I$(HW_1_AVR_IDIR) -c src/UI/board.c -o bin/uiboard.o
	avr-gcc $(AVRFLAGS) -I$(IDIR) -I$(HW_1_AVR_IDIR) -c src/UI/numpad.c -o bin/uinumpad.o
	avr-gcc $(AVRFLAGS) -I$(IDIR) -I$(HW_1_AVR_IDIR) -c src/UI/config.c -o bin/uiconfig.o
	avr-gcc $(AVRFLAGS) -I$(IDIR) -I$(HW_1_AVR_IDIR) -c src/timer.c -o bin/timer.o
	avr-gcc $(AVRFLAGS) -I$(IDIR) -I$(HW_1_AVR_IDIR) -c src/utils.c -o bin/utils.o
	avr-gcc $(AVRFLAGS) -I$(IDIR) -I$(HW_1_AVR_IDIR) -c src/average.c -o bin/average.o
	avr-gcc $(AVRFLAGS) -I$(IDIR) -I$(HW_1_AVR_IDIR) -c src/system.c -o bin/system.o
	avr-gcc $(AVRFLAGS) -I$(IDIR) -I$(HW_1_AVR_IDIR) -c src/config.c -o bin/config.o
	avr-gcc $(AVRFLAGS) -I$(IDIR) -I$(HW_1_AVR_IDIR) -c src/programdata.c -o bin/programdata.o
	avr-gcc $(AVRFLAGS) -I$(IDIR) -I$(HW_1_AVR_IDIR) -c $(HW_1_AVR_IDIR)persistent_memory.c -o bin/persistent_memory.o
	avr-gcc -mmcu=atmega324pb bin/*.o -o bin/BoardComputer.elf
	avr-objcopy -j .text -j .data -O ihex bin/BoardComputer.elf bin/BoardComputer.hex
	avr-size --mcu=atmega324pa -C bin/BoardComputer.elf

flash:
	avrdude -p m324pb -c usbasp -C +"utils/324pb.conf" -B 0.5 -U flash:w:"bin/BoardComputer.hex":a

.PHONY: clean test flash boardcomputer

clean:
	rm -f bin/*
	rm -rf $(TDIR)

all: boardcomputer flash