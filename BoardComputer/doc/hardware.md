# Hardware

PCB gerber files and schematic are located in Hardware directory.</br>

## Periphals flow design

### Analog Inputs
There are 8 of analog inputs designed like that:</br>
```mermaid
graph LR
A{{Analog Input}}--> B{ADC}
A---R[Resistor]
subgraph Divider
    R-. Jumper ..-Vcc & Gnd
end
```
### Digital Inputs
```mermaid
graph LR
subgraph µC
    PB1{{PB1}}
    PB2{{PB2}}
    PC2{{PC2}}
    PC3{{PC2}}
    PD2{{PD2}}
    PD3{{PD3}}
    PD4{{PD4}}
    PD5{{PD5}}
    PD6{{PD6}}
    subgraph Timers
        T1[(T1)]
        ICP1{ICP1}
        T4[(T4)]
        ICP4{ICP4}
    end
    INT0{INT0}
    INT1{INT1}
    INT2{INT2}
    spi0{SPI0}
end
max{Max6675}
k1{{Key1}}--pull-up-->PD2-.->INT0
k2{{Key2}}--pull-up-->PD3-.->INT1
Hi0{{Hi0}}-->Hi0_d[Divider]-->PB1-.->T1
Hi1{{Hi1}}-->Hi1_d[Divider]-->PB2-.->INT2
D2{{DIN2}}-->o2[Logic Optoisolator]-->f2[RC filter]-->PD6-.->ICP1
D3{{DIN3}}-->o3[Logic Optoisolator]-->f3[RC filter]-->PC2-.->T4
D4{{DIN4}}-->o4[Logic Optoisolator]-->f4[RC filter]-->PC3-.->ICP4
D0{{DIN0}}-->o0[Logic Optoisolator]-->f0[RC filter]--->PD4
D1{{DIN1}}-->o1[Logic Optoisolator]-->f1[RC filter]--->PD5
EGT{{EGT+/-}}-->max--->spi0
```

### Digital Outputs
```mermaid
graph LR
subgraph µC
    PB3{{PB3}}
    PB4{{PB4}}
    PC0{{PC0}}
    PC1{{PC1}}
    PC4{{PC4}}
    PC5{{PC5}}
    subgraph Timers
        OC0A{T0PWMA}
        OC0B{T0PWMB}
        OC4A{T4PWMA}
    end
end
OC0A-->PB3-->fg0[Mosfet]-->f0{{FET0}}-.-fp0[Pull-up]
OC0B-->PB4-->fg1[Mosfet]-->f1{{FET1}}-.-fp1[Pull-up]
PC0-->fg2[Mosfet]-->f2{{FET2}}-.-fp2[Pull-up]
PC1-->fg3[Mosfet]-->f3{{FET3}}-.-fp3[Pull-up]
OC4A-->PC4-->fg4[Mosfet]-->f4{{FET4}}-.-fp4[Pull-up]
PC5-->fg5[Mosfet]-->f5{{FET5}}-.-fp5[Pull-up]
fp0 & fp1 & fp2 & fp3 & fp4 & fp5-.jumper...-pull{Vcc/Vin}
```

### Display
Project was designed to use any of NEXTION displays, which adds flexibility to whole system.</br>
Display must be connected to J1 Connector.</br>
Further informations can be obtained at manufacturers [webpage.](https://nextion.tech/datasheets/)</br>
