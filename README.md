# IR_IoT_hub

## OPERATIONS

There are two main operations:

- SENDER: Responds to an MQTT message sent across the network and emits a corrsponding IR signal.
- RECEIVER: Responds to an IR signal (eg, from a remote) and generates a corresponding MQTT message.

## APPLICATIONS

This project consists of four variations on a theme:

- **IR_IoT_hub_basic**: Sender and receiver for pretty much any Arduino IDE-compatible microcontroller. This is a 'scratchpad' app for playing around with IR signals. It has no IoT/network component. WORKING.
- **IR_IoT_hub_receiver**: Receiver-only version. Based on an ESP8266 board. WORKING.
- **IR_IoT_hub_sender**: Sender-only version. Also based on an ESP8266. WORKING.
- **IR_IoT_hub_ESP32**: Combines both sender and receiver functions. Based on an ESP32 board. You can choose at compilation time whether to use multitasking. The non-multitasking version works. The multitasking version not so much.

### IR remote receiver

When the hub receives an IR signal from a remote, it sends it out over the network as an MQTT message using the topic `home/irrec`.

The format of the message is:

`<device_type>_<device_id>_<protocol>_<address>_<command>_<flags>`

Here's an example:

`IRHUB_10_8_0_68_0`

The format is strict.

- **device_type** - 5 chars
- Five integer values (16-bit).

### IR networked sender

When the hub picks up an MQTT message on the topic `home/ircmd` - using the same format as above - it generates a corresponding IR signal. Malformed messages will have unpredictable consequences.

## NOTES

The software expects two 'libraries' in your main Arduino `libraries` folder. They are:

- MQTTconfig
- WifiConfig

Each of these just contains a header file with some defines. I've provided examples of these here. Just edit the header files and then move the folders into your `libraries` folder.

In my case, I have two access points at home with different names (SSIDs) but the same password, which is why I've configured 'main' and 'alt' SSIDs. You can just set both SSIDs to be the same if you're using only one access point.

## RESOURCES

- GitHub repo for this project: https://github.com/mspeculatrix/IR_IoT_hub
- IRremote library: https://github.com/Arduino-IRremote/Arduino-IRremote
- Adafruit MQTT library: https://github.com/adafruit/Adafruit_MQTT_Library

## DEV BOARDS

I based these projects on the microcontroller dev boards I happened to have lying around. From an electronics point of view, they are so simple that they could be converted easily to work with your preferred choice of microcontroller. For example, the Raspberry Pi Pico W would be a good candidate. Just select the right pins for your needs.

That said, the code for the ESP32 multitasking version is somewhat specific to that microcontroller.

The boards I actually use are:

- Basic: Arduino Nano
- Sender & Receiver: NodeMCU 1.0 (ESP8266)
- ESP32: DoIT generic ESP32 Dev Module.

## INTERPRETING SIGNALS & MESSAGES

It's up to you which signals you want to respond to and which MQTT messages are treated as valid (and which signals you send as a result). This code just has some examples. Adapt for your own purposes.
