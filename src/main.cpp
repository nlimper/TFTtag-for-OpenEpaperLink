#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <WiFi.h>

#include "main.h"
#include "commstructs.h"
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
	spr.pushImage(0, 0, 128, 296, (uint16_t*)buffer);
	spr.setBitmapColor(TFT_WHITE, 0);
	spr.pushRotated(90, 0);
    if (dataType == 0x21) {
        //has red
		spr.pushImage(0, 0, 128, 296, (uint16_t *)(buffer + 4736));
		spr.setBitmapColor(TFT_RED, 0);
		spr.pushRotated(90, 0);
	}
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

	init_udp();

	tft.fillScreen(tft.color565(30, 30, 30));
	tft.drawRect(11, 55, 298, 130, tft.color565(80, 80, 80));

	xTaskCreate(advertiseTagTask, "Tag alive", 6000, NULL, 2, NULL);
}

void loop() {
	vTaskDelay(10000 / portTICK_PERIOD_MS);
}

