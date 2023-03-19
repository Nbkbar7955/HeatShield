/*
 Name:    Endeavor_sandbox.ino
 Created: 8/27/2022 3:36:02 PM
 Author:  david

 PROTOTYPE - CC:50:E3:80:A2:E8  192.168.0.36
 PRODUCTION - 24:6F:28:9E:9B:D0  192.168.0.37  25?

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

float highTemperature = 150;
float lowTemperature = 120;

float highActiveTemperature = 150;
float lowActiveTemperature = 120;

float highStdbyTemperature = 130;
float lowStdbyTemperature = 110;

float highInactiveTemperature = 115;
float lowInactiveTemperature = 95;

float highOffTemperature = 95;
float lowOffTemperature = 65;

float tmpBlue = 0;
float tmpYellow = 0;
float tmpPurple = 0;

float waterTemp = 0;
float waterTempPrev = 0;
float boilerTemp = 0;
float boilerTempPrev = 0;
float otherTmp = 0;
float otherTempPrev = 0;


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
unsigned long millisBefore = 0; // will store last time LED was updated

const long interval = 2500; // interval at which to blink (milliseconds)

//======================================================================================
//======================================================================================
// my vars

bool PIDMODE = true;
bool shutdown = false;
bool startupIndicator = true;


// $$$
//======================================================================================
//======================================================================================
// my vars

const int processorLED = 2; //o LED on MicroProcessor
const int callForHeat = 4; //i CALLFORHEAT

const int blueRelay = 26; //o WATERPUMP RELAY
const int yellowRelay = 27; //o BURNER RELAY
const int purpleRelay = 18; //o HAVENT DECIDED YET MAYBE IGNITER?

const int speaker = 25; //o SOUNDALARM
const int PIN19 = 19; //o ??????
const int PB1 = 32; //i PB1
const int PB2 = 33; //i PB2
const int PB3 = 5; //i PB3 ??????
const int PB4 = -1; //i PB4 ?????
const int SW2 = 4; //i CALLFORHEAT SWITCH


const int count = 0;
bool purpleFlag = false;
bool testAlarm = false;
bool state = true;

char serverAddress[] = "192.168.0.45"; // server address
int port = 44364;

WiFiClient wifi;
HttpClient client = HttpClient(wifi, serverAddress, port);
int status = WL_IDLE_STATUS;

//
// ------------------------------------------------------------------------------------------
// Temperature 
// ------------------------------------------------------------------------------------------
//



MCP9600 Evr;
MCP9600 Wtr;
MCP9600 Blr;


//------------------------------------------------------------------------------------------


Adafruit_SSD1306 waterDsp(-1);
Adafruit_SSD1306 boilerDsp(-1);

#define OLED1 0x3C // OLED 1
#define OLED2 0x3D // OLED 2


// Prototypes
///------------------------------------------------------------------------------------------

auto coolDownCycle() -> void;
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
	Wtr.begin(0x060);
	Blr.begin(0x061);
	Evr.begin(0x067);
	
	waterDsp.begin(SSD1306_SWITCHCAPVCC, OLED1);
	waterDsp.clearDisplay();
	waterDsp.display();

	boilerDsp.begin(SSD1306_SWITCHCAPVCC, OLED2);
	boilerDsp.clearDisplay();
	boilerDsp.display();
	
	// OUTPUTS   pinModes
	// ----------------------------------------
	pinMode(processorLED, OUTPUT);
	digitalWrite(processorLED, LOW);

	// ????????????????????????
	pinMode(callForHeat, INPUT); // i PIN 4

	pinMode(PIN19, OUTPUT); // o PIN 19
	digitalWrite(PIN19, LOW);

	pinMode(speaker, OUTPUT); // o PIN 25
	digitalWrite(speaker, LOW);

	pinMode(blueRelay, OUTPUT); // o PIN 26
	digitalWrite(blueRelay, LOW);

	pinMode(yellowRelay, OUTPUT); // o PIN 27
	digitalWrite(yellowRelay, LOW);

	pinMode(purpleRelay, OUTPUT); // o PIN 18
	digitalWrite(purpleRelay, LOW);

	pinMode(PB1, OUTPUT); // o PIN 32
	digitalWrite(PB1, LOW);

	pinMode(PB2, OUTPUT); // o PIN 33
	digitalWrite(PB2, LOW);
	
	pinMode(PB3, OUTPUT); // o PIN 5
	digitalWrite(PB3, LOW);


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

	
	// Set your Static IP address
	//IPAddress local_IP(192, 168, 0, 30);
	// Set your Gateway IP address
	//IPAddress gateway(192, 168, 0, 1);
	//IPAddress subnet(255, 255, 0, 0);


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


	digitalWrite(purpleRelay, HIGH);
	delay(30000); // 30 sec pause before start
	digitalWrite(purpleRelay, LOW);
}


// Loop
//------------------------------------------------------------------------------------------
////------------------------------------------------------------------------------------------
/////------------------------------------------------------------------------------------------
/////------------------------------------------------------------------------------------------

void loop()
{	
	ArduinoOTA.handle();
	timeClient.update();


	// Code Below
	//------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------

		// startup indicator
	if(startupIndicator)
	{
		startupIndicator = false;

		for (int ndx = 0; ndx <= 5; ndx++)
		{
			ArduinoOTA.handle();
			timeClient.update();
			
			digitalWrite(purpleRelay, HIGH);
			delay(350);
			digitalWrite(purpleRelay, LOW);
			delay(350);
			
		}

	}

//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
	// MY STARTUP CODE ABOVE Code Above
	// OP Code Below
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
//
	updateDisplay();

	if (shutdown)
	{
		digitalWrite(purpleRelay, HIGH);
		delay(750);
		digitalWrite(purpleRelay, LOW);
		delay(750);

		updateDisplay();
		
	}
	else 
	{
		operationalCycle();
	}

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
		activeMode = "active";
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
	coolDownCycle();

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

	blueTemp = ((Wtr.getThermocoupleTemp() * 9 / 5) + 32);
	yellowTemp = ((Blr.getThermocoupleTemp() * 9 / 5) + 32);
	purpleTmp = ((Evr.getThermocoupleTemp() * 9 / 5) + 32);
	
	while (blueTemp < highTemperature) {

		ArduinoOTA.handle();
		timeClient.update();
		
		digitalWrite(yellowRelay, HIGH);// burner on
		digitalWrite(blueRelay, HIGH);// waterpump on

		blueTemp = ((Wtr.getThermocoupleTemp() * 9 / 5) + 32);
		
		updateDisplay();
	}
	
	ArduinoOTA.handle();
	timeClient.update();
	
	digitalWrite(yellowRelay, LOW);// burner off

	updateDisplay();
}

auto coolDownCycle() -> void
{
	ArduinoOTA.handle();
	timeClient.update();

	while (blueTemp >= lowTemperature) {

		ArduinoOTA.handle();
		timeClient.update();
		
		digitalWrite(yellowRelay, LOW);// burner off
		digitalWrite(blueRelay, HIGH);// waterpump on
		
		blueTemp = ((Wtr.getThermocoupleTemp() * 9 / 5) + 32);

		updateDisplay();
	}
}


auto updateDisplay() -> void
{

	ArduinoOTA.handle();
	timeClient.update();
	
	// (26°C × 9/5) + 32 = 78.8°F 
	waterTemp = ((Wtr.getThermocoupleTemp() * 9 / 5) + 32);
	boilerTemp = ((Blr.getThermocoupleTemp() * 9 / 5) + 32);
	otherTmp = ((Evr.getThermocoupleTemp() * 9 / 5) + 32);

	String wt = String(waterTemp);
	String bt = String(boilerTemp);
	String ev = String(otherTmp);


	

	String title = "HeatShield d1 ";
	title = title + version;

	waterDsp.setTextSize(1);
	waterDsp.clearDisplay();
	waterDsp.setTextColor(WHITE);

	waterDsp.setCursor(11, 1);
	waterDsp.println(title);

	waterDsp.setCursor(11, 11);
	waterDsp.println("w: " + wt);

	waterDsp.setCursor(50, 11);
	waterDsp.println(" b: " + bt);

	waterDsp.setCursor(22, 21);
	waterDsp.println("-> e: " + ev);

	waterDsp.display();

	// ==============================================

	title = "HeatShield d2 ";
	title = title + version;

	boilerDsp.setTextSize(1);
	boilerDsp.clearDisplay();
	boilerDsp.setTextColor(WHITE);

	boilerDsp.setCursor(11, 1);
	boilerDsp.println(title);

	boilerDsp.setCursor(11, 11);
	boilerDsp.println("w: " + wt);

	boilerDsp.setCursor(50, 11);
	boilerDsp.println(" -|-  b: " + bt);

	boilerDsp.setCursor(22, 21);
	boilerDsp.println("-> e: " + ev);

	boilerDsp.display();

	// ==============================================
	
}
