/*
 * Transfer from Mel-Frequency to Leds Effect
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */

#include "VisualEffect.h"

mode modes[] = {
MODE(off),
MODE(scroll_ears_2),
MODE(scroll_ears),
MODE(scroll),
MODE(solid),
MODE(solid_hue),
MODE(rainbow),
MODE(energy),
MODE(spectrum),
};

int VisualEffect::mode_count(void)
{
	return sizeof(modes) / sizeof(modes[0]);
}

int VisualEffect::mode_index(char *mode)
{
	for (int i = 0; i < VisualEffect::mode_count(); i++)
		if (!modes[i].name.compareTo(mode))
			return i;

	return 0;
}

void VisualEffect::visualize(float *_mel_data)
{
	mel_data = _mel_data;
	if (current_mode >= mode_count())
		return;

	(this->*modes[current_mode].visualize)();
}

VisualEffect::VisualEffect(uint16_t mel_num, uint16_t leds_num)
{
	_mel_num = mel_num;
	_leds_num = leds_num;
	_physic_leds = new CRGB[_leds_num];
	_leds[0] = new uint8_t[_leds_num / 2];
	_leds[1] = new uint8_t[_leds_num / 2];
	_leds[2] = new uint8_t[_leds_num / 2];
	_gain = new ExpFilter(_mel_num, 0.001, 0.99);
	_p_filt_r = new ExpFilter(leds_num / 2, 0.05, 0.99);
	_p_filt_g = new ExpFilter(leds_num / 2, 0.05, 0.99);
	_p_filt_b = new ExpFilter(leds_num / 2, 0.05, 0.99);
	_common_mode = new ExpFilter(_leds_num / 2, 0.99, 0.01);
	_r_filt = new ExpFilter(_leds_num / 2, 0.2, 0.99);
	_g_filt = new ExpFilter(_leds_num / 2, 0.04, 0.3);
	_b_filt = new ExpFilter(_leds_num / 2, 0.1, 0.5);
	_gauss02 = new gaussian_filter1d(0.2);
	_gauss40 = new gaussian_filter1d(4.0);

	_spectrum = new float[leds_num / 2];
	_prev_spectrum = new float[leds_num / 2];

	FastLED.addLeds<NEOPIXEL, led_strip_data_pin>(_physic_leds, led_strip_len);
}

VisualEffect::~VisualEffect()
{
	delete[] _leds[0];
	delete[] _leds[1];
	delete[] _leds[2];
	delete _gain;
	delete _p_filt_r;
	delete _p_filt_g;
	delete _p_filt_b;
	delete _common_mode;
	delete _r_filt;
	delete _g_filt;
	delete _b_filt;
	delete _gauss02;
	delete _gauss40;
	delete[] _spectrum;
	delete[] _prev_spectrum;
}

#define SET_PHYS_LED(phy, i) \
	_physic_leds[phy].r = _leds[0][i]; \
	_physic_leds[phy].g = _leds[1][i]; \
	_physic_leds[phy].b = _leds[2][i];

void VisualEffect::mirror(void)
{
	for (int i = 0; i < _leds_num / 2; i++) {
		SET_PHYS_LED(_leds_num / 2 + i, i);
		SET_PHYS_LED(_leds_num / 2 - i - 1, i);
	}
}

void VisualEffect::visualize_scroll_virt(uint8_t led_num)
{
	float rr, gg, bb;
	
	for (int i = 0; i < _mel_num; i++)
		mel_data[i] = mel_data[i] * mel_data[i];

	_gain->update0(mel_data);
	for (int i = 0; i < _mel_num; i++)
		if (_gain->value()[i] > 0.0)
			mel_data[i] /= (_gain->value()[i]);

	rr = 0.0;
	gg = 0.0;
	bb = 0.0;
	for (int i = 0; i < _mel_num; i++)
		if (i < _mel_num / 3)
			rr = std::max(rr, mel_data[i]);
		else if (i > _mel_num * 2 / 3)
			bb = std::max(bb, mel_data[i]);
		else
			gg = std::max(gg, mel_data[i]);

	for (int i = led_num - 1; i > 0; i--) {
		_leds[0][i] = (_leds[0][i - 1] == 0) ? 0 : _leds[0][i - 1] - 1;
		_leds[1][i] = (_leds[1][i - 1] == 0) ? 0 : _leds[1][i - 1] - 1;
		_leds[2][i] = (_leds[2][i - 1] == 0) ? 0 : _leds[2][i - 1] - 1;
	}
	_gauss02->process(_leds[0], led_num);
	_gauss02->process(_leds[1], led_num);
	_gauss02->process(_leds[2], led_num);

	_leds[0][0] = max_brightness * rr;
	_leds[1][0] = max_brightness * gg;
	_leds[2][0] = max_brightness * bb;
}

void VisualEffect::visualize_scroll_generic(scroll_line *lines, uint8_t num_lines)
{
	visualize_scroll_virt(_leds_num / 2); //len of longest line would be enought though..

	for (int i = 0; i < _leds_num / 2; i++) {
		for (int j = 0; j < num_lines; j++) {
			if (lines[j].len > i) {
				SET_PHYS_LED(lines[j].start + (i * lines[j].dir), i);
			}
		}
	}
}

/* like scroll but only add colorto the ears, starting at the tips */
void VisualEffect::visualize_scroll_ears(void)
{
	static scroll_line lines[4] = {
	{.start = 14, .len = 8, .dir = -1},
	{.start = 15, .len = 8, .dir = 1},
	{.start = 35, .len = 8, .dir = -1},
	{.start = 36, .len = 8, .dir = 1}};

	visualize_solid_hue();
	visualize_scroll_generic(lines, 4);
}

/* like scroll_ears but continue across the headband */
void VisualEffect::visualize_scroll_ears_2(void)
{
	static scroll_line lines[4] = {
	{.start = 14, .len = 15, .dir = -1},
	{.start = 15, .len = 11, .dir = 1},
	{.start = 35, .len = 11, .dir = -1},
	{.start = 36, .len = 16, .dir = 1}};
	visualize_scroll_generic(lines, 4);
}

/* scroll color based on current audio frequencies */
void VisualEffect::visualize_scroll(void)
{
	static scroll_line lines[2] = {
	{.start = _leds_num / 2 - 1, .len = _leds_num / 2, .dir = -1},
	{.start = _leds_num / 2, .len = _leds_num / 2, .dir = 1}};
	visualize_scroll_generic(lines, 2);
}

/* flashes for every beat depending on volume */
void VisualEffect::visualize_energy(void)
{
	float rr, gg, bb;
	int ri, gi, bi;

	_gain->update0(mel_data);
	for (int i = 0; i < _mel_num; i++)
		if (_gain->value()[i] > 0.0)
			mel_data[i] /= (_gain->value()[i]);

	rr = 0.0;
	gg = 0.0;
	bb = 0.0;
	ri = bi = gi = 0;
	for (int i = 0; i < _mel_num; i++)
		if (i < _mel_num / 3) {
			ri++;
			rr += mel_data[i];
		} else if (i > _mel_num * 2 / 3) {
			bi++;
			bb += mel_data[i];
		} else {
			gi++;
			gg += mel_data[i];
		}
	rr = rr * _leds_num / 2 / ri;
	bb = bb * _leds_num / 2 / bi;
	gg = gg * _leds_num / 2 / gi;

	for (int i = 0; i < _leds_num / 2; i++) {
		_leds[0][i] = (i + 1 > rr) ? 0 : max_brightness;
		_leds[1][i] = (i + 1 > gg) ? 0 : max_brightness;
		_leds[2][i] = (i + 1 > bb) ? 0 : max_brightness;
	}

	_p_filt_r->update(_leds[0]);
	_p_filt_g->update(_leds[1]);
	_p_filt_b->update(_leds[2]);
	_gauss40->process(_leds[0], _leds_num / 2);
	_gauss40->process(_leds[1], _leds_num / 2);
	_gauss40->process(_leds[2], _leds_num / 2);

	mirror();
}

/* spectrum of active frequencies */
void VisualEffect::visualize_spectrum(void)
{
	float one_unit = 1.0 / (_leds_num / 2 - 1);
	int j = 1;
	_spectrum[0] = mel_data[0];
	_spectrum[_leds_num / 2 - 1] = mel_data[_mel_num - 1];

	for (int i = 1; i < _mel_num; i++) {
		float t;
		while ((t = one_unit * j * (_mel_num - 1)) < i) {
			//_spectrum[j]=(0.5-0.5*cos(PI*((i-t)*mel_data[i-1]+(t-i+1)*mel_data[i])));
			_spectrum[j] = ((i - t) * mel_data[i - 1] + (t - i + 1) * mel_data[i]);
			j++;
		}
	}
	_common_mode->update0(_spectrum);

	for (int i = 0; i < _leds_num / 2; i++) {
		_leds[0][i] = (_spectrum[i] - _common_mode->value()[i]) * max_brightness;
		_leds[1][i] = fabs(_spectrum[i] - _prev_spectrum[i]) * max_brightness;
		_leds[2][i] = _spectrum[i] * max_brightness;
		_prev_spectrum[i] = _spectrum[i];
	}
	_r_filt->update(_leds[0]);
	_g_filt->update(_leds[1]);
	_b_filt->update(_leds[2]);
	_gauss02->process(_leds[0], _leds_num / 2);
	_gauss02->process(_leds[1], _leds_num / 2);
	_gauss02->process(_leds[2], _leds_num / 2);
	mirror();
}

void VisualEffect::visualize_solid(void)
{
	CRGB rgb;
	rgb.r = color & 0xFF;
	rgb.g = (color >> 8) & 0xFF;
	rgb.b = (color >> 16) & 0xFF;
	fill_solid(_physic_leds, _leds_num, rgb);
}

void VisualEffect::visualize_solid_hue(void)
{
	CHSV hsv;
	hsv.hue = gHue;
	hsv.val = max_brightness;
	hsv.sat = 240;
	fill_solid(_physic_leds, _leds_num, hsv);
	gHue++;
}

void VisualEffect::visualize_off(void)
{
	fill_solid(_physic_leds, _leds_num, CRGB::Black);
}

void VisualEffect::visualize_rainbow(void)
{
	fill_rainbow(_physic_leds, _leds_num, gHue, 7);
	gHue++;
}

void VisualEffect::set_mode(uint8_t i)
{
	current_mode = i % (sizeof(modes) / sizeof(*modes));
}
