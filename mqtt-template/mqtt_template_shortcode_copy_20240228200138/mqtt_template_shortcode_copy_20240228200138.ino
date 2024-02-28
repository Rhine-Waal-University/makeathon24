#include <Arduino.h>      // Include core Arduino functionality
#include <WiFi.h>         // Include WiFi library for WiFi connectivity
#include <PubSubClient.h> // Include the PubSubClient library for MQTT communication
//Include custom libraries

// Network credentials and MQTT broker information
const char* ssid = "FabLabWS";
const char* password = "57289887252221648185KaL!";
const char* mqtt_identifier = "placeholderID";
const char* mqtt_topic = "placeholderTopic";
const char* mqtt_server = "192.168.188.125";
const char* mqtt_user = "homeassistant";
const char* mqtt_pass = "pua0aeShait5Ieh5DieYiesh1lie8oCaexo5righoh6ohkiwuHoos2zoGh3ahxox";
int sending_mqtt_every_ms = 5000;   // Interval for sending MQTT messages in milliseconds

WiFiClient espClient;               // Create an instance of the WiFiClient class for WiFi connectivity
PubSubClient mqttClient(espClient); // Initialize the MQTT client with the WiFi client for internet access
char topicString[50];                     // Buffer for storing the MQTT topic string
unsigned long prevMillis;           // Tracks the last time a message was sent

//Place for defining custom variables

// Sets up WiFi connection using the provided credentials
void setup_wifi() {
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);               // Connect to the WiFi network
    while (WiFi.status() != WL_CONNECTED) {   // Wait until connection is established
        delay(500);                           // Wait for half a second before retrying
        Serial.print(".");
    }
    Serial.println("\nWiFi connected, IP address: ");
    Serial.println(WiFi.localIP());           // Print the IP address once connected
}

// Attempts to reconnect to the MQTT broker if the connection is lost
void reconnect() {
    while (!mqttClient.connected()) {
        Serial.print("Attempting MQTT connection...");
        // Try to connect to the MQTT broker with the given credentials
        if (mqttClient.connect(mqtt_identifier, mqtt_user, mqtt_pass)) {
            Serial.println("connected");
            mqttClient.subscribe(topicString);      // Subscribe to the specified topic
        } else {
            Serial.print("failed, rc=");
            Serial.print(mqttClient.state()); // Print the connection failure reason
            Serial.println(" try again in 5 seconds");
            delay(5000);                      // Wait 5 seconds before retrying
        }
    }
}

void setup() {
    Serial.begin(9600);                     // Initialize serial communication at baud rate
    setup_wifi();                             // Connect to WiFi
    mqttClient.setServer(mqtt_server, 1883);  // Set the MQTT server and port
    prevMillis = millis();                    // Initialize the timer for message sending

    //Define custom Pins etc.
}

void loop() {
    if (!mqttClient.connected()) reconnect(); // Ensure the MQTT connection is alive
    mqttClient.loop();                        // Process MQTT tasks
    unsigned long currentMillis = millis();   // Get the current time
    
    // Check if it's time to send a new message based on the interval
    if (currentMillis - prevMillis > sending_mqtt_every_ms) {
      snprintf(topicString, sizeof(topicString), "%s/%s", mqtt_identifier, mqtt_topic); //formatting of topic String

      char payloadString[15]; // Make sure this is large enough
      //Enter code for custom formatting of payloadString

       mqttClient.publish(topicString, payloadString);  // Publish the flow rate | mqttClient.publish("home/livingroom/temperature", "25.5", true) would be a standard input here. true sets the retain flag to true, the default is false. It should be true for state indicating values and false for values that will change the state of something. doesnt matter in this case.
  
  //debug
  Serial.println("Published: ");
  Serial.print(payloadString);
  Serial.println("To Topic: ");
  Serial.print(payloadString);
  
  prevMillis = currentMillis; // Reset the timer
  }
}
