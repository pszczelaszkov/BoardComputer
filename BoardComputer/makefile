PYTHON_VER = 3.9
test:
	python3 mainwrapper.py
	gcc -fpic -c src/USART.c -O1 -o bin/USART_.o
	gcc -fpic -c src/input.c -O1 -o bin/input_.o
	gcc -fpic -c src/nextion.c -O1 -o bin/nextion_.o
	gcc -fpic -c src/UI/board.c -O1 -o bin/uiboard_.o
	gcc -fpic -c src/UI/boardconfig.c -O1 -o bin/uiboardconfig_.o
	gcc -fpic -c src/timer.c -O1 -o bin/timer_.o
	gcc -fpic -c src/sensorsfeed.c -O1 -o bin/sensorsfeed_.o
	gcc -fpic -c src/countersfeed.c -O1 -o bin/countersfeed_.o
	gcc -fpic -c src/utils.c -O1 -o bin/utils_.o
	gcc -fpic -c src/programdata.c -O1 -o bin/programdata_.o
	gcc -fpic -c src/average.c -O1 -o bin/average_.o
	gcc -fpic -c src/system.c -O1 -o bin/system_.o
	gcc -fpic -c src/main_.c -I/usr/include/python${PYTHON_VER} -O1 -o bin/main_.o
	gcc -shared bin/*_.o -lpython${PYTHON_VER} -o bin/main_.so

boardcomputer:
	avr-gcc -c src/main.c -O1 -mmcu=atmega324pb -fshort-enums -o bin/main.o
	avr-gcc -c src/input.c -O1 -mmcu=atmega324pb -fshort-enums -o bin/input.o
	avr-gcc -c src/USART.c -O1 -mmcu=atmega324pb -fshort-enums -o bin/USART.o
	avr-gcc -c src/sensorsfeed.c -O1 -mmcu=atmega324pb -fshort-enums -o bin/sensorsfeed.o
	avr-gcc -c src/countersfeed.c -O1 -mmcu=atmega324pb -fshort-enums -o bin/countersfeed.o
	avr-gcc -c src/nextion.c -O1 -mmcu=atmega324pb -fshort-enums -o bin/nextion.o
	avr-gcc -c src/UI/board.c -O1 -mmcu=atmega324pb -fshort-enums -o bin/uiboard.o
	avr-gcc -c src/UI/boardconfig.c -O1 -mmcu=atmega324pb -fshort-enums -o bin/uiboardconfig.o
	avr-gcc -c src/timer.c -O1 -mmcu=atmega324pb -fshort-enums -o bin/timer.o
	avr-gcc -c src/utils.c -O1 -mmcu=atmega324pb -fshort-enums -o bin/utils.o
	avr-gcc -c src/average.c -O1 -mmcu=atmega324pb -fshort-enums -o bin/average.o
	avr-gcc -c src/system.c -O1 -mmcu=atmega324pb -fshort-enums -o bin/system.o
	avr-gcc -c src/programdata.c -O1 -mmcu=atmega324pb -fshort-enums -o bin/programdata.o
	avr-gcc -mmcu=atmega324pb bin/*.o -o bin/BoardComputer.elf
	avr-objcopy -j .text -j .data -O ihex bin/BoardComputer.elf bin/BoardComputer.hex
	avr-size --mcu=atmega324pa -C bin/BoardComputer.elf

flash:
	avrdude -p m324pb -c usbasp -C +"utils/324pb.conf" -B 0.5 -U flash:w:"bin/BoardComputer.hex":a

clean:
	rm bin/*

all: boardcomputer flash