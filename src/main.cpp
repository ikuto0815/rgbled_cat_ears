/* The main program of 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */

#include <FastLED.h>
#include "FFT.h"
#include "VisualEffect.h"

#include "ble.h"
#include "settings.h"

#include "ESP_I2S.h"
const uint8_t I2S_SCK = 12; /* mic SCK pin */
const uint8_t I2S_WS = 13; /* mic WS pin */
const uint8_t I2S_DIN = 1; /* mic SD pin */

const uint8_t LR_PIN = 44; /* Pull low to mic outputs on the left channel */

const uint16_t BUFFER_SIZE = 1024; /* samples per read, must be a power of 2 */
const uint8_t N_ROLLING_HISTORY = 2; /* number of buffers to process each time, must be a power of 2 */
const uint16_t SAMPLE_RATE = 44100; /* audio sample rate */
const uint16_t N_PIXELS = 52; /* LED strip length */
const uint16_t N_MEL_BIN = 18; /* number of mel frequency channels */
const float MIN_FREQUENCY = 200; /* min audio frequency to process */
const float MAX_FREQUENCY = 12000; /* max audio frequency to process, must be < SEMPLE_RATE/2 */
const float MIN_VOLUME_THRESHOLD = 0.0003; /* minimum audio volume to process */

const uint8_t BUTTON = 0; /* Boot button present on basically every ESP32-S3 board */

const int LED_STRIP_DATA_PIN = 2; /* LED strip data pin */

float y_data[BUFFER_SIZE * N_ROLLING_HISTORY];
class FFT fft(BUFFER_SIZE *N_ROLLING_HISTORY, N_MEL_BIN, MIN_FREQUENCY, MAX_FREQUENCY, SAMPLE_RATE,
	      MIN_VOLUME_THRESHOLD);
CRGB physic_leds[N_PIXELS];
class VisualEffect effect(N_MEL_BIN, N_PIXELS, physic_leds);

I2SClass i2s;
Ble *ble;


void setup()
{
	init_settings();

	pinMode(LR_PIN, OUTPUT);
	digitalWrite(LR_PIN, LOW);

	pinMode(BUTTON, INPUT);

	FastLED.addLeds<NEOPIXEL, LED_STRIP_DATA_PIN>(physic_leds, N_PIXELS);
	//FastLED.addLeds<APA102, LED_STRIP_DATA_PIN, LED_STRIP_CLOCK_PIN, GRB>(physic_leds, N_PIXELS);
	Serial.begin(115200);
	// Set up the pins used for audio input
	i2s.setPins(I2S_SCK, I2S_WS, -1, I2S_DIN);

	// Initialize the I2S bus in standard mode
	if (!i2s.begin(I2S_MODE_STD, SAMPLE_RATE, I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO, I2S_STD_SLOT_LEFT)) {
		Serial.println("Failed to initialize I2S bus!");
		return;
	}

	ble = new Ble();
}

void loop()
{
	static float mel_data[N_MEL_BIN];

	for (int i = 0; i < N_ROLLING_HISTORY - 1; i++)
		memcpy(y_data + i * BUFFER_SIZE, y_data + (i + 1) * BUFFER_SIZE, sizeof(float) * BUFFER_SIZE);

	int16_t l[BUFFER_SIZE];

	unsigned int read_num;
	i2s.readBytes((char *)l, BUFFER_SIZE * 2);
	//i2s_read(I2S_NUM_0, l, BUFFER_SIZE * 2, &read_num, portMAX_DELAY);

	for (int i = 0; i < BUFFER_SIZE; i++) {
		y_data[BUFFER_SIZE * (N_ROLLING_HISTORY - 1) + i] = l[i] / 32768.0;

		/*
		 * This should output the current time(in millisececonds) every second.
		 * The output frequency larger than one second greatly, means the CPU is overload.
		 */
#if 0
		static uint32_t ii = 0;
		ii++;
		if (ii % SAMPLE_RATE == 0)
			Serial.printf("%ld\n", millis());
#endif
	}

	fft.t2mel(y_data, mel_data);
	effect.set_mode(CurrentMode);
	effect.visualize(mel_data);
	FastLED.show();

	static uint32_t oldtime = 0;
	uint16_t button_state = digitalRead(BUTTON);
	if ((button_state == 0) && (millis() - oldtime > 1000)) {
		oldtime = millis();
		CurrentMode = (CurrentMode + 1) % mode_count();
		effect.set_mode(CurrentMode);
	} else if (button_state != 0)
		oldtime = 0;

	yield();
}
