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
}

void save_settings(void)
{
	Serial.println("saving settings");
	preferences.putUChar("max_brightness", max_brightness);
	preferences.putUInt("color", color);
	preferences.putUChar("mode", CurrentMode);
}
