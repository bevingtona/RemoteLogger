# "Prototyping" - New Projects

Multiple new projects were approached in connection with the remote logger project, for possible eventual addition to the system. These projects are contained in this folder and briefly documented in this document with the intention of someone else being able to pick them up later and continue without having to start over from scratch. 

## Table of Contents
[**ArduCAM - compact camera**](#arducam---compact-camera)<br>
&ensp;&ensp;[Timelapse camera with telemetry](#a-timelapse-camera-with-telemetry)<br>
[**New sensors**](#new-sensors)<br>
&ensp;&ensp;[RTK GNSS - SparkFun ZED-F9P](#rtk-gnss---sparkfun-zed-f9p)<br>
&ensp;&ensp;[Distance - SparkFun VL53L1X](#distance---sparkfun-vl53l1x)<br>
[**New telemetry**](#new-telemetry)<br>
&ensp;&ensp;[SWARM satellite modem](#swarm-satellite-modem)<br>
&ensp;&ensp;[Cellular LTE - Particle Boron](#cellular-lte---particle-boron)<br>
[**System**](#system)<br>
&ensp;&ensp;[Sending multiple messages in a single power cycle](#sending-multiple-messages-in-a-single-power-cycle)<br>
&ensp;&ensp;[Multiple I2C devices](#connecting-multiple-i2c-devices)<br>
&ensp;&ensp;[Switching to CircuitPython](#switching-to-circuitpython)<br>

---- 

### ArduCAM - Compact camera
- uses SPI and I2C communication
- this one is a OV5642 module 
#### A timelapse camera with telemetry
- timelapse camera using ArduCAM, RockBlock 9603, TPL, Feather Adalogger
    - photos taken as JPEGS every 2 hours and stored on SD card, sent as JPEGS over Iridium every 24 hours
    - timing is approximate due to TPL intervals but time is kept by high-precision RTC
- to send: JPEG image is cut into pieces 326 bytes long and sent as binary across Iridium network
    - JPEG images are variable size so this creates between 30 and 80 messages 
    - reconstructed on other side with simple Python code
- in theory telemetry method could be swapped out for something less expensive (main issue is expense of all those Iridium messages - at worst case 80 * 7 = 560 credits for a single image as all messages are full-size)
- each message starts with a 14-byte header
    - 0: sequence number for this image (1B)
    - 1: total messages in the sequence for this image (1B)
    - 2-3: image size in bytes (2B)
    - 4-6: image height and width in pixels (12 bits each)
    - 7: year of capture (1B)
    - 8: month of capture (1B)
    - 9: day of capture (1B)
    - 10: hour of capture (1B)
    - 11: image ID (1B)
    - 12-13: extra space in case there is more information needed in the header in future
- images are named with timestamp on SD card and reconstructed with the same name on the receiving side 
- Iridium can only send a message every ~40 seconds so send 12 messages per power cycle 
    - this means the receiving end needs to wait until it has all the messages to assemble the image

[back to top](#table-of-contents)

----

### New sensors
#### RTK GNSS - Sparkfun ZED-F9P
- I2C, default address 0x42
- can change the address to a different one
- never managed to get a GPS reading from it - always zeroes - but got time
    - does not work indoors or near buildings, may need a larger antenna
#### Distance - SparkFun VL53L1X
- I2C easy, default address 0x29
- quite fast to read distance
- can't change the address (can only have one on a single bus)

[back to top](#table-of-contents)

----

### New telemetry
#### SWARM satellite modem
- SWARM modem M138 
- they are not selling this modem anymore but Alex has a few and the network is still active
- need to set up with a ground plane and antenna outside 
- needs a GPS fix before it can work
    - do not stick the GPS antenna to a conductive surface
- set up SWARM network through Hive platform
    - $5 USD/month gets you 750 data packets (192B each), buy as a yearly subscription, can stack up to four on a single modem (3000 data packets per month)
    - to start you get some free data packets - good for prototyping
- can access messages through Hive, via REST API, or via webhooks (to site or database)
- can save up to 1000 outgoing data packets for up to 48 hours (this is changeable)
- Hive discards your data stored there after 30 days
- to hook up to an Arduino/other MCU, cut the traces between TXO - USB RXI and RXI - USB TXO (its default state is designed to hook up to computer with USB cable)
- can connect power to 3.3V if it can deliver enough current (1000mA) while transmitting - otherwise attach to 5V power
- use the Arduino SparkFun Swarm library 
- SWARM network does not have continuous coverage - have to predict pass times to send the messages 
    - there's an online predictor tool for this or a function to calculate it onboard
#### Cellular LTE - Particle Boron
- Particle website - account: rpagdin@unbc.ca password:boron101 - registered there or you can register to a separate account without deregistering
- have a web IDE and Particle Workbench extension in VSCode
- Sandbox plan - up to 100 devices, 100 000 data operations per month, 100 MB cellular data (free)
- communicate btwn Particle units and to external systems with `subscribe` and `publish`
- publish to webhooks -- database or website to catch them (telemetry)
- looks like Arduino but in fact is completely separate - all libraries are totally separate and internal and there are a few small syntax differences
    - missing libraries: CSV Parser, SDI-12
    - easy-to-use I2C library
    - SD library - would need an SD breakout as there is no built-in SD slot on the Particle Boron
- takes a while to connect on startup and the visual aid of LED is not always accurate
- had some issues flashing code over cellular so used Particle Workbench to connect via USB 

[back to top](#table-of-contents)

----

### System
#### Sending multiple messages in a single power cycle
- considered for when there is a backlog of data to send (sending has been disrupted for some reason for a while)
    - with most systems an Iridium message can accomodate 18 samples safely
    - particularly useful for low-power mode, since with a send only every 24 hours there will always be more data than can fit in one message
- can actually save money on telemetry since fewer messages are sent total and they may use credits more efficiently
- possible to produce multiple messages, concatenate them together in the `prep_msg` function, then separate them in the `send_msg` function and send them separately
- what happens if one of the multiple messages does not send successfully but others do?
    - only consider unsuccessful if all were unsuccessful -- avoid duplicates in the database (best effort basis)
- with too many messages and unsuccessful sends, the message send timeout may exceed the TPL timeout, which would then restart the system
- it would be helpful to be able to delete the first n lines of the hourly data file rather than having to remove the whole thing

#### Connecting multiple I2C devices
- can have up to 127 items on one bus (SDA/SCL) limited by address space of 7 bits
- need an instance of every device (not just every type) - each has its own address
- QWIIC cable to daisy-chain
#### Switching to CircuitPython
It was considered as option to convert the library and code over to CircuitPython because it is much simpler to write. Ultimately decided not to do that as the costs far outweigh the benefits.

Pros:
- easy string handling
- more developer resources in the form of documentation and forums

Cons:
- takes a lot more nonvolatile memory space on MCU - interpreter lives there
- library files have to be compiled by third-party software to .mpy files to be small enough to store on device -- have to be recompiled every time they are changed
    - all library files need to be stored on device
- slower
- no string encode/decode (cut off of full size Python) --> won't work for Iridium no matter what
- also no support for SWARM or Particle -- basically no telemetry
- no libraries built-in for SDI-12 and Iridium comms -- way too much work to write our own for minimal reward

[back to top](#table-of-contents)

----