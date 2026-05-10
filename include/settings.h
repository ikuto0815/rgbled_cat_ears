#pragma once

#include <inttypes.h>
#include <Preferences.h>

extern uint8_t max_brightness;
extern uint32_t color;
extern uint8_t CurrentMode;

extern Preferences preferences;

void init_settings(void);
void save_settings(void);
