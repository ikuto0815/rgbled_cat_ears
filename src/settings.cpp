#include <inttypes.h>
#include "settings.h"

uint8_t max_brightness = 255;
uint32_t color = 0xffff0000;
uint8_t CurrentMode = 1;

uint16_t led_strip_len = 60;
uint16_t led_offset_left = 11;
uint16_t led_ear_len = 8;
uint16_t led_middle_gap = 6;

Preferences preferences;

void init_settings(void)
{
	preferences.begin("rgbled_cat_ears", false);

	max_brightness = preferences.getUChar("max_brightness", max_brightness);
	color = preferences.getUInt("color", color);
	CurrentMode = preferences.getUChar("mode", CurrentMode);

	led_strip_len = preferences.getUShort("led_strip_len", led_strip_len);
	led_offset_left = preferences.getUShort("led_offset_left", led_offset_left);
	led_ear_len = preferences.getUShort("led_ear_len", led_ear_len);
	led_middle_gap = preferences.getUShort("led_middle_gap", led_middle_gap);
}

void save_settings(void)
{
	Serial.println("saving settings");
	preferences.putUChar("max_brightness", max_brightness);
	preferences.putUInt("color", color);
	preferences.putUChar("mode", CurrentMode);

	preferences.putUShort("led_strip_len", led_strip_len);
	preferences.putUShort("led_offset_left", led_offset_left);
	preferences.putUShort("led_ear_len", led_ear_len);
	preferences.putUShort("led_middle_gap", led_middle_gap);
}
