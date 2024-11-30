

/*
 Name:		Endeavor 2425
 Created:	8/27/2022 3:36:02 PM
 Update:	11/29/2024 00:30
			11/28/2024 15:00.00
 Author:	david

 version:	0.8.068

*/


//======================================================================================
//======================================================================================
// Include files
//======================================================================================
//======================================================================================

int ON = 0x0;
int OFF = 0x1;

int blrON = 0x1;
int blrOFF = 0x0;


#include <Adafruit_SSD1306.h>
#include <ArduinoHttpClient.h>
#include <ArduinoOTA.h>
#include <ESPmDNS.h>
#include <Preferences.h>
#include <SparkFun_MCP9600.h>
#include <SPI.h>
#include <WiFi.h>



///
/// TODO: Exception Handling
/// TODO: Safety Checking
/// TODO: Flow Sensor install and code
/// TODO: How to test pump running
/// TODO: Install and code CDS cell for flameOut check
/// TODO: refine testCycle test pin
/// TODO: rewire and code test relay to eShutdown
/// TODO: OLED address changes
/// 





//======================================================================================
//======================================================================================
// Setup Vars
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

int environmentHighTemp = 70;
int highOffSet = 0;

int environmentLowTemp = 67;
int lowOffSet = 0;

int boilerHighTemp = 975;
int boilerLowTemp = 300;

int insideWaterHighTemp = 140;
int insideWaterLowTemp = 115;

// 30,000 = 30 seconds
// 60,000 = 1 min
// 120,000 = 2 min
// 180,000 = 3 min
// 240,000 = 4 mins
// 300,000 = 5 mins
// 600,000 = 10 mins

unsigned long waterOnRunTime = 30000; // water on
unsigned long savedWaterOnRunTime = 0;

unsigned long waterOffRunTime = 180000; // water off
unsigned long savedOffWaterRunTime = 0;

int outsideWaterHighTemp = 200;
int outsideWaterLowTemp = 100;

unsigned long blinkInterval = 175;
unsigned long savedBlinkTime = 0;
unsigned long burnTime = 0;
long startUpTime = 0;

int boilerOnDelay = 30000; // seconds delay on and off to prevent cycling too fast (temp bouncing)
int boilerOffDelay = 30000; //seconds
int waterOnDelay = 30000; // seconds
int waterOffDelay = 30000; // seconds
long primePumpRunTime = 15000; // seconds (15)

int currentBoilerTemp = 2000; // set hi to start for compare
int currentWaterTemp = 2000; //""
int currentEnvironmentTemp = 2000; // ""

unsigned long prevBurnOffTime = 0;
int testBurnOffTimeInterval = 2000;
int testBurnerOnTimeInterval = 3000;
unsigned long prevBurnONTime = 0;


//======================================================================================
//======================================================================================
// WiFi 
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

const int processorLED = 2; //o LED on MicroProcessor
const int callForHeat = 4; //i CALLFORHEAT
const int flameOut = 5; //flame out

// SPI
const int misoSpi = 12;
const int mosiSpi = 13;
const int clkSpi = 14;
const int ssSpi = 15;


const int waterRelay = 17; //o WATERPUMP RELAY
const int burnerRelay = 16; //o BURNER RELAY
const int zoneTwoRelay = 18; //o
const int standbyRelay = 19; //o Upstairs

/// <summary>
/// TODO:Test and code speaker
/// TODO: test and code pushbuttons
/// TODO: code mode/options with PB 
/// </summary>

const int speaker = 32; //o SOUNDALARM
const int PB1 = 34; //i PB1
const int PB2 = 35; //i PB2
const int PB3 = 36; //i PB3 
const int PB4 = 39; //i PB4 


//======================================================================================
//======================================================================================
// Thermocouple Definitions
//======================================================================================
//======================================================================================
//

MCP9600 environmentThermocouple;  //64 green
MCP9600 insideWaterThermocouple; //61 blue
MCP9600 boilerThermocouple; //60 yellow
MCP9600 outsideWaterThermocouple; //64 white

/// TODO: consider other thermocouple amps

//
//======================================================================================
//======================================================================================
// Display Definitions
//======================================================================================
//======================================================================================
//


///  TODO: Setup 2nd I2C
///  
Adafruit_SSD1306 displayOne(-1);
Adafruit_SSD1306 displayTwo(-1);
Adafruit_SSD1306 displayThree(-1);
Adafruit_SSD1306 displayFour(-1);



#define OLED1 0x3C // OLED 1
#define OLED2 0x3D // OLED 2



//#define OLED3 = 0x3C // OLED 3
//#define	OLED4 = 0x3D // OLED 4


String displayOneLineOne = "";
String displayOneLineTwo = "";
String displayOneLineThree = "";

String displayTwoLineOne = "";
String displayTwoLineTwo = "";
String displayTwoLineThree = "";

String displayThreeLineOne = "";
String displayThreeLineTwo = "";
String displayThreeLineThree = "";

String displayFourLineOne = "";
String displayFourLineTwo = "";
String displayFourLineThree = "";


//======================================================================================
//======================================================================================
// Function Prototypes
//======================================================================================
//======================================================================================

auto getStatus(void)->String;
void primePump();
bool isFlameOut();
void disableEndeavor();
void heatUpBoiler();
void coolDownBoiler();
bool isInsideWaterTempMet(void);
bool soundAlert(int, int);
bool soundAlert(int);
bool soundAlert();
bool throwException(int);
bool safetyCheck(int);
bool testCycle();
void opCycle(void);
void heatUpTheHouse();
void turnOnWaterPump(void);
int environmentTemperature(void);
int insideWaterTemp(void);
int boilerTemp(void);
void updateBurnTime(void);
void updateDisplay();
bool saveState();
bool restoreState();
bool saveConfig();
bool restoreConfig();
bool commCycle();
bool threadCycle();
void turnOnBoiler(void);
void turnOffBoiler(void);
void myTests();
void blink();
void runMaintenance();
bool isEnvironmentTempMet();
void runHeatCycle();
void runWaterCycle();
bool waterOnTimeNotFinished();
bool waterOffTimeNotFinished();
void turnOffWaterPump();
int ambientTemp();

//
// writing
//

Preferences preferences;

//======================================================================================
//======================================================================================
// Setup
//======================================================================================
//======================================================================================
//

void setup()
{

	/*
	  preferences.begin("HSConfig",false);

	  preferences.getUInt("counter",counter);

	  // do something with counter

	  preferences.putUInt("counter", counter);


	  preferences.end();

	  */



	  //======================================================================================
	  // time  Var Inits
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

	//.begin(0x060); // yellow
	insideWaterThermocouple.begin(0x061);   // blue  
	boilerThermocouple.begin(0x60); // white
	environmentThermocouple.begin(0x064); // pink


	Serial.begin(115200);
	Serial.println("Booting");


	//======================================================================================
	// WiFi
	//======================================================================================

	WiFi.mode(WIFI_STA);
	WiFi.begin(networkName, networkNamePassPhrase);

	while (WiFi.waitForConnectResult() != WL_CONNECTED)
	{
		Serial.println("Connection Failed! Rebooting...");
		delay(5000);
		ESP.restart();
	}



	//======================================================================================
	//======================================================================================
	//	OTA Definition / Setup
	//======================================================================================
	//======================================================================================

		// Port defaults to 3232
	ArduinoOTA.setPort(3232);
	ArduinoOTA.setHostname("ENDEAVOR_87");



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
					Serial.println("MAC:  ");
					Serial.println(WiFi.macAddress());



					//======================================================================================
					//======================================================================================
					// Pin Modes
					//======================================================================================
					//======================================================================================


					pinMode(processorLED, OUTPUT);
					digitalWrite(processorLED, OFF);

					pinMode(callForHeat, OUTPUT); // i PIN 4
					digitalWrite(callForHeat, OFF);

					pinMode(speaker, OUTPUT); // o PIN 25
					digitalWrite(speaker, OFF);


					pinMode(waterRelay, OUTPUT); // o PIN 26
					digitalWrite(waterRelay, OFF);

					pinMode(burnerRelay, OUTPUT); // o PIN 27
					digitalWrite(burnerRelay, LOW);

					pinMode(standbyRelay, OUTPUT);
					digitalWrite(standbyRelay, OFF);

					pinMode(zoneTwoRelay, OUTPUT); // o PIN 18
					digitalWrite(zoneTwoRelay, ON);

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

					//======================================================================================
					//======================================================================================
					// Startup functions
					//======================================================================================
					//======================================================================================

					restoreConfig();
					restoreState();
					//======================================================================================
					//======================================================================================
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


bool TestMode = false;


bool testBoilerHighTemp = false;
bool testInsideWaterHighTemp = false;
bool testEnvironmentHighTemp = true;
bool testOutsideWaterTempHighTemp = false;


void loop() {

	runMaintenance();
	updateDisplay();

	if (TestMode) testCycle();



	opCycle();
}


void opCycle()
{
	runMaintenance();
	updateDisplay();

	if (!isEnvironmentTempMet()) {

		digitalWrite(callForHeat, ON);

		heatUpTheHouse();

		digitalWrite(callForHeat, OFF);
	}
}






void heatUpTheHouse()
{
	runMaintenance();
	updateDisplay();

	while (!isEnvironmentTempMet()) {
		runMaintenance();
		updateDisplay();

		runHeatCycle();
		runWaterCycle();
	}
}

void runHeatCycle() {

	runMaintenance();
	updateDisplay();

	if (isEnvironmentTempMet()) return;

	while (insideWaterTemp() < insideWaterHighTemp)
	{
		runMaintenance();
		updateDisplay();

		turnOffWaterPump();
		if (isEnvironmentTempMet()) break;

		if (insideWaterTemp() >= 120) turnOnWaterPump();

		heatUpBoiler();
		if (isEnvironmentTempMet()) break;

		coolDownBoiler();

		if (isEnvironmentTempMet()) break;
	}
	turnOffBoiler();
	runWaterCycle();
}

void runWaterCycle() {

	runMaintenance();
	updateDisplay();

	turnOffBoiler();
	if (isEnvironmentTempMet()) return;

	while (insideWaterTemp() >= insideWaterLowTemp) {
		runMaintenance();
		updateDisplay();

		if (isEnvironmentTempMet()) break;

		turnOffBoiler();
		turnOnWaterPump();
	}
	turnOffWaterPump();
}

void heatUpBoiler()
{
	runMaintenance();
	updateDisplay();

	if (isInsideWaterTempMet()) return;

	while (boilerTemp() <= boilerHighTemp)
	{
		runMaintenance();
		updateDisplay();

		turnOnBoiler();

		if (isInsideWaterTempMet()) break;
	}
	turnOffBoiler();
}

void coolDownBoiler()
{
	runMaintenance();
	updateDisplay();

	while (boilerTemp() >= boilerLowTemp)
	{
		runMaintenance();
		updateDisplay();

		turnOffBoiler();
		if (isInsideWaterTempMet()) return;
	}
}


bool isInsideWaterTempMet() {
	runMaintenance();
	updateDisplay();

	if (insideWaterTemp() >= insideWaterHighTemp) return true;
	if (insideWaterTemp() <= insideWaterLowTemp) return false;
	return false;
}

bool isEnvironmentTempMet() {
	runMaintenance();
	updateDisplay();

	if (environmentTemperature() >= environmentHighTemp) return true;
	if (environmentTemperature() <= environmentLowTemp) return false;
	return false;
}


void turnOnBoiler()
{
	runMaintenance();
	updateDisplay();

	digitalWrite(burnerRelay, blrON);

	//isFlameOut();
	//updateBurnTime();
}


void turnOffBoiler()
{
	runMaintenance();
	updateDisplay();

	digitalWrite(burnerRelay, blrOFF);

	//isFlameOut();
}


String getStatus()
{
	// condition ? expression1 : expression2;

	const auto burnStat = String(digitalRead(burnerRelay) ? "B+ " : "B- ");
	const auto  waterStat = String(digitalRead(waterRelay) ? "W+ " : "W- ");
	const auto  zoneTwoStat = String(digitalRead(zoneTwoRelay) ? "V+ " : "V- ");
	const auto cHeatStat = String(digitalRead(callForHeat) ? "H+ " : "H- ");

	return burnStat + waterStat + zoneTwoStat + cHeatStat;
}

void turnOnWaterPump()
{
	digitalWrite(waterRelay, ON);
}

void turnOffWaterPump() {
	digitalWrite(waterRelay, OFF);
}

int  environmentTemperature() {

	runMaintenance();
	int result = environmentThermocouple.getThermocoupleTemp(false);
	runMaintenance();
	updateDisplay();

	return result;
}

int insideWaterTemp() {

	runMaintenance();
	int result = insideWaterThermocouple.getThermocoupleTemp(false);
	runMaintenance();
	updateDisplay();

	return result;

}

int boilerTemp()
{
	runMaintenance();
	int result = boilerThermocouple.getThermocoupleTemp(false);
	runMaintenance();
	updateDisplay();

	return result;
}

bool isFlameOut()
{
	runMaintenance();

	return digitalRead(flameOut) ? true : false;
}



void disableEndeavor()
{
	runMaintenance();
	digitalWrite(burnerRelay, LOW);
	digitalWrite(zoneTwoRelay, OFF);
	digitalWrite(waterRelay, OFF);

}

void updateDisplay() {

	//if (displayOneLineOne == "") { displayOneLineOne = getStatus(); }
	if (displayOneLineOne == "") { displayOneLineOne = "UP: " + String(static_cast<int>((millis() - startUpTime) / 1000)); }
	if (displayOneLineTwo == "") { displayOneLineTwo = "B: " + String(boilerTemp()) + " |W: " + String(insideWaterTemp()); }
	if (displayOneLineThree == "") { displayOneLineThree = "E: " + String(environmentTemperature()); }

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
	unsigned long currentBlinkTime = millis();

	if (currentBlinkTime - savedBlinkTime >= blinkInterval) {
		savedBlinkTime = currentBlinkTime;
		digitalWrite(processorLED, !digitalRead(processorLED));
	}
}

void runMaintenance()
{
	ArduinoOTA.handle();
	blink();

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

bool testCycle()
{
	runMaintenance();
	updateDisplay();



	digitalWrite(zoneTwoRelay, OFF);
	digitalWrite(standbyRelay, ON);
	return true;

	while (true)
	{
		runMaintenance();
		updateDisplay();


		unsigned long currentCycle = millis();

		if (currentCycle - savedCycle >= cycleInterval) {
			savedCycle = currentCycle;

			digitalWrite(zoneTwoRelay, !digitalRead(zoneTwoRelay));
			digitalWrite(standbyRelay, !digitalRead(standbyRelay));

		}
	}
}

bool waterOnTimeNotFinished() {

	runMaintenance();
	updateDisplay();

	if (isInsideWaterTempMet()) return false;
	unsigned long currentOnWaterRunTime = millis();

	if (currentOnWaterRunTime - savedWaterOnRunTime < waterOnRunTime) {
		savedWaterOnRunTime = currentOnWaterRunTime;
		return true;
	}
	return false;
}
bool waterOffTimeNotFinished()
{
	runMaintenance();
	updateDisplay();

	if (isInsideWaterTempMet()) return false;
	unsigned long currentWaterOffRunTime = millis();

	if (currentWaterOffRunTime - savedOffWaterRunTime < waterOffRunTime) {
		savedWaterOnRunTime = currentWaterOffRunTime;
		return true;
	}
	return false;
}



void primePump()
{
	runMaintenance();

}

void updateBurnTime()
{
	runMaintenance();
	burnTime += millis();
}

String getStatusString()
{
	String retVal = "";


	return retVal;
}

void myTests()
{
	runMaintenance();
	/// TODO: This is test new stuff area

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

bool safetyCheck(int)
{
	runMaintenance();
	return true;
}


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
	if (displayThreeLineTwo == "") { displayThreeLineTwo = "B0: " + String(boilerTemp()) + " | " + String(boilerThermocouple.getThermocoupleTemp(false)); }
	if (displayThreeLineThree == "") { displayThreeLineThree = "I1: " + String(insideWaterTemp()) + " | " + String(insideWaterThermocouple.getThermocoupleTemp(false)); }
	if (displayFourLineOne == "") { displayFourLineOne = "BT: " + String(((burnTime / 1000 / 60) / 60)); }
	//if (displayFourLineTwo == "") { displayFourLineTwo = "O4: " + String(outsideWaterTemp()) + " | " + String(outsideWaterThermocouple.getThermocoupleTemp(false)); }
	if (displayFourLineThree == "") { displayFourLineThree = "E5: " + String(environmentTemperature()) + " | " + String(environmentThermocouple.getThermocoupleTemp(false)); }


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

