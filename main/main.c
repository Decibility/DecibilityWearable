#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "driver/gpio.h"
#include "led_strip.h"
#include "esp_adc/adc_continuous.h"

#include "esp_log.h"
#include "sdkconfig.h"

#include "decibility_leds/decibility_leds.h"
#include "decibility_adc/decibility_adc.h"

static const char *TAG = "DecibilityWearable";

TaskHandle_t s_task_handle;

void app_main(void)
{
    // Initialize LED Strips
    led_strip_handle_t freq_led_strip, volume_led_strip;
    decibility_led_init(&freq_led_strip, &volume_led_strip);

    // Get handle to function that will process ADC output
    // Currently this is main, so we get the current task handle
    // This must be set before ADC init
    s_task_handle = xTaskGetCurrentTaskHandle();

    // Initializes ADC and cereates a Task for it, returning the ADC handle
    adc_continuous_handle_t handle = NULL;
    decibility_adc_init(&handle);

    // Starts Continuous ADC Conversion
    ESP_ERROR_CHECK(adc_continuous_start(handle));

    // Variables to store the output of the ADC
    esp_err_t ret;                     // Error Code that adc_continuous_read() will return
    uint32_t ret_num = 0;              // Will store number of data points that adc_continuous_read() returns
    uint8_t result[NUM_SAMPLES] = {0}; // Buffer that will store ADC Results

    memset(result, 0xcc, NUM_SAMPLES); // Fills result array with 0xCC at every index

    int delay = 0; // Delay used for updating LEDs
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

            uint16_t max = 0;
            for (int i = 0; i < ret_num; i += SOC_ADC_DIGI_RESULT_BYTES)
            {
                adc_digi_output_data_t *p = (adc_digi_output_data_t *)&result[i];
                uint32_t chan_num = DECIBILITY_ADC_GET_CHANNEL(p);
                uint32_t data = DECIBILITY_ADC_GET_DATA(p);
                /* Check the channel number validation, the data is invalid if the channel num exceed the maximum channel */
                if (chan_num < SOC_ADC_CHANNEL_NUM(DECIBILITY_ADC_UNIT))
                {
                    if (data > max)
                    {
                        max = data;
                    }
                }
                else
                {
                    ESP_LOGW(TAG, "Invalid data");
                }
            }

            if (max > 200)
            {
                delay = 60;
                led_strip_set_pixel(volume_led_strip, DOWN_ARROW, 0, 10, 0);
                led_strip_set_pixel(volume_led_strip, CENTER, 0, 10, 0);
                led_strip_set_pixel(volume_led_strip, UP_ARROW, 0, 10, 0);
            }
            else
            {
                if (delay == 0)
                {
                    led_strip_set_pixel(volume_led_strip, DOWN_ARROW, 0, 0, 10);
                    led_strip_set_pixel(volume_led_strip, CENTER, 0, 0, 10);
                    led_strip_set_pixel(volume_led_strip, UP_ARROW, 0, 0, 10);
                }
                else
                {
                    delay--;
                }
            }
            led_strip_refresh(volume_led_strip);
        }
    }

    // Deactivate the ADC
    ESP_ERROR_CHECK(adc_continuous_stop(handle));
    ESP_ERROR_CHECK(adc_continuous_deinit(handle));
}