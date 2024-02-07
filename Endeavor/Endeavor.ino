
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
#include <Preferences.h>


#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

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
// operational Variables
//======================================================================================
//======================================================================================

Preferences preferences;

float BTempHigh = 950;
float BTempLow = 300;

float ITempHigh = 125;
float ITempLow = 90;

float ETempHigh = 69;
float ETempLow = 65;

float OTempHigh = 200;
float OTempLow = 100;

long waterRunTime = 240000;
int averageNumber = 10;

int temperatureRunCycles = 5;
const long blinkInterval = 750;

unsigned long currentBlinkMillis = 0;
unsigned long previousBlinkMillis = 0;

unsigned long burnTime = 0;



long startUpTime = 0;

int boilerOnDelay = 30; // delay on and off to prevent cycling too fast (temp bouncing)
int boilerOffDelay = 30;
int waterOnDelay = 30;
int waterOffDelay = 30;






//======================================================================================
//======================================================================================
// WiFi 
//======================================================================================
//======================================================================================

const char* networkName = "Wilson.Net-2.4G";
const char* networkNamePassPhrase = "wilsonwebsite.com";

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
const int flameOut = 5; //flame out

// SPI
const int misoSPI = 12;
const int mosiSPI = 13;
const int clkSPI = 14;
const int ssSPI = 15;


const int waterRelay = 17; //o WATERPUMP RELAY
const int burnerRelay = 16; //o BURNER RELAY
const int igniterRelay = 18; //o
const int safetyRelay = 19;

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

MCP9600 EThermocouple;  //64 green
MCP9600 IThermocouple; //61 blue
MCP9600 BThermocouple; //60 yellow
MCP9600 OThermocouple; //64 white

/// TODO: consider other thermocouple amps


//======================================================================================
//======================================================================================
// OLED Definitions
//======================================================================================
//======================================================================================
//


///  TODO: Setup 2nd I2C
///  
Adafruit_SSD1306 displayTwo(-1);
Adafruit_SSD1306 displayOne(-1);
Adafruit_SSD1306 displayThree(-1);
Adafruit_SSD1306 displayFour(-1);

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


void disableEndeavor();


auto heatUp() -> void;
auto coolDown() -> void;
auto iTmpCheck(void) -> bool;
auto soundAlert(int, int) -> bool;
auto soundAlert(int) -> bool;
auto soundAlert() -> bool;
auto throwException(int) -> bool;
auto safetyCheck(int) -> bool;
auto testCycle() -> bool;
auto opCycle(void) -> void;
auto runHeatCycle() -> bool;
auto stopHeatCycle() -> bool;
auto waterCycle(int) -> bool;
auto waterCycle(void) -> bool;


int ETemp(void);
int ITemp(void);
int OTemp(void);
int BTemp(void);

void updateBurnTime(void);

auto updateDisplay() -> void;
auto saveState() -> bool;
auto restoreState() -> bool;
auto saveConfig() -> bool;
auto restoreConfig() -> bool;
auto commCycle() -> bool;
auto threadCycle() -> bool;

void turnOnBoiler(void);
void turnOffBoiler(void);

void myTests();


//======================================================================================
//======================================================================================
// Temporary Definitions
//======================================================================================
//
// TESTMODE
//
//======================================================================================
//

bool TestMode = true;

unsigned long prevBurnOffTime = 0;
int testBurnOffTimeInterval = 2000;
int testBurnerOnTimeInterval = 3000;
unsigned long prevBurnONTime = 0;





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
	WiFi.begin(networkName, networkNamePassPhrase);

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

		pinMode(safetyRelay, OUTPUT);
		digitalWrite(safetyRelay, HIGH);

		pinMode(igniterRelay, OUTPUT); // o PIN 18
		digitalWrite(igniterRelay, HIGH);

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


	//**********************************************************************************************
	//**********************************************************************************************
	//**********************************************************************************************
	// TODO: REMOVE This turnOnBoiler()
//turnOnBoiler();
	//**********************************************************************************************
	//**********************************************************************************************
	//**********************************************************************************************


}


//======================================================================================
//======================================================================================
// Loop
//======================================================================================
//======================================================================================
////======================================================================================
/////======================================================================================
/////======================================================================================
/////======================================================================================
/////======================================================================================
//




void loop() {

	ArduinoOTA.handle();
	digitalWrite(safetyRelay, LOW);
	updateDisplay();

	currentBlinkMillis = millis();

	if (currentBlinkMillis - previousBlinkMillis >= blinkInterval) {
		previousBlinkMillis = currentBlinkMillis;
		digitalWrite(processorLED, !digitalRead(processorLED));
	}

	if (TestMode) {
		testCycle();
	}

	if (digitalRead(callForHeat) == HIGH) {
		opCycle();
	} else
	{
		disableEndeavor();
	}

}


void opCycle()
{
	ArduinoOTA.handle();
	digitalWrite(safetyRelay, LOW);
	updateDisplay();

	const auto currentETemp = ETemp();

	if (currentETemp <= ETempLow) runHeatCycle();
	else if (currentETemp >= ETempHigh) stopHeatCycle();

	updateDisplay();
}

auto runHeatCycle() -> bool
{
	ArduinoOTA.handle();
	digitalWrite(safetyRelay, LOW);
	
	while (ETemp() <= ETempLow)
	{
		ArduinoOTA.handle();
		digitalWrite(safetyRelay, LOW);
		updateDisplay();

		heatUp();
		coolDown();
		waterCycle();
	}
	stopHeatCycle();
	return true;
	
}

auto stopHeatCycle() -> bool
{
	ArduinoOTA.handle();
	digitalWrite(safetyRelay, LOW);

	updateDisplay();
	
	turnOffBoiler();

	return true;
}

void disableEndeavor()
{
	ArduinoOTA.handle();
	digitalWrite(safetyRelay, LOW);
	updateDisplay();

	digitalWrite(burnerRelay, HIGH);
	digitalWrite(igniterRelay, HIGH);
	digitalWrite(waterRelay, HIGH);

	
	
}

auto heatUp() -> void
{
	ArduinoOTA.handle();
	digitalWrite(safetyRelay, LOW);

	auto currentBTemp = BTemp();
	
	while (currentBTemp < BTempHigh)
	{
		ArduinoOTA.handle();
		digitalWrite(safetyRelay, LOW);
		updateDisplay();

		turnOnBoiler();
		currentBTemp = BTemp();

		if (iTmpCheck()) return;
	}	
}


auto coolDown() -> void
{
	ArduinoOTA.handle();
	digitalWrite(safetyRelay, LOW);
	updateDisplay();
	
	int currentBTemp = BTemp();

	while (currentBTemp >= BTempLow)
	{
		ArduinoOTA.handle();
		digitalWrite(safetyRelay, LOW);
		updateDisplay();
		
		turnOffBoiler();
		currentBTemp = BTemp();
		
		if (iTmpCheck()) return;
	}
	turnOffBoiler();
}

auto iTmpCheck() -> bool
{
	ArduinoOTA.handle();
	digitalWrite(safetyRelay, LOW);
	updateDisplay();

	if(ITemp() < ITempHigh)
	{
		return false;
	}
	return true;
}


void turnOnBoiler()
{
	ArduinoOTA.handle();
	digitalWrite(safetyRelay, LOW);


	
	digitalWrite(burnerRelay, LOW);
	digitalWrite(igniterRelay, LOW);
}


void turnOffBoiler()
{
	ArduinoOTA.handle();
	digitalWrite(safetyRelay, LOW);
	
	digitalWrite(burnerRelay, HIGH);
	digitalWrite(igniterRelay, HIGH);
}

bool waterCycle(int runTime)
{
	
	ArduinoOTA.handle();
	digitalWrite(safetyRelay, LOW);
	updateDisplay();

	/// TODO: wrong logic?
	digitalWrite(waterRelay, LOW);

	if (runTime == 0) runTime = waterRunTime;
		
	delay(runTime);
	digitalWrite(waterRelay, HIGH);
	return true;
}

auto waterCycle() -> bool
{
	ArduinoOTA.handle();
	digitalWrite(safetyRelay, LOW);
	
	return waterCycle(0);
}


int  ETemp()
{
	ArduinoOTA.handle();
	digitalWrite(safetyRelay, LOW);
	
	return int(EThermocouple.getThermocoupleTemp(false));

}

int ITemp()
{
	ArduinoOTA.handle();
	digitalWrite(safetyRelay, LOW);

	return int(IThermocouple.getThermocoupleTemp(false));

}

auto OTemp() -> int
{
	ArduinoOTA.handle();
	digitalWrite(safetyRelay, LOW);

	return int(OThermocouple.getThermocoupleTemp(false));

}

int BTemp()
{
	ArduinoOTA.handle();
	digitalWrite(safetyRelay, LOW);

	return int(BThermocouple.getThermocoupleTemp(false));

}



void updateBurnTime()
{
	ArduinoOTA.handle();
	digitalWrite(safetyRelay, LOW);

	if (digitalRead(burnerRelay) == LOW)
	{
		burnTime += millis();
	}
}

auto updateDisplay() -> void {

	ArduinoOTA.handle();
	digitalWrite(safetyRelay, LOW);

	displayOne.clearDisplay();
	displayOne.setTextSize(1);
	displayOne.setTextColor(WHITE);

	displayOne.setCursor(11, 1);
	displayOne.println("UP: " + String(int((millis() - startUpTime) / 1000)));

	displayOne.setCursor(11, 11);
	displayOne.println("B0: " + String(BTemp()) + " | " + String(BThermocouple.getThermocoupleTemp(false)));

	displayOne.setCursor(11, 22);
	displayOne.println("I1: " + String(ITemp()) + " | " + String(IThermocouple.getThermocoupleTemp(false)));

	displayOne.display();


	displayTwo.clearDisplay();
	displayTwo.setTextSize(1);
	displayTwo.setTextColor(WHITE);

	displayTwo.setCursor(11, 1);
	displayTwo.println(String("BT: " + String( (((burnTime / 1000) / 60) / 60))));

	displayTwo.setCursor(11, 11);
	displayTwo.println("O4: " + String(OTemp()) + " | " + String(OThermocouple.getThermocoupleTemp(false)));

	displayTwo.setCursor(11, 22);
	displayTwo.println("E5: " + String(ETemp()) + " | " + String(EThermocouple.getThermocoupleTemp(false)));

	displayTwo.display();

}



void myTests()
{
	/// TODO: This is test new stuff area

	///////////////////////////////////////////////////////////////////////
	///return to disable
	///////////////////////////////////////////////////////////////////////
	
}


auto testCycle() -> bool
{
	ArduinoOTA.handle();
	digitalWrite(safetyRelay, LOW);

	unsigned long curTime = millis();
	
	updateDisplay();

	return true;
}


bool saveState()
{
	/// TODO: code saveState
	return true;
}

bool restoreState()
{
	/// TODO: code restoreState
	return true;
}

bool saveConfig()
{
	/// TODO: code saveConfig
	return true;
}

bool restoreConfig()
{
	/// TODO: code restoreConfig
	return true;
}

bool commCycle()
{
	/// TODO: code commCycle
	return true;
}

bool threadCycle()
{
	/// TODO: code threadCycle
	return true;
}



bool soundAlert(int sound, int frequency) {
	return true;
}
bool soundAlert(int sound) {
	return soundAlert(sound, 0);
}

bool soundAlert() {
	return soundAlert(0);

}

bool throwException(int)
{
	digitalWrite(safetyRelay, HIGH);
	return true;
}

auto safetyCheck(int) -> bool
{
	digitalWrite(safetyRelay, LOW);
	return true;
}













