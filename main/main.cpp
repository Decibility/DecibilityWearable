#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_adc/adc_continuous.h"

#include "decibility_leds/decibility_leds.h"
#include "decibility_adc/decibility_adc.h"
#include "decibility_threads/decibility_threads.h"

#include "sdkconfig.h"

const char *TAG = "DecibilityWearable";

// Check if Bluetooth is available
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

// Check Serial Port Profile
#if !defined(CONFIG_BT_SPP_ENABLED)
#error Serial Port Profile for Bluetooth is not available or not enabled. It is only available for the ESP32 chip.
#endif

#include "BluetoothSerial.h"
BluetoothSerial SerialBT;

extern "C" void app_main()
{
    SerialBT.begin("Decibility ESP-32");

    // Sets the priority of app_main to be higher than the threads being added
    // This makes sure app_main completed adding threads before any threads are started
    vTaskPrioritySet(NULL, tskIDLE_PRIORITY + 3);

    // Creates Task that initializes and updates LED strips
    xTaskCreate(update_LEDs, "Updates LEDs", 1e5, NULL, tskIDLE_PRIORITY + 1, NULL);

    // Creates task for ADC, ADC is initialized in the task
    xTaskCreate(adc_read, "Reads from ADC", 1e5, NULL, tskIDLE_PRIORITY + 2, NULL);

    vTaskDelete(NULL);
}