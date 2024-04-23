#ifndef WiFiSelfEnroll_h
#define WiFiSelfEnroll_h
#if defined(ARDUINO_ARCH_ESP32)
    #include <Preferences.h>            /// use permanent flash storage  https://randomnerdtutorials.com/esp32-save-data-permanently-preferences/
    #include <WiFi.h>
    #include <WebServer.h>              /// create webserver
#elif  defined(ARDUINO_ARCH_ESP8266)
    #include <Preferences.h>            /// use vshymanskyy/Preferences
    #include <ESP8266WiFi.h>
    #include <ESP8266WebServer.h>       /// create webserver
#endif

class WiFiSelfEnroll{
    private:
        static String ssid;
        static char myssid[32];
        static String password;
        static char mypassword[20];
        static String deviceid;
        static char mydeviceid[30];
        static Preferences preferences;
        static WiFiClient wificlient;
#if defined(ARDUINO_ARCH_ESP32)        
        static WebServer server;
#elif  defined(ARDUINO_ARCH_ESP8266)
        static ESP8266WebServer server;
#endif    
    private:
        /// @brief true if the Adhoc WiFi is active
        bool    APMode;
        static void _HomePage();
        static void _EnrollPage();
        static void _APIScan() ;
        static void _APISave();
        static void _APISettings();
        static void _Reboot();  
        void _loop();
    public:
        void ReadWiFiConfig();
        void SetupStation(const char * ssid, const char * password);    
        void setup();  
        void setup(const char * adhoc_ssid, const char * adhoc_password);    
        char * GetSSID();
        char * GetPassword();
        char * GetDeviceID();
        bool IsConfigOK();
};
#endif