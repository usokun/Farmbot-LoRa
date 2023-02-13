#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <SoftwareSerial.h>

#define rxPin 9
#define txPin 10

// MQTT Broker
const char *mqtt_broker = "103.163.139.230";
const char *topic = "farmbot/sensors";
const char *mqtt_username = "farmbot-mqtt";
const char *mqtt_password = "farmbot0123";
const int mqtt_port = 1883;

WiFiClient espClient;
PubSubClient client(espClient);
SoftwareSerial uartSerial(rxPin, txPin);

void connectToWifi()
{
//  WiFi.begin("Ini Wifi", "satusampaitiga");
  WiFi.begin("Redmi", "bruh12345");
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
      String client_id = "farmbot-nodemcu-LoraWifi";
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

void setup()
{
  Serial.begin(115200);
  uartSerial.begin(115200);
  while (!Serial) {
    ; //wait for serial
  }
  Serial.println();

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

  /*
      Ekspektasi data yang nyampe:  T:28.44;H:56.91;PH:7.95;N:31:P:42:K:104/1
  */

  if (Serial.available() > 0) 
  {
    String data_masuk = Serial.readString();
    Serial.println(data_masuk);
    client.publish(topic, data_masuk.c_str());

  }
  client.loop();
  delay(500);
}