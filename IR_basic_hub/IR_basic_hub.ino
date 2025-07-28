/*

IR_basic_hub

IR remote input/output

Uses Arduino Nano - IRremote library

Library: https://github.com/Arduino-IRremote/Arduino-IRremote

*/

#include <IRremote.hpp>

// Pin assignments
#define IR_SENSOR_PIN  2
#define LED_IR_PIN     3
#define LED_SEND_PIN   7
#define LED_RECV_PIN   6
#define USE_ACTIVE_LOW_OUTPUT_FOR_SEND_PIN

void flashLED(uint8_t led, uint8_t times, int pulseLen = 100) {
  for (uint8_t i = 0; i < times; i++) {
    digitalWrite(led, HIGH);
    delay(pulseLen / 2);
    digitalWrite(led, LOW);
    delay(pulseLen / 2);
  }
}

void setup() {
  Serial.begin(115200);

  IrReceiver.begin(IR_SENSOR_PIN, ENABLE_LED_FEEDBACK); // Start the receiver
  IrSender.begin(LED_IR_PIN);

  pinMode(LED_SEND_PIN, OUTPUT);
  pinMode(LED_RECV_PIN, OUTPUT);

  flashLED(LED_SEND_PIN, 2);  // Just to show we're alive
  flashLED(LED_RECV_PIN, 2);
}

void loop() {

  // Use ONE of the following two routines - not both.

  // ----- RECEIVE IR SIGNALS -------------------------------------------------
  if (IrReceiver.decode()) {
    if (IrReceiver.decodedIRData.decodedRawData != 0) { // Ignore repeats
      flashLED(LED_RECV_PIN, 1);
      IrReceiver.printIRResultShort(&Serial); // Complete received data
      IrReceiver.printIRSendUsage(&Serial);   // Statement to send this signal
      Serial.print("Protocol: ");
      Serial.println(IrReceiver.decodedIRData.protocol);
      Serial.print("Address : ");
      Serial.println(IrReceiver.decodedIRData.address);
      Serial.print("Command : ");
      Serial.println(IrReceiver.decodedIRData.command);
      Serial.print("Flags   : ");
      Serial.println(IrReceiver.decodedIRData.flags);
      Serial.print("Raw data: ");
      Serial.println(IrReceiver.decodedIRData.decodedRawData, HEX);
      IrReceiver.printIRResultRawFormatted(&Serial, true);
    }
    IrReceiver.resume();
  }

  // ----- SEND IR SIGNAL -----------------------------------------------------
  Serial.println("Sending signal");
  IrSender.sendNEC(0x1, 0x0, 1);
  delay(2000);

}
