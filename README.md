# BoardComputer
I'm developing this project as i'm in need of unified standalone controller which will fit into OEM Board Computer mountings.</br>
Although device itself is versatile, specs in short:</br>
Target uC: <a href="https://www.microchip.com/wwwproducts/en/ATmega324PB">ATMEGA324PB</a></br>
Display: <a href="https://nextion.tech/basic-series-introduction/">NX4024T032</a></br>
Main connectors: 2x <a href="https://www.phoenixcontact.com/skedd">SKEDD</a></br>
PCB size: 10cmx10cm</br>
INPUTS:</br>
-EGT(k-type TC)</br>
-8x Analog + divider with Vcc/Gnd coupling.</br>
-5x Digital with Low Pass filter.</br>
-2x 12v with divider.</br>
-2x Keys</br>
</br>
OUTPUTS:</br>
-6x N-Channel FET with selectable pull-up Vcc/+12</br>
</br>
Communication:</br>
-SPI(Programming)</br>
-TWI(Extensions)</br>
-USART(Raw configuration)</br>
</br>>
<b>Display Preview:</b></br>
![Display](https://github.com/pszczelaszkov/BoardComputer/blob/master/Previews/DisplayPreview.png)
</br>
<b>Board Prototype:</b></br>
![Board](https://github.com/pszczelaszkov/BoardComputer/blob/master/Previews/BoardComputerFront.png)
![Board](https://github.com/pszczelaszkov/BoardComputer/blob/master/Previews/BoardComputerBack.png)
