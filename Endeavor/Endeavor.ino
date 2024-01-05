
/*
 Name:    Endeavor 2324
 Created: 8/27/2022 3:36:02 PM
 Author:  david

 PROTOTYPE - CC:50:E3:80:A2:E8  192.168.0.36
 PRODUCTION - 24:6F:28:9E:9B:D0  192.168.0.37  25?

*/

#include <SparkFun_MCP9600.h>
//#include <HttpClient.h>
#include <ArduinoOTA.h>
#include <ESPmDNS.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <SPI.h>
#include <Adafruit_SSD1306.h>
#include <ArduinoHttpClient.h>




String callForHeatVar = "active";
String active = "active";
String stdby = "stdby";
String inActive = "inActive";

String Mode = stdby;
String currentFunction = "F:";


//======================================================================================
//======================================================================================
// SET TEMPERATURE VALUES
//======================================================================================
//======================================================================================

float highActiveTemperature = 155;
float lowActiveTemperature = 130;

float highStdbyTemperature = 155;
float lowStdbyTemperature = 130;

float highInactiveTemperature = 155;
float lowInactiveTemperature = 130;

float highTemperature = 155;
float lowTemperature = 130;

float tmpBlue = 0;
float tmpYellow = 0;
float tmpPurple = 0;

float waterInTemp = 0;
float waterTempPrev = 0;

float waterOutTemp = 0;
float waterOutTempPrev = 0;

float boilerTemp = 0;
float boilerTempPrev = 0;

float envTemp = 0;
float otherTempPrev = 0;


float currentWaterTemp = 0;
float blueTempPrev = 0;

float currentBoilerTemp = 0;
float yellowTempPrev = 0;

float currentEvTemp = 0;
float purpleTempPrev = 0;

String blueT = "";
String yellowT = "";
String purpleT = "";
bool coolDown = false;

String CHstatus = "-";


//------------------------------------------------------------------------------------------

const char* ssid = "Wilson.Net-2.4G";
const char* password = "wilsonwebsite.com";

const long utcOffsetInSeconds = 3600;
char daysOfTheWeek[7][12] = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);


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



const int count = 0;
bool purpleFlag = false;
bool testAlarm = false;
bool state = true;

char serverAddress[] = "192.168.0.67"; // server address
int port = 44364;

WiFiClient wifi;
HttpClient client = HttpClient(wifi, serverAddress, port);
int status = WL_IDLE_STATUS;

//
// ------------------------------------------------------------------------------------------
// Temperature 
// ------------------------------------------------------------------------------------------
//

MCP9600 evThermocouple;
MCP9600 waterInThermocouple;
MCP9600 boilerThermocouple;
MCP9600 waterOutThermocouple;


//------------------------------------------------------------------------------------------

Adafruit_SSD1306 displayTwo(-1);
Adafruit_SSD1306 displayOne(-1);

#define OLED1 0x3C // OLED 1
#define OLED2 0x3D // OLED 2



// Prototypes
///------------------------------------------------------------------------------------------
///
///

auto checkCallForHeat() -> bool;
auto operationalCycle() -> void;
auto updateDisplay() -> void;
auto coolDownCycle(float) -> void;
auto heatUpCycle(float) -> void;
auto PB1_Fired() -> void;
auto PB2_Fired() -> void;


long startUpTime = 0;

///------------------------------------------------------------------------------------------

// Setup
//------------------------------------------------------------------------------------------
//

void setup()
{

	
	displayOne.begin(SSD1306_SWITCHCAPVCC,OLED1);
	displayOne.clearDisplay();
	displayOne.display();

	displayTwo.begin(SSD1306_SWITCHCAPVCC, OLED2);
	displayTwo.clearDisplay();
	displayTwo.display();

	// Begin Thermocouples
	waterInThermocouple.begin(0x060);
	//waterOutThermocouple.begin(0x65);
	boilerThermocouple.begin(0x061);
	evThermocouple.begin(0x064);
	

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


	Serial.println("Ready");
		Serial.print("IP address: ");
		Serial.println(WiFi.localIP());

		timeClient.begin();

		// OUTPUTS   pinModes
// ----------------------------------------
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

		Mode = stdby; // starting mode
		// ----------------------------------------
		//


		startUpTime = millis();
}


// Loop
//------------------------------------------------------------------------------------------
////------------------------------------------------------------------------------------------
/////------------------------------------------------------------------------------------------
/////------------------------------------------------------------------------------------------
///
///
///






int TestMode = 1;





//variables for blinking an LED with Millis
unsigned long previous_millis = 0;  // will store last time LED was updated
const long interval = 1000;  // interval at which to blink (milliseconds)



void loop() {

	ArduinoOTA.handle();
	timeClient.update();


	const auto current_millis = millis();



	if (TestMode == 1) {
		//loop to blink without delay


		if (current_millis - previous_millis >= interval) {
			// save the last time you blinked the LED
			previous_millis = current_millis;

			// set the LED with the ledState of the variable:
			digitalWrite(processorLED, !digitalRead(processorLED));

		}


		updateDisplay();

	}
}


auto updateDisplay() -> void
{
	ArduinoOTA.handle();
	timeClient.update();


	// (26°C × 9/5) + 32 = 78.8°F 
	waterInTemp = waterInThermocouple.getThermocoupleTemp(false);
	boilerTemp = boilerThermocouple.getThermocoupleTemp(false);
	envTemp = evThermocouple.getThermocoupleTemp(false);
	waterOutTemp = 0; //sWaterOutTemp = waterOutThermocouple.getThermocoupleTemp(false);

	const String sWaterInTemp = String(waterInTemp);
	const String sBoilerTemp = String(boilerTemp);
	const String sEVTemp = String(envTemp);
	const String sWaterOutTemp = String(waterOutTemp);

	
	// ==============================================

	displayOne.setTextSize(1);
	displayOne.clearDisplay();
	displayOne.setTextColor(WHITE);

	displayOne.setCursor(11, 1);
	displayOne.println("UP: "  + String(int((millis() - startUpTime) / 1000)) );

	displayOne.setCursor(11, 11);
	displayOne.println("BLR->" + sBoilerTemp);

	displayOne.setCursor(11, 22);
	displayOne.println("WIT->" + sWaterInTemp);

	displayOne.display();

	// ==============================================	

	displayTwo.setTextSize(1);
	displayTwo.clearDisplay();
	displayTwo.setTextColor(WHITE);

	displayTwo.setCursor(11, 1);
	displayTwo.println(" step here? ");

	displayTwo.setCursor(11, 11);
	displayTwo.println("WOT->" + sWaterOutTemp);

	displayTwo.setCursor(11, 22);
	displayTwo.println("ENV->" + sEVTemp);

	displayTwo.display();
	// ==============================================
}



// Functions
//------------------------------------------------------------------------------------------


auto operationalCycle() -> void
{
	ArduinoOTA.handle();
	timeClient.update();

	currentFunction = "op";

	updateDisplay();
	checkCallForHeat();

	if (Mode == active) heatUpCycle(highActiveTemperature);
	if (Mode == stdby) heatUpCycle(highStdbyTemperature);
	if (Mode == inActive) heatUpCycle(highInactiveTemperature);

	updateDisplay();

}

// -------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------
// Methods
// ------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------



auto heatUpCycle(float tHigh) -> void
{

	ArduinoOTA.handle();
	timeClient.update();

	currentFunction = "heat";
	highTemperature = tHigh;

	currentWaterTemp = waterInThermocouple.getThermocoupleTemp(false);
	currentBoilerTemp = boilerThermocouple.getThermocoupleTemp(false);
	currentEvTemp = evThermocouple.getThermocoupleTemp(false);

	while (currentWaterTemp <= tHigh && !coolDown)
	{
		ArduinoOTA.handle();
		timeClient.update();

		digitalWrite(waterLowRelay, HIGH);
		digitalWrite(burnerRelay, HIGH); // burner on
		digitalWrite(waterRelay, HIGH); // waterpump on

		currentWaterTemp = waterInThermocouple.getThermocoupleTemp(false);
		updateDisplay();
	}

	digitalWrite(waterLowRelay, LOW);
	digitalWrite(burnerRelay, LOW); // burner off
	digitalWrite(waterRelay, LOW); // waterpump off

	updateDisplay();

	if (Mode == active) coolDownCycle(lowActiveTemperature);
	if (Mode == stdby) coolDownCycle(lowStdbyTemperature);
	if (Mode == inActive) coolDownCycle(lowInactiveTemperature);

}

auto coolDownCycle(float tLow) -> void
{
	ArduinoOTA.handle();
	timeClient.update();

	coolDown = true;
	currentFunction = "cool";
	lowTemperature = tLow;

	currentWaterTemp = waterInThermocouple.getThermocoupleTemp(false);
	currentBoilerTemp = boilerThermocouple.getThermocoupleTemp(false);
	currentEvTemp = evThermocouple.getThermocoupleTemp(false);

	while (currentWaterTemp >= tLow)
	{
		ArduinoOTA.handle();
		timeClient.update();

		digitalWrite(waterLowRelay, LOW);
		digitalWrite(burnerRelay, LOW); // burner off
		digitalWrite(waterRelay, LOW); // waterpump off

		currentWaterTemp = waterInThermocouple.getThermocoupleTemp(false);
		updateDisplay();
	}
	coolDown = false;
}



auto checkCallForHeat() -> bool
{
	ArduinoOTA.handle();
	timeClient.update();


	if (digitalRead(callForHeat) == HIGH)
	{
		CHstatus = "+";
		Mode = active;
		return true;
	}
	CHstatus = "-";
	Mode = stdby;
	return false;
}

auto PB1_Fired() -> void
{
	digitalWrite(burnerRelay, HIGH);
	digitalWrite(waterLowRelay, HIGH);
}

auto PB2_Fired() -> void
{
	digitalWrite(burnerRelay, LOW);
	digitalWrite(waterLowRelay, LOW);
}
