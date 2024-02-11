	
/*
 Name:    Endeavor 2324
 Created: 8/27/2022 3:36:02 PM
 Author:  david

*/


//======================================================================================
//======================================================================================
// Include files
//======================================================================================
//======================================================================================


#include <Adafruit_SSD1306.h>
#include <ArduinoHttpClient.h>
#include <ArduinoOTA.h>
#include <ESPmDNS.h>
#include <Preferences.h>
#include <SparkFun_MCP9600.h>
#include <SPI.h>
#include <WiFi.h>


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
const int misoSpi = 12;
const int mosiSpi = 13;
const int clkSpi = 14;
const int ssSpi = 15;


const int waterRelay = 17; //o WATERPUMP RELAY
const int burnerRelay = 16; //o BURNER RELAY
const int igniterRelay = 18; //o
const int primePumpRelay = 19;

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

//
//======================================================================================
//======================================================================================
// OLED Definitions
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

#define OLED3 0x3C // OLED 3
#define OLED4 0x3D // OLED 4


//======================================================================================
//======================================================================================
// Function Prototypes
//======================================================================================
//======================================================================================

void primePump();
bool isFlameOut();
void disableEndeavor();
auto heatUp() -> void;
auto coolDown() -> void;
auto isITempHighMet(void) -> bool;
auto soundAlert(int, int) -> bool;
auto soundAlert(int) -> bool;
auto soundAlert() -> bool;
auto throwException(int) -> bool;
auto safetyCheck(int) -> bool;
auto testCycle() -> bool;
auto opCycle(void) -> void;
void runHeatCycle();
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
void blink();
void runMaintenance();

//======================================================================================
//======================================================================================
// operational Variables
//======================================================================================
//======================================================================================


//
// writing
//

Preferences preferences;

//
// temperatures
//

float bTempHigh = 950;
float bTempLow = 300;

float iTempHigh = 125;
float iTempLow = 90;

float eTempHigh = 69;
float eTempLow = 65;

float oTempHigh = 200;
float oTempLow = 100;

//
// others
//

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


unsigned long blinkInterval = 750;
unsigned long savedBlinkTime = 0;
unsigned long burnTime = 0;
long startUpTime = 0;

int boilerOnDelay = 30; // seconds delay on and off to prevent cycling too fast (temp bouncing)
int boilerOffDelay = 30; //seconds
int waterOnDelay = 30; // seconds
int waterOffDelay = 30; // seconds
long primePumpRunTime = 15; // seconds

long waterRunTime = 4 * 60; // minutes

//======================================================================================
//======================================================================================
// Temporary Variables
//======================================================================================
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
//======================================================================================
//	OTA Definition / Setup
//======================================================================================
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
//======================================================================================
//	Serial
//======================================================================================
//======================================================================================

		Serial.println("Ready");
		Serial.print("IP address: ");
		Serial.println(WiFi.localIP());


//======================================================================================
//======================================================================================
// Pin Modes
//======================================================================================
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

	pinMode(primePumpRelay, OUTPUT);
	digitalWrite(primePumpRelay, HIGH);

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



void loop() {

	runMaintenance();
	
	if (TestMode) testCycle();
	opCycle();
}


void opCycle()
{
	runMaintenance();
	while (ETemp() <= eTempLow) runHeatCycle();
}

void runHeatCycle()
{
	runMaintenance();
	
	while (ITemp() < iTempHigh)
	{
		ArduinoOTA.handle();
		updateDisplay();
		blink();

		heatUp();
		coolDown();
	}
	waterCycle();
	
}


auto heatUp() -> void
{
	runMaintenance();
	
	while (BTemp() <= bTempHigh)
	{
		ArduinoOTA.handle();
		updateDisplay();
		blink();

		turnOnBoiler();
		if (isITempHighMet()) break;
	}	
}


auto coolDown() -> void
{
	runMaintenance();

	while (BTemp() >= bTempLow)
	{
		ArduinoOTA.handle();	
		updateDisplay();
		blink();
		
		turnOffBoiler();
		if (isITempHighMet()) break;
	}
}

auto isITempHighMet() -> bool
{
	runMaintenance();
	if(ITemp() < iTempHigh)	return false;
	return true;
}


void turnOnBoiler()
{
	runMaintenance();
	digitalWrite(burnerRelay, LOW);
	digitalWrite(igniterRelay, LOW);
	updateBurnTime();
}


void turnOffBoiler()
{
	runMaintenance();
	digitalWrite(burnerRelay, HIGH);
	digitalWrite(igniterRelay, HIGH);
}

bool waterCycle(int runTime)
{	
	runMaintenance();

	//TODO: waterCycle Blocks

	
	/// TODO: wrong logic?

	if (runTime == 0) runTime = waterRunTime;	
	digitalWrite(waterRelay, LOW);
	delay(runTime * 1000);
	digitalWrite(waterRelay, HIGH);
	return true;
}

auto waterCycle() -> bool
{
	runMaintenance();	
	return waterCycle(0);
}


int  ETemp()
{
	runMaintenance();
	return int(EThermocouple.getThermocoupleTemp(false));

}

int ITemp()
{
	runMaintenance();
	return int(IThermocouple.getThermocoupleTemp(false));

}

auto OTemp() -> int
{
	runMaintenance();
	return int(OThermocouple.getThermocoupleTemp(false));

}

int BTemp()
{
	runMaintenance();
	return int(BThermocouple.getThermocoupleTemp(false));

}



// TODO: get realtime values
// TODO: write flameOut logic

bool isFlameOut()
{
	runMaintenance();
	int flame = 0;

	flame = analogRead(map(flame, 0, 1023, 0, 255));

	if (flame == 0) return false;
	if (flame == 255) return true;

	return false;
}

void primePump()
{
	runMaintenance();
	long currentTime = millis();

	// turn it on
	if (currentTime < primePumpRunTime * 1000) digitalWrite(primePumpRelay, LOW);
	// turn it off
	else digitalWrite(primePumpRelay, HIGH);

}




void disableEndeavor()
{
	runMaintenance();
	digitalWrite(burnerRelay, HIGH);
	digitalWrite(igniterRelay, HIGH);
	digitalWrite(waterRelay, HIGH);

}





void updateBurnTime()
{
	runMaintenance();
	burnTime += millis();
}

auto updateDisplay() -> void {

	runMaintenance();
	
	if (displayOneLineOne == "") {displayOneLineOne = "UP: " + String(int((millis() - startUpTime) / 1000));}
	if (displayOneLineTwo == "") {displayOneLineTwo = "B0: " + String(BTemp()) + " | " + String(BThermocouple.getThermocoupleTemp(false)); }
	if (displayOneLineThree == "") {displayOneLineThree = "I1: " + String(ITemp()) + " | " + String(IThermocouple.getThermocoupleTemp(false));}
	if (displayTwoLineOne == "") {displayTwoLineOne = "BT: " + String(((burnTime / 1000 / 60) / 60));}
	if (displayTwoLineTwo == "") {displayTwoLineTwo = "O4: " + String(OTemp()) + " | " + String(OThermocouple.getThermocoupleTemp(false));}
	if (displayTwoLineThree == "") {displayTwoLineThree = "E5: " + String(ETemp()) + " | " + String(EThermocouple.getThermocoupleTemp(false));}

	
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

	
	if (displayThreeLineOne == "") {displayThreeLineOne = "UP: " + String(int((millis() - startUpTime) / 1000));}
	if (displayThreeLineTwo == "") {displayThreeLineTwo = "B0: " + String(BTemp()) + " | " + String(BThermocouple.getThermocoupleTemp(false)); }
	if (displayThreeLineThree == "") {displayThreeLineThree = "I1: " + String(ITemp()) + " | " + String(IThermocouple.getThermocoupleTemp(false));}
	if (displayFourLineOne == "") {displayFourLineOne = "BT: " + String(((burnTime / 1000 / 60) / 60));}
	if (displayFourLineTwo == "") {displayFourLineTwo = "O4: " + String(OTemp()) + " | " + String(OThermocouple.getThermocoupleTemp(false));}
	if (displayFourLineThree == "") {displayFourLineThree = "E5: " + String(ETemp()) + " | " + String(EThermocouple.getThermocoupleTemp(false));}


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

}



void myTests()
{
	runMaintenance();
	/// TODO: This is test new stuff area
	
}

void blink()
{
	unsigned long currentBlinkTime = millis();

	if (currentBlinkTime - savedBlinkTime >= blinkInterval) {
		savedBlinkTime = currentBlinkTime;
		digitalWrite(processorLED, !digitalRead(processorLED));
		digitalWrite(callForHeat, !digitalRead(callForHeat));
	}
}

void runMaintenance()
{
	ArduinoOTA.handle();
	updateDisplay();
	blink();
}


auto testCycle() -> bool
{
	runMaintenance();

	return true;
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

auto safetyCheck(int) -> bool
{
	runMaintenance();
	return true;
}













