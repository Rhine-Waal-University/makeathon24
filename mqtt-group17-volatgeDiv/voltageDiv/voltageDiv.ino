#include "Adafruit_ADS1X15.h"
#include "Wire.h"
#include <WiFi.h>
#include <PubSubClient.h>

Adafruit_ADS1115 ads;  // Ensure this matches the library's constructor signature.

// WiFi-Credentials
const char *ssid = "FabLabWS";
const char *password = "57289887252221648185KaL!";

// MQTT Broker IP address
const char *mqtt_identifier = "VoltageDiv";
const char *mqtt_server = "192.168.188.125";
const char *mqtt_user = "mqtt-user";
const char *mqtt_pass = "mqtt";

long lastMsg = 0;
char msg[50];
char topic[50];
float R1 = 10000;
float R2 = 3000;
long sending_mqtt_every_ms = 5000;



WiFiClient espClient;
PubSubClient client(espClient);

int value = 0;

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char *topic, byte *message, unsigned int length) {
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(mqtt_identifier, mqtt_user, mqtt_pass)) {
      Serial.println("connected");
      // Subscribe
      strcpy(topic, mqtt_identifier);
      strcat(topic, "/output");
      client.subscribe(topic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}


void setup() {
  Serial.begin(9600);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  ads.begin();  // Start communication with ADS1115
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > sending_mqtt_every_ms) {
    lastMsg = now;

    int16_t adc0;
    adc0 = ads.readADC_SingleEnded(0);
    float voltage = adc0 * (6.144 / 32768.0) * ((R1 + R2) / R2);

    Serial.print("Voltage = ");
    Serial.println(voltage);

    snprintf(msg, 50, "%f", voltage);  // Convert voltage to a string
    client.publish(topic, msg);        // Publish voltage
  }
}