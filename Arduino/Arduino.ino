#include <Adafruit_NeoPixel.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <SocketIoClient.h>
#include <SparkFun_VL53L1X.h>
#include <Wire.h>

// CONFIGURATION START

// Period between measurements.
static constexpr int distanceSensorDelay = 100; // milliseconds

// Below this distance, a hand is detected.
static constexpr int handDetectionThreshold = 120; // millimeters

// LED strip data signal.
static constexpr int ledPin = D5;

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
static SocketIoClient webSocket;

static void setColorHandler(const char *payload, size_t)
{
	Serial.println("Got message for setting color.");

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
		auto colorNode = doc.as<JsonArray>();
		if (colorNode.size() != 3)
		{
			Serial.println("Invalid data.");
		}
		else
		{
			auto r = colorNode[0].as<int>();
			auto g = colorNode[1].as<int>();
			auto b = colorNode[2].as<int>();

			Serial.print("Set color to: ");
			Serial.print(r);
			Serial.print(", ");
			Serial.print(g);
			Serial.print(", ");
			Serial.print(b);
			Serial.println(".");

			// Mise à jour de la couleur des leds.
			ledStrip.setPixelColor(0, r, g, b);
			ledStrip.show();
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

	webSocket.on("setColor", setColorHandler);
	webSocket.begin(serverHost, serverPort);

	// LED strip setup.

	ledStrip.begin();
	ledStrip.show();

	Serial.println("Setup done!");
}

static int sensorTimer = 0;
static bool handDetected = false;

void loop()
{
	webSocket.loop();

	if (sensorTimer == 0)
	{
		sensorTimer = distanceSensorDelay;

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

		if (newHandDetected && !handDetected)
		{
			Serial.println("Hand detected.");
			webSocket.emit("handDetected");
		}

		if (!newHandDetected && handDetected)
		{
			Serial.println("Hand lost.");
			webSocket.emit("handLost");
		}

		handDetected = newHandDetected;
	}
	else
	{
		--sensorTimer;
	}

	delay(1);
	ESP.wdtFeed();
}
