

/*
 Name:		Endeavor 2425
 Created:	08/27/2022 3:36:02 PM
 Author:	David Wilson
 version:	0.0.0

 Changes:	11/29/2024 00:30:00
			11/28/2024 15:00.00
			12/06/2024 02:28.00
			12/11/2024
			12/15/2024 23:00
			12/16/2024 22:00
			12/16/2024 23:00
			12/18/2024 20:00
			12/19/2024 00:30
			12/20/2024 23:30
			12/21/2024 13:00
			12/21/2024 13:30
			12/22/2024 14:00
			12/23/2024 12:30
			01/05/2025 18:25
			01/05/2025 18:36
			01/08/2025 19:30
			01/14/2025 19:00
			01/14/2025 19:30
			01/14/2025 21:23
			01/14/2025 22:22
			01/17/2025 16:05
			01/17/2025 19:41
			01/17/2025 21:33
			01/17/2025 22:21
			01/19/2025 19:48
			01/19/2025 21:53 - push fm IRONMAN
			01/19/2025 22:40 - broke
			01/19/2025 23:59 - try fix
			01/20/2025 00:06 - push
			01/21/2025 13:52 - still rebooting n locking
			01/21/2025 15:30 - fix was hitting I2C buss to hard and TC(s) too fast/often
			01/22/2025 00:20 - chg vars
			01/22/2025 00:48 - remove breaks
			01/27/2025 02:30 - CHG PARMS
			01/30/2025 02:04 - chg parms
			01/31/2025 17:02 - chg
			02/19/2025 15:48 - chgs
							 - add delay to calc BoilreTemp()
							 - add *TempOffSet
							 - cleanup unused vars
							 - cleanup function prototypes
							 - commit
			02/19/2025 18:27 - compile error. fix was clean solution

			







*/


//======================================================================================
//======================================================================================
// Variables Used Here for Convenience
//======================================================================================
//======================================================================================

const char* hostName = "ENDEAVOR_12";
uint8_t ON = 0x0;
uint8_t OFF = 0x1;

//======================================================================================
//======================================================================================
// Include files
//======================================================================================
//======================================================================================


#include <Adafruit_SSD1306.h>
#include <ArduinoHttpClient.h>
#include <ArduinoOTA.h>
#include <ESPmDNS.h>
#include <SparkFun_MCP9600.h>
#include <SPI.h>
#include <WiFi.h>
#include "../../../../AppData/Local/arduino15/packages/esp32/hardware/esp32/3.0.7/libraries/Update/src/Update.h"




//======================================================================================
//======================================================================================
// Working Variables
//======================================================================================
//======================================================================================
//======================================================================================
//======================================================================================
//======================================================================================
//======================================================================================
//======================================================================================
//======================================================================================
//======================================================================================
//======================================================================================
//======================================================================================
//======================================================================================
//======================================================================================




bool callForHeatActive = false; // will be coded aft thermost installed

int MAX_WATER_TEMP = 175; // 175 MAX Wtr temp. Shutdown if met or exceeded
int MIN_WATER_TEMP = 100; // 100 MIN +/- 1 if not met then heat back up

int envHighTemp = 70; // 70 current Hi for LR temp
int envHighOffSet = 0; // 0 used to adjust theermocouple readi

int envLowTemp = 67; // 67 current Lo for LR kick on at this var
int envLowOffSet = 0; // 0 offset for testing

int boilerHighTemp = 950; // 950 top temp for boiler
int boilerHighTempOffSet = 0;

int boilerLowTemp = 400; // 400 bottom temp for boiler
int boilerLowTempOffSet = 0;

int waterHighTemp = 140; // 140 hi water stop heating water. start pumping
int waterHighTempOffSet = 0;

int waterLowTemp = 130; // 130 lo temp. stop pumping and heat water
int waterLowTempOffSet = 0;

int callForHeatWaterTmp = 120; // 120 water pause
int callForHeatWaterTmpOffSet = 0;

int currentBoilerTemp = 0; // global boiler temp updated by runMaintxx
int currentBoilerTempOffSet = 0;

int currentWaterTemp = 0; // global water temp updated by runMaintxx
int currentWaterTempOffSet = 0;

int currentEnvTemp = 0; // global Env temp updated by runMaintxx
int currentEnvTempOffSet = 0;

int spinner = 0; // for spin


///////////////////////////////////////////////////////////////////////////

String timeString = " XX:XX:XX ";
String boilerStatus = "B- ";
String waterStatus = "W- ";
String valveStatus = "V- ";
String callForHeatStatus = "H- ";
String flameStatus = "F- ";

// 30,000 = 30 seconds
// 60,000 = 1 min
// 120,000 = 2 min
// 180,000 = 3 min
// 240,000 = 4 mins
// 300,000 = 5 mins
// 600,000 = 10 mins


unsigned long blinkInterval = 250;// 250 blink
unsigned long savedBlinkTime = 0; //blink begining

unsigned long burnTime = 0; // calculate how long we've burned
long startUpTime = 0; // 0 for blink()


//======================================================================================
//======================================================================================
// WiFi bef setup
//======================================================================================
//======================================================================================
const char* networkName = "Wilson.Net-2.4G";
const char* networkNamePassPhrase = "wilsonwebsite.com";

char serverAddress[] = "192.168.0.67"; // server address
uint16_t port = 44364;

WiFiClient wifi;
HttpClient client = HttpClient(wifi, serverAddress, port);
int status = WL_IDLE_STATUS;

//======================================================================================
//======================================================================================
// Pin Definitions
//======================================================================================
//======================================================================================
//

// misc
const int processorLED = 2; //o 2 LED on MicroProcessor
const int callForHeatPin = 4; //i 4 CALLFORHEAT
const int flameOut = 5; //5 flame out
// SPI
const int misoSpi = 12;// 12 MISO
const int mosiSpi = 13;// 13 MOSI
const int clkSpi = 14;// 14 CLK
const int ssSpi = 15;// 15 SS
// Relays
const int waterRelay = 17; //o 17 WATERPUMP RELAY
const int zoneTwoRelay = 18; //o 18 2nd floor
const int standbyRelay = 19; //o 19
const int burnerRelay = 16; //o 16 BURNER RELAY
const int yellowRelay = 34; //o 34 testing BURNER RELAY
// others
const int speaker = 32; //o 32 SOUNDALARM
const int PB1 = 34; //i 34 PB1
const int PB2 = 35; //i 35 PB2
const int PB3 = 36; //i 36 PB3 
const int PB4 = 39; //i 39 PB4 


//======================================================================================
//======================================================================================
// Thermocouple Definitions
//======================================================================================
//======================================================================================
//

MCP9600 envTC;  //64 green
MCP9600 waterTC; //65 blue
MCP9600 boilerTC; //60 yellow

//MCP9600 yellowTC; //65 yellow


//
//======================================================================================
//======================================================================================
// Display Definitions
//======================================================================================
//======================================================================================
//

Adafruit_SSD1306 displayOne(-1);
//Adafruit_SSD1306 displayTwo(-1);
//Adafruit_SSD1306 displayThree(-1);
//Adafruit_SSD1306 displayFour(-1);



#define OLED1 0x3C // OLED 1
#define OLED2 0x3D // OLED 2



//#define OLED3 = 0x3C // OLED 3
//#define	OLED4 = 0x3D // OLED 4


String displayOneLineOne = "x";
String displayOneLineTwo = "x";
String displayOneLineThree = "x";

//String displayTwoLineOne = "x";
//String displayTwoLineTwo = "x";
//String displayTwoLineThree = "x";
/*
String displayThreeLineOne = "x";
String displayThreeLineTwo = "x";
String displayThreeLineThree = "x";

String displayFourLineOne = "x";
String displayFourLineTwo = "x";
String displayFourLineThree = "x";
*/


//======================================================================================
//======================================================================================
// Function Prototypes
//======================================================================================
//======================================================================================


void disableEndeavor();
void safetyCheck();
void updateBurnTime(void);
void updateDisplay();
void turnOnBoiler(void);
void turnOffBoiler(void);
void blink();
void runMaintenance();
void turnOffWater();
void turnOnWater();
void turnOnValve();
void turnOffValve();
void mySystemRun();
void waterRun();

int calcBoilerTemp(); // func to calc avg blrTemp
int calcWaterTemp();
int calcEnvTemp();

bool testCycle();
bool isCallForHeat();
bool isFlameOut();

String spin();



//
// writing prefs
// need #include <Preferences.h>
//Preferences preferences;

//======================================================================================
//======================================================================================
// Setup
//======================================================================================
//======================================================================================
//

void setup()
{


	//======================================================================================
	// time  Var Inits
	//======================================================================================	

	startUpTime = millis(); // blink()

	//======================================================================================
	// Display Init
	//======================================================================================

	displayOne.begin(SSD1306_SWITCHCAPVCC, OLED1);
	displayOne.clearDisplay();
	displayOne.display();

	//displayTwo.begin(SSD1306_SWITCHCAPVCC, OLED2);
	//displayTwo.clearDisplay();
	//displayTwo.display();

	//======================================================================================
	// Thermocouple Init
	//======================================================================================

	waterTC.begin(0x065);   // 65 blue water  
	boilerTC.begin(0x60); // 60// yellow boiler
	envTC.begin(0x064); // 64 bare Env


	// ************************************
	//pinMode(yellowRelay, OUTPUT); // o PIN 27
	//digitalWrite(yellowRelay, LOW);
	// ************************************

	pinMode(standbyRelay, OUTPUT);
	digitalWrite(standbyRelay, OFF);

	pinMode(zoneTwoRelay, OUTPUT); // o PIN 18
	digitalWrite(zoneTwoRelay, OFF);

	pinMode(PB1, INPUT); // i PIN 34
	pinMode(PB1, INPUT_PULLDOWN);
	digitalWrite(PB1, OFF);

	pinMode(PB2, INPUT); // i PIN 35
	pinMode(PB2, INPUT_PULLDOWN);
	digitalWrite(PB2, OFF);

	pinMode(PB3, INPUT); // i PIN 36
	pinMode(PB3, INPUT_PULLDOWN);
	digitalWrite(PB3, OFF);

	pinMode(PB4, INPUT); // i PIN 39
	pinMode(PB4, INPUT_PULLDOWN);
	digitalWrite(PB4, OFF);

	pinMode(processorLED, OUTPUT);
	digitalWrite(processorLED, OFF);

	pinMode(callForHeatPin, INPUT); // i PIN 4
	pinMode(callForHeatPin, INPUT_PULLDOWN);
	digitalWrite(callForHeatPin, OFF);

	pinMode(speaker, OUTPUT); // o PIN 25
	digitalWrite(speaker, OFF);

	pinMode(waterRelay, OUTPUT); // o PIN 26
	digitalWrite(waterRelay, OFF);

	pinMode(burnerRelay, OUTPUT); // o PIN 27
	digitalWrite(burnerRelay, OFF);


	//======================================================================================
	//======================================================================================
	//	OTA Definition / Setup
	//======================================================================================
	//======================================================================================
	Serial.begin(115200);
	Serial.println("Booting");

	//======================================================================================
	// WiFi

	WiFi.mode(WIFI_STA);
	WiFi.begin(networkName, networkNamePassPhrase);

	while (WiFi.waitForConnectResult() != WL_CONNECTED)
	{
		Serial.println("Connection Failed! Rebooting...");
		delay(5000);
		ESP.restart();
	}


	// Port defaults to 3232
	ArduinoOTA.setPort(3232);
	ArduinoOTA.setHostname(hostName);

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
	Serial.print("MAC: ");
	Serial.println(WiFi.macAddress());


}

//======================================================================================
//======================================================================================
//======================================================================================
//======================================================================================
// Loop
//======================================================================================
//======================================================================================
//======================================================================================
//======================================================================================
//======================================================================================
//======================================================================================
//======================================================================================
//======================================================================================
//======================================================================================
//======================================================================================
//======================================================================================
//======================================================================================
//======================================================================================
//======================================================================================
// Loop
//======================================================================================
//======================================================================================
//======================================================================================
//======================================================================================
//======================================================================================
//======================================================================================
//======================================================================================
//======================================================================================
// Loop 
//======================================================================================
//======================================================================================
//======================================================================================
//======================================================================================
//======================================================================================
//======================================================================================
//======================================================================================
//======================================================================================
//======================================================================================
//======================================================================================
//======================================================================================
//======================================================================================
//======================================================================================
//======================================================================================



bool TestMode = false;

void loop()
{

	turnOnValve();

	runMaintenance();
	updateDisplay();

	if (TestMode) testCycle();

	mySystemRun();
}


void mySystemRun()
{
	runMaintenance();
	updateDisplay();

	while (currentWaterTemp <= waterHighTemp) // heating up water
	{
		runMaintenance();
		updateDisplay();

		// HEAT boiler
		while (currentBoilerTemp < boilerHighTemp) // heating up boiler
		{
			runMaintenance();

			boilerStatus = "B^ ";
			updateDisplay();

			turnOnBoiler();
		}
		turnOffBoiler();

		// COOL boiler
		while (currentBoilerTemp >= boilerLowTemp) // cooling down
		{
			runMaintenance();

			boilerStatus = "B` ";
			updateDisplay();

			turnOffBoiler();
		}
		turnOffBoiler();
	}
	turnOffBoiler();
	boilerStatus = "B- ";
}

bool isCallForHeat()
{
	ArduinoOTA.handle();

	//callForHeatActive = digitalRead(callForHeatPin);

	if (callForHeatActive) if (currentEnvTemp < envHighTemp) callForHeatActive = true;
		else callForHeatActive = false;
	else if (currentEnvTemp <= envLowTemp) callForHeatActive = true;
		else callForHeatActive = false;
	if (callForHeatActive) callForHeatStatus = "H+ ";
	else callForHeatStatus = "H- ";

	return callForHeatActive;
}

void turnOnBoiler()
{
	digitalWrite(burnerRelay, ON);
	//boilerStatus = "B+ ";
	safetyCheck();

	isFlameOut();
	//updateBurnTime();
}

void turnOffBoiler()
{

	digitalWrite(burnerRelay, OFF);
	//boilerStatus = "B- ";

	isFlameOut();
	//updateBurnTime();

}

void turnOnValve()
{
	digitalWrite(zoneTwoRelay, ON);
	valveStatus = "V+ ";
}

void turnOffValve()
{
	digitalWrite(zoneTwoRelay, OFF);
	valveStatus = "V- ";
}

void turnOnWater()
{ 
	digitalWrite(waterRelay, ON);
	waterStatus = "W+ ";
}

void turnOffWater() {

	digitalWrite(waterRelay, OFF);
	waterStatus = "W- ";
}


void runMaintenance()
{
	ArduinoOTA.handle();
	blink();

	currentBoilerTemp = calcBoilerTemp();
	currentWaterTemp = calcWaterTemp();
	currentEnvTemp = calcEnvTemp();

	callForHeatActive = isCallForHeat();


	if (callForHeatActive) turnOnWater();
	else turnOffWater();

	safetyCheck();
	delay(10);

}

void updateDisplay()
{
	ArduinoOTA.handle();

	displayOneLineOne = spin() + " " + timeString + " "; // +String((millis() - startUpTime) / 1000);
	displayOneLineTwo = "B:" + String(currentBoilerTemp) + " W:" + String(currentWaterTemp) + " E:" + String(currentEnvTemp);
	displayOneLineThree = callForHeatStatus + boilerStatus + waterStatus + valveStatus + flameStatus;

	// Display 1

	displayOne.clearDisplay();
	displayOne.setTextColor(1);
	displayOne.setTextSize(1);
	displayOne.setTextColor(WHITE);

	displayOne.setCursor(11, 1);
	displayOne.println(displayOneLineOne);

	displayOne.setCursor(11, 11);
	displayOne.println(displayOneLineTwo);

	displayOne.setCursor(11, 22);
	displayOne.println(displayOneLineThree);

	displayOne.display();

}

void blink()
{
	ArduinoOTA.handle();

	unsigned long currentBlinkTime = millis();

	if (currentBlinkTime - savedBlinkTime >= blinkInterval) {
		savedBlinkTime = currentBlinkTime;
		digitalWrite(processorLED, !digitalRead(processorLED));
	}
}

String spin() {

	ArduinoOTA.handle();

	spinner++;
	if (spinner > 4) spinner = 1;

	String one = "|";
	String two = "/";
	String three = "-";
	String four = "\\";

	switch (spinner)
	{
	case 1:
		return one;
	case 2:
		return two;
	case 3:
		return three;
	case 4:
		return four;
	default:
		return one;
	}
}

int calcBoilerTemp()
{
	ArduinoOTA.handle();

	delay(20);
	return (int)boilerTC.getThermocoupleTemp(false);
}

int calcWaterTemp()
{
	ArduinoOTA.handle();

	delay(20);
	return (int)waterTC.getThermocoupleTemp(false);

}

int calcEnvTemp()
{
	ArduinoOTA.handle();

	delay(20);
	return (int)envTC.getThermocoupleTemp(false);
}

void safetyCheck()
{
	ArduinoOTA.handle();

	if (currentWaterTemp >= MAX_WATER_TEMP) disableEndeavor();

}

void disableEndeavor() {  // NOLINT(clang-diagnostic-missing-noreturn)

	runMaintenance();
	updateDisplay();

	while (true)
	{
		runMaintenance();

		timeString = " !!.!!.!! ";
		updateDisplay();


		digitalWrite(burnerRelay, OFF);
		digitalWrite(zoneTwoRelay, ON);
		digitalWrite(waterRelay, ON);

		blinkInterval = 100;
		blink();
	}
}


bool isFlameOut()
{
	runMaintenance();
	updateDisplay();

	if (digitalRead(flameOut)) {
		flameStatus = "F+ ";
		return false;
	}
	else {
		flameStatus = "F- ";
		return true;
	}

}
void updateBurnTime()
{

}

void waterRun()
{
	while (callForHeatActive && (currentWaterTemp >= callForHeatWaterTmp))
	{
		runMaintenance();
		updateDisplay();

		turnOnWater();
	}


	while (callForHeatActive && (currentWaterTemp <= waterLowTemp))
	{

		runMaintenance();
		updateDisplay();

		turnOffWater();
	}

	turnOffWater();
}







// ----------------------------------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------------------------


// ---------------------------------------------------------------------------------------------------------------------


unsigned long savedCycle = 0;
unsigned long cycleInterval = 1500;

bool testCycle() // BOOKMARK
{

	runMaintenance();
	updateDisplay();

	turnOffBoiler();
	turnOffWater();

	return true;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


/*
void opCycle()
{
	runMaintenance();
	updateDisplay();

	mySystemRun();
}




String getStatus()
{
	runMaintenance();
	updateDisplay();

	// condition ? expression1 : expression2;

	const auto boilerStat = boilerStatus;
	const auto  waterStat = waterStatus;
	const auto  zoneTwoStat = valveStatus;
	const auto callForHeatTrigger = callForHeatStatus;

	return boilerStat + waterStat + zoneTwoStat + callForHeatTrigger;
}

bool isWaterTempLow() {
	runMaintenance();
	updateDisplay();

	if (currentWaterTemp <= MIN_WATER_TEMP) return true;
	return false;

}

void runSingleHeatCycle(int setPoint) {
	runMaintenance();
	updateDisplay();

	if (setPoint <= 0) setPoint = waterLowTemp;
	if (currentWaterTemp >= setPoint) return;

	// Start from the beginning
	turnOffWater();
	turnOffBoiler();

	// let's start
	while (currentWaterTemp <= setPoint)
	{

		runMaintenance();
		updateDisplay();

		// heat up
		while (currentBoilerTemp <= boilerHighTemp)
		{

			runMaintenance();
			updateDisplay();

			if (!callForHeatActive) break;
			turnOnBoiler();
		}
		turnOffBoiler();

		// cool down
		while (currentBoilerTemp >= boilerLowTemp)
		{
			runMaintenance();
			updateDisplay();

			turnOffBoiler();
		}
		turnOffBoiler();
		pushHeat();
	}
	turnOffBoiler();
}

bool waterOnTimeNotFinished() {

	runMaintenance();
	updateDisplay();

	if (isWaterHighTempMet()) return false;
	unsigned long currentOnWaterRunTime = millis();

	if (currentOnWaterRunTime - savedWaterRunTime < waterOnRunTime) {
		savedWaterRunTime = currentOnWaterRunTime;
		return true;
	}
	return false;
}
bool waterOffTimeNotFinished()
{
	runMaintenance();
	updateDisplay();

	if (isWaterHighTempMet()) return false;
	unsigned long currentWaterOffRunTime = millis();

	if (currentWaterOffRunTime - savedOffWaterRunTime < waterOffRunTime) {
		savedWaterRunTime = currentWaterOffRunTime;
		return true;
	}
	return false;
}



void primePump()
{
	runMaintenance();

}


String getStatusString()
{
	String retVal = "";


	return retVal;
}

bool saveState()
{
	runMaintenance();

	/// TODO: code saveState
	return true;
}

bool restoreState()
{
	runMaintenance();
	/// TODO: code restoreState
	return true;
}

bool saveConfig()
{
	runMaintenance();
	/// TODO: code saveConfig
	return true;
}

bool restoreConfig()
{
	runMaintenance();
	/// TODO: code restoreConfig
	return true;
}

bool commCycle()
{
	runMaintenance();
	/// TODO: code commCycle
	return true;
}

bool threadCycle()
{
	runMaintenance();
	/// TODO: code threadCycle
	return true;
}



bool soundAlert(int sound, int frequency) {

	runMaintenance();
	return true;
}
bool soundAlert(int sound) {

	runMaintenance();
	return soundAlert(sound, 0);
}

bool soundAlert() {

	runMaintenance();
	return soundAlert(0);

}

bool throwException(int)
{
	runMaintenance();
	return true;
}


*/


/*



	if (displayTwoLineThree == "") {
		displayOneLineThree = "E: " + String(environmentTemperature()); }
	if (displayTwoLineOne == "") { displayOneLineOne = "UP: " + String(static_cast<int>((millis() - startUpTime) / 1000)); }
	if (displayTwoLineTwo == "") { displayOneLineTwo = "B: " + String(boilerTemp()) + " |I: " + String(insideWaterTemp()); }
	//if (displayTwoLineThree == "") { displayOneLineThree = "E: " + String(environmentTemperature()) + " |O: " + String(outsideWaterTemp()); }//
	if (displayOneLineThree == "") { displayOneLineThree = "E: " + String(environmentTemperature()) + " |"; }



	// Display 2

	displayTwo.clearDisplay();
	displayTwo.setTextSize(1);
	displayTwo.setTextColor(WHITE);

	displayTwo.setCursor(11, 1);
	displayOne.println(displayTwoLineOne);

	displayTwo.setCursor(11, 11);
	displayOne.println(displayTwoLineTwo);

	displayTwo.setCursor(11, 22);
	displayOne.println(displayTwoLineThree);

	displayTwo.display();


	if (displayThreeLineOne == "") { displayThreeLineOne = "UP: " + String(static_cast<int>((millis() - startUpTime) / 1000)); }
	if (displayThreeLineTwo == "") { displayThreeLineTwo = "B0: " + String(boilerTemp()) + " | " + String(boilerTC.getThermocoupleTemp(false)); }
	if (displayThreeLineThree == "") { displayThreeLineThree = "I1: " + String(insideWaterTemp()) + " | " + String(waterTC.getThermocoupleTemp(false)); }
	if (displayFourLineOne == "") { displayFourLineOne = "BT: " + String(((burnTime / 1000 / 60) / 60)); }
	//if (displayFourLineTwo == "") { displayFourLineTwo = "O4: " + String(outsideWaterTemp()) + " | " + String(outsideWaterThermocouple.getThermocoupleTemp(false)); }
	if (displayFourLineThree == "") { displayFourLineThree = "E5: " + String(environmentTemperature()) + " | " + String(envTC.getThermocoupleTemp(false)); }


	/*
	// Display 3

	displayThree.clearDisplay();
	displayThree.setTextSize(1);
	displayThree.setTextColor(WHITE);

	displayThree.setCursor(11, 1);
	displayThree.println(displayTwoLineOne);

	displayThree.setCursor(11, 11);
	displayThree.println(displayTwoLineTwo);

	displayThree.setCursor(11, 22);
	displayThree.println(displayTwoLineThree);

	displayThree.display();

		// Display 4

	displayFour.clearDisplay();
	displayFour.setTextSize(1);
	displayFour.setTextColor(WHITE);

	displayFour.setCursor(11, 1);
	displayFour.println(displayTwoLineOne);

	displayFour.setCursor(11, 11);
	displayFour.println(displayTwoLineTwo);

	displayFour.setCursor(11, 22);
	displayFour.println(displayTwoLineThree);

	displayFour.display();

*/


/*
// Temp met for maint mode HIGH
bool isMaintHighWaterTempMet() {

	runMaintenance();
	updateDisplay();

	if (currentWaterTemp > waterMaintHighTemp) return true;
	return false;
}
*/
/*
// Temp met for maint mode LOWS
bool isMaintLowWaterTempMet() {

	runMaintenance();
	updateDisplay();

	if (currentWaterTemp < waterMaintLowTemp) return true;
	return false;
}
*/
/*

// Keep water/house @waterMaintHighTemp to try avoid call for heat
void maintenanceMode()
{
	runMaintenance();
	updateDisplay();

	// Start from the beginning
	turnOffWater();
	turnOffBoiler();

	//if (callForHeatActive) return;


	// let's start
	while (!isMaintHighWaterTempMet())
	{
		runMaintenance();
		updateDisplay();

		// FIRE
		// fire the boiler until we reach the highest temp (boilerHighTemp) and water on met

		while (currentBoilerTemp < boilerHighTemp && !isMaintHighWaterTempMet())
		{
			runMaintenance();
			updateDisplay();

			turnOnBoiler();
		}
		turnOffBoiler();


		// COOL DOWN
		// now let it cool down while still heating the water

		while (currentBoilerTemp > boilerLowTemp)
		{
			runMaintenance();
			updateDisplay();
			if (isMaintHighWaterTempMet()) break;

			turnOffBoiler();
		}
	}
	turnOffBoiler();

	// Pump water
	while (!isMaintLowWaterTempMet())
	{
		runMaintenance();
		updateDisplay();

		turnOnWater();
	}
	turnOffWater();


} */


/*
void heatTheHouse()
{
	runMaintenance();
	runMode = "2.0";
	updateDisplay();

	if (callForHeatActive)
	{
		runMaintenance();
		runMode = "2.1";
		updateDisplay();

		turnOnWater();
	}
	else {
		turnOffWater();
	}
}


void pushHeat()  // BLOCKING
{

	runMaintenance();
	runMode = "6.0";
	updateDisplay();

	if (currentWaterTemp > waterLowTemp)
	{
		runMaintenance();
		runMode = "6.1";
		updateDisplay();

		thirtySecondWaterPush();
	}
}
*/

/*
void boilerCycle()
{
	runMaintenance();
	runMode = "5.0";
	updateDisplay();


	while (callForHeatActive)
	{
		runMaintenance();
		runMode = "5.1";
		updateDisplay();

		// HEAT boiler
		while (currentBoilerTemp < boilerHighTemp) // heating up
		{
			runMaintenance();
			runMode = "5.2";
			updateDisplay();
			turnOnBoiler();
		}
		turnOffBoiler();
		pushHeat();


		// COOL boiler
		while (currentBoilerTemp > boilerLowTemp) // cooling down
		{
			runMaintenance();
			runMode = "5.3";
			updateDisplay();
			turnOffBoiler();
		}
		turnOffBoiler();
	}
	turnOffBoiler();
}

void runWaterCycle()
{
	runMaintenance();
	runMode = "7.0";
	updateDisplay();

	while (callForHeatActive)
	{
		runMaintenance();
		runMode = "7.1";
		updateDisplay();

		while (currentWaterTemp > waterLowTemp) // pushing water
		{

			runMaintenance();
			runMode = "7.2";
			updateDisplay();

			if (!callForHeatActive) break;

			turnOnWater();
		}
		runMaintenance();
		runMode = "7.3";
		updateDisplay();

		turnOffWater();
	}
	turnOffWater();
}

void thirtySecondWaterPush()  // BLOCKING
{
	runMaintenance();
	runMode = "8.0";
	updateDisplay();

	int numTimes = 120;
	int delayTime = 250;

	for (int ndx = 0; ndx < numTimes; ndx++)
	{
		runMaintenance();
		runMode = "8.1";
		updateDisplay();

		turnOnWater();
		delay(delayTime);
		runMode = "8.2";
		updateDisplay();
	}
	turnOffWater();
}

//
// BLOCKING maint and update runs all else 5 min block
// get some heat out

void fiveMinWaterPush()
{
	runMaintenance();
	runMode = "7";
	updateDisplay();
	return;

	// 5 mins = 125ms * 2400 times
}

bool isMaintWaterRunTimeUp() {

	ArduinoOTA.handle();

	unsigned long currentTime = millis();

	if (currentTime - waterMaintSavedTime > waterMaintRunTime) {
		waterMaintSavedTime = currentTime;
		return true;
	}
	return false;
}

/*
bool isWaterHighfTempMet() {
	runMaintenance();
	updateDisplay();

	if (currentWaterTemp > waterHighTemp) return true;
	return false;
}

bool isWaterLowTempMet()
{
	runMaintenance();
	updateDisplay();

	if (currentWaterTemp < waterLowTemp) return true;
	return false;
}
*/