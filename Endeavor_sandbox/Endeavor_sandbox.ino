/*
 Name:		Endeavor_sandbox.ino
 Created:	8/27/2022 3:36:02 PM
 Author:	david

 PROTOTYPE - CC:50:E3:80:A2:E8  192.168.0.36
 PRODUCTION - XX:XX:XX:XX:XX:XX  192.168.0.12

*/

//#include <HttpClient.h>
#include <WiFi.h>
#include <ArduinoOTA.h>
#include <ESPmDNS.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <ArduinoHttpClient.h>


class led_flash;
const char* ssid = "Wilson.Net-2.4G";
const char* password = "Pertle-Duck";
float version = 0.5;

const long utcOffsetInSeconds = 3600;
char daysOfTheWeek[7][12] = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

//variables for blinking an LED with Millis
unsigned long previous_millis = 0;  // will store last time LED was updated
const long interval = 500;  // interval at which to blink (milliseconds)

/**
 * \brief: ledState used to set the LED
 */

const int led = 2;
const int callForHeat = 4;
const int soundAlarmReset = 5;
const int soundAlarm = 25;
const int waterPump = 26;
const int boiler = 27;
const int manualReport = 32;
const int manualConfig = 33;
bool state = true;

char serverAddress[] = "192.168.0.36";  // server address
int port = 44364;

WiFiClient wifi;
HttpClient client = HttpClient(wifi, serverAddress, port);
int status = WL_IDLE_STATUS;

//------------------------------------------------------------------------------------------

Adafruit_SSD1306 waterDsp(-1);
Adafruit_SSD1306 boilerDsp(-1);
// this is the default address of the display(0x78) on back of display
#define OLED1 0x3C
// this is after switching over the jumper.
#define OLED2 0x3D





void setup() {

    waterDsp.begin(SSD1306_SWITCHCAPVCC, OLED1);
    waterDsp.clearDisplay();
    waterDsp.display();
	
    boilerDsp.begin(SSD1306_SWITCHCAPVCC, OLED2);
    boilerDsp.clearDisplay();
    boilerDsp.display();
	

    pinMode(led, OUTPUT);
    digitalWrite(led, LOW);

    pinMode(callForHeat, INPUT);
    pinMode(callForHeat, INPUT_PULLDOWN);
    digitalWrite(callForHeat,LOW);

    pinMode(soundAlarmReset, INPUT);
    pinMode(soundAlarmReset, INPUT_PULLDOWN);
    digitalWrite(soundAlarmReset, LOW);

    pinMode(soundAlarm, OUTPUT);
    //pinMode(soundAlarm, INPUT_PULLDOWN);
    digitalWrite(soundAlarm, LOW);
	
    pinMode(waterPump, OUTPUT);
    //pinMode(waterPump, INPUT_PULLDOWN);
    digitalWrite(waterPump, LOW);
	
    pinMode(boiler, OUTPUT);
    //pinMode(boiler, INPUT_PULLDOWN);
    digitalWrite(boiler, LOW);

    pinMode(manualReport, INPUT); // report button
    pinMode(manualReport, INPUT_PULLDOWN);
    digitalWrite(manualReport, LOW);

    pinMode(manualConfig, INPUT);
    pinMode(manualConfig, INPUT_PULLDOWN);
    digitalWrite(manualConfig, LOW);

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






void fireDBcall();







void loop() {
	
    ArduinoOTA.handle();
    timeClient.update();
	
    //tone(soundAlarm, 1000, 500);
    //delay(1000);
    //noTone(soundAlarm);
	
    //loop to blink without delay
    const auto current_millis = millis();

    if (current_millis - previous_millis >= interval) {
        // save the last time you blinked the LED
        previous_millis = current_millis;

        digitalWrite(boiler, !digitalRead(boiler));
        digitalWrite(waterPump, !digitalRead(waterPump));
        digitalWrite(led, !digitalRead(led));

    }


    for (int i = 0; i < 270; i += 10) {

        waterDsp.setTextSize(1);
        waterDsp.setTextColor(WHITE);
        waterDsp.setCursor(0, 0);
        waterDsp.println("Display 1");
        waterDsp.fillCircle(i, 30, 5, 1);
        waterDsp.display();
        waterDsp.clearDisplay();

        boilerDsp.setTextSize(1);
        boilerDsp.setTextColor(WHITE);
        boilerDsp.setCursor(0, 0);
        boilerDsp.println("Display 2");
        boilerDsp.fillCircle(i - 127, 30, 5, 1);
        boilerDsp.display();
        boilerDsp.clearDisplay();
    }
}





void fireDBcall()
{
    //String serverName = "http://192.168.0.36";
    //    //"http://192.168.0.21:44364/Discovery_Sandbox.asmx?op=endeavor";
    //String URLPath = "/Discovery_Sandbox.asmx?op=endeavor";

    String C1 = "C1=1.537,";
    String C2 = "C2=1.537,";
    String C3 = "C3=1.537,";

    String C4 = "C4=1.537,";
    String C5 = "C5=1.53,";
    String C6 = "C6='first call'";

    String dataStream = "?op=endeavor ";
	dataStream += C1 + C2 + C3 + C4 + C5 + C6;
	


    client.get("http://192.168.0.21:44364/Discovery_Sandbox.asmx?op=isAlive");

    // read the status code and body of the response
    int statusCode = client.responseStatusCode();
    String response = client.responseBody();


    String postData = dataStream;

    client.beginRequest();
    client.post(dataStream);
    client.sendHeader("Content-Type", "application/x-www-form-urlencoded");
    client.sendHeader("Content-Length", postData.length());
    client.sendHeader("X-Custom-Header", "custom-header-value");
    client.beginBody();
    client.print(postData);
    client.endRequest();

    // read the status code and body of the response
    statusCode = client.responseStatusCode();
    response = client.responseBody();
}

