#pragma once

#include <cmath>
#include <inttypes.h>
#include <FastLED.h>

typedef struct scroll_line {
	int start;
	int len;
	int dir;
} scroll_line;

class VisualEffect {
	private:
		class ExpFilter *_gain, *_p_filt_r, *_p_filt_g, *_p_filt_b, *_common_mode, *_r_filt, *_g_filt, *_b_filt;
		class gaussian_filter1d *_gauss02, *_gauss40;
		uint8_t *_leds[3];
		uint16_t _mel_num, _leds_num;
		void mirror(void);
		float *_spectrum, *_prev_spectrum;
		uint8_t gHue = 0;
		CRGB *_physic_leds;
		float *mel_data;
		uint8_t current_mode = 1;

		void visualize_scroll_virt(uint8_t led_num);
	public:
		VisualEffect(uint16_t mel_num, uint16_t leds_num, CRGB *physic_leds);
		void visualize_scroll(void);
		void visualize_scroll_ears(void);
		void visualize_scroll_ears_2(void);
		void visualize_scroll_generic(scroll_line *lines, uint8_t num_lines);
		void visualize_energy(void);
		void visualize_spectrum(void);
		void visualize_solid(void);
		void visualize_solid_hue(void);
		void visualize_off(void);
		void visualize_rainbow(void);
		void visualize(float *_mel_data);

		void set_mode(uint8_t i);
		~VisualEffect();
};

#define MODE(name) \
	{ #name, &VisualEffect::visualize_ ## name }

typedef struct mode {
	String name;
	void (VisualEffect::*visualize)(void);
} mode;

extern mode modes[];
int mode_count(void);
int mode_index(char *mode);
