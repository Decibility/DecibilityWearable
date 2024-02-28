#pragma once

#include <stdint.h>

////***** Constants *****//
#define LED_UPDATE_PERIOD_MS 10
#define AUDIO_SEND_FREQUENCY_HZ 4
#define AUDIO_SEND_DELAY_MS (int)((double)1000 / AUDIO_SEND_FREQUENCY_HZ)

////***** External Variables *****//

extern const char *TAG; // Tag for the application to be included in logs

//***** Background Threads *****//

void adc_read(void *pvParameters); // Reads ADC Continuously

//***** Periodic Threads *****//

void update_LEDs(void *pvParameters); // Updates LEDs regularly
