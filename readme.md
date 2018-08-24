## State model

State name | button color | LLED | RLED
--------|---------|------|-----
OFF timeout | yellow
OFF manual | 
OFF init | 
LEFT | 
RIGHT | 
BOTH | 
Unarmed Motion |
Armed Motion |   | Green | Green


## Photon Pinout


pin | Function | pin | Function
----| ------- | ----| -------
VIN|  | 3v3 |
GND|  | RST |
Tx|  | VBAT |
Rx|  | GND |
WKP| btn blue | D7 |
DAC|  | D6 | SW2
A5| btn green  | D5 | APDS9960 INT
A4| btn red  | D4 | underpin strip
A3|   | D3 |
A2|  | D2 | PIXEL_PIN
A1|   | D1 | APDS9960 SCL
A0| PIR  | D0 | APDS9960 SDA
---

## [AdafruitRugged Metal Pushbutton - 16mm 6V RGB Momentary](https://www.adafruit.com/product/3350) ![](https://cdn-shop.adafruit.com/145x109/3350-01.jpg)
[datasheet](https://cdn-shop.adafruit.com/product-files/3350/C5279+datasheet+PM161F-10E-RGB-12V-S-IP67.pdf)


Mark | Func | wire color | conn | RJ45 |RJ45color | Breadboard
-----|------|-----|---|---|---|---
R | red | red | A4  | 2 | orange | E8
SW1 | switch 1 | grey  | gnd  | 7 | W/brn | E2
G |green | green | A5 | 6  | grn | E7
B | blue | blue | WKP | 4 | blue | E5
Sw2 | switch2 | white | D6 | 8 | brn | F6
C+ | cathode | orange | +5V | 1 | w/orange | E1

---
## [APDS 9960](https://www.adafruit.com/product/3595?gclid=CjwKCAjwkMbaBRBAEiwAlH5v_vXEeEj5wqdsNyfCPhlPawC_t2XqHsi7MnRGNWbrNEu6n_z87asYChoCqK0QAvD_BwE)  ![](https://cdn-shop.adafruit.com/145x109/3595-00.jpg)

Pinout L->R

pin | brd |P0 |pos1 | pos2
--|--|--|--|--
1 | VIN|||
2 | 3V  | Vin | D1 | G26
3 | gnd  | gnd | D2 | G27
4 | SCL | D1 | G11 | G28
5 | SDA | D0 | G12 | G29
6 | Int | D5 | G7 | G30
