# IR_IoT_hub

## APPLICATIONS

This project consists of five applications:

- **IR_basic_hub**: Based on the Arduino Nano (although pretty much any microcontroller supported by the Arduino IDE would do). This is a 'scratchpad' app for playing around with IR signals. It has no IoT/network component.
- **IR_IoT_hub_receiver**: Receives IR signals (eg, those emitted by a remote control) and sends out the data as an MQTT message. Based on an ESP8266 board.
- **IR_IoT_hub_sender**: Watches for suitable MQTT messages and converts them to corresponding IR signals. Also based on an ESP8266.
- **IR_IoT_hub**: Combines both sender and receiver functions. Based on an ESP32 board.
- **IR_IoT_hub_multi**: A variation on `IR_IoT_hub` but using the ESP32's multitasking capabilities.
