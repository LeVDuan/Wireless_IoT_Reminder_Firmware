//----------------------------------------------------------------------
//  IDE: Visual Studio Code + PlatformIO extension
//  Organization: School of Information and Communication Technogloy,
//                Ha Noi Univerversity of Science and Technology
//  Website: https://soict.hust.edu.vn
//  Example: Read NFC Card via RC522 module and send card information to data center SiOT 
//  Purpose: using SPI interface to connect RC522 module
//
//  Using SIOT Center and virtual device:
//  1. Open the SIOT Center http://siot.soict.ai
//  2. Login with account sinno@soict.hust.edu.vn and password is 123456
//  3. On the left navigation menu, click Arduino Siot Client to see realtime data
//  Shortlink: http://siot.soict.ai/devices/siot-demo
//
//  Dependancies:
//    - miguelbalboa/MFRC522@^1.4.10    
//
//  References:
//    - Thank for the original source code from Miki Balboa, see: https://github.com/miguelbalboa/rfid
//    - Youtube: https://youtu.be/VQAy33XYFEY
//----------------------------------------------------------------------

 
#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>

#define ENABLE_SIOT     // use it to send NFC card infor to SIOT Data Hub. comment it to ignore.
#ifdef ENABLE_SIOT
  #include <siot_core_lib.h>    // SIOT Core Lib - all packages or you could select each package manually.
  // Handler adhoc wifi station
  WiFiSelfEnroll * MyWiFi = new WiFiSelfEnroll();

  // SIoT Management Handler, send/receive data to/from SIOT API Center 
  SIOTClient siotclient;  
#endif

#if defined(ARDUINO_ARCH_ESP32)  
 #if defined(ARDUINO_ESP32_DEV)    // for board DOIT ESP32 DevKit as Readme.md
  #define SS_PIN 21     //   SS/SDA
  #define RST_PIN 0    //   Reset
 #elif defined(ARDUINO_ESP32C3_DEV)    // for board ESP32-C3-DevKitM-1 dual USB as Readme.md
  //  Các tổ hợp vị tri SPI đã thử thành công
  #define SS_PIN   5  // 19 //  19 // 10     //SS/SDA
  #define SCLK_PIN 4  // 18 //  18 //  6   
  #define MOSI_PIN 18 // 5 //  5  //  7   
  #define MISO_PIN 19 // 4 //  4  //  2   
  #define RST_PIN   0 // 0 //  0  //  0
  #endif
#elif  defined(ARDUINO_ARCH_ESP8266)
  // ESP8266 luôn cố định 2 cụm  SPI_PINS_HSPI với SCK=14,MISO=12,MOSI=13, CS=15  
  #define SS_PIN   15 
  #define RST_PIN  0
#endif

MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class

MFRC522::MIFARE_Key key; 

// Init array that will store new NUID 
byte nuidPICC[4];
String json;
char tmp[4];


/**
 * Helper routine to dump a byte array as hex values to Serial. 
 */
void printHex(byte *buffer, byte bufferSize) {  
  json =  "{\"value\":\"";
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
    sprintf(tmp, "%2X" , buffer[i]); 
    json = json + tmp;
  }
  json = json + "\"}";  
}
/**
 * Helper routine to dump a byte array as dec values to Serial.
 */
void printDec(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], DEC);
  }
}
void setup() { 
  // Built-in led ready to bright, to be the lamp indicator
  pinMode(LED_BUILTIN, OUTPUT);
  // Turn of the lamp indicator
  digitalWrite(LED_BUILTIN, LOW);

#ifdef ENABLE_SIOT
  // Make sure WiFi ssid/password is correct. Otherwise, raise the Adhoc AP Station with ssid = SOICT_CORE_BOARD and password =  12345678
  MyWiFi->setup();
  // Release the memory allocated for WiFi Station Handler after finishing his work
  delete  MyWiFi;
  MyWiFi = _NULL;
  // Declare the virtual device information which you have register at http://siot.soict.ai (free)
  siotclient.Checkin("siot-demo","eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJfaWQiOiIwZDI1NWI5Ni05OWMyLTRmNTItOTg1NS0yZWRhNmI3MzYyMzAiLCJpYXQiOjE2ODcwNzQ4OTd9.SHRCTNASXYEPkYR-ZVadu1P19UPRTHQoHcZjzAoUaiM");
#endif
  
  Serial.begin(115200);

#if defined(ARDUINO_ARCH_ESP32) &&  defined(ARDUINO_ESP32C3_DEV) // for board ESP32-C3 DevKit as Readme.md
  // ESP32 cho phép chọn chân pin bất kì làm SPI
  SPI.begin(SCLK_PIN, MISO_PIN, MOSI_PIN, SS_PIN);
#else  // defined(ARDUINO_ARCH_ESP8266)
  // ESP8266 luôn cố định 2 cụm  SPI_PINS_HSPI với SCK=14,MISO=12,MOSI=13, CS=15
  SPI.begin(); // Init SPI bus
#endif  
  rfid.PCD_Init(); // Init MFRC522 

  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }

  Serial.println(F("This code scan the MIFARE Classsic NUID."));
  Serial.print(F("Using the following key:"));
  printHex(key.keyByte, MFRC522::MF_KEY_SIZE);
}
 
void loop() {

  // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
  if ( ! rfid.PICC_IsNewCardPresent())
    return;

  // Verify if the NUID has been readed
  if ( ! rfid.PICC_ReadCardSerial())
    return;


  // Turn on the lamp indicator whenever find something
  digitalWrite(LED_BUILTIN, HIGH);

  Serial.print(F("PICC type: "));
  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
  Serial.println(rfid.PICC_GetTypeName(piccType));

  // Check is the PICC of Classic MIFARE type
  if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI && 
    piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
    piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
    Serial.println(F("Your tag is not of type MIFARE Classic."));
    return;
  }

  if (rfid.uid.uidByte[0] != nuidPICC[0] || 
    rfid.uid.uidByte[1] != nuidPICC[1] || 
    rfid.uid.uidByte[2] != nuidPICC[2] || 
    rfid.uid.uidByte[3] != nuidPICC[3] ) {
    Serial.println(F("A new card has been detected."));

    // Store NUID into nuidPICC array
    for (byte i = 0; i < 4; i++) {
      nuidPICC[i] = rfid.uid.uidByte[i];
    }
   
    Serial.println(F("The NUID tag is:"));
    Serial.print(F("In hex: "));
    printHex(rfid.uid.uidByte, rfid.uid.size);

#ifdef ENABLE_SIOT
     siotclient.Send("nfc-siot", json);
#endif     

    Serial.println();
    Serial.print(F("In dec: "));
    printDec(rfid.uid.uidByte, rfid.uid.size);
    Serial.println();
  }
  else 
  {
    Serial.println(F("Card read previously."));
#ifdef ENABLE_SIOT
    siotclient.Send("nfc-siot", json);
#endif    
  }

  delay(200);
  // Turn off the lamp indicator whenever find something
  digitalWrite(LED_BUILTIN, LOW);

  // Halt PICC
  rfid.PICC_HaltA();

  // Stop encryption on PCD
  rfid.PCD_StopCrypto1();
}