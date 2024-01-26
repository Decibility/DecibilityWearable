#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_adc/adc_continuous.h"

#include "decibility_leds/decibility_leds.h"
#include "decibility_adc/decibility_adc.h"
#include "decibility_threads/decibility_threads.h"

#include "sdkconfig.h"

const char *TAG = "DecibilityWearable";

extern "C" void app_main()
{
    // Sets the priority of app_main to be higher than the threads being added
    // This makes sure app_main completed adding threads before any threads are started
    vTaskPrioritySet(NULL, tskIDLE_PRIORITY + 3);

    // Creates task for ADC, ADC is initialized in the task
    xTaskCreate(adc_read, "Reads from ADC", 1e5, NULL, tskIDLE_PRIORITY + 2, NULL);

    // Creates Task that initializes and updates LED strips
    xTaskCreate(update_LEDs, "Updates LEDs", 1e5, NULL, tskIDLE_PRIORITY + 1, NULL);

    vTaskDelete(NULL);
}