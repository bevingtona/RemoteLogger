Realtime DIY Hydrometric Data Logger with Arduino Feather M0
================

# Introduction

This project that allows you to measure and log hydrometric data in
real-time using an Arduino Feather M0 microcontroller. This project is
perfect for environmental monitoring applications, such as river water
level measurements, flood monitoring, and water quality assessment.

With this hydrometric data logger, you can collect water level data and
transmit it to your e-mail or a cloud service for real-time monitoring
and analysis. The use of the Arduino Feather M0 ensures low power
consumption and compatibility with various sensors, making it a
versatile and reliable solution for your hydrometric data logging needs.

## **Features**

- Realtime measurement (Water level, water temperature, electrical
  conductivity, turbidity, …)

- Low power consumption and solar charging for extended operation

- Customizable data logging intervals

- Data transmission capabilities for remote monitoring (satellite and
  cellular)

- Easily expandable for additional sensors (e.g., temperature, pH)

- Open-source and relatively low-cost

# Wiring Diagrams

1.  Feather M0 + RockBlock + Hydros-21
2.  Feather M0 + RockBlock + Hydros-21 + Analite
3.  Feather M0 + RockBlock + OTT-PLS 500
4.  Feather M0 + RockBlock + OTT-PLS 500 + Analite
5.  Feather M0 + RockBlock + MaxBotix 7052 + SHT-30

# How-to build

1.  Order equipment

2.  Set-up work station

3.  Solder components that require it

4.  Assemble using wiring diagram
- Charge controller: Cut 'therm' trace if thermistor is connected, cut 1.0A trace and solder 1.5A trace for faster charging, test charge rate by connecting multimeter (10A to OUT, COM to GRD) and set multimeter to 200mA, should read 1.5A ([Data sheet](https://cdn-learn.adafruit.com/downloads/pdf/adafruit-bq24074-universal-usb-dc-solar-charger-breakout.pdf))
   
6. Testing

8.  Register modem

9.  Charge battery

10.  Load code

11.  Bench test

# Code

## Input

1.  param.txt

## Output

1.  CSV:
    1.  2023-07-21 06:00:00 4.31V, 23400kb, 234mm, 22.3C, 123EC:
2.  Message:
    1.  ABD:23072106:431,234,234,223,123:

## Logic

1.  Load libraries
2.  Define pins
3.  Define global variables
4.  Setup
    1.  Open Serial connection

    2.  Set pin modes

    3.  Checks, print

    4.  Read params, print

    5.  Sample, print

    6.  Onstart sample, print

    7.  Send irid, print
5.  Loop
    1.  If batt \> 3.8V -\> High power mode

        1.  Blink every 10s, then deep sleep

        2.  Sample and log at interval (usually 10 min)

        3.  Send data at interval (usually 2 hours)

        4.  Sync clock every 5 days at noon

    2.  If batt \< 3.8V -\> Low power mode

        1.  Blink every 30s, then deep sleep

        2.  Sample and log every 1 hr

        3.  Send data every 12 hr

        4.  Sync clock every 5 days at noon


# **Materials**

To build the Hydrometric Data Logger, you will need the following
materials:

## Data Logger with Telemetry

|                                                                  | <sup><sub>**Approx \$ CAD**</sub></sup> | <sup><sub>**Water level**</sub></sup> | <sup><sub>**Water level + Analite**</sub></sup> | <sup><sub>Ultrasonic</sub></sup> | <sup><sub>Temp. only</sub></sup> | <sup><sub>Source</sub></sup>                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             |
|------------------------------------------------------------------|:---------------------------------------:|:-------------------------------------:|:-----------------------------------------------:|:--------------------------------:|:--------------------------------:|----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| <sup><sub>**Data logger**</sub></sup>                            |                                         |                                       |                                                 |                                  |                                  |                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          |
| <sup><sub>Feather M0</sub></sup>                                 |       <sup><sub>27.96</sub></sup>       |        <sup><sub>1</sub></sup>        |             <sup><sub>1</sub></sup>             |     <sup><sub>1</sub></sup>      |     <sup><sub>1</sub></sup>      | <sup><sub>[Digikey](https://www.digikey.ca/en/products/detail/adafruit-industries-llc/2796/5804105)</sub></sup>                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          |
| <sup><sub>Featherwing</sub></sup>                                |       <sup><sub>20.95</sub></sup>       |        <sup><sub>1</sub></sup>        |             <sup><sub>1</sub></sup>             |     <sup><sub>1</sub></sup>      |     <sup><sub>1</sub></sup>      | <sup><sub>[Digikey](https://www.digikey.ca/en/products/detail/adafruit-industries-llc/2926/5959339)</sub></sup>                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          |
| <sup><sub>Latching Relay</sub></sup>                             |       <sup><sub>11.14</sub></sup>       |        <sup><sub>1</sub></sup>        |             <sup><sub>2</sub></sup>             |     <sup><sub>1</sub></sup>      |     <sup><sub>1</sub></sup>      | <sup><sub>[Digikey](https://www.digikey.ca/en/products/detail/adafruit-industries-llc/2923/5979892)</sub></sup>                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          |
| <sup><sub>RTC PCF8523</sub></sup>                                |       <sup><sub>9.74</sub></sup>        |        <sup><sub>1</sub></sup>        |             <sup><sub>1</sub></sup>             |     <sup><sub>1</sub></sup>      |     <sup><sub>1</sub></sup>      | <sup><sub>[Digikey](https://www.digikey.ca/en/products/detail/adafruit-industries-llc/3295/6238007)</sub></sup>                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          |
| <sup><sub>CR1220</sub></sup>                                     |       <sup><sub>8.89</sub></sup>        |      <sup><sub>0.125</sub></sup>      |           <sup><sub>0.125</sub></sup>           |     <sup><sub>1</sub></sup>      |     <sup><sub>1</sub></sup>      | <sup><sub>[Amazon](https://www.amazon.ca/CR1220-ECR1220-Lithium-Cell-Batteries/dp/B07542B14R/ref=sr_1_1_sspa?crid=2LFB4XHHCVEET&keywords=cr1220&qid=1690869545&s=electronics&sprefix=cr1220%2Celectronics%2C139&sr=1-1-spons&sp_csd=d2lkZ2V0TmFtZT1zcF9hdGY&psc=1)</sub></sup>                                                                                                                                                                                                                                                                                                                                                                                                                           |
| <sup><sub>Terminal Block 2P</sub></sup>                          |       <sup><sub>0.95</sub></sup>        |        <sup><sub>2</sub></sup>        |             <sup><sub>4</sub></sup>             |     <sup><sub>2</sub></sup>      |     <sup><sub>2</sub></sup>      | <sup><sub>[Digikey](https://www.digikey.ca/en/products/detail/adafruit-industries-llc/2138/6827101)</sub></sup>                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          |
| <sup><sub>32 GB Micro SD</sub></sup>                             |       <sup><sub>11.29</sub></sup>       |        <sup><sub>1</sub></sup>        |             <sup><sub>1</sub></sup>             |     <sup><sub>1</sub></sup>      |     <sup><sub>1</sub></sup>      | <sup><sub>[Amazon](https://www.amazon.ca/Sandisk-SDSQUAR-032G-GN6MA-Ultra-Micro-Adapter/dp/B073JWXGNT/ref=asc_df_B073JWXGNT/?tag=googleshopc0c-20&linkCode=df0&hvadid=292991886665&hvpos=&hvnetw=g&hvrand=15742836061598794976&hvpone=&hvptwo=&hvqmt=&hvdev=c&hvdvcmdl=&hvlocint=&hvlocphy=9001480&hvtargid=pla-348080513499&psc=1)</sub></sup>                                                                                                                                                                                                                                                                                                                                                          |
|                                                                  |                                         |                                       |                                                 |                                  |                                  |                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          |
| <sup><sub>**3.7V Power**</sub></sup>                             |                                         |                                       |                                                 |                                  |                                  |                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          |
| <sup><sub>3.7V LiPo</sub></sup>                                  |       <sup><sub>24.50</sub></sup>       |        <sup><sub>1</sub></sup>        |             <sup><sub>1</sub></sup>             |     <sup><sub>1</sub></sup>      |     <sup><sub>1</sub></sup>      | <sup><sub>[Adafruit](https://www.adafruit.com/product/353)</sub></sup>                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   |
| <sup><sub>LiPo Fire Bag</sub></sup>                              |       <sup><sub>10.99</sub></sup>       |        <sup><sub>1</sub></sup>        |             <sup><sub>1</sub></sup>             |     <sup><sub>1</sub></sup>      |     <sup><sub>1</sub></sup>      | <sup><sub>[Amazon](https://www.amazon.ca/gp/product/B08KW2TL5G/ref=ppx_yo_dt_b_search_asin_title?ie=UTF8&psc=1)</sub></sup>                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              |
| <sup><sub>Battery Thermistor</sub></sup>                         |       <sup><sub>5.61</sub></sup>        |        <sup><sub>1</sub></sup>        |             <sup><sub>1</sub></sup>             |     <sup><sub>1</sub></sup>      |     <sup><sub>1</sub></sup>      | <sup><sub>[Digikey](https://www.digikey.ca/en/products/detail/adafruit-industries-llc/372/6051772)</sub></sup>                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           |
| <sup><sub>JST Jumper</sub></sup>                                 |       <sup><sub>1.33</sub></sup>        |        <sup><sub>1</sub></sup>        |             <sup><sub>1</sub></sup>             |     <sup><sub>1</sub></sup>      |     <sup><sub>1</sub></sup>      | <sup><sub>[Digikey](https://www.digikey.ca/en/products/detail/adafruit-industries-llc/4714/13175532)</sub></sup>                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         |
| <sup><sub>Charge Controller</sub></sup>                          |       <sup><sub>22.33</sub></sup>       |        <sup><sub>1</sub></sup>        |             <sup><sub>1</sub></sup>             |     <sup><sub>1</sub></sup>      |     <sup><sub>1</sub></sup>      | <sup><sub>[Digikey](https://www.digikey.ca/en/products/detail/adafruit-industries-llc/4755/13231325)</sub></sup>                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         |
| <sup><sub>Solar Panel</sub></sup>                                |      <sup><sub>121.08</sub></sup>       |        <sup><sub>1</sub></sup>        |             <sup><sub>1</sub></sup>             |     <sup><sub>1</sub></sup>      |     <sup><sub>1</sub></sup>      | <sup><sub>[Mouser](https://www.mouser.ca/c/power/solar-panels-solar-cells/?m=Adafruit)</sub></sup>                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                       |
| <sup><sub>Solar Panel Cable</sub></sup>                          |       <sup><sub>6.12</sub></sup>        |        <sup><sub>1</sub></sup>        |             <sup><sub>1</sub></sup>             |     <sup><sub>1</sub></sup>      |     <sup><sub>1</sub></sup>      | <sup><sub>[Digikey](https://www.digikey.ca/en/products/detail/tensility-international-corp/CA-2216/1129479)</sub></sup>                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  |
| <sup><sub>Solar Panel Adapter</sub></sup>                        |       <sup><sub>2.10</sub></sup>        |        <sup><sub>1</sub></sup>        |             <sup><sub>1</sub></sup>             |     <sup><sub>1</sub></sup>      |     <sup><sub>1</sub></sup>      | <sup><sub>[Digikey](https://www.digikey.ca/en/products/detail/adafruit-industries-llc/2788/7244959)</sub></sup>                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          |
|                                                                  |                                         |                                       |                                                 |                                  |                                  |                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          |
| <sup><sub>**12V Power**</sub></sup>                              |                                         |                                       |                                                 |                                  |                                  |                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          |
| <sup><sub>12V Battery</sub></sup>                                |       <sup><sub>33.72</sub></sup>       |                                       |             <sup><sub>1</sub></sup>             |                                  |                                  | <sup><sub>[Digikey](https://www.digikey.ca/en/products/detail/power-sonic-corporation/PS-1270%2520F1/13577496)</sub></sup>                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               |
| <sup><sub>Crimp Connectors</sub></sup>                           |       <sup><sub>0.09</sub></sup>        |                                       |             <sup><sub>2</sub></sup>             |                                  |                                  | <sup><sub>[Amazon](https://www.amazon.ca/dp/B0B4H54KPS/ref=sspa_dk_detail_1?psc=1&pd_rd_i=B0B4H54KPS&pd_rd_w=H6tjt&content-id=amzn1.sym.d8c43617-c625-45bd-a63f-ad8715c2c055&pf_rd_p=d8c43617-c625-45bd-a63f-ad8715c2c055&pf_rd_r=P07VXJ55N2Q3YRMZF5ZC&pd_rd_wg=MMRHw&pd_rd_r=9a85380a-cfb0-4c70-923b-382be7c91366&s=hi&sp_csd=d2lkZ2V0TmFtZT1zcF9kZXRhaWw)</sub></sup>                                                                                                                                                                                                                                                                                                                                  |
|                                                                  |                                         |                                       |                                                 |                                  |                                  |                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          |
| <sup><sub>**Telemetry (Iridium)**</sub></sup>                    |                                         |                                       |                                                 |                                  |                                  |                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          |
| <sup><sub>RockBLOCK 9603N</sub></sup>                            |      <sup><sub>499.99</sub></sup>       |        <sup><sub>1</sub></sup>        |             <sup><sub>1</sub></sup>             |     <sup><sub>1</sub></sup>      |     <sup><sub>1</sub></sup>      | <sup><sub>[Digikey](https://www.digikey.ca/en/products/detail/sparkfun-electronics/WRL-14498/7902308)</sub></sup>                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        |
| <sup><sub>RockBLOCK Cable</sub></sup>                            |       <sup><sub>6.94</sub></sup>        |        <sup><sub>1</sub></sup>        |             <sup><sub>1</sub></sup>             |     <sup><sub>1</sub></sup>      |     <sup><sub>1</sub></sup>      | <sup><sub>[Digikey](https://www.digikey.ca/en/products/detail/sparkfun-electronics/CAB-14720/9373056)</sub></sup>                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        |
| <sup><sub>Antennae</sub></sup>                                   |      <sup><sub>118.23</sub></sup>       |        <sup><sub>1</sub></sup>        |             <sup><sub>1</sub></sup>             |     <sup><sub>1</sub></sup>      |     <sup><sub>1</sub></sup>      | <sup><sub>[Digikey](https://www.digikey.ca/en/products/detail/taoglas-limited/IAA-01-121111/2332661)</sub></sup>                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         |
| <sup><sub>PN22222</sub></sup>                                    |       <sup><sub>0.60</sub></sup>        |        <sup><sub>1</sub></sup>        |             <sup><sub>1</sub></sup>             |     <sup><sub>1</sub></sup>      |     <sup><sub>1</sub></sup>      | <sup><sub>[Adafruit](https://www.adafruit.com/product/756)</sub></sup>                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   |
| <sup><sub>Terminal Block 3P</sub></sup>                          |       <sup><sub>2.68</sub></sup>        |        <sup><sub>1</sub></sup>        |             <sup><sub>1</sub></sup>             |     <sup><sub>1</sub></sup>      |     <sup><sub>1</sub></sup>      | <sup><sub>[Digikey](https://www.digikey.ca/en/products/detail/te-connectivity-amp-connectors/282834-3/1153264)</sub></sup>                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               |
| <sup><sub>Metal Plate</sub></sup>                                |       <sup><sub>11.96</sub></sup>       |       <sup><sub>0.5</sub></sup>       |            <sup><sub>0.5</sub></sup>            |    <sup><sub>0.5</sub></sup>     |    <sup><sub>0.5</sub></sup>     | <sup><sub>[Home Depot](https://www.homedepot.ca/product/paulin-6-x-18-inch-16-gauge-steel-sheet/1000861865?eid=PS_GOOGLE_D25H%20-%20E-Comm_GGL_Shopping_PLA_EN_Hardware_Hardware__PRODUCT_GROUP_pla-1640865175826&gclid=CjwKCAjwt52mBhB5EiwA05YKo3DtEeMO5ZfuEN30yGz32r5iy7peNJhYW1x35Y_PcesGT0Z2AFNZ9xoCLDgQAvD_BwE&gclsrc=aw.ds)</sub></sup>                                                                                                                                                                                                                                                                                                                                                            |
|                                                                  |                                         |                                       |                                                 |                                  |                                  |                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          |
| <sup><sub>**Bulk units**</sub></sup>                             |                                         |                                       |                                                 |                                  |                                  |                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          |
| <sup><sub>24 AWG Hookup Wire</sub></sup>                         |       <sup><sub>41.97</sub></sup>       |      <sup><sub>0.05</sub></sup>       |           <sup><sub>0.05</sub></sup>            |    <sup><sub>0.05</sub></sup>    |    <sup><sub>0.05</sub></sup>    | <sup><sub>[Digikey](https://www.digikey.ca/en/products/detail/adafruit-industries-llc/3174/6198257)</sub></sup>                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          |
| <sup><sub>Desicants</sub></sup>                                  |       <sup><sub>16.99</sub></sup>       |       <sup><sub>0.1</sub></sup>       |            <sup><sub>0.1</sub></sup>            |    <sup><sub>0.1</sub></sup>     |    <sup><sub>0.1</sub></sup>     | <sup><sub>[Amazon](https://www.amazon.ca/Dry-Premium-Packets-Desiccants-Dehumidifier/dp/B00VJ02VMK/ref=sxts_rp_s_1_0?content-id=amzn1.sym.852cfc8b-1c03-4e66-a255-23c00e9b0072%3Aamzn1.sym.852cfc8b-1c03-4e66-a255-23c00e9b0072&cv_ct_cx=desiccants&hvadid=249751390777&hvdev=c&hvlocphy=9001480&hvnetw=g&hvqmt=e&hvrand=17567589881241820109&hvtargid=kwd-324000731735&hydadcr=20848_10090735&keywords=desiccants&pd_rd_i=B00VJ02VMK&pd_rd_r=39c2c56a-01bd-4f72-bb7d-82119c257a2a&pd_rd_w=Yz2w3&pd_rd_wg=4lUgM&pf_rd_p=852cfc8b-1c03-4e66-a255-23c00e9b0072&pf_rd_r=Z9N6WBCQEEKBS6K19JR7&qid=1690871226&sbo=RZvfv%2F%2FHxDF%2BO5021pAnSA%3D%3D&sr=1-1-f0029781-b79b-4b60-9cb0-eeda4dea34d6)</sub></sup> |
| <sup><sub>Shrink Wrap</sub></sup>                                |       <sup><sub>13.99</sub></sup>       |      <sup><sub>0.05</sub></sup>       |           <sup><sub>0.05</sub></sup>            |    <sup><sub>0.05</sub></sup>    |    <sup><sub>0.05</sub></sup>    | <sup><sub>[Amazon](https://www.amazon.ca/xrime-850pcs-Shrink-Tubing-Assortment/dp/B07MJLJKVC/ref=asc_df_B07MJLJKVC/?tag=googleshopc0c-20&linkCode=df0&hvadid=292944236606&hvpos=&hvnetw=g&hvrand=3385782868889843414&hvpone=&hvptwo=&hvqmt=&hvdev=c&hvdvcmdl=&hvlocint=&hvlocphy=9001480&hvtargid=pla-646785267770&psc=1)</sub></sup>                                                                                                                                                                                                                                                                                                                                                                    |
| <sup><sub>Nylon mounting screws</sub></sup>                      |       <sup><sub>25.32</sub></sup>       |      <sup><sub>1.27</sub></sup>       |           <sup><sub>0.05</sub></sup>            |    <sup><sub>0.05</sub></sup>    |    <sup><sub>0.05</sub></sup>    | <sup><sub>[Digikey](https://www.digikey.ca/en/products/detail/adafruit-industries-llc/3299/6596885)</sub></sup>                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          |
| <sup><sub>Wood to mount boards to</sub></sup>                    |       <sup><sub>14.86</sub></sup>       |      <sup><sub>0.125</sub></sup>      |           <sup><sub>0.125</sub></sup>           |   <sup><sub>0.125</sub></sup>    |   <sup><sub>0.125</sub></sup>    | <sup><sub>[Home Depot](https://www.homedepot.ca/product/alexandria-moulding-1-8-inch-x-48-inch-x-48-inch-hardboard-panel/1000132235?rrec=true)</sub></sup>                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               |
| <sup><sub>Cable extension with shield (5 x 24AWG) 1M</sub></sup> |       <sup><sub>7.78</sub></sup>        |                                       |             <sup><sub>5</sub></sup>             |     <sup><sub>5</sub></sup>      |     <sup><sub>5</sub></sup>      | <sup><sub>[Digikey](https://www.digikey.ca/en/products/detail/tensility-international-corp/30-01043/9607986)</sub></sup>                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 |
|                                                                  |                                         |                                       |                                                 |                                  |                                  |                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          |
| <sup><sub>**Enclosure**</sub></sup>                              |                                         |                                       |                                                 |                                  |                                  |                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          |
| <sup><sub>Case</sub></sup>                                       |       <sup><sub>66.48</sub></sup>       |        <sup><sub>1</sub></sup>        |             <sup><sub>1</sub></sup>             |     <sup><sub>1</sub></sup>      |     <sup><sub>1</sub></sup>      | <sup><sub>[Digikey](https://www.digikey.ca/en/products/detail/bud-industries/PTQ-11058/13907380)</sub></sup>                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             |
| <sup><sub>Cable gland 3-6.5mm M12</sub></sup>                    |       <sup><sub>2.75</sub></sup>        |        <sup><sub>1</sub></sup>        |             <sup><sub>1</sub></sup>             |     <sup><sub>1</sub></sup>      |     <sup><sub>1</sub></sup>      | <sup><sub>[Digikey](https://www.digikey.ca/en/products/detail/amphenol-industrial-operations/AIO-CSM12/3904956)</sub></sup>                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              |
| <sup><sub>Cable gland 5-10mm M18</sub></sup>                     |       <sup><sub>3.24</sub></sup>        |        <sup><sub>1</sub></sup>        |             <sup><sub>1</sub></sup>             |     <sup><sub>1</sub></sup>      |     <sup><sub>1</sub></sup>      | <sup><sub>[Digikey](https://www.digikey.ca/en/products/detail/amphenol-industrial-operations/AIO-CSM18/3904948)</sub></sup>                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              |

## Sensors

1.  **Hydrometric** (In-stream)

    1.  [Hydros-21](https://www.metergroup.com/en/meter-environment/products/hydros-21-water-level-sensor-conductivity-temperature-depth)
        (\~900\$CAD) Water level, water temperature, electrical
        conductivity

    2.  [DFRobot](https://www.dfrobot.com/product-1863.html) (\~50\$CAD)
        Water level

    3.  [Analite
        195](https://www.esonetyellowpages.com/manuals/manual-nep180_series_1254507878.pdf)
        (\~2500\$CAD) Turbidity

    4.  [DS18B20](https://www.adafruit.com/product/381) (\~10\$CAD)
        Water temperature

2.  **Hydrometric** (Non-contact)

    1.  [XL-MaxSonar® - WR/WRC MB7052 7m](https://www.digikey.ca/en/products/detail/maxbotix-inc/MB7052-100/7896790)
        (\~170\$CAD) Water level
    2.  [HRXL-MaxSonar-WR MB7368 10m](https://www.digikey.ca/en/products/detail/maxbotix-inc/MB7368-100/10279126) (\~206.08\$CAD) Water level


        

        

3.  **Climate** (Temp/RH/Snow)

    1.  [SHT-30](https://www.digikey.ca/en/products/detail/adafruit-industries-llc/4099/10230011)
        (\~40\$CAD) Temp/RH

    2.  [MaxBotix
        7052](https://www.digikey.ca/en/products/detail/maxbotix-inc/MB7052-100/7896790)
        (\~170\$CAD) Snow depth

## Modems

1.  RockBlock 9603

2.  SWARM

3.  Particle Boron LTE

# How-To Guide

- Reset unit

- Download data

- Change parameters

- Upload code

- Measure battery voltage

- Check if sending

# Extra ..

## Setup PostgreSQL in the cloud

## Setup CGI in the cloud

## Send data to PostgreSQL database via CGI

## Setup R Shiny App in the cloud

## Reminders: Database commands

Linux commands

1.  Change directory: cd
2.  Copy file: cp source.py dest.py
3.  Edit file: vim cgi.py
4.  Vim edit: i
5.  Vim save: Esc :w
6.  Vim save and close: Esc :x
7.  CGI location: cd ../../var/www

Post data to

1.  <http://IPADDRESS/CGI.py?data=HEX&imei=1>
