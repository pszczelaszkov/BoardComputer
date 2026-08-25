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
SERIAL_TTY=/dev/tnt0 BC_DIR=/path/to/simdir bin/BoardComputer
```

### ADC simulation (`BC_DIR`)
On init the app creates `ADC0`..`ADC7` under `BC_DIR` (defaults to `.` if unset) and seeds missing regular files with `0`.
Each `ADC_start_conversion()` reads the current mux channel file as an ASCII integer (0–1023) and pushes it like the AVR ADC ISR.
Write raw values into those files while the app runs, e.g.:
```
echo 512 > "$BC_DIR/ADC3"
```

### Counters simulation (RT signals)
Fuel and speed remain signal-driven (no files). Send pulse amounts with `sigqueue`:
- `SIGRTMIN+0` (`COUNTERS_SIG_FUEL`) — fuel ticks in `sival_int`
- `SIGRTMIN+1` (`COUNTERS_SIG_SPEED`) — speed pulses in `sival_int`

## Note
When switching between testing and build remember to clean:</br>
```
make clean
```
If extra parameters are needed, they can be set with EXTRA_FLAGS variable.
```
EXTRA_FLAGS=-D__DEBUG__
```
