boardcomputer: src/main.c
	avr-gcc src/main.c -O1 -mmcu=atmega324pb -o bin/BoardComputer.elf
	avr-objcopy -R .eeprom -R .fuse -R .lock -R .signature -O ihex bin/BoardComputer.elf bin/BoardComputer.hex
	avr-size --mcu=atmega324pa -C bin/BoardComputer.elf

flash:
	avrdude -p m324pb -c usbasp -C +"utils/324pb.conf" -B 0.5 -U flash:w:"bin/BoardComputer.hex":a

clean:
	rm bin/*

all: boardcomputer flash