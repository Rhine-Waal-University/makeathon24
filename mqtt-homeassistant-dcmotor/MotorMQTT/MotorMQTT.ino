#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

// WiFi-Credentials
const char *ssid = "FabLabWS";
const char *password = "57289887252221648185KaL!";

// MQTT Broker IP address
const char *mqtt_identifier = "MotorSlave";
const char *mqtt_server = "192.168.188.125";
const char *mqtt_user = "mqtt-user";
const char *mqtt_pass = "mqtt";

int sending_mqtt_every_ms = 5000;

//Pins for Motor Control
const char ENA_PIN = 25; // the Arduino pin connected to the EN1 pin L298N
const char IN1_PIN = 33; // the Arduino pin connected to the IN1 pin L298N
const char IN2_PIN = 32; // the Arduino pin connected to the IN2 pin L298N

int mSpeed = 100;
bool mPower = false;

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
        delay(2000);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}

void callback(char *topic, byte *message, unsigned int length)
{
    String messageTemp;
    
    for (int i = 0; i < length; i++)
    {
        messageTemp += (char)message[i];
    }
    
    if (String(topic) == "MotorSlave/power")
    {
        Serial.println("Power: " + messageTemp); 
        if (messageTemp == "ON")
        {
            mPower = true;
          
        }
        else if (messageTemp == "OFF")
        {
            mPower = false;
        }
    }
    
    else if (String(topic) == "MotorSlave/speed")
    {
        Serial.println("Speed " + messageTemp); 
        mSpeed = messageTemp.toInt();
    }
 
    if (mPower){
      analogWrite(ENA_PIN, map(mSpeed, 0, 100, 0, 255));
      digitalWrite(IN1_PIN, LOW);   // control motor A spins clockwise
      digitalWrite(IN2_PIN, HIGH);
    }else{
      analogWrite(ENA_PIN, 0);
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
            // Subscribe power
            strcpy(topic, mqtt_identifier);
            strcat(topic, "/power");
            client.subscribe(topic);

            strcpy(topic, mqtt_identifier);
            strcat(topic, "/direction");
            client.subscribe(topic);

            strcpy(topic, mqtt_identifier);
            strcat(topic, "/speed");
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
    Serial.begin(9600);
     // initialize digital pins as outputs.
    pinMode(ENA_PIN, OUTPUT);
    pinMode(IN1_PIN, OUTPUT);
    pinMode(IN2_PIN, OUTPUT);


    digitalWrite(IN1_PIN, HIGH); // control motor spins clockwise
    digitalWrite(IN2_PIN, LOW);  // control motor spins clockwise
    
    //start WIFI & MTTQ connection
    setup_wifi();
    client.setServer(mqtt_server, 1883);
    client.setCallback(callback);
}

void loop()
{
    if (!client.connected())
    {
        reconnect();
    }
    client.loop();
}
