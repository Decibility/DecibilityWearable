#pragma once

#include "led_strip.h"

// Constants for Decibility LED Strips
#define FREQ_STRIP_GPIO 7
#define FREQ_STRIP_LEDS 3
#define FREQ_STRIP_RES 1e7

#define VOLUME_STRIP_GPIO 9
#define VOLUME_STRIP_LEDS 3
#define VOLUME_STRIP_RES 1e7

#define DOWN_ARROW 0
#define CENTER 1
#define UP_ARROW 2

#define NUM_COLORS 3

// Generic LED Strip Configuration Function
void led_strip_init(led_strip_handle_t *led_strip, int gpio, uint32_t num_leds, uint32_t resolution_hz);

// LED Setup for Decibility
void decibility_led_init(led_strip_handle_t *freq_led_strip, led_strip_handle_t *volume_led_strip);