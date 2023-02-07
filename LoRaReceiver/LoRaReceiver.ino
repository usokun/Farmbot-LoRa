#include <SPI.h>
#include <LoRa.h>

#define LORA_RST 9
#define LORA_CLK 13
#define LORA_MISO 12
#define LORA_MOSI 11
#define LORA_CS 10

#define DIO0 2
#define DIO1 6

#define FREQUENCY 915.0       
#define BANDWIDTH 125.0
#define SPREADING_FACTOR 9
#define CODING_RATE 7
#define OUTPUT_POWER 10
#define PREAMBLE_LEN 8
#define GAIN 0

void setup() {
  Serial.begin(9600);

  LoRa.setPins(LORA_CS, LORA_RST, DIO0);

  LoRa.setFrequency(FREQUENCY);
  LoRa.setSignalBandwidth(BANDWIDTH);
  LoRa.setSpreadingFactor(SPREADING_FACTOR);
  LoRa.setCodingRate4(CODING_RATE);
  LoRa.setTxPower(OUTPUT_POWER);
  LoRa.setPreambleLength(PREAMBLE_LEN);
  LoRa.setGain(GAIN);
  LoRa.setSyncWord(0x12);

  while (!Serial);

  Serial.println("LoRa Receiver Callback");

  if (!LoRa.begin(FREQUENCY)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }else{
    Serial.println("Starting Success!");

  }

  // Uncomment the next line to disable the default AGC and set LNA gain, values between 1 - 6 are supported
  // LoRa.setGain(6);
  
  // register the receive callback
  // LoRa.onReceive(onReceive);

  // // put the radio into receive mode
  // Serial.println("Receive mode lora:");
  // LoRa.receive();
}

void loop() {
  delay(2500);
  Serial.println("Trying to receive data:");
  LoRa.onReceive(onReceive);
  LoRa.receive();
}

void onReceive(int packetSize) {
  // received a packet
  Serial.print("Received packet '");

  // read packet
  for (int i = 0; i < packetSize; i++) {
    Serial.print((char)LoRa.read());
  }

  // print RSSI of packet
  Serial.print("' with RSSI ");
  Serial.println(LoRa.packetRssi());
}