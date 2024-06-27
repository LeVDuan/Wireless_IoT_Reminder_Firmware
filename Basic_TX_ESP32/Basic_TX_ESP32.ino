/*
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
// Pin config
const static uint8_t PIN_RADIO_CE = 4;
const static uint8_t PIN_RADIO_CSN = 5;
const static uint8_t PIN_RADIO_MOSI = 23;
const static uint8_t PIN_RADIO_MISO = 19;
const static uint8_t PIN_RADIO_SCK = 18;
// The transmitter does not need this vibration function, although it still has a built-in vibration motor on the hardware.
//  const static uint8_t PIN_VIBRATION_MOTOR_IN = 15;

// Set the transmitter radio id to 100 by default
const static uint8_t TRANSMITTER_RADIO_ID = 100;
static uint8_t DESTINATION_RADIO_ID;
// Receiver devices are identified based on 4-bit jumpers -> 16 devices numbered from 0 -> 15
const static uint8_t RECEIVER_RADIO_ID_RANGE = 16;

// The array contains a list of ids of active devices.
uint8_t activeDevices[RECEIVER_RADIO_ID_RANGE];
static uint8_t activeCount = 0;

// Define the opcode signal of the instruction corresponding to the control modes
typedef enum
{
  VBR,   // Vibrate
  LGT,   // Light
  VLG,   // Vibrate and light
  BRD,   // Broadcast
  REQ,   // Request data
  BEGIN, // begin receive data
  END    // end receive data
} Opcode_t;
String opcodeToString[] = {
    "VBR:", "LGT:", "VLG:", "BRD:", "REQ:"};
// Note the packed attribute.
struct __attribute__((packed)) RadioPacket
{
  Opcode_t opcode;
  uint8_t fromID;
  uint16_t pin;
  uint8_t controlTime,
      periodTime,
      pauseTime;
};

// Declare packet data as a global variable
NRFLite _radio;
RadioPacket _radioData;
// Feedback information of REQ signal
String reqResponse;

/*
instructionInput format:
  VBR/VGT/VLG opcode  : {opcode} {destination ID} {control time} {period time} {pause time}
    Ex: VBR 9 5 3 2
    {destination ID}: Radio Id of the receiving device. In the example ID is 9
    {controlTime}   : If the value is 0, then {vibrate/light up} until the button on the device is pressed,
                      if not 0, then {vibrate/light up} until the specified time period.ype int, in seconds.
                      In the example, control time is 5s.
    {period} {pause}: Specify the time for the device to repeat.
                      In the example, then after 3 seconds of vibrate/light up the device will stop for 2s
                      then continue to vibrate/light up.
  Request data           : REQ {destination ID}
    Sends a request signal to receive status information of the specified device via id.
    If destination ID has a value of 100, it means sending signals to all active devices.
  Broadcast           : BRD
    Send vibration in 3s signals to all devices
*/
// Process the instruction after being read from the serial port and assign the values ​​to the data packet.
void readInstructionInput(String instructionInput)
{
  // Get opcode
  int index = instructionInput.indexOf(' '); // Find the position of the first space
  String opcodeInput = instructionInput.substring(0, index);
  if (opcodeInput == "VBR")
  {
    _radioData.opcode = VBR;
    // Serial.println("Vibrate signal");
  }
  else if (opcodeInput == "LGT")
  {
    _radioData.opcode = LGT;
    // Serial.println("Light up signal");
  }
  else if (opcodeInput == "VLG")
  {
    _radioData.opcode = VLG;
    // Serial.println("Vibrate and light up signal");
  }
  else if (opcodeInput == "BRD")
  {
    _radioData.opcode = BRD;
    _radioData.controlTime = 3;
    // Serial.println("BroadCast signal");
    return;
  }
  else if (opcodeInput == "REQ")
  {
    _radioData.opcode = REQ;
    // Serial.println("Request data signal");
    // Get {destination ID}
    String restStr = instructionInput.substring(index + 1);
    sscanf(restStr.c_str(), "%d", &DESTINATION_RADIO_ID);
    return;
  }

  // Get the remaining ingredients: {destination ID} {control time} {period time} {pause time}
  String restStr = instructionInput.substring(index + 1);
  sscanf(restStr.c_str(), "%d %d %d %d", &DESTINATION_RADIO_ID,
         &_radioData.controlTime,
         &_radioData.periodTime,
         &_radioData.pauseTime);
}

/*
The function sends a BEGIN signal to the receiver and receives a response, 
continues to send an END signal and waits for the ACK packet to respond with a REQ signal.
*/
void requestData(uint8_t id)
{
  // Serial.println("Requesting data");
  RadioPacket radioData;

  radioData.opcode = BEGIN;
  radioData.fromID = TRANSMITTER_RADIO_ID; // When the receiver sees this packet type, it will load an ACK data packet.
  if (_radio.send(id, &radioData, sizeof(radioData)))
  {
    // Serial.println("...Success");
    radioData.opcode = END;

    if (_radio.send(id, &radioData, sizeof(radioData)))
    {
      while (_radio.hasAckData()) // Look to see if the receiver provided the ACK data.
      {
        RadioPacket ackData;
        _radio.readData(&ackData);

        if (ackData.opcode == REQ)
        {
          // String msg = "  Received ACK data from ";
          reqResponse += String(ackData.fromID) + ":" + ackData.pin + "-";
        }
      }
    }
    else
    {
      reqResponse += "-1";
      return;
    }
  }
  else
  {
    reqResponse += "-1";
    return;
  }
}


void setup()
{
  // Initialize serial communication
  Serial.begin(9600);

  memset(activeDevices, 0, sizeof(activeDevices));

  // Configure SPI pins.
  SPI.begin(PIN_RADIO_SCK, PIN_RADIO_MISO, PIN_RADIO_MOSI, PIN_RADIO_CSN);
  // Indicate to NRFLite that it should not call SPI.begin() during initialization since it has already been done.
  uint8_t callSpiBegin = 0;

  if (!_radio.init(TRANSMITTER_RADIO_ID, PIN_RADIO_CE, PIN_RADIO_CSN, NRFLite::BITRATE2MBPS, 100, callSpiBegin))
  {
    Serial.println("Cannot communicate with radio");
    while (1)
      ; // Wait here forever.
  }
  _radioData.fromID = TRANSMITTER_RADIO_ID;
  Serial.println("Setup done!");
}

void loop()
{
  while (!Serial.available())
  {
    // Wait until data is sent from the serial port
  }
  /* instructionInput format:
  VBR/VGT/VLG opcode  : {opcode} {destination ID} {control time} {period time} {pause time}
  Request data        : REQ {destination ID}
  Broadcast           : BRD
  */
  String instructionInput = Serial.readStringUntil('\n');
  readInstructionInput(instructionInput);
  if (_radioData.opcode == BRD)
  {
    activeCount = 0;
    // Serial.println("Sending broadcast: ...");
    for (uint8_t i = 0; i < RECEIVER_RADIO_ID_RANGE; i++)
    {
      if (_radio.send(i, &_radioData, sizeof(_radioData)))
      {
        activeDevices[activeCount] = i;
        activeCount++;
        // Serial.println("Device ID " + String(i) + " active!");
      }
    }
    Serial.println(opcodeToString[_radioData.opcode] + activeCount);
  }
  else if (_radioData.opcode == REQ)
  {
    // Serial.println("Sending request data: ...");
    reqResponse = opcodeToString[_radioData.opcode];

    if (activeCount == 0) // In case there is no active device
    {
      Serial.println(reqResponse + "-1");
    }
    else
    {
      if (DESTINATION_RADIO_ID == 100) // In case id is 100, send signal to all active devices
      {
        for (uint8_t i = 0; i < activeCount; i++)
        {
          requestData(activeDevices[i]);
        }
      }
      else
      {
        requestData(DESTINATION_RADIO_ID); // Only send signals to the specified id
      }
      Serial.println(reqResponse);
    }
  }
  else // The remaining cases are control signals
  {
    // Serial.print("Sending to ID " + String(DESTINATION_RADIO_ID) + ": ...");

    if (_radio.send(DESTINATION_RADIO_ID, &_radioData, sizeof(_radioData)))
    {
      // Serial.println("...Success");
      Serial.println(opcodeToString[_radioData.opcode] + "1");
    }
    else
    {
      // Serial.println("...Fail");
      Serial.println(opcodeToString[_radioData.opcode] + "-1");
    }
  }
  // Serial.println("wait command: ...");
  delay(5);
}