/*

Demonstrates simple RX operation with an ESP32.
Any of the Basic_TX examples can be used as a transmitter.

ESP's require the use of '__attribute__((packed))' on the RadioPacket data structure
to ensure the bytes within the structure are aligned properly in memory.

The ESP32 SPI library supports configurable SPI pins and NRFLite's mechanism to support this is shown.

Radio    ESP32 module
CE    -> 4
CSN   -> 5
MOSI  -> 23

MISO  -> 19
SCK   -> 18
IRQ   -> No connection
VCC   -> No more than 3.6 volts
GND   -> GND

*/

#include "SPI.h"
#include "NRFLite.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
// Pin config
const static uint8_t PIN_RADIO_CE = 4;
const static uint8_t PIN_RADIO_CSN = 5;
const static uint8_t PIN_RADIO_MOSI = 23;
const static uint8_t PIN_RADIO_MISO = 19;
const static uint8_t PIN_RADIO_SCK = 18;
const static uint8_t PIN_VIBRA_IN = 15;
const static uint8_t PIN_LIGHT_IN = 2;

// Jumper for ID pin
const static uint8_t PIN_ID_1 = 27;
const static uint8_t PIN_ID_2 = 26;
const static uint8_t PIN_ID_3 = 25;
const static uint8_t PIN_ID_4 = 33;

// RADIO ID config
static uint8_t TRANSMITTER_RADIO_ID = 100;
static uint8_t RADIO_ID;

// Battery
/*
Esp 32 operates at 2.5-3.3V
Equivalent to analog value 3102-4095
*/
const static uint8_t PIN_BATTERY = 34;
const static uint16_t BATTER_RANGE = 993;
const static uint16_t BATTER_LOWER_LIMIT = 3102;


typedef enum {
  VBR,  // Vibrate
  LGT,  // Light
  VLG,  // Vibrate and light
  BRD,
  REQ,
  BEGIN,
  END  // Broadcast
} Opcode_t;

// Note the packed attribute.
struct __attribute__((packed)) RadioPacket {
  Opcode_t opcode;
  uint8_t fromID;
  uint16_t pin;
  uint8_t controlTime,
    periodTime,
    pauseTime;
};

NRFLite _radio;
RadioPacket _radioData;
RadioPacket _resData;

void displayMSG(String msg) {
  display.clearDisplay();
  display.setTextSize(1.5);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  // Display static text
  display.printf("Device ID: %d\n", RADIO_ID);
  display.display(); 
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(10, 20);
  // Display static text
  display.println(msg);
  display.display(); 
}


int getID() {
  uint8_t radio_id = -1;
  pinMode(PIN_ID_1, INPUT_PULLUP);
  pinMode(PIN_ID_2, INPUT_PULLUP);
  pinMode(PIN_ID_3, INPUT_PULLUP);
  pinMode(PIN_ID_4, INPUT_PULLUP);

  // PIN_ID_1 PIN_ID_2 PIN_ID_3 PIN_ID_4
  // Equivalent to 4 bits to identify 2^4 = 16 devices
  uint8_t stt_1 = digitalRead(PIN_ID_1), bit_1,
          stt_2 = digitalRead(PIN_ID_2), bit_2,
          stt_3 = digitalRead(PIN_ID_3), bit_3,
          stt_4 = digitalRead(PIN_ID_4), bit_4;

  // Check jumper pin 27
  if (stt_1 == LOW) {  // có jumper kết nối
    bit_1 = 1;
  } else {
    bit_1 = 0;
  }
  // Check jumper pin 26
  if (stt_2 == LOW) {  // có jumper kết nối
    bit_2 = 1;
  } else {
    bit_2 = 0;
  }
  // Check jumper pin 25
  if (stt_3 == LOW) {  // có jumper kết nối
    bit_3 = 1;
  } else {
    bit_3 = 0;
  }
  // Check jumper pin 33
  if (stt_4 == LOW) {  // có jumper kết nối
    bit_4 = 1;
  } else {
    bit_4 = 0;
  }

  radio_id = bit_1 * 8 + bit_2 * 4 + bit_3 * 2 + bit_4 * 1;
  return radio_id;
}

/* CMD VIBRATE/LIGHT/VIBATE_LIGHT:  {controlTime} {period} {pause}
    Ex: 5 3 2
    {controlTime}   : If the value is 0, then {vibrate/light up} until the button on the device is pressed, 
                      if not 0, then {vibrate/light up} until the specified time period.ype int, in seconds
    {period} {pause}: Specify the time for the device to repeat. 
                    For example, set the value to 3 2, 
                    then after 3 seconds of vibrate/light up the device will stop for 2 seconds then continue to vibrate/light up.
*/
void handleControlCommand() {

  uint8_t controlPin = 0;
  if (_radioData.opcode == VBR) {
    controlPin = PIN_VIBRA_IN;
  } else if (_radioData.opcode == LGT) {
    controlPin = PIN_LIGHT_IN;  // Chân pin của đèn mặc định D2 của ESP32 CH340c
  }

  unsigned long startTime = millis();
  unsigned long currentTime, elapsedTime;
  while (1) {
    currentTime = millis();
    elapsedTime = currentTime - startTime;
    if (elapsedTime >= _radioData.controlTime * 1000 && _radioData.controlTime != 0) {
      Serial.println(currentTime - startTime);
      return;
    }
    if (_radioData.opcode == VLG) {
      digitalWrite(PIN_VIBRA_IN, HIGH);
      digitalWrite(PIN_LIGHT_IN, HIGH);
      delay(_radioData.periodTime * 1000);
      digitalWrite(PIN_VIBRA_IN, LOW);
      digitalWrite(PIN_LIGHT_IN, LOW);
      delay(_radioData.pauseTime * 1000);
    } else {
      digitalWrite(controlPin, HIGH);
      delay(_radioData.periodTime * 1000);
      digitalWrite(controlPin, LOW);
      delay(_radioData.pauseTime * 1000);
    }
  }
}
uint16_t getBatteryPercentFromAnalog() {
  uint16_t analogBattery = analogRead(PIN_BATTERY);
  if(analogBattery <= BATTER_LOWER_LIMIT) {
    return 0;
  } else{
    return ((analogBattery - BATTER_LOWER_LIMIT)*100)/ BATTER_RANGE;
  }
}

void setup() {
  Serial.begin(9600);

  // OLed setting
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  // get device ID
  RADIO_ID = getID();
  String msg = "Device ID: ";
  msg+= String(RADIO_ID);
  Serial.println(msg);
  displayMSG("");

  // Motor and light Input config
  pinMode(PIN_VIBRA_IN, OUTPUT);
  pinMode(PIN_LIGHT_IN, OUTPUT);

  // Configure SPI pins.
  SPI.begin(PIN_RADIO_SCK, PIN_RADIO_MISO, PIN_RADIO_MOSI, PIN_RADIO_CSN);
  // Indicate to NRFLite that it should not call SPI.begin() during initialization since it has already been done.
  uint8_t CallSpiBegin = 0;

  if (!_radio.init(RADIO_ID, PIN_RADIO_CE, PIN_RADIO_CSN, NRFLite::BITRATE2MBPS, 100, CallSpiBegin)) {
    Serial.println("Cannot communicate with radio");
    while (1)
      ;  // Wait here forever.
  } else {
    Serial.println("Setup done!");
  }
}


void loop() {
  while (_radio.hasData()) {
    _radio.readData(&_radioData);

    Serial.print("Has Data: ");
    Serial.print(String(_radioData.opcode) + " ");
    Serial.print(String(_radioData.fromID) + " ");
    Serial.print(String(_radioData.controlTime) + "\n");


    if (_radioData.opcode == BRD) {
      displayMSG("Vibrate ...");
      digitalWrite(PIN_VIBRA_IN, HIGH);
      delay(_radioData.controlTime * 1000);
      digitalWrite(PIN_VIBRA_IN, LOW);
      displayMSG("Waiting signal...");
    } else if (_radioData.opcode == BEGIN) {

      Serial.println("Active!");
      Serial.println("Received data request, adding ACK data packet");

      RadioPacket ackData;
      ackData.opcode = REQ;
      ackData.pin = getBatteryPercentFromAnalog();
      ackData.fromID = RADIO_ID;

      Serial.println(analogRead(PIN_BATTERY));

      // Add the data to send back to the transmitter into the radio.
      // The next packet we receive will be acknowledged with this data.
      _radio.addAckData(&ackData, sizeof(ackData));
      String msg = "Battery: " +  String(ackData.pin) + "%";
      displayMSG(msg);

    } else if (_radioData.opcode == END) {  // BRD broadcast mode
      Serial.println("End send to transmitter");
    } else {
      handleControlCommand();
    }
      Serial.println("wait data");

  }
}
