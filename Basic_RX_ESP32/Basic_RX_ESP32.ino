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
// #include <Wire.h>
// #include <Adafruit_GFX.h>
// #include <Adafruit_SSD1306.h>

// #define SCREEN_WIDTH 128
// #define SCREEN_HEIGHT 64

// // declaration for ssd1306 display connected using I2C
// #define OLED_RESET  -1 //reset pin
// #define SCREEN_ADDRESS 0x78

// Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const static uint8_t PIN_RADIO_CE = 4;
const static uint8_t PIN_RADIO_CSN = 5;
const static uint8_t PIN_RADIO_MOSI = 23;
const static uint8_t PIN_RADIO_MISO = 19;
const static uint8_t PIN_RADIO_SCK = 18;
const static uint8_t PIN_VIBRA_IN = 15;

static uint8_t RADIO_ID = 2;


struct __attribute__((packed)) RadioPacket // Note the packed attribute.
{
  uint8_t FromRadioId;
  uint8_t VibrationTime;
  String mess;
};

NRFLite _radio;
RadioPacket _radioData;


void setup()
{
  Serial.begin(9600);

  //   // intialize the OLED object
  //   if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
  //     Serial.println(F("SSD1306 allocation failed"));
  //     for(;;); // don't proceed, loop forever
  //   }
  //   // Clear the buffer.
  // // display.clearDisplay();


  // // Display Text
  // display.setTextSize(1);
  // display.setTextColor(WHITE);
  // display.setCursor(0,28);
  // display.println("Hello Duan!");
  // display.display();
  // delay(2000);
  // display.clearDisplay();



    // // check ID
    // pinMode(GPIO_NUM_33, INPUT_PULLUP);
    // int id = digitalRead(GPIO_NUM_33);
    // if(id == LOW) { // có jumper kết nối
    //   RADIO_ID = 0;
    //   Serial.println("Id: 0");
    // } else {
    //   RADIO_ID = 15;
    //   Serial.println("Id: 15");
    // }

    // Motor Iput config
    pinMode(PIN_VIBRA_IN, OUTPUT);
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
  
    // Serial.println("Loop: ");
    // Display Text
  // display.setTextSize(1);
  // display.setTextColor(BLACK);
  // display.setCursor(0,28);
  // display.println("Hello Duan!");
  // display.display();
  // delay(2000);

    while (_radio.hasData())
    {
      _radio.readData(&_radioData); 
      Serial.println("Has data: "); 
      _radioData.mess.toUpperCase();  
      Serial.println(_radioData.mess);
      digitalWrite(PIN_VIBRA_IN, HIGH);
      delay(_radioData.VibrationTime*1000);
      digitalWrite(PIN_VIBRA_IN, LOW);
    }
}
