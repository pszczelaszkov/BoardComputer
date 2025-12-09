# Build HW

There are 3 targets:

## HW_1
First hardware version, it's based on Atmega324PB
Device specs for Atmega 324PB may not be included in avr-gcc/libavr as it is older than device itself.</br>
If it's so additional files are in [Utils](../utils)</br>
They need to be copied into avr library directory.</br>
To start build type:</br>
```
make HW_1
```
By default board will transmit USART on J1 Connector, set __DEBUG__ for output on H5.
## Flashing
Default method of flashing is using usbasp with avrdude and its done by typing:
```
make flash
```

# Testing
## Auto testing
Tests are written in python with help of pytest and cffi library.</br>
Firmware is compiled as python library so **Python Development files are needed.**</br>
Cffi can be installed by typing:</br>
```
pip install -r python_requirements.txt
```
To test type:</br>
```
make x86_test && pytest
```
## Manual testing
It's possible to build firmware as standalone application.
```
make x86_standalone
```
With help of [tty0tty](https://github.com/freemed/tty0tty)
And use of nextion editor/simulator [Nextion](https://nextion.tech/nextion-editor/#_section1)

It is possible to use simulated serial as output and manualy test firmware + UI combo on PC.
Standalone application starts as follows:
```
SERIAL_TTY=/dev/tnt0 bin/BoardComputer
```
## Note
When switching between testing and build remember to clean:</br>
```
make clean
```
If extra parameters are needed, they can be set with EXTRA_FLAGS variable.
```
EXTRA_FLAGS=-D__DEBUG__
```
