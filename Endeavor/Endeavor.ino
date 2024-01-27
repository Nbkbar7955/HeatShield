
/*
 Name:    Endeavor 2324
 Created: 8/27/2022 3:36:02 PM
 Author:  david

Blah Blah Blah...

*/

#include <Arduino_JSON.h>
#include <SparkFun_MCP9600.h>
#include <ArduinoOTA.h>
#include <ESPmDNS.h>
#include <SPI.h>
#include <Adafruit_SSD1306.h>
#include <ArduinoHttpClient.h>
#include <WiFi.h>
//#include <Preferences.h>


#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64



//======================================================================================
//======================================================================================
// Variables
//======================================================================================
//======================================================================================

float BTempHigh = 900;
float BTempLow = 400;

float ITempHigh = 180;
float ITempLow = 130;

float ETempHigh = 72;
float ETempLow = 66;

float OTempHigh = 200;
float OTempLow = 100;

long waterRunTime = 240000;
int averageNumber = 10;

bool burnerLit = true;
bool flameOut = false;





//======================================================================================
//======================================================================================
// WiFi 
//======================================================================================
//======================================================================================

const char* ssid = "Wilson.Net-2.4G";
const char* password = "wilsonwebsite.com";

char serverAddress[] = "192.168.0.67"; // server address
int port = 44364;

WiFiClient wifi;
HttpClient client = HttpClient(wifi, serverAddress, port);
int status = WL_IDLE_STATUS;

//======================================================================================
//======================================================================================
// Pin Definitions
//======================================================================================
//======================================================================================
//

const int processorLED = 2; //o LED on MicroProcessor
const int callForHeat = 4; //i CALLFORHEAT
const int testPin = 5; //test mode

// SPI
const int misoSPI = 12;
const int mosiSPI = 13;
const int clkSPI = 14;
const int ssSPI = 15;


const int waterRelay = 17; //o WATERPUMP RELAY
const int burnerRelay = 16; //o BURNER RELAY
const int waterLowRelay = 18; //o
const int testRelay = 19;

const int speaker = 32; //o SOUNDALARM
const int PB1 = 34; //i PB1
const int PB2 = 35; //i PB2
const int PB3 = 36; //i PB3 ??????
const int PB4 = 39; //i PB4 ?????



//======================================================================================
//======================================================================================
// Thermocouple Definitions
//======================================================================================
//======================================================================================
//

MCP9600 EThermocouple;  //64 pink
MCP9600 IThermocouple; //61 blue
MCP9600 BThermocouple; //60 yellow
MCP9600 OThermocouple; //64 white


//======================================================================================
//======================================================================================
// OLED Definitions
//======================================================================================
//======================================================================================
//

Adafruit_SSD1306 displayTwo(-1);
Adafruit_SSD1306 displayOne(-1);

#define OLED1 0x3C // OLED 1
#define OLED2 0x3D // OLED 2

#define OLED3 0x3C // OLED 3
#define OLED4 0x3D // OLED 4


//======================================================================================
//======================================================================================
// Prototypes
//======================================================================================
//======================================================================================
//

auto safetyCheck(bool) -> bool;
auto testCycle() -> bool;
auto opCycle(void) -> void;
auto runHeatCycle() -> bool;
auto stopHeatCycle() -> bool;
auto startBurnCycle(int) -> bool;
auto startBurnCycle(void) -> bool;
auto stopBurnCycle(int) -> bool;
auto stopBurnCycle(void) -> bool;
auto waterCycle(int) -> bool;
auto waterCycle(void) -> bool;
auto ETemp(void) -> float;
auto ITemp(void) -> float;
auto OTemp(void) -> float;
auto BTemp(void) -> float;
auto updateDisplay() -> void;
auto saveState() -> bool;
auto restoreState() -> bool;
auto saveConfig() -> bool;
auto restoreConfig() -> bool;
auto commCycle() -> bool;
auto threadCycle() -> bool;

void myTests();


//======================================================================================
//======================================================================================
// Temporary Definitions
//======================================================================================
//======================================================================================
//

bool TestMode = true;
long startUpTime = 0;


unsigned long currentBlinkMillis = 0;
unsigned long previousBlinkMillis = 0;

unsigned long previousRelayMillis = 0;
unsigned long currentRelayMillis = 0;

const long blinkInterval = 250;
const long relayInterval = 750;

int nextRelay = 1;



//======================================================================================
//======================================================================================
// Setup
//======================================================================================
//======================================================================================
//

void setup()
{


	//======================================================================================
	// Temporary Var Inits
	//======================================================================================	
	
	startUpTime = millis();

	
	//======================================================================================
	// Display Init
	//======================================================================================

	
	displayOne.begin(SSD1306_SWITCHCAPVCC, OLED1);
	displayOne.clearDisplay();
	displayOne.display();

	displayTwo.begin(SSD1306_SWITCHCAPVCC, OLED2);
	displayTwo.clearDisplay();
	displayTwo.display();

	//======================================================================================
	// Thermocouple Init
	//======================================================================================
	
	BThermocouple.begin(0x060); // yellow
	IThermocouple.begin(0x061);   // blue  
	OThermocouple.begin(0x62); // white
	EThermocouple.begin(0x064); // pink


	//======================================================================================
	// Serial
	//======================================================================================

	Serial.begin(115200);
	Serial.println("Booting");


	//======================================================================================
	// WiFi
	//======================================================================================

	WiFi.mode(WIFI_STA);
	WiFi.begin(ssid, password);

	while (WiFi.waitForConnectResult() != WL_CONNECTED)
	{
		Serial.println("Connection Failed! Rebooting...");
		delay(5000);
		ESP.restart();
	}



	//======================================================================================
	// OTA
	//======================================================================================
	
	// Port defaults to 3232
	ArduinoOTA.setPort(3232);
	ArduinoOTA.setHostname("Endeavor");



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

					//======================================================================================
					// Serial
					//======================================================================================

					Serial.println("Ready");
					Serial.print("IP address: ");
					Serial.println(WiFi.localIP());


	//======================================================================================
	// PinModes
	//======================================================================================

	pinMode(processorLED, OUTPUT);
	digitalWrite(processorLED, LOW);

	pinMode(callForHeat, OUTPUT); // i PIN 4
	digitalWrite(callForHeat, LOW);

	pinMode(speaker, OUTPUT); // o PIN 25
	digitalWrite(speaker, LOW);


	pinMode(waterRelay, OUTPUT); // o PIN 26
	digitalWrite(waterRelay, HIGH);

	pinMode(burnerRelay, OUTPUT); // o PIN 27
	digitalWrite(burnerRelay, HIGH);

	pinMode(testRelay, OUTPUT);
	digitalWrite(testRelay, HIGH);

	pinMode(waterLowRelay, OUTPUT); // o PIN 18
	digitalWrite(waterLowRelay, HIGH);

	pinMode(PB1, INPUT); // i PIN 34
	pinMode(PB1, INPUT_PULLDOWN);
	digitalWrite(PB1, LOW);

	pinMode(PB2, INPUT); // i PIN 35
	pinMode(PB2, INPUT_PULLDOWN);
	digitalWrite(PB2, LOW);

	pinMode(PB3, INPUT); // i PIN 36
	pinMode(PB3, INPUT_PULLDOWN);
	digitalWrite(PB3, LOW);

	pinMode(PB4, INPUT); // i PIN 39
	pinMode(PB4, INPUT_PULLDOWN);
	digitalWrite(PB4, LOW);

	restoreConfig();
	restoreState();


}


//======================================================================================
//======================================================================================
// Loop
//======================================================================================
//======================================================================================
//


void loop() {

	ArduinoOTA.handle();
	currentBlinkMillis = millis();

	if (currentBlinkMillis - previousBlinkMillis >= blinkInterval) {
		previousBlinkMillis = currentBlinkMillis;

		digitalWrite(processorLED, !digitalRead(processorLED));
	}

	if (TestMode) {
		while(testCycle()) { ArduinoOTA.handle(); }
	}

	
	saveConfig();
	saveState();
	threadCycle();
	commCycle();
	safetyCheck(flameOut);
	opCycle();
}


auto safetyCheck(bool) -> bool
{
	return true;
}

auto testCycle() -> bool
{

	ArduinoOTA.handle();
	currentRelayMillis = millis();

	// If time for next relay
	if (currentRelayMillis - previousRelayMillis >= relayInterval) {
		// save the last time you blinked the LED
		previousRelayMillis = currentRelayMillis;

		updateDisplay();

		switch (nextRelay) {

		case 1:
			digitalWrite(burnerRelay, LOW);
			digitalWrite(waterRelay, HIGH);
			digitalWrite(waterLowRelay, HIGH);
			digitalWrite(testRelay, HIGH);
			break;
		case 2:
			digitalWrite(burnerRelay, HIGH);
			digitalWrite(waterRelay, LOW);
			digitalWrite(waterLowRelay, HIGH);
			digitalWrite(testRelay, HIGH);
			break;
		case 3:
			digitalWrite(burnerRelay, HIGH);
			digitalWrite(waterRelay, HIGH);
			digitalWrite(waterLowRelay, LOW);
			digitalWrite(testRelay, HIGH);
			break;
		case 4:
			digitalWrite(burnerRelay, HIGH);
			digitalWrite(waterRelay, HIGH);
			digitalWrite(waterLowRelay, HIGH);
			digitalWrite(testRelay, LOW);
			//digitalWrite(speaker, HIGH);
			break;
		default:
			digitalWrite(burnerRelay, LOW);
			digitalWrite(waterRelay, LOW);
			digitalWrite(waterLowRelay, LOW);
			digitalWrite(testRelay, LOW);
			break;
		}

		nextRelay++;
		if (nextRelay >= 4) nextRelay = 1;
	}
	return true;
}

void opCycle()
{
	ArduinoOTA.handle();

	const auto currentETemp = ETemp();

	if (currentETemp < ETempLow) runHeatCycle();
	else if (currentETemp >= ETempHigh) stopHeatCycle();
	
}

auto runHeatCycle() -> bool
{
	ArduinoOTA.handle();

	float currentBTemp = BTemp();
	float currentETemp = ETemp();

	while (currentETemp <= ETempLow)
	{
		ArduinoOTA.handle();

		while (currentBTemp < BTempHigh)
		{
			ArduinoOTA.handle();
			startBurnCycle();
			currentBTemp = BTemp();
		}
		stopBurnCycle();

		
		waterCycle();
		currentETemp = ETemp();
	}

	return true;
}

auto stopHeatCycle() -> bool
{
	ArduinoOTA.handle();

	digitalWrite(burnerRelay, LOW);
	digitalWrite(waterRelay, LOW);
	
	return true;
}

bool startBurnCycle(int)
{
	ArduinoOTA.handle();

	// Put inside boiler routines here
	
	digitalWrite(burnerRelay, HIGH);
	
	return false;
}

auto startBurnCycle() -> bool
{
	ArduinoOTA.handle();
	return startBurnCycle(0);
}

auto stopBurnCycle(int) -> bool
{
	ArduinoOTA.handle();
	digitalWrite(burnerRelay, LOW);
	return true;
}

auto stopBurnCycle() -> bool
{
	ArduinoOTA.handle();
	return stopBurnCycle(0);
}

bool waterCycle(int)
{
	ArduinoOTA.handle();

	digitalWrite(waterRelay, HIGH);
	delay(waterRunTime);
	
	return false;
}

auto waterCycle() -> bool
{
	ArduinoOTA.handle();
	return waterCycle(0);
}


float ETemp()
{
	ArduinoOTA.handle();
	if (!EThermocouple.isConnected()) return(0);
	if (!EThermocouple.available()) return(0);

	float tempValue = 0;
	
	for (int numberTimes = 0; numberTimes <= averageNumber; numberTimes++)
	{
		ArduinoOTA.handle();
		tempValue += EThermocouple.getThermocoupleTemp(false);
	}
	return(tempValue / averageNumber);


}

float ITemp()
{
	ArduinoOTA.handle();
	if (!IThermocouple.isConnected()) return(0);
	if (!IThermocouple.available()) return(0);

	
	float tempValue = 0;

	for (int numberTimes = 0; numberTimes <= averageNumber; numberTimes++)
	{
		ArduinoOTA.handle();
		tempValue += IThermocouple.getThermocoupleTemp(false);
	}
	return(tempValue / averageNumber);

}

float OTemp()
{
	ArduinoOTA.handle();
	if (!OThermocouple.isConnected()) return(0);
	if (!OThermocouple.available()) return(0);
	
	float tempValue = 0;

	for (int numberTimes = 0; numberTimes <= averageNumber; numberTimes++)
	{
		ArduinoOTA.handle();
		tempValue += OThermocouple.getThermocoupleTemp(false);
	}
	return(tempValue / averageNumber);
}

float BTemp()
{
	ArduinoOTA.handle();
	if (!BThermocouple.isConnected()) return(0);
	if (!BThermocouple.available()) return(0);

	float tempValue = 0;

	for (int numberTimes = 0; numberTimes <= averageNumber; numberTimes++)
	{
		ArduinoOTA.handle();
		tempValue += BThermocouple.getThermocoupleTemp(false);
	}
	return(tempValue / averageNumber);
}

auto updateDisplay() -> void {

	ArduinoOTA.handle();

	displayOne.clearDisplay();
	displayOne.setTextSize(1);
	displayOne.setTextColor(WHITE);

	displayOne.setCursor(11, 1);
	displayOne.println("UP: " + String(int((millis() - startUpTime) / 1000)));

	displayOne.setCursor(11, 11);
	displayOne.println("B:Y:0: " + String(BTemp()));

	displayOne.setCursor(11, 22);
	displayOne.println("I:B:1: " + String(ITemp()));

	displayOne.display();


	displayTwo.clearDisplay();
	displayTwo.setTextSize(1);
	displayTwo.setTextColor(WHITE);

	displayTwo.setCursor(11, 1);
	displayTwo.println("Relay:" + String(nextRelay - 1));

	displayTwo.setCursor(11, 11);
	displayTwo.println("O:W:4: " + String(OTemp()));

	displayTwo.setCursor(11, 22);
	displayTwo.println("E:P:5: " + String(ETemp()));

	displayTwo.display();

}

bool saveState()
{
	return true;
}

bool restoreState()
{
	return true;
}

bool saveConfig()
{
	return true;
}

bool restoreConfig()
{
	return true;
}

bool commCycle()
{
	return true;
}

bool threadCycle()
{
	return true;
}

void myTests()
{

	const char input[] = "{\"result\":true,\"count\":42,\"foo\":\"bar\"}";

	JSONVar myObject = JSON.parse(input);

	

	
	

	
}













