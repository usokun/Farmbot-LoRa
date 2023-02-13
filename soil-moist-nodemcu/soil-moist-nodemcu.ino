#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define delaytime 5000



// MQTT Broker
const char *mqtt_broker = "103.163.139.230";
const char *topic = "farmbot/sensors";
const char *mqtt_username = "farmbot-mqtt";
const char *mqtt_password = "farmbot0123";
const int mqtt_port = 1883;

WiFiClient espClient;
PubSubClient client(espClient);

String groupID = "2"; // groupID: soil-moist-sensor 1~2 = group 2 ; 3~4 = group 3 ; 5~6 = group 4
double sensorV1;
double sensorV2;
const int digitalPin_1 = 5;
const int digitalPin_2 = 4;

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
      String client_id = "farmbot-soil-moist-" + groupID;
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

    sensorV1 = digitalRead(digitalPin_1);
    sensorV2 = digitalRead(digitalPin_2);
    double val_avg = (sensorV1 + sensorV2)/2;
    // String value_to_publish = String("SMoist:" + nodeNumber + ":" + val_avg );
    // SMoist:28.21/2
    String value_to_publish = String("SMoist:" + val_avg + "/" + groupID);

    Serial.println("Value 1, Value 2, Avg Value: ");
    Serial.print(sensorV1);
    Serial.print(", ");
    Serial.print(sensorV2);
    Serial.print(", ");
    Serial.print(val_avg);
    Serial.println();

    client.publish(topic, value_to_publish.c_str());
    client.loop();
    delay(delaytime);

}