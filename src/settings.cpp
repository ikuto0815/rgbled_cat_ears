#include <inttypes.h>
#include "settings.h"

uint8_t max_brightness = 255;
uint32_t color = 0xffff0000;
uint8_t CurrentMode = 1;

Preferences preferences;

void init_settings(void)
{
	preferences.begin("rgbled_cat_ears", false);

	max_brightness = preferences.getUChar("max_brightness", 255);
	color = preferences.getUInt("color", 0xffff0000);
	CurrentMode = preferences.getUChar("mode", 1);
}

void save_settings(void)
{
	Serial.println("saving settings");
	preferences.putUChar("max_brightness", max_brightness);
	preferences.putUInt("color", color);
	preferences.putUChar("mode", CurrentMode);
}
