/******************************************************************************
 *
 *  IR_IoT_hub
 *
 *  Receives IR commands from a remote and sends associated messages out over
 *  MQTT.
 *
 *  Also watches for MQTT messages and sends out corresponding IR signals.
 *
 *  Has a slight issue in that the two functions briefly block each other.
 *  Use board: ESP32 Dev Module
 *
 *****************************************************************************/

#include <WiFi.h>
#include <IRremote.hpp>
#include <Adafruit_MQTT.h>
#include <Adafruit_MQTT_Client.h>
 // Use my config files in the libraries/ folder
#include <MQTTconfig.h>
#include <WifiConfig.h>

#define PUB_TOPIC "home/irrem"
#define SUB_TOPIC "home/ircmd"
#define HUB_NAME "IRHUB01" // Each device should have a unique name

// Pin assignments
#define IR_SENSOR_PIN    23
#define LED_IR_PIN       32
#define LED_SEND_PIN     33
#define LED_RECV_PIN     22
#define WIFI_CONNECT_LED 21
#define USE_ACTIVE_LOW_OUTPUT_FOR_SEND_PIN

#define WIFI_MAX_TRIES   12
#define ERR_WIFI_CONNECT  1    // Not doing much with error codes yet. One day.

#define HUBNAME_LENGTH    7
#define MSG_DATA_ITEMS    4

/******************************************************************************
 ***  GLOBALS                                                               ***
 *****************************************************************************/

WiFiClient mqttWifiClient;
char mqtt_msg[20];

typedef struct {
  char hub_name[HUBNAME_LENGTH + 1];            // Add one for terminator
  int  data[MSG_DATA_ITEMS];
} msgIn;

typedef enum {
  PROTO,
  ADDR,
  CMD,
  FLAGS
} data_field_t;

// --- MQTT CLIENT ------------------------------------------------------------
// Create an MQTT_Client class to connect to the MQTT server.
Adafruit_MQTT_Client mqtt(&mqttWifiClient, MQTT_BROKER, MQTT_PORT);
Adafruit_MQTT_Subscribe mqtt_sub = Adafruit_MQTT_Subscribe(&mqtt, SUB_TOPIC);
Adafruit_MQTT_Publish mqtt_pub = Adafruit_MQTT_Publish(&mqtt, PUB_TOPIC);

// --- WIFI -------------------------------------------------------------------
const char* ssid[] = { WIFI_SSID_MAIN, WIFI_SSID_ALT };
int wifi_status = WL_IDLE_STATUS;
IPAddress ip;
uint8_t server_errors = 0;

/******************************************************************************
 ***  FUNCTIONS                                                             ***
 *****************************************************************************/

 // Just for a bit of blinkenlight bling.
void flashLED(uint8_t led, uint8_t times, int pulseLen = 100) {
  for (uint8_t i = 0; i < times; i++) {
    digitalWrite(led, HIGH);
    delay(pulseLen / 2);
    digitalWrite(led, LOW);
    delay(pulseLen / 2);
  }
}

// ----------------------------------------------------------------------------
// --- MQTT FUNCTIONS                                                       ---
// ----------------------------------------------------------------------------
// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care of connecting.
void MQTT_connect() {
  if (mqtt.connected()) {
    return;                               // do nothing if already connected
  }
  int8_t ret;
  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) {   // connect() returns 0 for connected
    mqtt.disconnect();
    delay(5000);                          // wait 5 seconds
    retries--;
    if (retries == 0) {
      while (1);                          // die and wait for WDT to reset
    }
  }
}

// Subscription callback
void mqttSubCallback(char* data, uint16_t len) {
  if (len > 0) {
    msgIn msg;
    bool msgParsedOK = parseMQTTmessage(data, &msg);
    if (!msgParsedOK) Serial.println("*** Parsing issue ***");

    Serial.print(msg.hub_name); Serial.print(" >> ");
    for (uint8_t i = 0; i < MSG_DATA_ITEMS; i++) {
      Serial.print(msg.data[i]);
      Serial.print(" ");
    }
    Serial.println();

    // Send the received data as an IR signal. This section probably
    // needs expanding to cope with a wider variety of signals.
    switch (msg.data[PROTO]) {
      case NEC:
        IrSender.sendNEC(msg.data[ADDR], msg.data[CMD], 1);
        flashLED(LED_SEND_PIN, 2);              // flash to confirm
        break;
    }
  }
}

// Doing some error checking here but a malformed message _will_ crash
// the microcontroller. It's the sort of Denial of Service attack
// my wife would use.
bool parseMQTTmessage(char* data, msgIn* msg) {
  bool success = true;              // let's be optimistic
  // Check we actually have data and the string is not too short.
  if (data == NULL || strlen(data) < HUBNAME_LENGTH + (MSG_DATA_ITEMS * 2)) {
    return false;
  }

  // Create a mutable copy of the input string because strtok
  // modifies the string.
  char* tmp_str = strdup(data);

  char* token;                       // to store each item in the string
  uint8_t item_count = 0;

  // Tokenize the string using '_' as the delimiter
  token = strtok(tmp_str, "_");     // gets the first token/item.

  while (token != NULL) {
    if (item_count == 0) {                    // First item: hubname string
      if (strlen(token) != HUBNAME_LENGTH) {  // check it's the correct length
        success = false;
        break;
      }
      strncpy(msg->hub_name, token, HUBNAME_LENGTH);
      msg->hub_name[HUBNAME_LENGTH] = '\0';   // Ensure null termination
    } else if (item_count >= 1 && item_count <= MSG_DATA_ITEMS) {
      // Remaining items: integer values
      char* endptr;
      long val = strtol(token, &endptr, 10);  // strtol for robust conversion
      // Check if conversion was successful and token was a number
      if (*endptr != '\0' || endptr == token) {
        success = false;
        break;
      }
      msg->data[item_count - 1] = (int)val;   // cast to int
    } else {
      success = false;                        // Too many items
      break;
    }
    item_count++;
    token = strtok(NULL, "_");                // Get the next token/item
  }
  return success;
}

// ----------------------------------------------------------------------------
// --- WIFI FUNCTIONS                                                       ---
// ----------------------------------------------------------------------------
uint8_t wifiConnect() {
  uint8_t error = 0;
  uint8_t ssid_idx = 0;
  uint8_t connect_counter = 0;
  while (connect_counter < WIFI_MAX_TRIES) {
    Serial.print("Attempting to connect to "); Serial.println(ssid[ssid_idx]);
    WiFi.begin(ssid[ssid_idx], WIFI_PASSWORD);  // try to connect
    // delay to allow time for connection
    flashLED(WIFI_CONNECT_LED, 10, 250);
    wifi_status = WiFi.status();
    connect_counter++;
    if (wifi_status != WL_CONNECTED) {
      ssid_idx = 1 - ssid_idx;    // swap APs
    } else {
      Serial.println("Connected!");
      connect_counter = WIFI_MAX_TRIES; // to break out of the loop
      ip = WiFi.localIP();
      Serial.print("IP: "); Serial.println(ip);
      server_errors = 0;
    }
  }
  if (wifi_status != WL_CONNECTED) {  // wifi connection failed
    error = ERR_WIFI_CONNECT;
  }
  return error;
}

/******************************************************************************
 ***  SETUP                                                                 ***
 *****************************************************************************/
void setup() {
  uint8_t error = 0;
  delay(1000);
  Serial.begin(115200);

  pinMode(LED_SEND_PIN, OUTPUT);
  pinMode(LED_RECV_PIN, OUTPUT);
  pinMode(WIFI_CONNECT_LED, OUTPUT);

  Serial.println(F("\n\n\n+--------------+"));
  Serial.println(F("|  ESP IR HUB  |"));
  Serial.println(F("+--------------+"));
  Serial.println(F("\nConnecting to wifi"));
  error = wifiConnect();

  if (error == 0) {
    IrReceiver.begin(IR_SENSOR_PIN, ENABLE_LED_FEEDBACK); // Start the receiver
    IrSender.begin(LED_IR_PIN);

    flashLED(LED_SEND_PIN, 2);                            // Show we're alive
    flashLED(LED_RECV_PIN, 2);
    flashLED(WIFI_CONNECT_LED, 2);

    // START MQTT SUBSCRIPTION
    mqtt.subscribe(&mqtt_sub);
    mqtt_sub.setCallback(mqttSubCallback);
  } else {
    while (1) {
      flashLED(LED_SEND_PIN, 2);            // Flash lights in a desperate
      flashLED(LED_RECV_PIN, 2);            // attempt to get attention
      flashLED(WIFI_CONNECT_LED, 2);
      delay(500);
    }
  }
  Serial.println(F("\nRUNNING...\n"));
}

/******************************************************************************
 ***  LOOP                                                                  ***
 *****************************************************************************/

void loop() {
  // Ensure the connection to the MQTT server is alive (this will make the
  // first connection and automatically reconnect when disconnected).
  // See the MQTT_connect function definition above.
  MQTT_connect();

  if (IrReceiver.decode()) {
    if (IrReceiver.decodedIRData.decodedRawData != 0) {
      flashLED(LED_RECV_PIN, 1);
      Serial.println("-----------------------");
      IrReceiver.printIRResultShort(&Serial);
      IrReceiver.printIRSendUsage(&Serial);
      Serial.print("Proto: ");
      Serial.print(IrReceiver.decodedIRData.protocol);
      Serial.print("  +  Addr: ");
      Serial.print(IrReceiver.decodedIRData.address);
      Serial.print("  +  Cmd: ");
      Serial.print(IrReceiver.decodedIRData.command);
      Serial.print("  +  Flags: ");
      Serial.print(IrReceiver.decodedIRData.flags);
      Serial.print("  +  Raw data: ");
      Serial.println(IrReceiver.decodedIRData.decodedRawData, HEX);
      sprintf(mqtt_msg, "%s_%i_%i_%i_%i",
        HUB_NAME,
        IrReceiver.decodedIRData.protocol,
        IrReceiver.decodedIRData.address,
        IrReceiver.decodedIRData.command,
        IrReceiver.decodedIRData.flags);
      mqtt_pub.publish(mqtt_msg);
    }
    IrReceiver.resume();
  }

  // The problem with this is that it's blocking and we might miss incoming
  // IR signals.
  mqtt.processPackets(200);

  // Ping the server to keep the mqtt connection alive.
  // NOT required if you are publishing once every KEEPALIVE seconds
  if (!mqtt.ping()) {
    mqtt.disconnect();
  }

}
