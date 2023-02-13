#include <Adafruit_BMP280.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// MQTT Broker
const char *mqtt_broker = "103.163.139.230";
const char *topic = "farmbot/sensors";
const char *mqtt_username = "farmbot-mqtt";
const char *mqtt_password = "farmbot0123";
const int mqtt_port = 1883;

String groupID = "5";

WiFiClient espClient;
PubSubClient client(espClient);

Adafruit_BMP280 bmp; // I2C

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
      String client_id = "farmbot-air-temp-1";
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

void setup() {
    Serial.begin(9600);
    connectToWifi();  
    client.setServer(mqtt_broker, mqtt_port);
    client.setCallback(callback);
    connectToMQTTBroker();
    Serial.println(F("BMP280 Sensor"));

  if (!bmp.begin(BMP280_ADDRESS_ALT, BMP280_CHIPID)) {
    Serial.println(F("Could not find a valid BMP280 sensor, check wiring or "
                      "try a different address!"));
    while (1) delay(10);
  }

  /* Default settings from datasheet. */
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */
}

void loop() {
    if(WiFi.status() != WL_CONNECTED){
        connectToWifi();
    }

    if (!client.connected()) {
        connectToMQTTBroker();
    }
    double temp = bmp.readTemperature();
    double press = bmp.readPressure();

    String temp_v = String(temp);
    String press_v = String(press);
    // jenis_sensor:nodeNumber:value;jenis_sensor:nodeNumber:value
    // String value_to_publish = "ATemp:" + nodeNumber + ":" + temp_v + ";" + "APress:" + nodeNumber + ":" + press_v;
    // EXPECTED DATA TO PUBLISH: ATemp:30.12;APress:100312.21/5
    String value_to_publish = "ATemp:" + temp_v + ";" + "APress:" + press_v + "/" + groupID;
    Serial.print(value_to_publish);
    Serial.println();
    
    client.publish(topic, value_to_publish.c_str());
    client.loop();
    delay(10000);
}
