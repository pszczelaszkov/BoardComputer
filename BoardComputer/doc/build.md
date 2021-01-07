# Build
## Compiling
Device specs for Atmega 324PB may not be included in avr-gcc/libavr as it is older than device itself.</br>
If it's so additional files are in [Utils](../utils)</br>
They need to be copied into avr library directory.</br>
To start build type:</br>
```
make boardcomputer
```
## Flashing
Default method of flashing is using usbasp with avrdude and its done by typing:
```
make flash
```
or if build&flash action is needed:</br>
```
make all
```
## Testing
Tests are written in python with help of unittest and cffi library.</br>
Firmware is compiled as python library so **Python Development files are needed.**</br>
Cffi can be installed by typing:</br>
```
pip install -r requirements.txt
```
To test type:</br>
```
make test
python test_*.py
```
## Note
When switching between testing and build remember to clean:</br>
```
make clean
```
