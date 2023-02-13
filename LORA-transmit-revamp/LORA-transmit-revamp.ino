#include <RadioLib.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <HardwareSerial.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define Display_power 19
#define RS485_power 18
#define RX_RS485 23
#define TX_RS485 22

// esp32
#define DIO0 25
#define DIO1 26

#define LORA_RST 33
#define LORA_CS 32

#define SPI_MOSI 13
#define SPI_MISO 12
#define SPI_SCK 14

#define ESP32_SDA 4
#define ESP32_SCL 5

#define FREQUENCY 915.0 // 868.0 or 915.0
#define BANDWIDTH 125.0
#define SPREADING_FACTOR 9
#define CODING_RATE 7
#define OUTPUT_POWER 10
#define PREAMBLE_LEN 8
#define GAIN 0

unsigned char test_command[8] = {0X01, 0X03, 0X00, 0X00, 0X00, 0X07, 0X04, 0X08};

unsigned char test_response[19] = {};

int moisture;
int tem;
int ph;
float moisture_value;
float tem_value;
float ph_value;
int P_value;
int N_value;
int K_value;
float P_float_value;
float N_float_value;
float K_float_value;
String groupID = "1";

SX1276 radio = new Module(LORA_CS, DIO0, LORA_RST, DIO1, SPI, SPISettings()); // 868Mhz or 915Mhz

HardwareSerial MySerial(1);

void setup()
{

  Serial.begin(115200);
  MySerial.begin(4800, SERIAL_8N1, RX_RS485, TX_RS485);

  //----------------------------------------------------
  Serial.println("Initializing DISPLAY POWER & RS485 POWER...");
  pinMode(Display_power, OUTPUT);
  pinMode(RS485_power, OUTPUT);
  delay(100);

  digitalWrite(Display_power, HIGH);
  digitalWrite(RS485_power, HIGH);
  delay(1000);

  //----------------------------------------------------
  Serial.println("Initializing SSD1306(Wire)...");
  Wire.begin(ESP32_SDA, ESP32_SCL);
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
  { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ; // Don't proceed, loop forever
  }
  Serial.println("SSD1306 found");

  //----------------------------------------------------
  Serial.println("SPI Begin...");
  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);

  // initialize SX1278 with default settings
  Serial.print(F("[SX1276] Initializing ... "));

  int state = radio.begin(FREQUENCY, BANDWIDTH, SPREADING_FACTOR, CODING_RATE, 0x12, OUTPUT_POWER, PREAMBLE_LEN, GAIN);
  // int state = radio.begin();
  if (state == ERR_NONE)
  {
    Serial.println(F("Starting LoRa radio: SUCCESS!"));
  }
  else
  {
    Serial.print(F("Starting LoRa radio: FAILED!, code "));
    Serial.println(state);
    while (true)
      ;
  }
  //--- writing init test_command
  Serial.println("Writing initial test for sensor...");
  writeInitTestCommand();
}

void loop()
{

  delay(5000); // LOOP EVERY 5 SECS

  //----------------------------------------------------
  Serial.println("Getting Sensor Value based on test_command:");
  writeInitTestCommand();
  getSensorValue();

  //----------------------------------------------------
  Serial.println("Transmitting Sensor Data...");
  transmitData();
}

void transmitData()
{
  String pkt = String("T:" + (String)tem_value + ";H:" + (String)moisture_value + ";PH:" + (String)ph_value + ";N:" + (String)N_value + ";P:" + (String)P_value + ";K:" + (String)K_value + "/" + groupID); 
  int state = radio.transmit(pkt);
  if (state == ERR_NONE)
  {
    // the packet was successfully transmitted
    Serial.println(F("Sensor Data was SUCCESSFULLY TRANSMITTED!"));
    Serial.println(pkt);
    // print measured data rate
    Serial.print(F("[SX1278] Datarate:\t"));
    Serial.print(radio.getDataRate());
    Serial.println(F(" bps"));
  }
  else if (state == ERR_PACKET_TOO_LONG)
  {
    // the supplied packet was longer than 256 bytes
    Serial.println(F("too long!"));
  }
  else if (state == ERR_TX_TIMEOUT)
  {
    // timeout occurred while transmitting packet
    Serial.println(F("timeout!"));
  }
  else
  {
    // some other error occurred
    Serial.print(F("failed, code "));
    Serial.println(state);
  }
}

void writeInitTestCommand()
{
  MySerial.write(test_command, 8);
  int i = 0;
  while (MySerial.available() > 0)
  {
    test_response[i] = MySerial.read();
    i++;
    yield();
  }
  for (int j = 0; j < 19; j++)
  {
    Serial.print((int)test_response[j]);
    Serial.print("  ");
  }
  Serial.print("\n");

  delay(1000);
  callbackWriteStatus();
}

void callbackWriteStatus()
{
  Serial.println(F("Writing Init test_command done."));
}

void getSensorValue()
{
  Serial.println("Getting Sensor Value based on test_command:");
  Serial.println("Moist, Temp, pH:");

  writeInitTestCommand();

  moisture = CaculateValue((int)test_response[3], (int)test_response[4]);
  moisture_value = moisture * 0.1;
  tem = CaculateValue((int)test_response[5], (int)test_response[6]);
  tem_value = tem * 0.1;
  ph = CaculateValue((int)test_response[9], (int)test_response[10]);
  ph_value = ph * 0.1;

  Serial.println(moisture);
  Serial.println(moisture_value);
  Serial.println(tem_value);
  Serial.println(ph_value);
  value_show(moisture_value, tem_value, ph_value);

  delay(1000);
  Serial.println("N, P, K:");
  N_value = CaculateValue((int)test_response[11], (int)test_response[12]);
  P_value = CaculateValue((int)test_response[13], (int)test_response[14]);
  K_value = CaculateValue((int)test_response[15], (int)test_response[16]);

  Serial.print("N= ");
  Serial.print(N_value);
  Serial.println(" mg/kg");
  Serial.print("P= ");
  Serial.print(P_value);
  Serial.println(" mg/kg");
  Serial.print("K= ");
  Serial.print(K_value);
  Serial.println(" mg/kg");

  NPK_Show(N_value, P_value, K_value);
}

void logo_show()
{
  display.clearDisplay();

  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(2); // Draw 2X-scale text
  display.setCursor(10, 0);
  display.println(F("Makerfabs"));
  display.setTextSize(1);
  display.setCursor(10, 16);
  display.println(F("RS485-LoRa"));
  display.display(); // Show initial text
  delay(100);

  // Scroll in various directions, pausing in-between:
  display.startscrollright(0x00, 0x01);
  delay(4000);
  display.startscrolldiagright(0x00, 0x07);
  delay(2000);
  display.startscrolldiagleft(0x00, 0x07);
  delay(2000);
  display.stopscroll();
}

int CaculateValue(int x, int y)
{
  int t = 0;
  t = x * 256;
  t = t + y;
  return t;
}

void value_show(float h, float t, float ph_f)
{
  display.clearDisplay();

  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1); // Draw 2X-scale text
  display.setCursor(2, 0);
  display.print(F("T:"));
  display.print(t, 1);
  display.print(F("C"));

  display.setCursor(66, 0);
  display.print(F("H:"));
  display.print(h, 1);
  display.print(F("%"));

  display.setCursor(2, 16);
  display.print(F("PH:"));
  display.print(ph_f, 1);

  display.display(); // Show initial text
  delay(3000);
}

void NPK_Show(int N, int P, int K)
{
  display.clearDisplay();

  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1); // Draw 2X-scale text
  display.setCursor(2, 0);
  display.print(F("N:"));
  display.print(N);
  // display.print(F("C"));

  display.setCursor(66, 0);
  display.print(F("P:"));
  display.print(P);
  // display.print(F("%"));

  display.setCursor(2, 16);
  display.print(F("K:"));
  display.print(K);
  display.print(F(" mg/kg"));

  display.display(); // Show initial text
  delay(3000);
}

/*
 * unsigned char testcode[236] = {0XFD,0XFD,0X30,0X3,...
 * Serial.write(testcode,236);
 *
 */
