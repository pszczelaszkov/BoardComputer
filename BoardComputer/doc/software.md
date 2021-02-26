# Software

## Firmware Design
Firmware is written in C, there are few rules that dictates shape of code:</br>
- No floating point variables should be used.</br>
- Since dividing is software implemented, most of dividing operations should be performed at initialize.</br>
- Code should be divided into modules with some encapsulation mechanisms.</br>
- Nested interrupts or loops in them are not allowed.</br>
 
As for overall code styling, some of [JSF-AV](https://www.stroustrup.com/JSF-AV-rules.pdf) rules are applied.</br>

### System structure

