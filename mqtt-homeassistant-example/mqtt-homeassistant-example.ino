#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <DHT_U.h>
#include <Adafruit_Sensor.h>

#define DHTPIN 3     // Digital pin connected to the DHT sensor 

#define DHTTYPE    DHT22     // DHT 22 (AM2302)
DHT_Unified dht(DHTPIN, DHTTYPE);
uint32_t delayMS;

// WiFi-Credentials
const char *ssid = "FabLabWS";
const char *password = "57289887252221648185KaL!";

// MQTT Broker IP address
const char *mqtt_identifier = "DHTSlave";
const char *mqtt_server = "192.168.188.125";
const char *mqtt_user = "mqtt-user";
const char *mqtt_pass = "mqtt";

int sending_mqtt_every_ms = 5000;

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
char topic[50];
int value = 0;

void setup_wifi()
{
    delay(10);
    // We start by connecting to a WiFi network
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}

void callback(char *topic, byte *message, unsigned int length)
{
    Serial.print("Message arrived on topic: ");
    Serial.print(topic);
    Serial.print(". Message: ");
    String messageTemp;

    for (int i = 0; i < length; i++)
    {
        Serial.print((char)message[i]);
        messageTemp += (char)message[i];
    }
    Serial.println();

    // Feel free to add more if statements to control more GPIOs with MQTT

    // If a message is received on the topic esp32/output, you check if the message is either "on" or "off".
    // Changes the output state according to the message
    strcpy(topic, mqtt_identifier);
    strcat(topic, "/output");
    if (String(topic) == topic)
    {
        Serial.print("Changing output to " + messageTemp);
        if (messageTemp == "ON")
        {
            digitalWrite(5, HIGH);
        }
        else if (messageTemp == "OFF")
        {
            digitalWrite(5, LOW);
        }
    }
}

void reconnect()
{
    // Loop until we're reconnected
    while (!client.connected())
    {
        Serial.print("Attempting MQTT connection...");
        // Attempt to connect
        if (client.connect(mqtt_identifier, mqtt_user, mqtt_pass))
        {
            Serial.println("connected");
            // Subscribe
            strcpy(topic, mqtt_identifier);
            strcat(topic, "/output");
            client.subscribe(topic);
        }
        else
        {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            // Wait 5 seconds before retrying
            delay(5000);
        }
    }
}

void setup(void)
{
    Serial.begin(115200);
    
    pinMode(5, OUTPUT);
    setup_wifi();
    client.setServer(mqtt_server, 1883);
    client.setCallback(callback);
    dht.begin();
 
  sensor_t sensor;
  dht.temperature().getSensor(&sensor);
  // Print humidity sensor details.
  dht.humidity().getSensor(&sensor);
  delayMS = sensor.min_delay / 1000;
}

void loop()
{
  sensors_event_t event;
    if (!client.connected())
    {
        reconnect();
    }
    client.loop();

    long now = millis();
    if (now - lastMsg > sending_mqtt_every_ms)
    {
        lastMsg = now;
        dht.temperature().getEvent(&event);
        // Temperature in Celsius
        
        // Uncomment the next line to set temperature in Fahrenheit
        // (and comment the previous temperature line)
        // temperature = 1.8 * bme.readTemperature() + 32; // Temperature in Fahrenheit

        // Convert the value to a char array
        char tempString[8];
        dtostrf(event.temperature, 1, 2, tempString);
        Serial.print("Temperature: ");
        Serial.println(tempString);
        strcpy(topic, mqtt_identifier);
        strcat(topic, "/temperature");
        client.publish(topic, tempString);


        dht.humidity().getEvent(&event);
        

        // Convert the value to a char array
        char humString[8];
        dtostrf(event.relative_humidity, 1, 2, humString);
        Serial.print("Humidity: ");
        Serial.println(humString);
        strcpy(topic, mqtt_identifier);
        strcat(topic, "/humidity");
        client.publish(topic, humString);
    }
}