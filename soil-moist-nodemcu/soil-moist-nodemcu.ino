#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define pinSensor1 0
#define pinSensor2 1
#define delaytime 300000

// MQTT Broker
const char *mqtt_broker = "103.163.139.230";
const char *topic = "farmbot/sensors";
const char *mqtt_username = "farmbot-mqtt";
const char *mqtt_password = "farmbot0123";
const int mqtt_port = 1883;

WiFiClient espClient;
PubSubClient client(espClient);

String nodeNumber = "1"; // which node? pls declare
double sensorV1;
double sensorV2;

void connectToWifi()
{
 WiFi.begin("Ini Wifi", "satusampaitiga");
  // WiFi.begin("Redmi", "bruh12345");
  Serial.print("Connecting to wifi");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.println(WiFi.status());
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.print("Connected, IP address: ");
  Serial.println(WiFi.localIP());
}

void connectToMQTTBroker() {
  while (!client.connected())
    {
      String client_id = "farmbot-nodemcu-" + nodeNumber;
      client_id += String(WiFi.macAddress());
      Serial.printf("The client %s connects to the Agribot MQTT Broker \n", client_id.c_str());
      if (client.connect(client_id.c_str(), mqtt_username, mqtt_password))
      {
        Serial.println("Connected to Farmbot Broker");
      }
      else
      {
        Serial.print("failed with state ");
        Serial.print(client.state());
        delay(1000);
      }
    }
}

void callback(char *topic, byte *payload, unsigned int length) {
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
  Serial.print("Message:");
  for (int i = 0; i < length; i++) {
      Serial.print((char) payload[i]);
  }
  Serial.println();
  Serial.println("-----------------------");
}

void setup(){
    Serial.begin(9600);

    connectToWifi();  
    client.setServer(mqtt_broker, mqtt_port);
    client.setCallback(callback);
    connectToMQTTBroker();
}

void loop(){
    if(WiFi.status() != WL_CONNECTED){
        connectToWifi();
    }

    if (!client.connected()) {
        connectToMQTTBroker();
    }

    sensorV1 = digitalRead(pinSensor1);
    sensorV2 = digitalRead(pinSensor2);
    double val_avg = (sensorV1 + sensorV2)/2;
    String value_to_publish = String("SMoist:" + nodeNumber + ":" + val_avg );

    Serial.println("Value 1, Value 2: ");
    Serial.print(sensorV1);
    Serial.print(", ");
    Serial.print(sensorV2);

    client.publish(topic, value_to_publish.c_str());
    client.loop();
    delay(delaytime);

}