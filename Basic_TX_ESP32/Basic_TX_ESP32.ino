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

const static uint8_t TRANSMITTER_RADIO_ID = 100;
static uint8_t DESTINATION_RADIO_ID;
const static uint8_t ACTIVE = 1;
const static uint8_t INACTIVE = 0;
// Receiver devices are identified based on 4-bit jumpers -> 16 devices numbered from 0 -> 15
const static uint8_t RECEIVER_RADIO_ID_RANGE = 16;
uint8_t activeDevices[RECEIVER_RADIO_ID_RANGE];

// Define the opcode of the instruction corresponding to the control modes
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
  int pin;
  uint8_t controlTime,
    periodTime,
    pauseTime;
};

NRFLite _radio;
RadioPacket _radioData;

/* instructionInput format: 
  VBR/VGT/VLG opcode  : {opcode} {destination ID} {control time} {period time} {pause time}
    Ex: VBR 9 5 3 2
      {destination ID}: Radio Id of the receiving device. In the example ID is 9
      {controlTime}   : If the value is 0, then {vibrate/light up} until the button on the device is pressed, 
                        if not 0, then {vibrate/light up} until the specified time period.ype int, in seconds.
                        In the example, control time is 5s.
      {period} {pause}: Specify the time for the device to repeat. 
                        In the example, then after 3 seconds of vibrate/light up the device will stop for 2s 
                        then continue to vibrate/light up.
  Broadcast           : BRD
    Send vibration in 5s signals to all devices
*/
void readInstructionInput(String instructionInput) {
  // Get opcode
  int index = instructionInput.indexOf(' ');  // Find the position of the first space
  String opcodeInput = instructionInput.substring(0, index);
  if (opcodeInput == "VBR") {
    _radioData.opcode = VBR;
    Serial.println("Vibrate mode");
  } else if (opcodeInput == "LGT") {
    _radioData.opcode = LGT;
    Serial.println("Light mode");
  } else if (opcodeInput == "VLG") {
    _radioData.opcode = VLG;
    Serial.println("Vibrate and light mode");
  } else if (opcodeInput == "BRD") {
    _radioData.opcode = BRD;
    _radioData.controlTime = 5;
    Serial.println("BroadCast mode");
    return;
  } else if (opcodeInput == "REQ") {
    _radioData.opcode = REQ;
    Serial.println("Request mode");
    return;
  }

  // Get the remaining ingredients: {destination ID} {control time} {period time} {pause time}
  String restStr = instructionInput.substring(index + 1);
  sscanf(restStr.c_str(), "%d %d %d %d", &DESTINATION_RADIO_ID,
         &_radioData.controlTime,
         &_radioData.periodTime,
         &_radioData.pauseTime);
}

void setup() {
  Serial.begin(9600);
  memset(activeDevices, 0, sizeof(activeDevices));

  // Configure SPI pins.
  SPI.begin(PIN_RADIO_SCK, PIN_RADIO_MISO, PIN_RADIO_MOSI, PIN_RADIO_CSN);
  // Indicate to NRFLite that it should not call SPI.begin() during initialization since it has already been done.
  uint8_t callSpiBegin = 0;

  if (!_radio.init(TRANSMITTER_RADIO_ID, PIN_RADIO_CE, PIN_RADIO_CSN, NRFLite::BITRATE2MBPS, 100, callSpiBegin)) {
    Serial.println("Cannot communicate with radio");
    while (1)
      ;  // Wait here forever.
  }
  _radioData.fromID = TRANSMITTER_RADIO_ID;
  Serial.println("Setup done!");
}

void loop() {
  while (!Serial.available()) {
    // Chờ đến khi có dữ liệu được gửi đến từ serial monitor
  }

  /* instructionInput format: 
  VBR/VGT/VLG opcode  : {opcode} {destination ID} {control time} {period time} {pause time}
  Broadcast           : BRD
  */
  String instructionInput = Serial.readStringUntil('\n');
  readInstructionInput(instructionInput);
  uint32_t _lastBroadcastTime;
  if (_radioData.opcode == BRD) {
    Serial.println("Sending broadcast: ...");
    for (uint8_t i = 0; i < RECEIVER_RADIO_ID_RANGE; i++) {
      if (_radio.send(i, &_radioData, sizeof(_radioData))) {
        activeDevices[i] = ACTIVE;
        Serial.println("Device ID " + String(i) + " active!");
      } else {
        activeDevices[i] = INACTIVE;
        Serial.println("Device ID " + String(i) + " inactive!");
      }
    }

  } else if (_radioData.opcode == REQ) {
    Serial.println("Sending request get status: ...");
    for (uint8_t i = 0; i < RECEIVER_RADIO_ID_RANGE; i++) {
      if (activeDevices[i] == ACTIVE) {
        Serial.println(String(i));
        requestData(i);
      }
    }
    Serial.println();
  } else {
    Serial.print("Sending to ID " + String(DESTINATION_RADIO_ID) + ": ...");

    if (_radio.send(DESTINATION_RADIO_ID, &_radioData, sizeof(_radioData))) {
      Serial.println("...Success");
      Serial.println(DESTINATION_RADIO_ID);
      Serial.println(_radioData.opcode);
      Serial.println(_radioData.controlTime);
      Serial.println(_radioData.periodTime);
      Serial.println(_radioData.pauseTime);
    } else {
      Serial.println("...Failed");
    }
  }
  delay(500);
}

void requestData(uint8_t id) {
  Serial.println("Requesting data");

  RadioPacket radioData;
  radioData.opcode = BEGIN;
  radioData.fromID = TRANSMITTER_RADIO_ID; // When the receiver sees this packet type, it will load an ACK data packet.
  if (_radio.send(id, &radioData, sizeof(radioData))) {
    Serial.println("...Success");
    Serial.print("  Sending EndGetData");
    radioData.opcode = END;

    if (_radio.send(id, &radioData, sizeof(radioData))) {
      while (_radio.hasAckData())  // Look to see if the receiver provided the ACK data.
      {
        RadioPacket ackData;
        _radio.readData(&ackData);

        if (ackData.opcode == REQ) {
          String msg = "  Received ACK data from ";
          msg += ackData.fromID;
          msg += ", pin status: ";
          msg += ackData.pin;
          Serial.println(msg);
        }
      }
    } else {
      Serial.println("...Failed");
    }
  } else {
    Serial.println("...Failed");
  }
}