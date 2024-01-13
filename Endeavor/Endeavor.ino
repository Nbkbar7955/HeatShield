
/*
 Name:    Endeavor 2324
 Created: 8/27/2022 3:36:02 PM
 Author:  david

 PROTOTYPE - CC:50:E3:80:A2:E8  192.168.0.36
 PRODUCTION - 24:6F:28:9E:9B:D0  192.168.0.37  25?

 OLED 0.96 module 12864 (UMLIFE) 128x64

*/

#include <SparkFun_MCP9600.h>
//#include <HttpClient.h>
#include <ArduinoOTA.h>
#include <ESPmDNS.h>
//#include <NTPClient.h>
// #include <WiFiUdp.h>
#include <SPI.h>
#include <Adafruit_SSD1306.h>
#include <ArduinoHttpClient.h>


#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

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


// Define NTP Client to get time
//WiFiUDP ntpUDP;



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

MCP9600 evThermocouple;  //65 pink
MCP9600 waterInThermocouple; //61 blue
MCP9600 boilerThermocouple; //60 yellow
MCP9600 waterOutThermocouple; //64 white



//------------------------------------------------------------------------------------------









Adafruit_SSD1306 displayTwo(-1);
Adafruit_SSD1306 displayOne(-1);

#define OLED1 0x3C // OLED 1
#define OLED2 0x3D // OLED 2

//#define OLED3 0x3C // OLED 3
//#define OLED4 0x3D // OLED 4


// Prototypes
///------------------------------------------------------------------------------------------




auto updateDisplay() -> void;


long startUpTime = 0;

///------------------------------------------------------------------------------------------
// Setup
//------------------------------------------------------------------------------------------
//

void setup()
{



	

	displayOne.begin(SSD1306_SWITCHCAPVCC, OLED1);
	displayOne.clearDisplay();
	displayOne.display();

	displayTwo.begin(SSD1306_SWITCHCAPVCC, OLED2);
	displayTwo.clearDisplay();
	displayTwo.display();

	// Begin Thermocouples
	boilerThermocouple.begin(0x060); // yellow
	waterInThermocouple.begin(0x061);   // blue  
	waterOutThermocouple.begin(0x64); // white
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	evThermocouple.begin(0x065); // pink







	
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
String activeRelay = "None";




//variables for blinking an LED with Millis
unsigned long previous_millis = 0;
unsigned long previousRelayMillis = 0;

const long interval = 250;
const long relayInterval = 750;
int nextRelay = 1;


void loop() {

	ArduinoOTA.handle();

	const auto current_millis = millis();
	const auto currentRelayMillis = millis();


	if (TestMode == 1) {


		if (current_millis - previous_millis >= interval) {
			// save the last time you blinked the LED
			previous_millis = current_millis;

			// set the LED with the ledState of the variable:
			digitalWrite(processorLED, !digitalRead(processorLED));

		}




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
				break;
			default:
				digitalWrite(burnerRelay, LOW);
				digitalWrite(waterRelay, LOW);
				digitalWrite(waterLowRelay, LOW);
				digitalWrite(testRelay, LOW);
				break;
			}

			nextRelay++;	
			if (nextRelay > 4) nextRelay = 1;
		
		
		
	}




	}
}








auto updateDisplay() -> void {

	ArduinoOTA.handle();


	// (26°C × 9/5) + 32 = 78.8°F 
	
	boilerTemp = boilerThermocouple.getThermocoupleTemp(false);
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	envTemp = evThermocouple.getThermocoupleTemp(false);
	waterOutTemp = waterOutThermocouple.getThermocoupleTemp(false);
	waterInTemp = waterInThermocouple.getThermocoupleTemp(false);

	//61/blu
	const String sWaterInTemp = String(waterInTemp);
	// 60/yel
	const String sBoilerTemp = String(boilerTemp);
	// 65/pnk
	const String sEVTemp = String(envTemp);
	//64/wht
	const String sWaterOutTemp = String(waterOutTemp);
	

	// ==============================================


	displayOne.clearDisplay();
	displayOne.setTextSize(1);
	displayOne.setTextColor(WHITE);

	displayOne.setCursor(11, 1);
	displayOne.println("UP: " + String(int((millis() - startUpTime) / 1000)));

	displayOne.setCursor(11, 11);
	displayOne.println("B:Y:0: " + sBoilerTemp);

	displayOne.setCursor(11, 22);
	displayOne.println("I:B:1: " + sWaterInTemp);

	displayOne.display();




	displayTwo.clearDisplay();
	displayTwo.setTextSize(1);
	displayTwo.setTextColor(WHITE);

	displayTwo.setCursor(11, 1);
	displayTwo.println("Relay:" + String(nextRelay - 1));

	displayTwo.setCursor(11, 11);
	displayTwo.println("O:W:4: " + sWaterOutTemp);

	displayTwo.setCursor(11, 22);
	displayTwo.println("E:P:5: " + sEVTemp);

	displayTwo.display();
	
	// ==============================================
}













