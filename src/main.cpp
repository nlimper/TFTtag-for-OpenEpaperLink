#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <Thermal_Printer.h>
#include <WiFi.h>

#include "commstructs.h"
#include "main.h"
#include "udp.h"

TFT_eSPI tft = TFT_eSPI();
const int backlightPin = 16;

const char *ssid = "***";
const char *password = "***";

void advertiseTagTask(void *parameter) {
	sendAvail(0xFC);
	while (true) {
		vTaskDelay(60000 / portTICK_PERIOD_MS);
		sendAvail(0);
	}
}

void drawImage(uint8_t *buffer, uint8_t dataType) {
	TFT_eSprite spr = TFT_eSprite(&tft);
	tft.fillRect(12, 56, 296, 128, TFT_BLACK);
	tft.setPivot(tft.width() / 2, tft.height() / 2);
	spr.setColorDepth(1);
	spr.createSprite(128, 296);
	spr.setPivot(spr.width() / 2, spr.height() / 2);
	spr.pushImage(0, 0, 128, 296, (uint16_t *)buffer);
	spr.setBitmapColor(TFT_WHITE, 0);
	spr.pushRotated(90, 0);
	if (dataType == 0x21) {
		// has red
		spr.pushImage(0, 0, 128, 296, (uint16_t *)(buffer + 4736));
		spr.setBitmapColor(TFT_RED, 0);
		spr.pushRotated(90, 0);
	}

	TFT_eSprite printbuf = TFT_eSprite(&tft);
	printbuf.setColorDepth(1);
	printbuf.createSprite(384, 130);
	printbuf.fillRect(0, 0, 384, 130, 0);
	printbuf.setPivot(printbuf.width() / 2, printbuf.height() / 2);
	spr.setBitmapColor(TFT_WHITE, TFT_BLACK);
	spr.pushImage(0, 0, 128, 296, (uint16_t *)buffer);
	spr.pushRotated(&printbuf, 90, 0);
	if (dataType == 0x21) {
		spr.pushImage(0, 0, 128, 296, (uint16_t *)(buffer + 4736));
		spr.pushRotated(&printbuf, 90, 0);
	}
	printbuf.drawRect(43, 0, 298, 130, 1);
	tpSetBackBuffer((uint8_t *)printbuf.getPointer(), 384, 130);
	tpPrintBuffer();
	tpPrint((char *)" \r\n \r\n \r\n");
}

void setup() {
	Serial.begin(115200);

	tft.init();
	tft.setRotation(1);
	pinMode(backlightPin, OUTPUT);
	analogWrite(backlightPin, 200);

	tft.fillScreen(TFT_BLACK);
	tft.setCursor(0, 0, 2);
	tft.setTextColor(TFT_WHITE);

	tft.println(" Init\n");

	WiFi.begin(ssid, password);
	tft.print(" Wifi");

	Serial.print("Connecting to WiFi");
	while (WiFi.status() != WL_CONNECTED) {
		tft.print(".");
		delay(500);
	}
	tft.println("connected!");

	Serial.println((char *)"Scanning for BLE printer");
	tft.println("Scanning for BLE printer");
	if (tpScan()) {
		Serial.println((char *)"Found a printer!, connecting...");
		tft.println("Found a printer!, connecting...");
		if (tpConnect()) {
			Serial.println((char *)"printer connected!");
			tft.println("printer connected!");
		}
	} else {
		Serial.println((char *)"Didn't find a printer :( ");
		tft.println("Didn't find a printer :(");
	}
	vTaskDelay(1000 / portTICK_PERIOD_MS);

	init_udp();

	tft.fillScreen(tft.color565(30, 30, 30));
	tft.drawRect(11, 55, 298, 130, tft.color565(80, 80, 80));

	xTaskCreate(advertiseTagTask, "Tag alive", 6000, NULL, 2, NULL);
}

void loop() {
	vTaskDelay(10000 / portTICK_PERIOD_MS);
}
