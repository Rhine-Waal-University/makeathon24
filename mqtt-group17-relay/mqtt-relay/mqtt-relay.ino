#include <Arduino.h>       // Include core Arduino functionality
#include <WiFi.h>          // Include WiFi library for WiFi connectivity
#include <PubSubClient.h>  // Include the PubSubClient library for MQTT communication
//Include custom libraries

// Network credentials and MQTT broker information
const char *ssid = "FabLabWS";
const char *password = "57289887252221648185KaL!";
const char *mqtt_identifier = "group17-relay";
const char *mqtt_topic = "signal";
const char *mqtt_server = "192.168.188.125";
const char *mqtt_user = "mqtt-user";
const char *mqtt_pass = "mqtt";

int sending_mqtt_every_ms = 5000;  // Interval for sending MQTT messages in milliseconds

WiFiClient espClient;                // Create an instance of the WiFiClient class for WiFi connectivity
PubSubClient mqttClient(espClient);  // Initialize the MQTT client with the WiFi client for internet access
char msg[50];
char topicString[50];      // Buffer for storing the MQTT topic string
unsigned long prevMillis;  // Tracks the last time a message was sent

//Place for defining custom variables
int SIGNAL_PIN = 13;

// Sets up WiFi connection using the provided credentials
void setup_wifi() {
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);              // Connect to the WiFi network
  while (WiFi.status() != WL_CONNECTED) {  // Wait until connection is established
    delay(5000);                           // Wait for half a second before retrying
    Serial.print(".");
  }
  Serial.println("\nWiFi connected, IP address: ");
  Serial.println(WiFi.localIP());  // Print the IP address once connected
}

void callback(char *topic, byte *message, unsigned int length) {
  String messageTemp;

  for (int i = 0; i < length; i++) {
    messageTemp += (char)message[i];
  }

    if (String(topic) == topicString) {
    if (messageTemp == "ON") {
      Serial.println("ON");
      digitalWrite(SIGNAL_PIN, HIGH);
    } else if (messageTemp == "OFF") {
      Serial.println("OFF");
      digitalWrite(SIGNAL_PIN, LOW);
    }
  }
}

// Attempts to reconnect to the MQTT broker if the connection is lost
void reconnect() {
  // Loop until we're reconnected
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (mqttClient.connect(mqtt_identifier, mqtt_user, mqtt_pass)) {
      Serial.println("connected");
      // Subscribe power
      strcpy(topicString, mqtt_identifier);
      strcat(topicString, "/signal");
      mqttClient.subscribe(topicString);
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(9600);                       // Initialize serial communication at baud rate
  setup_wifi();                             // Connect to WiFi
  mqttClient.setServer(mqtt_server, 1883);  // Set the MQTT server and port
  mqttClient.setCallback(callback);
  prevMillis = millis();  // Initialize the timer for message sending

  pinMode(SIGNAL_PIN, OUTPUT);
  digitalWrite(SIGNAL_PIN, LOW);
  //Define custom Pins etc.
}

void loop() {
  if (!mqttClient.connected()) reconnect();  // Ensure the MQTT connection is alive
  mqttClient.loop();                         // Process MQTT tasks
}