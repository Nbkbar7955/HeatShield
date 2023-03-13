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

int yellowLow = 0;

String activeMode = "active";

float highTemperature = 160;
float lowTemperature = 130;

float highActiveTemperature = 160;
float lowActiveTemperature = 130;

float highStdbyTemperature = 130;
float lowStdbyTemperature = 110;

float highInactiveTemperature = 115;
float lowInactiveTemperature = 95;

float highOffTemperature = 95;
float lowOffTemperature = 65;

float tmpBlue = 0;
float tmpYellow = 0;
float tmpPurple = 0;

float blueTemp = 0;
float blueTempPrev = 0;
float yellowTemp = 0;
float yellowTempPrev = 0;
float purpleTmp = 0;
float purpleTempPrev = 0;

String blueT = "";
String yellowT = "";
String purpleT = "";


//------------------------------------------------------------------------------------------

////class led_flash;
const char* ssid = "Wilson.Net-2.4G";
const char* password = "Pertle-Duck";

const long utcOffsetInSeconds = 3600;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

// variables for blinking an LED with Millis
unsigned long millisBefore = 0; // will store lastss time LED was updated
//unsigned long current_millis = 0;  // will store lastss time LED was updated

const long interval = 1000; // interval at which to blink (milliseconds)
bool cycleRelays = true;

const int procLED = 2; //o
const int callForHeat = 4; //i CALLFORHEAT
const int soundAlarmReset = 5; //o ??????

const int PIN19 = 19; //o ??????
const int speaker = 25; //o SOUNDALARM
const int blueRelay = 26; //o WATERPUMP RELAY
const int yellowRelay = 27; //o BURNER RELAY
const int purpleRelay = 18; //o ??????
const int PB1 = 32; //i PB1
const int PB2 = 33; //i PB2

const int count = 0;
bool purpleFlag = false;
bool testAlarm = false;
bool state = true;

char serverAddress[] = "192.168.0.35"; // server address
int port = 44364;

WiFiClient wifi;
HttpClient client = HttpClient(wifi, serverAddress, port);
int status = WL_IDLE_STATUS;


// ------------------------------------------------------------------------------------------
// Tempreture 
// ------------------------------------------------------------------------------------------


MCP9600 purpleThermocouple; // OTHER THERMOCOUPLE
MCP9600 blueThermocouple; // WATER THERMOCOUPLE
MCP9600 yellowThermocouple; // BOILER THERMOCOUPLE


//------------------------------------------------------------------------------------------
//
Adafruit_SSD1306 displayOne(-1); // OLEB ?????
Adafruit_SSD1306 displayTwo(-1); //OLED ?????

#define OLED1 0x3C // OLED 1
#define OLED2 0x3D // OLED 2


// Prototypes
///------------------------------------------------------------------------------------------

auto waterCycle() -> void;
auto burnCycle() -> void;
auto operationalCycle() -> void;
auto updateDisplay() -> void;

///------------------------------------------------------------------------------------------


// Setup
//------------------------------------------------------------------------------------------
//

void setup()
{
	Wire.begin();

	// Begin Thermocouples
	blueThermocouple.begin(0x060);
	yellowThermocouple.begin(0x061);
	purpleThermocouple.begin(0x067);

	displayOne.begin(SSD1306_SWITCHCAPVCC, OLED1);
	displayOne.clearDisplay();
	displayOne.display();

	displayTwo.begin(SSD1306_SWITCHCAPVCC, OLED2);
	displayTwo.clearDisplay();
	displayTwo.display();


	// OUTPUTS   pinModes
	// ----------------------------------------
	pinMode(procLED, OUTPUT);
	digitalWrite(procLED, LOW);

	// ????????????????????????
	pinMode(callForHeat, INPUT); // i PIN 4

	pinMode(soundAlarmReset, OUTPUT); // o PIN 5
	digitalWrite(soundAlarmReset, LOW);


	pinMode(PIN19, OUTPUT); // o PIN 19
	digitalWrite(PIN19, LOW);

	pinMode(speaker, OUTPUT); // o PIN 25
	digitalWrite(speaker, LOW);

	pinMode(blueRelay, OUTPUT); // o PIN 26
	digitalWrite(blueRelay, LOW);

	pinMode(yellowRelay, OUTPUT); // o PIN 27
	digitalWrite(yellowRelay, HIGH);

	pinMode(purpleRelay, OUTPUT); // o PIN 18
	digitalWrite(purpleRelay, LOW);

	pinMode(PB1, OUTPUT); // o PIN 32
	digitalWrite(PB1, LOW);

	pinMode(PB2, OUTPUT); // o PIN 33
	digitalWrite(PB2, LOW);


	// ----------------------------------------


	Serial.begin(115200);
	Serial.println("Booting");

	WiFi.mode(WIFI_STA);
	WiFi.begin(ssid, password);

	while (WiFi.waitForConnectResult() != WL_CONNECTED)
	{
		Serial.println("Connection Failed! Rebooting...");
		delay(5000);
		ESP.restart();
	}

	// Port defaults to 3232
	ArduinoOTA.setPort(3232);

	// Hostname defaults to esp3232-[MAC]
	ArduinoOTA.setHostname("HeatShieldCtrl");

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
		.onStart([]()
		{
			String type;
			if (ArduinoOTA.getCommand() == U_FLASH)
				type = "sketch";
			else // U_SPIFFS
				type = "filesystem";

			// NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
			Serial.println("Start updating " + type);
		})
		.onEnd([]()
		{
			Serial.println("\nEnd");
		})
		.onProgress([](unsigned int progress, unsigned int total)
		{
			Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
		})
		.onError([](ota_error_t error)
		{
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

void loop()
{
	ArduinoOTA.handle();
	timeClient.update();


	// Code Below
	//------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------

	const auto millisNow = millis();

	if (cycleRelays)
	{
		if (millisNow - millisBefore < interval)
		{
			return;
		}
		// save the last time you blinked the LED
		millisBefore = millisNow;

		// digitalWrite(yellowRelay, !digitalRead(yellowRelay));
		// digitalWrite(blueRelay, !digitalRead(blueRelay));
		//
		// 
		digitalWrite(procLED, !digitalRead(procLED));
		digitalWrite(purpleRelay, !digitalRead(purpleRelay));
	}
	operationalCycle();

	// Code Above
	//------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------
}


// Functions
//------------------------------------------------------------------------------------------


auto operationalCycle() -> void
{
	ArduinoOTA.handle();
	timeClient.update();

	updateDisplay();


	// if call for heat go to activate immediately
	// ----------------------------------------------------------------------------------------------
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// ACTIVE MODE FORCED  PID
	// //
	if (digitalRead(callForHeat) == HIGH)
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
		highTemperature = highActiveTemperature;
		lowTemperature = lowActiveTemperature;

		digitalWrite(blueRelay, HIGH);
	}
	else
	{
		if (activeMode == "stdby")
		{
			highTemperature = highStdbyTemperature;
			lowTemperature = lowStdbyTemperature;

			digitalWrite(blueRelay, HIGH);
		}
		else
		{
			if (activeMode == "inactive")
			{
				highTemperature = highInactiveTemperature;
				lowTemperature = lowOffTemperature;
			}
			else
			{
				highTemperature = highOffTemperature;
				lowTemperature = lowOffTemperature;
			}
		}
	}
	// -------------------------------------------------------------------------------------

	burnCycle();
	waterCycle();
	updateDisplay();
}


// ------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------
// Methods
// ------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------


auto burnCycle() -> void
{
	ArduinoOTA.handle();
	timeClient.update();

	updateDisplay();

	blueTemp = ((blueThermocouple.getThermocoupleTemp() * 9 / 5) + 32);
	yellowTemp = ((yellowThermocouple.getThermocoupleTemp() * 9 / 5) + 32);

	// if we hit the bottom
	if (blueTemp <= lowTemperature)
	{
		// go to high
		while (blueTemp < highTemperature)
		{
			ArduinoOTA.handle();
			timeClient.update();

			ArduinoOTA.handle();
			updateDisplay();

			// save the current temp
			blueTempPrev = blueTemp;

			// fire the burner to start raising the temp
			digitalWrite(yellowRelay, HIGH);
			delay(120000); // 120,000 = 2min - run for x mins         
			digitalWrite(yellowRelay, LOW); // then stop to use yellowRelay temp to continue water increase

			blueTemp = ((blueThermocouple.getThermocoupleTemp() * 9 / 5) + 32);
			while (blueTemp > blueTempPrev) // while water temp still rising
			{
				ArduinoOTA.handle();
				timeClient.update();

				updateDisplay();

				blueTempPrev = blueTemp;
				yellowLow = ((yellowThermocouple.getThermocoupleTemp() * 9 / 5) + 32);
				blueTemp = ((blueThermocouple.getThermocoupleTemp() * 9 / 5) + 32);
			}
		}
	}
}

auto waterCycle() -> void
{
	ArduinoOTA.handle();
	timeClient.update();

	if (activeMode == "active" || activeMode == "stdby")
	{
		digitalWrite(blueRelay, HIGH);
	}
	else
	{
		digitalWrite(blueRelay, LOW);
	}
}


auto updateDisplay() -> void
{
	ArduinoOTA.handle();
	timeClient.update();


	// (26°C × 9/5) + 32 = 78.8°F 
	blueTemp = ((blueThermocouple.getThermocoupleTemp() * 9 / 5) + 32);
	yellowTemp = ((yellowThermocouple.getThermocoupleTemp() * 9 / 5) + 32);
	purpleTmp = ((purpleThermocouple.getThermocoupleTemp() * 9 / 5) + 32);

	blueT = String(blueTemp);
	yellowT = String(yellowTemp);
	purpleT = String(purpleTmp);

	String title = "HeatShield - PID";
	//title = title + version;

	displayOne.setTextSize(1);
	displayOne.clearDisplay();
	displayOne.setTextColor(WHITE);

	displayOne.drawLine(0, 0, 127, 20, WHITE);

	displayOne.setCursor(11, 0);
	displayOne.println(title);

	displayOne.drawLine(0, 7, 127, 7, WHITE);

	displayOne.setCursor(11, 11);
	displayOne.println("w:" + blueT + "  |  ");

	displayOne.setCursor(70, 11);
	displayOne.println("b:" + yellowT);

	displayOne.setCursor(11, 22);
	displayOne.println("e:" + purpleT);

	displayOne.display();


	displayTwo.clearDisplay();
	displayTwo.setTextColor(WHITE);

	displayTwo.setCursor(11, 0);
	displayTwo.println("*PID*");

	displayTwo.drawLine(0, 7, 127, 7, WHITE);

	displayTwo.setCursor(11, 11);
	displayTwo.println("Mode: " + activeMode);

	displayTwo.setCursor(11, 22);
	displayTwo.println("highTemperature: " + String(highTemperature));

	displayTwo.setCursor(11, 33);
	displayTwo.println("lowTemperature: " + String(lowTemperature));

	displayTwo.display();
}
