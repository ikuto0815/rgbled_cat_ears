#pragma once

#include <inttypes.h>
#include <Preferences.h>

extern uint8_t max_brightness;
extern uint32_t color;
extern uint8_t CurrentMode;

static const int led_strip_data_pin = 2; /* LED strip data pin, has to be const because of fastled lib implementation... */
extern uint16_t led_strip_len; /* LED strip length */
extern uint16_t led_offset_left; /* number of LEDs until the left ear */
extern uint16_t led_ear_len; /* number of LEDs per side of the ears */
extern uint16_t led_middle_gap; /* number of LEDs between the ears */

extern Preferences preferences;

void init_settings(void);
void save_settings(void);
