PYTHON_VER = 3.8
IDIR = src/
TDIR = test/src/

TESTFLAGS = -O1 -fpic -D__DEBUG__ -I$(IDIR)
AVRFLAGS = -O1 -mmcu=atmega324pb -fshort-enums -I$(IDIR)

test: clean
	cp -R $(IDIR) $(TDIR)
	python3 testsuite/fileswrapper.py $(TDIR) test/generatedDefinitions
	gcc $(TESTFLAGS) -c $(TDIR)USART.c -o bin/USART_.o
	gcc $(TESTFLAGS) -c $(TDIR)input.c -o bin/input_.o
	gcc $(TESTFLAGS) -c $(TDIR)nextion.c -o bin/nextion_.o
	gcc $(TESTFLAGS) -c $(TDIR)UI/board.c -o bin/uiboard_.o
	gcc $(TESTFLAGS) -c $(TDIR)UI/boardconfig.c -o bin/uiboardconfig_.o
	gcc $(TESTFLAGS) -c $(TDIR)UI/numpad.c -o bin/numpad_.o
	gcc $(TESTFLAGS) -c $(TDIR)timer.c -o bin/timer_.o
	gcc $(TESTFLAGS) -c $(TDIR)sensorsfeed.c -o bin/sensorsfeed_.o
	gcc $(TESTFLAGS) -c $(TDIR)countersfeed.c -o bin/countersfeed_.o
	gcc $(TESTFLAGS) -c $(TDIR)utils.c -o bin/utils_.o
	gcc $(TESTFLAGS) -c $(TDIR)programdata.c -o bin/programdata_.o
	gcc $(TESTFLAGS) -c $(TDIR)average.c -o bin/average_.o
	gcc $(TESTFLAGS) -c $(TDIR)system.c -o bin/system_.o
	gcc $(TESTFLAGS) -c $(TDIR)main.c -I/usr/include/python${PYTHON_VER} -o bin/main_.o
	gcc -shared bin/*_.o -lpython${PYTHON_VER} -o bin/testmodule.so

boardcomputer:
	avr-gcc $(AVRFLAGS) -c src/main.c -o bin/main.o
	avr-gcc $(AVRFLAGS) -c src/input.c -o bin/input.o
	avr-gcc $(AVRFLAGS) -c src/USART.c -o bin/USART.o
	avr-gcc $(AVRFLAGS) -c src/sensorsfeed.c -o bin/sensorsfeed.o
	avr-gcc $(AVRFLAGS) -c src/countersfeed.c -o bin/countersfeed.o
	avr-gcc $(AVRFLAGS) -c src/nextion.c -o bin/nextion.o
	avr-gcc $(AVRFLAGS) -c src/UI/board.c -o bin/uiboard.o
	avr-gcc $(AVRFLAGS) -c src/UI/boardconfig.c -o bin/uiboardconfig.o
	avr-gcc $(AVRFLAGS) -c src/UI/numpad.c -o bin/numpad.o
	avr-gcc $(AVRFLAGS) -c src/timer.c -o bin/timer.o
	avr-gcc $(AVRFLAGS) -c src/utils.c -o bin/utils.o
	avr-gcc $(AVRFLAGS) -c src/average.c -o bin/average.o
	avr-gcc $(AVRFLAGS) -c src/system.c -o bin/system.o
	avr-gcc $(AVRFLAGS) -c src/programdata.c -o bin/programdata.o
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