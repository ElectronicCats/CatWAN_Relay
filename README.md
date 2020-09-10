# CatWAN Relay


<a href="https://electroniccats.com/store/catwan-relay-board/">
  <img src="https://electroniccats.com/wp-content/uploads/badge_store.png" height="104" />
</a>

Are you interested in applying LoRa in an industrial environment or even connect a CNC or a PLC as well to a LoRaWAN network? 

This board allows you to communicate with every LoRa device which works in the 915 Mhz band.

In this board you will find 3 relays outputs and 3 opto-isolated inputs, and it can be powered from 12v to 24v with a on/off jumper.

By now the provided firmware works as an invisible way to connect two wireless relays based on interrupts, this board its compatible with the [Arduino LoRa library](https://github.com/sandeepmistry/arduino-LoRa) from @sandeepmistry but it can work with the [Radio Head library](https://www.airspayce.com/mikem/arduino/RadioHead/) this using only the LoRa radio if you want to use LoRaWAN you should use the [Arduino LMIC](https://github.com/matthijskooijman/arduino-lmic/tree/non-avr-printf).

This device can work in networks LoRaWAN compatible with classes A, B and C, although we currently do not have a firmware for this way of working. The CatWAN Relay firmware is completely open source and you can find it in our repository along with the schematic. If you want to reprogram this device you can do it through Arduino IDE and its USB port or if you do not have to use a J-Link. ATMEL-ICE or a DIY SWD programmer

This device has a SAMD21 ARM Cortex microcontroller at 48Mhz with native USB 2.1, with 256Kb for programming, compatible with Arduino and Circuit Python.

Check [our repository](https://github.com/ElectronicCats/Arduino_Boards_Index) for Arduino IDE more details and software downloads!

Designed by Electronic Cats.
# License 

![OpenSourceLicense](https://github.com/ElectronicCats/AjoloteBoard/raw/master/OpenSourceLicense.png)

Firmware released under an GNU AGPL v3.0 license. See the LICENSE file for more information.

Hardware released under an CERN Open Hardware Licence v1.2. See the LICENSE_HARDWARE file for more information.

Electronic Cats is a registered trademark, please do not use if you sell these PCBs.
