# BoardComputer
![CI](https://github.com/pszczelaszkov/BoardComputer/actions/workflows/x86-test.yml/badge.svg) ![CI](https://github.com/pszczelaszkov/BoardComputer/actions/workflows/avr-hw1-size.yml/badge.svg)</br>
Versatile firmware with custom HW, capable of reading various sensors and firing optional outputs.</br>
Combined with Nextion HMI allows many deployments like Board/Trip computer or full dashboard.</br>
Currently preferred display: <a href="https://nextion.tech/basic-series-introduction/">NX4024T032</a></br>

Summary of HW_1:</br>
µC: <a href="https://www.microchip.com/wwwproducts/en/ATmega324PB">ATMEGA324PB</a></br>
Main connectors: 2x <a href="https://www.phoenixcontact.com/skedd">SKEDD</a></br>
PCB size: 10cmx10cm</br>
INPUT:</br>
-1x K-Type TC</br>
-8x Analog</br>
-5x Digital(Gated Optocoupler)</br>
-2x Divided(12v).</br>
-2x Buttons</br>
</br>
OUTPUT:</br>
-6x N-Channel FET with pull-up Vcc/+12</br>
</br>
Communication:</br>
-SPI(Programming)</br>
-TWI(Extensions)</br>
-UART(Display)</br>
-UART(Raw configuration)</br>

# Testing
Integration tests are done with help of custom framework backed with CFFI library and are written in python.</br>
It is possible to build x86 standalone binary and fire it on Linux for faster manual testing.</br>

# Instructions
<a href="https://github.com/pszczelaszkov/BoardComputer/blob/master/BoardComputer/doc/index.md">Documentation</a></br>
# Preview
<b>Display Example:</b></br>
![Display](https://github.com/pszczelaszkov/BoardComputer/blob/master/Previews/DisplayPreview.png)
![Config](https://github.com/pszczelaszkov/BoardComputer/blob/master/Previews/ConfigPreview.png)
</br>
<b>HW1 Board Preview:</b></br>
![Board](https://github.com/pszczelaszkov/BoardComputer/blob/master/Previews/BoardComputerFront.png)
![Board](https://github.com/pszczelaszkov/BoardComputer/blob/master/Previews/BoardComputerBack.png)
