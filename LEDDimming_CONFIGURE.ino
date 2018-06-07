/*
light:
  - platform: mqtt
    command_topic: "led1/onOff"
    brightness_command_topic: "led1/brightness"
    state_topic: "led1/state"
    retain: true
*/


#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>


//USER CONFIGURED SECTION START//
const char* ssid = "YOUR_WIRELESS_SSID";
const char* password = "YOUR_WIRELESS_SSID";
const char* mqtt_server = "YOUR_MQTT_SERVER_ADDRESS";
const int mqtt_port = 1880;
const char *mqtt_user = "YOUR_MQTT_USERNAME";
const char *mqtt_pass = "YOUR_MQTT_PASSWORD";
const char *mqtt_client_name = "Doorbell"; // Client connections can't have the same connection name
//USER CONFIGURED SECTION END//

WiFiClient espClient;
PubSubClient client(espClient);

// Variables
const int LEDPin = 5;  //marked as D1 on the board
bool boot = true;

//Functions

void setup_wifi() {
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

void reconnect() 
{
  int retries = 0;
  while (!client.connected()) {
    if(retries < 15)
    {
      Serial.print("Attempting MQTT connection...");
      if (client.connect(mqtt_client_name, mqtt_user, mqtt_pass)) 
      {
        Serial.println("connected");
        if(boot == true)
        {
          client.publish("checkIn/LEDMCU","Rebooted");
          boot = false;
        }
        if(boot == false)
        {
          client.publish("checkIn/LEDMCU","Reconnected"); 
        }
        client.subscribe("led1/brightness");
        client.subscribe("led1/onOff");
      } 
      else 
      {
        Serial.print("failed, rc=");
        Serial.print(client.state());
        Serial.println(" try again in 5 seconds");
        retries++;
        delay(5000);
      }
    }
    if(retries > 14)
    {
    ESP.restart();
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) 
{
  String newTopic = topic;
  payload[length] = '\0';
  String OnOff = String((char *)payload);
  int newBrightness = OnOff.toInt();
  if (newTopic == "led1/brightness") 
  {
    analogWrite(LEDPin, newBrightness);
  }
  if (newTopic == "led1/onOff") 
  {
    if (OnOff == "ON")
    {
      analogWrite(LEDPin, newBrightness);
      client.publish("led1/state","ON"); 
    }
    if (OnOff == "OFF")
    {
      analogWrite(LEDPin, 0);
      client.publish("led1/state","OFF"); 
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(LEDPin, OUTPUT);
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
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


