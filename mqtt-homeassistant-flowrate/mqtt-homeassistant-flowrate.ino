#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

// Define digital pin connected to the water flow sensor
#define FLOW_SENSOR_PIN 2
// Define pulses per liter (adjust according to your sensor)
#define PULSE_FACTOR 7.5

// WiFi-Credentials
const char *ssid = "FabLabWS"; // SSID of your WiFi network
const char *password = "57289887252221648185KaL!"; // Password of your WiFi network

// MQTT Broker IP address
const char *mqtt_identifier = "WaterflowSensor"; // MQTT client identifier
const char *mqtt_server = "192.168.188.125"; // IP address of MQTT broker
const char *mqtt_user = "mqtt-user"; // MQTT username
const char *mqtt_pass = "mqtt"; // MQTT password

// Interval for sending MQTT messages
int sending_mqtt_every_ms = 5000; // Time interval for sending MQTT messages (in milliseconds)

WiFiClient espClient; // Create an instance of WiFi client
PubSubClient client(espClient); // Create an instance of MQTT client
long lastMsg = 0; // Variable to store the timestamp of the last message sent
char topic[50]; // Buffer to store MQTT topic

// Variables for tracking pulse count and flow rate
volatile unsigned long pulseCount; // Variable to store the pulse count from the sensor
float flowRate; // Variable to store the calculated flow rate
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

void setup(void)
{
    Serial.begin(115200); // Initialize serial communication

    pinMode(FLOW_SENSOR_PIN, INPUT_PULLUP); // Configure the pin connected to the sensor as input with internal pull-up resistor
    attachInterrupt(digitalPinToInterrupt(FLOW_SENSOR_PIN), pulseCounter, FALLING); // Attach interrupt to the pin to count pulses from the sensor

    setup_wifi(); // Set up WiFi connection
    client.setServer(mqtt_server, 1883); // Configure MQTT server and port
    client.setCallback(callback); // Set callback function for MQTT client

    prevMillis = millis(); // Initialize previous timestamp variable
}

void loop()
{
    if (!client.connected()) // Check if MQTT client is not connected
    {
        reconnect(); // Reconnect to MQTT broker
    }
    client.loop(); // Maintain MQTT client connection

    unsigned long currentMillis = millis(); // Get current timestamp
    if (currentMillis - prevMillis > sending_mqtt_every_ms) // Check if sending interval has elapsed
    {
        detachInterrupt(digitalPinToInterrupt(FLOW_SENSOR_PIN)); // Detach interrupt to prevent interference while calculating flow rate
        flowRate = pulseCount / PULSE_FACTOR; // Calculate flow rate based on pulse count and pulse factor
        pulseCount = 0; // Reset pulse count for the next interval

        Serial.print("Flow rate: ");
        Serial.print(flowRate);
        Serial.println(" L/min");

        // Convert the flow rate value to a char array
        char flowString[8];
        dtostrf(flowRate, 1, 2, flowString);

        strcpy(topic, mqtt_identifier); // Copy MQTT client identifier to topic buffer
        strcat(topic, "/flow_rate"); // Append flow rate topic to MQTT client identifier
        client.publish(topic, flowString); // Publish flow rate value to MQTT broker

        prevMillis = currentMillis; // Update previous timestamp
        attachInterrupt(digitalPinToInterrupt(FLOW_SENSOR_PIN), pulseCounter, FALLING); // Reattach interrupt for pulse counting
    }
}

// Interrupt service routine for pulse counting
void pulseCounter()
{
    pulseCount++; // Increment pulse count when interrupt occurs
}
