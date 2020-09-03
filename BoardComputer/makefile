boardcomputer: src/main.c
	avr-gcc src/main.c -O1 -mmcu=atmega8 -o bin/BoardComputer.elf
	avr-objcopy -R .eeprom -R .fuse -R .lock -R .signature -O ihex bin/BoardComputer.elf bin/BoardComputer.hex
	avr-size --mcu=atmega8 -C bin/BoardComputer.elf

flash:
	avrdude -p m8 -c usbasp -B 0.5 -U flash:w:"bin/BoardComputer.hex":a

clean:
	rm bin/*

all: boardcomputer flash