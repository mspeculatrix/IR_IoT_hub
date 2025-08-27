/******************************************************************************
 *
 *  IR_IoT_hub_receiver
 *
 *  Receives IR commands from a remote and sends associated messages out over
 *  MQTT.
 *
 *  Using ESP8266 board: NodeMCU 1.0
 *
 *****************************************************************************/

#include <ESP8266WiFi.h>
#include <IRremote.hpp>
#include <Adafruit_MQTT.h>
#include <Adafruit_MQTT_Client.h>
 // Use my config files in the libraries/ folder
#include <MQTTconfig.h>
#include <WifiConfig.h>

#define DEBUG true

#define HUB_NAME "IRREC"
#define DEVICE_ID 1

// Pin assignments
#define IR_SENSOR_PIN      5  // D1
#define LED_PIN            2  // D4
// #define USE_ACTIVE_LOW_OUTPUT_FOR_SEND_PIN

/* ----- COMMON STUFF ------------------------------------------------------- */

#define PUB_TOPIC "home/irrem"
#define SUB_TOPIC "home/ircmd"

#define WIFI_MAX_TRIES    12
#define ERR_WIFI_CONNECT   1

#define HUBNAME_LENGTH     5
#define MSG_DATA_ITEMS     5

typedef struct { 		// for MQTT message data
  char hub_name[HUBNAME_LENGTH + 1];  // Add one for terminator
  uint16_t  data[MSG_DATA_ITEMS];
} msgIn;

typedef enum {			// labels for decoded MQTT message data
  DEVID,
  PROTO,
  ADDR,
  CMD,
  FLAGS
} data_field_t;

char mqtt_msg[25];

/* -------------------------------------------------------------------------- */

/******************************************************************************
 ***  GLOBALS                                                               ***
 *****************************************************************************/

WiFiClient mqttWifiClient;

// --- MQTT CLIENT ------------------------------------------------------------
// Create an MQTT_Client class to connect to the MQTT server.
Adafruit_MQTT_Client mqtt(&mqttWifiClient, MQTT_BROKER, MQTT_PORT);
Adafruit_MQTT_Publish mqtt_pub = Adafruit_MQTT_Publish(&mqtt, PUB_TOPIC);

// --- WIFI -------------------------------------------------------------------
const char* ssid[] = { WIFI_SSID_MAIN, WIFI_SSID_ALT };
int wifi_status = WL_IDLE_STATUS;
IPAddress ip;
uint8_t server_errors = 0;

/******************************************************************************
 ***  FUNCTIONS                                                             ***
 *****************************************************************************/

 // Because blinkenlights are essential
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
    flashLED(LED_PIN, 50);                // Also acts as a delay
    retries--;
    if (retries == 0) {
      while (1);                          // die and wait for WDT to reset
    }
  }
}

// ----------------------------------------------------------------------------
// --- WIFI FUNCTIONS                                                       ---
// ----------------------------------------------------------------------------
uint8_t wifiConnect() {
  uint8_t error = 0;
  uint8_t ssid_idx = 0;
  uint8_t connect_counter = 0;
  const char* ssid = WIFI_SSID_MAIN;
  const char* password = WIFI_PASSWORD;
  WiFi.mode(WIFI_STA);
  while (connect_counter < WIFI_MAX_TRIES) {
    if (DEBUG) {
      Serial.print("Attempting to connect to ");
      Serial.println(ssid);
    }
    WiFi.begin(ssid, password);  // try to connect
    flashLED(LED_PIN, 50);       // Delay to allow connection
    wifi_status = WiFi.status();
    connect_counter++;
    if (wifi_status != WL_CONNECTED) {
      ssid_idx = 1 - ssid_idx;    // swap APs
      if (DEBUG) Serial.println("- connection failed");
    } else {
      connect_counter = WIFI_MAX_TRIES; // to break out of the loop
      ip = WiFi.localIP();
      if (DEBUG) {
        Serial.println("Connected!");
        Serial.print("IP: "); Serial.println(ip);
      }
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

  pinMode(LED_PIN, OUTPUT);

  if (DEBUG) {
    Serial.println(F("\n\n\n+---------------------+"));
    Serial.println(F("|   IR HUB RECEIVER   |"));
    Serial.println(F("+---------------------+"));
    Serial.println(F("\nConnecting to wifi"));
  }
  error = wifiConnect();

  if (error == 0) {
    IrReceiver.begin(IR_SENSOR_PIN, ENABLE_LED_FEEDBACK); // Start the receiver
    flashLED(LED_PIN, 2);

  } else {
    while (1) {
      flashLED(LED_PIN, 2);            // attempt to get attention
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
      flashLED(LED_PIN, 1);
      if (DEBUG) {
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
        sprintf(mqtt_msg, "%s_%u_%u_%u_%u_%u",
          HUB_NAME,
          DEVICE_ID,
          IrReceiver.decodedIRData.protocol,
          IrReceiver.decodedIRData.address,
          IrReceiver.decodedIRData.command,
          IrReceiver.decodedIRData.flags);
      }
      mqtt_pub.publish(mqtt_msg);
    }
    IrReceiver.resume();
  }

  // Ping the server to keep the mqtt connection alive.
  // NOT required if you are publishing once every KEEPALIVE seconds
  if (!mqtt.ping()) {
    mqtt.disconnect();
  }

}
