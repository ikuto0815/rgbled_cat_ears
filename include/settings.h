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

static const uint16_t BUFFER_SIZE = 1024; /* samples per read, must be a power of 2 */
static const uint8_t N_ROLLING_HISTORY = 2; /* number of buffers to process each time, must be a power of 2 */
static const uint16_t SAMPLE_RATE = 44100; /* audio sample rate */
static const uint16_t N_MEL_BIN = 18; /* number of mel frequency channels */
extern float fft_min_frequency; /* min audio frequency to process */
extern float fft_max_frequency; /* max audio frequency to process, must be < SEMPLE_RATE/2 */
extern float fft_min_volume_threshold; /* minimum audio volume to process */

extern Preferences preferences;

void init_settings(void);
void save_settings(void);
