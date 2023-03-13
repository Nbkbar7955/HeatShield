/*
 Name:    Endeavor_sandbox.ino
 Created: 8/27/2022 3:36:02 PM
 Author:  david

 PROTOTYPE - CC:50:E3:80:A2:E8  192.168.0.36
 PRODUCTION - 24:6F:28:9E:9B:D0  192.168.0.37

*/

#include <SparkFun_MCP9600.h>
#include <Wire.h>
//#include <HttpClient.h>
#include <WiFi.h>
#include <ArduinoOTA.h>
#include <ESPmDNS.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ArduinoHttpClient.h>




//------------------------------------------------------------------------------------------
// Operational Vars

String version = "0.7";

int boilerLow = 0;

String activeMode = "active";

float  tHigh = 155;
float  tLow = 130;

float aHigh = 155;
float aLow = 135;

float sHigh = 135;
float sLow = 115;

float iHigh = 115;
float iLow = 95;

float oHigh = 95;
float oLow = 65;

float tmpWater = 0;
float tmpBoiler = 0;
float tmpOther = 0;

float waterTemp = 0;
float waterTempPrev = 0;
float boilerTemp = 0;
float boilerTempPrev = 0;
float otherTmp = 0;
float otherTempPrev = 0;

String wt = "";
String bt = "";
String ev = "";


//------------------------------------------------------------------------------------------

////class led_flash;
const char* ssid = "Wilson.Net-2.4G";
const char* password = "Pertle-Duck";

const long utcOffsetInSeconds = 3600;
char daysOfTheWeek[7][12] = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

// variables for blinking an LED with Millis
unsigned long previous_millis = 0;  // will store lastss time LED was updated
//unsigned long current_millis = 0;  // will store lastss time LED was updated

const long interval = 5000;  // interval at which to blink (milliseconds)
bool testRelays = true;

const int led = 2;//o
const int callForHeat = 4; //o
const int soundAlarmReset = 5;//i CALLFORHEAT
const int cpuStdbyRelay = 18;//o ??????
const int cpuStdby = 19;//o ??????
const int soundAlarm = 25;//o SOUNDALARM
const int waterPump = 26;//o WATERPUMP RELAY
const int boiler = 27;//o BURNER RELAY
const int manualReport = 32;//i PB1
const int manualConfig = 33;//i PB2

const int count = 0;
bool cpuStdbyFlag = false;
bool testAlarm = false;
bool state = true;

char serverAddress[] = "192.168.0.35";  // server address
int port = 44364;

WiFiClient wifi;
HttpClient client = HttpClient(wifi, serverAddress, port);
int status = WL_IDLE_STATUS;


// ------------------------------------------------------------------------------------------
// Tempreture 
// ------------------------------------------------------------------------------------------


MCP9600 Oth; // OTHER THERMOCOUPLE
MCP9600 Wtr; // WATER THERMOCOUPLE
MCP9600 Blr; // BOILER THERMOCOUPLE


//------------------------------------------------------------------------------------------
//
Adafruit_SSD1306 waterDsp(-1); // OLEB ?????
Adafruit_SSD1306 boilerDsp(-1); //OLED ?????

#define OLED1 0x3C // OLED 1
#define OLED2 0x3D // OLED 2



// Prototypes
///------------------------------------------------------------------------------------------

auto waterCycle() -> void;
auto burnCycle() -> void;
auto opCycle() -> void;
auto updateDisplay() -> void;

///------------------------------------------------------------------------------------------



// Setup
//------------------------------------------------------------------------------------------
//

void setup() {

    Wire.begin();
	
  // Begin Thermocouples
    Wtr.begin(0x060);
    Blr.begin(0x061);
    Oth.begin(0x067);

    waterDsp.begin(SSD1306_SWITCHCAPVCC, OLED1);
    waterDsp.clearDisplay();
    waterDsp.display();

    boilerDsp.begin(SSD1306_SWITCHCAPVCC, OLED2);
    boilerDsp.clearDisplay();
    boilerDsp.display();


    // OUTPUTS   pinModes
    // ----------------------------------------
    pinMode(led, OUTPUT);
    digitalWrite(led, LOW);

  // ????????????????????????
    pinMode(callForHeat, INPUT);  // i PIN 4

    pinMode(soundAlarmReset, OUTPUT); // o PIN 5
    digitalWrite(soundAlarmReset, LOW);

    pinMode(cpuStdbyRelay, OUTPUT);  // o PIN 18
    digitalWrite(cpuStdbyRelay, LOW);

    pinMode(cpuStdby, OUTPUT); // o PIN 19
    digitalWrite(cpuStdby, LOW);

    pinMode(soundAlarm, OUTPUT); // o PIN 25
    digitalWrite(soundAlarm, LOW);

    pinMode(waterPump, OUTPUT); // o PIN 26
    digitalWrite(waterPump, LOW);

    pinMode(boiler, OUTPUT); // o PIN 27
    digitalWrite(boiler, HIGH);
  
    pinMode(manualReport, OUTPUT); // o PIN 32
    digitalWrite(manualReport, LOW);
  
    pinMode(manualConfig, OUTPUT); // o PIN 33
    digitalWrite(manualConfig, LOW);
  


    // ----------------------------------------


    Serial.begin(115200);
    Serial.println("Booting");

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    while (WiFi.waitForConnectResult() != WL_CONNECTED) {
        Serial.println("Connection Failed! Rebooting...");
        delay(5000);
        ESP.restart();
    }

    // Port defaults to 3232
    ArduinoOTA.setPort(3232);

    // Hostname defaults to esp3232-[MAC]
    ArduinoOTA.setHostname("HCv0.03");

    // No authentication by default
    // ArduinoOTA.setPassword("admin");

    // Password can be set with it's md5 value as well
    // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
    // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

    /*
    // Set your Static IP address
        IPAddress local_IP(192, 168, 0, 37);
    // Set your Gateway IP address
    IPAddress gateway(192, 168, 0, 1);
    IPAddress subnet(255, 255, 0, 0);
*/

    ArduinoOTA
        .onStart([]() {
        String type;
        if (ArduinoOTA.getCommand() == U_FLASH)
            type = "sketch";
        else // U_SPIFFS
            type = "filesystem";

        // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
        Serial.println("Start updating " + type);
            })
        .onEnd([]() {
                Serial.println("\nEnd");
            })
                .onProgress([](unsigned int progress, unsigned int total) {
                Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
                    })
                .onError([](ota_error_t error) {
                        Serial.printf("Error[%u]: ", error);
                        if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
                        else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
                        else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
                        else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
                        else if (error == OTA_END_ERROR) Serial.println("End Failed");
                    });

                    ArduinoOTA.begin();

                    Serial.println("Ready");
                    Serial.print("IP address: ");
                    Serial.println(WiFi.localIP());

  timeClient.begin();




}





// Loop
//------------------------------------------------------------------------------------------

void loop() {

    ArduinoOTA.handle();
    timeClient.update();


    // Code Below
    //------------------------------------------------------------------------------------------
    //------------------------------------------------------------------------------------------

    const auto current_millis = millis();

    if (testRelays) {
        if (current_millis - previous_millis >= interval) {
            // save the last time you blinked the LED
            previous_millis = current_millis;


            digitalWrite(boiler, !digitalRead(boiler));
            digitalWrite(waterPump, !digitalRead(waterPump));
            digitalWrite(led, !digitalRead(led));


  
        }
        //opCycle();

     // Code Above
     //------------------------------------------------------------------------------------------
     //------------------------------------------------------------------------------------------


    }
}


// Functions
//------------------------------------------------------------------------------------------


auto opCycle() -> void
{
    ArduinoOTA.handle();
    timeClient.update();
	
    updateDisplay();
  
    
  // if call for heat go to activate immediately
	// ----------------------------------------------------------------------------------------------
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// ACTIVE MODE FORCED  PID
	// //
    if ( digitalRead(callForHeat) == HIGH && activeMode != "active" )
    {
        activeMode = "active";
    }
    else
    {
        activeMode = "stdby";
    }

  
    // -------------------------------------------------------------------------------------
  
    if (activeMode == "active")
    {
        tHigh = aHigh;
        tLow = aLow;
      
        digitalWrite(waterPump, HIGH);

    }
    else
    {
      if (activeMode == "stdby")
        {
            tHigh = sHigh;
            tLow = sLow;
        
            digitalWrite(waterPump, HIGH);
      }
        else
        {
          if (activeMode == "inactive")
          {
                tHigh = iHigh;
                tLow = iLow;
          }
            else
            {
                tHigh = oHigh;
                tLow = oLow;
            }
        }
    }
    // -------------------------------------------------------------------------------------

    burnCycle();
    waterCycle();
    updateDisplay();
}

auto waterCycle() -> void
{
    ArduinoOTA.handle();
    timeClient.update();
  
  if (activeMode == "active"|| activeMode == "stdby")
  {
        digitalWrite(waterPump, HIGH);
  }
  else
  {
        digitalWrite(waterPump, LOW); 
  }
}


auto burnCycle() -> void
{
    ArduinoOTA.handle();
    timeClient.update();
	
    updateDisplay();
  
    waterTemp = ((Wtr.getThermocoupleTemp() * 9 / 5) + 32);
    boilerTemp = ((Blr.getThermocoupleTemp() * 9 / 5) + 32);

  // if we hit the bottom
    if (waterTemp <= tLow) {
      
        // go to high
        while (waterTemp < tHigh) {
        	
            ArduinoOTA.handle();
            timeClient.update();
          
            ArduinoOTA.handle();
            updateDisplay();
          
            // save the current temp
            waterTempPrev = waterTemp;
          
            // fire the burner to start raising the temp
            digitalWrite(boiler, HIGH);         
            delay(120000); // 120,000 = 2min - run for x mins         
            digitalWrite(boiler, LOW); // then stop to use boiler temp to continue water increase

            waterTemp = ((Wtr.getThermocoupleTemp() * 9 / 5) + 32);
            while (waterTemp > waterTempPrev) // while water temp still rising
            {
            	
                ArduinoOTA.handle();
                timeClient.update();
            	
                updateDisplay();
              
                waterTempPrev = waterTemp;
                boilerLow = ((Blr.getThermocoupleTemp() * 9 / 5) + 32);
                waterTemp = ((Wtr.getThermocoupleTemp() * 9 / 5) + 32);

            }         
        }
    }  
}

auto updateDisplay() -> void
{
    ArduinoOTA.handle();
    timeClient.update();


    // (26°C × 9/5) + 32 = 78.8°F 
    waterTemp = ((Wtr.getThermocoupleTemp() * 9 / 5) + 32);
    boilerTemp = ((Blr.getThermocoupleTemp() * 9 / 5) + 32);
    otherTmp = ((Oth.getThermocoupleTemp() * 9 / 5) + 32);

    wt = String(waterTemp);
    bt = String(boilerTemp);
    ev = String(otherTmp);

    String title = "HeatShield - PID";
    //title = title + version;
	
    waterDsp.setTextSize(1);
    waterDsp.clearDisplay();
    waterDsp.setTextColor(WHITE);

    waterDsp.drawLine(0, 0, 127, 20, WHITE);

    waterDsp.setCursor(11, 0);
    waterDsp.println(title);

    waterDsp.drawLine(0, 7, 127, 7, WHITE);

    waterDsp.setCursor(11, 11);
    waterDsp.println("w:" + wt + "  |  ");

    waterDsp.setCursor(70, 11);
    waterDsp.println("b:" + bt);

    waterDsp.setCursor(11, 22);
    waterDsp.println("e:" + ev);

    waterDsp.display();


    boilerDsp.clearDisplay();
    boilerDsp.setTextColor(WHITE);

    boilerDsp.setCursor(11, 0);
    boilerDsp.println("*PID*");

    boilerDsp.drawLine(0, 7, 127, 7, WHITE);

    boilerDsp.setCursor(11, 11);
    boilerDsp.println("Mode: " + activeMode);

    boilerDsp.setCursor(11, 22);
    boilerDsp.println("tHigh: " + String(tHigh));

    boilerDsp.setCursor(11, 33);
    boilerDsp.println("tLow: " + String(tLow));

    boilerDsp.display();
}




