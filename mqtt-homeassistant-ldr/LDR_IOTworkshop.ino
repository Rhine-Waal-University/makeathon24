#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

// WiFi-Credentials
const char *ssid = "FabLabWS"; // SSID of your WiFi network
const char *password = "57289887252221648185KaL!"; // Password of your WiFi network

// MQTT Broker IP address
const char *mqtt_identifier = "LDR"; // MQTT client identifier
const char *mqtt_server = "192.168.188.125"; // IP address of MQTT broker
const char *mqtt_user = "homeassistant"; // MQTT username
const char *mqtt_pass = "pua0aeShait5Ieh5DieYiesh1lie8oCaexo5righoh6ohkiwuHoos2zoGh3ahxox"; // MQTT password

// Interval for sending MQTT messages
int sending_mqtt_every_ms = 5000; // Time interval for sending MQTT messages (in milliseconds)


WiFiClient espClient; // Create an instance of WiFi client
PubSubClient client(espClient); // Create an instance of MQTT client
long lastMsg = 0; // Variable to store the timestamp of the last message sent
char topic[50]; // Buffer to store MQTT topic

// These constants won't change. They're used to give names to the pins used:
const int analogInPin = A6;  // Analog input pin that the LDR is attached to
const int analogOutPin = 2;  // Analog output pin that the LED is attached to

int sensorValue = 0;  // value read from the LDR
int outputValue = 0;  // value output to the PWM (analog out)

unsigned long prevMillis; // Variable to store the timestamp of the previous execution


// Function to set up WiFi connection
void setup_wifi()
{
    delay(10);
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password); // Connect to WiFi network with provided credentials
    while (WiFi.status() != WL_CONNECTED) // Wait for WiFi connection to be established
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP()); // Print local IP address assigned to the device
}

// MQTT callback function
void callback(char *topic, byte *message, unsigned int length)
{
    // Handle MQTT messages if needed
}

// Function to reconnect to MQTT broker
void reconnect()
{
    while (!client.connected()) // Check if MQTT client is not connected
    {
        Serial.print("Attempting MQTT connection...");
        if (client.connect(mqtt_identifier, mqtt_user, mqtt_pass)) // Attempt to connect to MQTT broker
        {
            Serial.println("connected");
            client.subscribe(topic); // Subscribe to MQTT topic
        }
        else
        {
            Serial.print("failed, rc=");
            Serial.print(client.state()); // Print MQTT connection state
            Serial.println(" try again in 5 seconds");
            delay(5000); // Wait 5 seconds before retrying connection
        }
    }
}




void setup() {
  delay(5000);
  // initialize serial communications at 9600 bps:
  Serial.begin(9600);
  while(!Serial);
  Serial.println("Test");
  //attachInterrupt(digitalPinToInterrupt(FLOW_SENSOR_PIN), pulseCounter, FALLING); // Attach interrupt to the pin to count pulses from the sensor

  setup_wifi(); // Set up WiFi connection
  client.setServer(mqtt_server, 1883); // Configure MQTT server and port
  client.setCallback(callback); // Set callback function for MQTT client

  prevMillis = millis(); // Initialize previous timestamp variable

}

void loop() {
  
  
    if (!client.connected()) // Check if MQTT client is not connected
    {
        reconnect(); // Reconnect to MQTT broker
    }
    client.loop(); // Maintain MQTT client connection

    unsigned long currentMillis = millis(); // Get current timestamp
    if (currentMillis - prevMillis > sending_mqtt_every_ms) // Check if sending interval has elapsed
    {
        mesaureLight();
        Serial.print("Light Level: ");
        Serial.print(outputValue);
        Serial.println(" Lux");

        // Convert the flow rate value to a char array
        char ldrString[8];
        dtostrf(outputValue, 1, 2, ldrString);

        strcpy(topic, mqtt_identifier); // Copy MQTT client identifier to topic buffer
        strcat(topic, "/light_level"); // Append flow rate topic to MQTT client identifier
        client.publish(topic, ldrString); // Publish flow rate value to MQTT broker

        prevMillis = currentMillis; // Update previous timestamp
    }
}
void mesaureLight() {
  // read the analog in value:
  sensorValue = analogRead(analogInPin);
  // map it to the range of the analog out:
  outputValue = map(sensorValue, 1750, 4096, 0, 255);
  // change the analog out value:
  analogWrite(analogOutPin, outputValue);

  // print the results to the Serial Monitor:
  Serial.print("sensor = ");
  Serial.print(sensorValue);
  Serial.print("\t output = ");
  Serial.println(outputValue);

}



