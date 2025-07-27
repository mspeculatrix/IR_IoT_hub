# IR_IoT_hub

IR signal hub for home automation.

This has two main functions:

- Receiving IR signals (eg, from a remote) and converting them to MQTT messages.
- Receiving MQTT messages and turning them into outgoing IR signals.

This version has a slight problem in that the two functions briefly block each other, which can lead to missed signals/messages. I've addressed this with another version - `IR_IoT_hub_multi` - that uses multitasking.

## IR remote receiver

When the hub receives an IR signal from a remote, it sends it out over the network as an MQTT message using the topic `home/irrem`.

The format of the message is:

`<hubname>_<protocol>_<address>_<command>_<flags>`

Here's an example:

`IRHUB01_8_0_68_0`

The format is strict.

- **hubname** - 7 chars
- Four integer values (some might be 16-bit).

## IR networked emitter

When the hub picks up an MQTT message on the topic `home/ircmd` - using the same format as above - it generates a corresponding IR signal.

## NOTES

The software expects two 'libraries' in your main `libraries` folder. They are:

- MQTTconfig
- WifiConfig

Each of these just contains a header file with some defines. I've provided examples of these here. Just edit the header files and then move the folders into your `libraries` folder.

In my case, I have two access points at home with different names (SSIDs) but the same password, which is why I've configured 'main' and 'alt' SSIDs. You can just set both SSIDs to be the same if you're using only one access point.

## RESOURCES

- GitHub repo for this project: https://github.com/mspeculatrix/ESP32_IR_hub
- IRremote library: https://github.com/Arduino-IRremote/Arduino-IRremote
- Adafruit MQTT library: https://github.com/adafruit/Adafruit_MQTT_Library
