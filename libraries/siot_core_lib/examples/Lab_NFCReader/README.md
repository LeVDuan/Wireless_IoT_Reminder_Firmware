# NFC READER

## Overview

- Put your NFC Card (13.56HMz) over the NFC reader module
- to view the NFC card:
  - acccess the SIoT Data Hub website, or
  - connect the device via rs232 cable with baud-rate 115200, or

![Data in SiOIT](../../images/4d4013f214a56aa918cd860acfef6b7b34d33b35cec0a1abf57ce543133b2d6c.png)  



## Pins and codes for board DOIT ESP32 DevKit

```Arduino
#define SS_PIN 21     //   SS/SDA
#define RST_PIN 0    //   Reset
```

*In the example, pin settings are changed automatically by your board setting. So you just connect board to MRC522 as images belows and keep the source code.*

[![DOIT ESP32 DevKit](http://img.youtube.com/vi/VQAy33XYFEY/0.jpg)](https://www.youtube.com/embed/VQAy33XYFEY?si=405xoTWnA44ltDjW "Video Title")


## Pins and codes for board ESP32-C3-DevKitM-1 dual USB

```Arduino
  #define SS_PIN   5      //SS/SDA
  #define SCLK_PIN 4  
  #define MOSI_PIN 18 
  #define MISO_PIN 19 
  #define RST_PIN   0 
```

*In the example, pin settings are changed automatically by your board setting. So you just connect board to MRC522 as images belows and keep the source code.*

![ESP32-C3-DevKitM-1 dual USB img1](../../images/20230820_115109.jpg)
![ESP32-C3-DevKitM-1 dual USB img2](../../images/20230820_115129.jpg)

## Pins and codes for board Wemos D1 mini

```Arduino
  #define SS_PIN   15 
  #define RST_PIN  0
```

*In the example, pin settings are changed automatically by your board setting. So you just connect board to MRC522 as images belows and keep the source code.*

![Wemos D1 mini 1](../../images/wemos_d1_mini_1.png)
![Wemos D1 mini 2](../../images/wemos_d1_mini_2.jpg)
![Wemos D1 mini 3](../../images/wemos_d1_mini_3.jpg)
