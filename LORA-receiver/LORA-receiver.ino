#include <RadioLib.h>
#include <SPI.h>
#include <SoftwareSerial.h>

// SX1276 has the following connections:
// NSS pin:   10
// DIO0 pin:  2
// RESET pin: 9
// DIO1 pin:  6

#define rxPin 4
#define txPin 5

SX1276 radio = new Module(10, 2, 9, 6);
SoftwareSerial uartSerial (rxPin, txPin);

void setup() {
  Serial.begin(115200);
  uartSerial.begin(115200);

  // initialize SX1276 with default settings
  Serial.print(F("[SX1276] Initializing ... "));
  //int state = radio.begin();
  int state = radio.begin(915.0, 125.0, 9, 7, 0x12, 10, 8, 0);
  if (state == ERR_NONE) {
    Serial.println(F("success!"));
  } else {
    Serial.print(F("failed, code "));
    Serial.println(state);
    while (true);
  }
}

void loop() {
  delay(500);
  Serial.print(F("[SX1262] Waiting for incoming transmission ... "));

  // you can receive data as an Arduino String
  // NOTE: receive() is a blocking method!
  //       See example ReceiveInterrupt for details
  //       on non-blocking reception method.
  String str;
  int state = radio.receive(str);
  /*
      Ekspektasi data yang nyampe:  T:29.00;H:66.90;N:31;P:42;K=104
  */

  // you can also receive data as byte array
/*
  byte byteArr[8];
  int state = radio.receive(byteArr, 8);
*/

  if (state == ERR_NONE) {
    // packet was successfully received
    Serial.println(F("success!"));

    // print the data of the packet
    Serial.print(F("[SX1276] Data:\t\t"));
    Serial.println(str);
//    for (int i = 0; i < 8; i++)
//    {
//      Serial.print(byteArr[i]);
//    }
//    Serial.println();

    // print the RSSI (Received Signal Strength Indicator)
    // of the last received packet
    Serial.print(F("[SX1276] RSSI:\t\t"));
    Serial.print(radio.getRSSI());
    Serial.println(F(" dBm"));

  // print the SNR (Signal-to-Noise Ratio)
  // of the last received packet
    Serial.print(F("[SX1276] SNR:\t\t"));
    Serial.print(radio.getSNR());
    Serial.println(F(" dB"));
  
  //Write data to serial
  char* buf = (char*) malloc(sizeof(char)*str.length()+1);
  str.toCharArray(buf, str.length()+1);

  Serial.print(buf);
  Serial.println("printing written data to uartSerial... ");
  uartSerial.print(buf);
  free(buf);

  } else if (state == ERR_RX_TIMEOUT) {
    // timeout occurred while waiting for a packet
    Serial.println(F("timeout!"));

  } else if (state == ERR_CRC_MISMATCH) {
    // packet was received, but is malformed
    Serial.println(F("CRC error!"));

  } else {
    // some other error occurred
    Serial.print(F("failed, code "));
    Serial.println(state);

  }
}
