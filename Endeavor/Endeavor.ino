/*
 Name:    Endeavor 2324
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
float highTemperature = 155;//highStdbyTemperature;
float lowTemperature = 130;//lowActiveTemperature;


//======================================================================================
//======================================================================================
//
//

/*
// Definitions

MCP9600 t_boiler;
MCP9600 t_water_inside;
// MCP9600 t_water_outside;
MCP9600 t_living_room;
// MCP9600 t_master_bedroom;

Adafruit_SSD1306 dsp_display_one(-1);
Adafruit_SSD1306 dsp_display_two(-1);
Adafruit_SSD1306 dsp_display_three(-1);
Adafruit_SSD1306 dsp_display_four(-1);

#define OLED1 0x3C // OLED 1 I2C_1
#define OLED2 0x3D // OLED 2 I2C_1 
#define OLED3 0x3C // OLED 3 I2C_2 
#define OLED4 0x3D // OLED 4 I2C_2 



// Settings

	t_boiler.begin(0x060);
	t_water_inside.begin(0x061);
	// t_water_outside.begin(0x??);
	t_living_room.begin(0x067);
	// t_master_bedroom.begin(0x??);


	// Pin Assignments
	int p_blink 2;
	int p_call_for_heat 4;
	int p_test_pin 5;
	int p_MISO;
	int p_MOSI 13;
	int p_CLK 14;
	int p_SS 15;
	int p_boiler_relay 16;
	int p_water_hi_relay 17;
	int p_water_lo_relay 18;
	int p_test_relay 19;
	int p_I2C_1_SDA 21;
	int p_I2C_1_SCL 22;
	// 23 NC
	int p_I2C_2_SDA 25;
	int p_I2C_2_SCL 26;
	// 27 NC
	int p_speaker 32;
	// 33 NC
	int p_PB1 34;
	int p_PB2 35;
	int p_PB3 36;
	int p_PB4 39;

// Operational Variables
	float op_boiler_temp = 0;
	float op_boiler_temp_CURR = 0;
	float op_boiler_temp_PREV = 0;
	
	float op_water_inside_temp = 0;
	float op_water_inside_temp_CURR = 0;
	float op_water_inside_temp_PREV = 0;
	
	float op_water_outside_temp = 0;
	float op_water_outside_temp_CURR = 0;
	float op_water_outside_temp_PREV = 0;

	float op_living_room_temp = 0;
	float op_living_room_temp_CURR = 0;
	float op_living_room_temp_PREV = 0;

	string s_boiler_temp = "";
	string s_water_inside_temp = "";
	string s_water_outside_temp = "";
	string s_living_room_temp = "";
	string s_call_heat_status = "-";

	bool b_cool_down_state = false;
	bool b_testmode = false;

	// WiFi attachment
	const char* ssid = "Wilson.Net-2.4G";
	const char* password = "wilsonwebsite.com";
	char serverAddress[] = "192.168.0.45"; // server address
	int port = 44364;

	WiFiClient wifi;
	HttpClient client = HttpClient(wifi, serverAddress, port);
	int status = WL_IDLE_STATUS;

	// Time Variables
	const long utcOffsetInSeconds = 3600;
	char daysOfTheWeek[7][12] = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };

//  NTP Setup
	WiFiUDP ntpUDP;
	NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);
	int status = WL_IDLE_STATUS;

	// BEGIN CODE?

	setup
	{
	}

	loop
	{
	}


// Prototypes
	auto checkCallForHeat() -> bool;
	auto operationalCycle() -> void;
	auto updateDisplay() -> void;
	auto coolDownCycle(float) -> void;
	auto heatUpCycle(float) -> void;
	auto PB1_Fired() -> void;
	auto PB2_Fired() -> void;
	
 */

float tmpBlue = 0;
float tmpYellow = 0;
float tmpPurple = 0;

float waterTemp = 0;
float waterTempPrev = 0;

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

////class led_flash;
const char* ssid = "Wilson.Net-2.4G";
const char* password = "wilsonwebsite.com";

const long utcOffsetInSeconds = 3600;
char daysOfTheWeek[7][12] = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

//======================================================================================
//======================================================================================
//




// $$$
//======================================================================================
//======================================================================================
//
const int processorLED = 2; //o LED on MicroProcessor
const int callForHeat = 4; //i CALLFORHEAT

const int waterRelay = 26; //o WATERPUMP RELAY
const int burnerRelay = 27; //o BURNER RELAY
const int purpleRelay = 18; //o HAVENT DECIDED YET MAYBE IGNITER?
const int testRelay = 19;

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

MCP9600 evThermocouple;
MCP9600 waterThermocouple;
MCP9600 boilerThermocouple;

//------------------------------------------------------------------------------------------

Adafruit_SSD1306 waterDsp(-1);
Adafruit_SSD1306 boilerDsp(-1);

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






///------------------------------------------------------------------------------------------

// Setup
//------------------------------------------------------------------------------------------
//

void setup()
{
	Wire.begin();

	// Begin Thermocouples
	waterThermocouple.begin(0x060);
	boilerThermocouple.begin(0x061);
	evThermocouple.begin(0x067);

	waterDsp.begin(SSD1306_SWITCHCAPVCC, OLED1);
	waterDsp.clearDisplay();
	waterDsp.display();

	boilerDsp.begin(SSD1306_SWITCHCAPVCC, OLED2);
	boilerDsp.clearDisplay();
	boilerDsp.display();



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
	ArduinoOTA.setHostname("Endeavor");

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

					// OUTPUTS   pinModes
					// ----------------------------------------
					pinMode(processorLED, OUTPUT);
					digitalWrite(processorLED, LOW);

					// ????????????????????????
					pinMode(callForHeat, OUTPUT); // i PIN 4
					digitalWrite(callForHeat, LOW);

					pinMode(PIN19, OUTPUT); // o PIN 19
					digitalWrite(PIN19, LOW);

					pinMode(speaker, OUTPUT); // o PIN 25
					digitalWrite(speaker, LOW);

					pinMode(waterRelay, OUTPUT); // o PIN 26
					digitalWrite(waterRelay, HIGH);

					pinMode(burnerRelay, OUTPUT); // o PIN 27
					digitalWrite(burnerRelay, HIGH);

					pinMode(testRelay, OUTPUT);
					digitalWrite(testRelay, HIGH);

					pinMode(purpleRelay, OUTPUT); // o PIN 18
					digitalWrite(purpleRelay, HIGH);

					pinMode(PB1, INPUT); // i PIN 32
					pinMode(PB1, INPUT_PULLDOWN);


					pinMode(PB2, INPUT); // i PIN 33
					pinMode(PB2, INPUT_PULLDOWN);

					pinMode(PB3, INPUT); // i PIN 5
					pinMode(PB3, INPUT_PULLDOWN);

					Mode = stdby; // starting mode
					// ----------------------------------------
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
const long interval = 500;  // interval at which to blink (milliseconds)

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
			digitalWrite(waterRelay, !digitalRead(waterRelay));
			digitalWrite(burnerRelay, !digitalRead(burnerRelay));

		}

		//checkCallForHeat();
		//operationalCycle();
	}
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

	currentWaterTemp = waterThermocouple.getThermocoupleTemp(false);
	currentBoilerTemp = boilerThermocouple.getThermocoupleTemp(false);
	currentEvTemp = evThermocouple.getThermocoupleTemp(false);

	while (currentWaterTemp <= tHigh && !coolDown)
	{
		ArduinoOTA.handle();
		timeClient.update();

		digitalWrite(purpleRelay, HIGH);
		digitalWrite(burnerRelay, HIGH); // burner on
		digitalWrite(waterRelay, HIGH); // waterpump on

		currentWaterTemp = waterThermocouple.getThermocoupleTemp(false);
		updateDisplay();
	}

	digitalWrite(purpleRelay, LOW);
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

	currentWaterTemp = waterThermocouple.getThermocoupleTemp(false);
	currentBoilerTemp = boilerThermocouple.getThermocoupleTemp(false);
	currentEvTemp = evThermocouple.getThermocoupleTemp(false);

	while (currentWaterTemp >= tLow)
	{
		ArduinoOTA.handle();
		timeClient.update();

		digitalWrite(purpleRelay, LOW);
		digitalWrite(burnerRelay, LOW); // burner off
		digitalWrite(waterRelay, LOW); // waterpump off

		currentWaterTemp = waterThermocouple.getThermocoupleTemp(false);
		updateDisplay();
	}
	coolDown = false;
}


auto updateDisplay() -> void
{
	ArduinoOTA.handle();
	timeClient.update();


	// (26°C × 9/5) + 32 = 78.8°F 
	waterTemp = waterThermocouple.getThermocoupleTemp(false);
	boilerTemp = boilerThermocouple.getThermocoupleTemp(false);
	envTemp = evThermocouple.getThermocoupleTemp(false);

	const String sWaterTemp = String(waterTemp);
	const String sBoilerTemp = String(boilerTemp);
	const String sEVTemp = String(envTemp - 20);


	String waterTitle = "M->" + Mode + " F->" + currentFunction;


	waterDsp.setTextSize(1);
	waterDsp.clearDisplay();
	waterDsp.setTextColor(WHITE);

	waterDsp.setCursor(11, 1);
	waterDsp.println(waterTitle);

	waterDsp.setCursor(11, 11);
	waterDsp.println("W->" + sWaterTemp);

	waterDsp.setCursor(11, 22);
	waterDsp.println("B->" + sBoilerTemp);

	waterDsp.display();

	// ==============================================

//	String timeDsp = " " + String(timeClient.getHours()) + ":" + String(timeClient.getMinutes()) + ":" + String(timeClient.getSeconds());



	boilerDsp.setTextSize(1);
	boilerDsp.clearDisplay();
	boilerDsp.setTextColor(WHITE);

	boilerDsp.setCursor(11, 1);
	boilerDsp.println("E->" + sEVTemp);

	boilerDsp.setCursor(11, 11);
	boilerDsp.println("HT->" + String(int(highTemperature)));

	boilerDsp.setCursor(11, 22);
	boilerDsp.println("L->" + String(int(lowTemperature)));

	boilerDsp.display();

	// ==============================================
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
	digitalWrite(purpleRelay, HIGH);
}

auto PB2_Fired() -> void
{
	digitalWrite(burnerRelay, LOW);
	digitalWrite(purpleRelay, LOW);
}
