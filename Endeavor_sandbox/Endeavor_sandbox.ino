/*
 Name:		Endeavor_sandbox.ino
 Created:	8/27/2022 3:36:02 PM
 Author:	david
*/

#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

const char* ssid = "Wilson.Net-2.4G";
const char* password = "Pertle-Duck";

//variables for blinking an LED with Millis
const int led = 2; // ESP32 Pin to which onboard LED is connected

unsigned long previousMillis = 0;  // will store last time LED was updated

const long interval = 500;  // interval at which to blink (milliseconds)

int ledState = LOW;  // ledState used to set the LED

const int p26 = 26;
const int p27 = 27;

int p26State = LOW;
int p27State = HIGH;

void setup() {

    pinMode(led, OUTPUT);
    pinMode(p26, OUTPUT);
    pinMode(p27, OUTPUT);


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
}

void loop() {
    ArduinoOTA.handle();

    //loop to blink without delay
    unsigned long currentMillis = millis();

    if (currentMillis - previousMillis >= interval) {
        // save the last time you blinked the LED
        previousMillis = currentMillis;

        // if the LED is off turn it on and vice-versa:
        ledState = not(ledState);
        p26State = not(p26State);
        p27State = not(p27State);

        // set the LED with the ledState of the variable:
        digitalWrite(led, ledState);
        digitalWrite(p26, p26State);
        digitalWrite(p27, p27State);

    }
}
