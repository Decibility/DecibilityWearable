#include "decibility_threads/decibility_threads.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "../decibility_leds/decibility_leds.h"
#include "led_strip.h"

#include "../decibility_adc/decibility_adc.h"
#include "esp_adc/adc_continuous.h"

#include <string.h>

#include "esp_log.h"

//***** Global Variables *****//
uint16_t recent_max; // Has the max adc reading since the last time the LEDs were updated

//***** Background Threads *****//

// Reads ADC Continuously
TaskHandle_t s_task_handle;
void adc_read(void *pvParameters)
{
    // Get handle to function that will process ADC output
    // Currently this is main, so we get the current task handle
    // This must be set before ADC init
    s_task_handle = xTaskGetCurrentTaskHandle();

    // Initializes ADC
    adc_continuous_handle_t handle = NULL;
    decibility_adc_init(&handle);

    // Starts Continuous ADC Conversion
    ESP_ERROR_CHECK(adc_continuous_start(handle));

    esp_err_t ret;                     // Error Code that adc_continuous_read() will return
    uint32_t ret_num = 0;              // Will store number of data points that adc_continuous_read() returns
    uint8_t result[NUM_SAMPLES] = {0}; // Buffer that will store ADC Results

    memset(result, 0xcc, NUM_SAMPLES); // Fills result array with 0xCC at every index

    recent_max = 0;

    while (1)
    {
        // Waits until ADC is ready to read from
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        // Reads until there is no available data
        while (1)
        {
            ret = adc_continuous_read(handle, result, NUM_SAMPLES, &ret_num, 0);

            if (ret == ESP_ERR_TIMEOUT)
            {
                break;
            }
            else if (ret != ESP_OK)
            {
                ESP_LOGW(TAG, "ADC Continuous Read did not return OK");
                continue;
            }

            for (int i = 0; i < ret_num; i += SOC_ADC_DIGI_RESULT_BYTES)
            {
                adc_digi_output_data_t *p = (adc_digi_output_data_t *)&result[i];
                uint32_t chan_num = DECIBILITY_ADC_GET_CHANNEL(p);
                uint32_t data = DECIBILITY_ADC_GET_DATA(p);
                /* Check the channel number validation, the data is invalid if the channel num exceed the maximum channel */
                if (chan_num < SOC_ADC_CHANNEL_NUM(DECIBILITY_ADC_UNIT))
                {
                    if (data > recent_max)
                    {
                        recent_max = data;
                    }
                }
                else
                {
                    ESP_LOGW(TAG, "Invalid data");
                }
            }
        }
    }
}

//***** Periodic Threads *****//

// Updates LEDs regularly
void update_LEDs(void *pvParameters)
{
    led_strip_handle_t freq_led_strip, volume_led_strip;
    decibility_led_init(&freq_led_strip, &volume_led_strip);

    recent_max = 0;

    while (1)
    {
        if (recent_max > 200)
        {
            led_strip_set_pixel(volume_led_strip, DOWN_ARROW, 0, 10, 0);
            led_strip_set_pixel(volume_led_strip, CENTER, 0, 10, 0);
            led_strip_set_pixel(volume_led_strip, UP_ARROW, 0, 10, 0);
        }
        else
        {
            led_strip_set_pixel(volume_led_strip, DOWN_ARROW, 0, 0, 10);
            led_strip_set_pixel(volume_led_strip, CENTER, 0, 0, 10);
            led_strip_set_pixel(volume_led_strip, UP_ARROW, 0, 0, 10);
        }
        led_strip_refresh(volume_led_strip);
        recent_max = 0;

        led_strip_set_pixel(freq_led_strip, DOWN_ARROW, 5, 0, 5);
        led_strip_set_pixel(freq_led_strip, CENTER, 0, 5, 5);
        led_strip_set_pixel(freq_led_strip, UP_ARROW, 5, 5, 0);
        led_strip_refresh(freq_led_strip);

        // Wait for the next period
        vTaskDelay(pdMS_TO_TICKS(LED_UPDATE_PERIOD_MS));
    }
}