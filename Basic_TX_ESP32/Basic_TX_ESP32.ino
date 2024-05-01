/*

Demonstrates simple TX operation with an ESP32.
Any of the Basic_RX examples can be used as a receiver.

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

Vibration ESP32 module
Motor 
VCC     -> 3V3
IN      -> 15
GND     -> GND

*/

#include "SPI.h"
#include "NRFLite.h"

const static uint8_t PIN_RADIO_CE = 4;
const static uint8_t PIN_RADIO_CSN = 5;
const static uint8_t PIN_RADIO_MOSI = 23;
const static uint8_t PIN_RADIO_MISO = 19;
const static uint8_t PIN_RADIO_SCK = 18;
const static uint8_t PIN_VIBRA_IN = 15;

const static uint8_t RADIO_ID = 100;
static uint8_t DESTINATION_RADIO_ID = 2;


// Note the packed attribute.
typedef enum {
  VIBRATE,
  LIGHT,
  VIBATE_LIGHT,
  BROAD_CAST
} Opcode_t;

struct __attribute__((packed)) RadioPacket  // Note the packed attribute.
{
  Opcode_t opcode;
  String command;
};

NRFLite _radio;
RadioPacket _radioData;

void setup()
{
  Serial.begin(9600);
    
  // Configure SPI pins.
  SPI.begin(PIN_RADIO_SCK, PIN_RADIO_MISO, PIN_RADIO_MOSI, PIN_RADIO_CSN);
  // Indicate to NRFLite that it should not call SPI.begin() during initialization since it has already been done.
  uint8_t callSpiBegin = 0;
    
  if (!_radio.init(RADIO_ID, PIN_RADIO_CE, PIN_RADIO_CSN, NRFLite::BITRATE2MBPS, 100, callSpiBegin))
  {
    Serial.println("Cannot communicate with radio");
    while (1); // Wait here forever.
  }
  Serial.println("Setup done!");
}

void loop()
{
  while (!Serial.available()) {
    // Chờ đến khi có dữ liệu được gửi đến từ serial monitor
  }

  // Đọc dữ liệu được gửi từ serial với định dạng: "DESTINATION_RADIO_ID VibraionTime Message"
  _radioData.opcode = VIBATE_LIGHT;
  _radioData.command = Serial.readStringUntil('\n');
  Serial.println(_radioData.opcode);

  Serial.print("Sending: ");
    
    if (_radio.send(DESTINATION_RADIO_ID, &_radioData, sizeof(_radioData)))
    {
        Serial.println("...Success");
        Serial.println(_radioData.command);
        _radioData.command = "";
    }
    else
    {
        Serial.println("...Failed");
        _radioData.command = "";
    }

    delay(1000);
}