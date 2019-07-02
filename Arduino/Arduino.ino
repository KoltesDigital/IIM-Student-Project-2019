#include <Adafruit_NeoPixel.h>

#include <ArduinoJson.h>

#include <ESP8266WiFi.h>

#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>

#include <SocketIoClient.h>

#include <Wire.h>
#include <SparkFun_VL53L1X.h>

static WiFiManager wifiManager;
static SocketIoClient webSocket;

static constexpr int ledPin = D5;
static constexpr int ledCount = 1;
static Adafruit_NeoPixel ledStrip(ledCount, ledPin, NEO_GRB + NEO_KHZ800);

static SFEVL53L1X distanceSensor;

static void setColorHandler(const char *payload, size_t)
{
	Serial.println("got message");

	constexpr int capacity = JSON_OBJECT_SIZE(1) + JSON_ARRAY_SIZE(3);
	StaticJsonDocument<capacity> doc;
	auto err = deserializeJson(doc, payload);

	if (err)
	{
		Serial.print("deserializeJson() failed with code ");
		Serial.println(err.c_str());
	}
	else
	{
		auto colorNode = doc.as<JsonArray>();
		if (colorNode.size() != 3)
		{
			Serial.println("Invalid color.");
		}
		else
		{
			auto r = colorNode[0].as<int>();
			auto g = colorNode[1].as<int>();
			auto b = colorNode[2].as<int>();

			// Mise à jour de la couleur des leds.
			ledStrip.setPixelColor(0, r, g, b);
			// ...

			ledStrip.show();
		}
	}

	Serial.println();
}

void setup()
{
	Serial.begin(9600);

	ledStrip.begin();
	ledStrip.show();

	Wire.begin();
	// Wire.setClock(400000); // use 400 kHz I2C

	if (distanceSensor.begin())
	{
		Serial.println("Distance sensor connected!");
	}

	// Nom et mot de passe du réseau WiFi.
	wifiManager.autoConnect("Linko", "SuperPassword");

	webSocket.on("setColor", setColorHandler);

	// Remplacer localhost par l'adresse IP du serveur.
	webSocket.begin("http://localhost:3000/");
}

void loop()
{
	webSocket.loop();

	distanceSensor.startRanging();
	int distance = distanceSensor.getDistance();
	distanceSensor.stopRanging();

	Serial.println(distance);
}
