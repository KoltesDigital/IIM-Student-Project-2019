#include <Adafruit_NeoPixel.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <SocketIoClient.h>
#include <SparkFun_VL53L1X.h>
#include <Wire.h>

// CONFIGURATION START

// Delay between measurements.
static constexpr int distanceMeasurementPeriod = 100; // milliseconds

// Below this distance, a hand is detected.
static constexpr int handDetectionThreshold = 120; // millimeters

// LED strip data signal.
static constexpr int ledPin = D5;

// Minimum value for light wave.
static constexpr int lightIntensityMin = 60; // [0, 255]

// Maximum value for light wave.
static constexpr int lightIntensityMax = 255; // [0, 255]

// Delay between led updates.
static constexpr int lightUpdatePeriod = 10; // milliseconds

// Server hostname or IP address.
static constexpr const char *serverHost = "localhost";

static constexpr int serverPort = 3000;

// WiFi network name.
static constexpr const char *wiFiSsid = "WIFI";

// WiFi password.
static constexpr const char *wiFiPassword = "WIFI";

// CONFIGURATION END

static constexpr int ledCount = 1;
static constexpr uint8_t distanceSensorAddress = 0x29;

static SFEVL53L1X distanceSensor;
static Adafruit_NeoPixel ledStrip(ledCount, ledPin, NEO_GRB + NEO_KHZ800);
static SocketIoClient socket;

static bool stateHandDetected = false;
static bool stateLight = false;

static int distanceMeasurementTimer = 0;

static int lightIntensity = 0;
static bool lightDesc = false;
static int lightUpdateTimer = 0;

static void connectHandler(const char *payload, size_t)
{
	socket.emit(stateHandDetected ? "handDetected" : "handLost");
}

static void setLightHandler(const char *payload, size_t)
{
	Serial.println("Got message for setting light.");

	constexpr int capacity = JSON_OBJECT_SIZE(1) + JSON_ARRAY_SIZE(3);
	StaticJsonDocument<capacity> doc;
	auto err = deserializeJson(doc, payload);

	if (err)
	{
		Serial.print("Deserialization failed: ");
		Serial.println(err.c_str());
	}
	else
	{
		if (!doc.is<bool>())
		{
			Serial.println("Invalid data.");
		}
		else
		{
			stateLight = doc.as<bool>();

			Serial.print("Set light: ");
			Serial.print(stateLight);
			Serial.println(".");

			if (stateLight)
			{
				lightDesc = false;
			}
		}
	}
}

void setup()
{
	ESP.wdtDisable();

	Serial.begin(9600);
	Serial.println("Linko");

	// Distance sensor setup.

	Wire.begin(D2, D1);
	Wire.setClock(100000);

	distanceSensor.setI2CAddress(distanceSensorAddress);
	if (distanceSensor.begin())
	{
		Serial.println("Distance sensor connected.");
	}

	// WiFi setup.

	WiFi.begin(wiFiSsid, wiFiPassword);

	Serial.print("Connecting to WiFi");
	while (WiFi.status() != WL_CONNECTED)
	{
		delay(100);
		Serial.print(".");
	}
	Serial.println();

	Serial.print("Connected, IP address: ");
	Serial.println(WiFi.localIP());

	// Web socket setup.

	socket.on("connect", connectHandler);
	socket.on("setLight", setLightHandler);
	socket.begin(serverHost, serverPort);

	// LED strip setup.

	ledStrip.begin();
	ledStrip.show();

	Serial.println("Setup done!");
}

void loop()
{
	socket.loop();

	if (distanceMeasurementTimer == 0)
	{
		distanceMeasurementTimer = distanceMeasurementPeriod;

		distanceSensor.startRanging();
		int distance = distanceSensor.getDistance();
		distanceSensor.stopRanging();

		int signalRate = distanceSensor.getSignalRate();

		byte rangeStatus = distanceSensor.getRangeStatus();

		Serial.print("Distance: ");
		Serial.print(distance);
		Serial.print("mm ");

		Serial.print("\tSignal rate: ");
		Serial.print(signalRate);

		Serial.print("\tRange status: ");
		switch (rangeStatus)
		{
		case 0:
			Serial.print("Good");
			break;
		case 1:
			Serial.print("Signal fail");
			break;
		case 2:
			Serial.print("Sigma fail");
			break;
		case 7:
			Serial.print("Wrapped target fail");
			break;
		default:
			Serial.print("Unknown: ");
			Serial.print(rangeStatus);
			break;
		}

		Serial.println();

		auto newHandDetected = (distance <= handDetectionThreshold);

		if (newHandDetected && !stateHandDetected)
		{
			Serial.println("Hand detected.");
			socket.emit("handDetected");
		}

		if (!newHandDetected && stateHandDetected)
		{
			Serial.println("Hand lost.");
			socket.emit("handLost");
		}

		stateHandDetected = newHandDetected;
	}
	else
	{
		--distanceMeasurementTimer;
	}

	if (lightUpdateTimer == 0)
	{
		lightUpdateTimer = lightUpdatePeriod;

		if (stateLight)
		{
			if (lightDesc)
			{
				--lightIntensity;
				if (lightIntensity <= lightIntensityMin)
				{
					lightDesc = false;
				}
			}
			else
			{
				++lightIntensity;
				if (lightIntensity >= lightIntensityMax)
				{
					lightDesc = true;
				}
			}
		}
		else
		{
			if (lightIntensity > 0)
			{
				--lightIntensity;
			}
		}
	}
	else
	{
		--lightUpdateTimer;
	}

	ledStrip.setPixelColor(0, lightIntensity, 0, 0);
	ledStrip.show();

	delay(1);
	ESP.wdtFeed();
}
