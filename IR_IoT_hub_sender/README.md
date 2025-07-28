# IR_IoT_hub_sender

Based on an ESP8266 dev board. This listens for MQTT messages and sends out corresponding IR signals.

You need to decide in the mqttSubCallback() function which messages to respond to and which signals they send. What's in the code is just a very basic example.
