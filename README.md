# IR_IoT_hub

## APPLICATIONS

This project consists of five applications:

- **IR_basic_hub**: Based on the Arduino Nano. This is a 'scratchpad' app for playing around with IR signals. It has no IoT/network component.
- **IR_IoT_hub_receiver**: Receives IR signals (eg, those emitted by a remote control) and sends out the data as an MQTT message. Based on an ESP8266 board.
- **IR_IoT_hub_sender**: Watches for suitable MQTT messages and converts them to corresponding IR signals. Also based on an ESP8266.
- **IR_IoT_hub**: Combines both sender and receiver functions. Based on an ESP32 board.
- **IR_IoT_hub_multi**: A variation on `IR_IoT_hub` but using the ESP32's multitasking capabilities.

## DEV BOARDS

I based these projects on the microcontroller dev boards I happened to have lying around. From an electronics point of view, they are so simple that they could be converted easily to work with your preferred choice of microcontroller. For example, the Raspberry Pi Pico W would be a good candidate. That said, the code for the ESP32 multitasking version is somewhat specific to that microcontroller.

## INTERPRETING SIGNALS & MESSAGES

It's up to you which signals you want to respond to and which MQTT messages are treated as valid (and which signals you send as a result). This code just has some examples. Adapt for your own purposes.
