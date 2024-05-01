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

const static uint8_t PIN_RADIO_CE = 4;
const static uint8_t PIN_RADIO_CSN = 5;
const static uint8_t PIN_RADIO_MOSI = 23;
const static uint8_t PIN_RADIO_MISO = 19;
const static uint8_t PIN_RADIO_SCK = 18;
const static uint8_t PIN_VIBRA_IN = 15;
const static uint8_t PIN_LIGHT_IN = 2;

const static uint8_t PIN_ID_1 = 27;
const static uint8_t PIN_ID_2 = 26;
const static uint8_t PIN_ID_3 = 25;
const static uint8_t PIN_ID_4 = 33;

static uint8_t TRANSMITTER_RADIO_ID = 100;
static uint8_t RADIO_ID;

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
void handleControlCommand(const char* command, Opcode_t opcode) {
  int controlTime, period, pause;
  Serial.println(command);
  sscanf(command, "%d %d %d", &controlTime, &period, &pause);  // Phân tích chuỗi
  Serial.print("controlTime = ");
  Serial.println(controlTime);
  Serial.print("period = ");
  Serial.println(period);
  Serial.print("pause = ");
  Serial.println(pause);

  uint8_t destinationPin = 0;
  if (opcode == VIBRATE) {
    destinationPin = PIN_VIBRA_IN;
  } else if (opcode == LIGHT) {
    destinationPin = PIN_LIGHT_IN;  // Chân pin của đèn mặc định D2 của ESP32 CH340c
  }

  unsigned long startTime = millis();
  unsigned long currentTime, elapsedTime;
  while (1) {
    currentTime = millis();
    elapsedTime = currentTime - startTime;
    if (elapsedTime >= controlTime * 1000 && controlTime != 0) {
      Serial.println(currentTime - startTime);
      return;
    }
    if (opcode == VIBATE_LIGHT) {
      digitalWrite(PIN_VIBRA_IN, HIGH);
      digitalWrite(PIN_LIGHT_IN, HIGH);
      delay(period * 1000);
      digitalWrite(PIN_VIBRA_IN, LOW);
      digitalWrite(PIN_LIGHT_IN, LOW);
      delay(pause * 1000);
    } else {
      digitalWrite(destinationPin, HIGH);
      delay(period * 1000);
      digitalWrite(destinationPin, LOW);
      delay(pause * 1000);
    }
  }
}

void respondBroadCast() {

}

void setup() {
  Serial.begin(9600);

  // get device ID
  RADIO_ID = getID();
  Serial.print("Device ID: ");
  Serial.println(RADIO_ID);

  // Motor and light Iput config
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
  int opcode, tmp1, tmp2, tmp3;
  while (_radio.hasData()) {
    _radio.readData(&_radioData);
    Serial.println("Has data: ");
    Serial.println(_radioData.command);
    Serial.println(_radioData.opcode);
    if (_radioData.opcode != BROAD_CAST) {
      handleControlCommand(_radioData.command.c_str(), _radioData.opcode);
    } else {
      Serial.println("send respond: ...");
      // respondBroadCast();
    }
  }
}
