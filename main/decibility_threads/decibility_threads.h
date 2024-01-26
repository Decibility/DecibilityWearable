#pragma once

#include <stdint.h>

////***** Constants *****//
#define LED_UPDATE_PERIOD_MS 10

////***** External Variables *****//

extern const char *TAG; // Tag for the application to be included in logs

//***** Background Threads *****//

void adc_read(void *pvParameters); // Reads ADC Continuously

//***** Periodic Threads *****//

void update_LEDs(void *pvParameters); // Updates LEDs regularly
