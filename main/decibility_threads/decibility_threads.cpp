#include "decibility_threads/decibility_threads.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "../decibility_leds/decibility_leds.h"
#include "led_strip.h"

#include "../decibility_adc/decibility_adc.h"
#include "esp_adc/adc_continuous.h"

#include "../decibility_bluetooth/decibility_bluetooth.h"

#include <string.h>
#include <cstring>

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

    esp_err_t ret;                                                     // Error Code that adc_continuous_read() will return
    uint32_t ret_num = 0;                                              // Will store number of data points that adc_continuous_read() returns
    uint8_t result[ADC_NUM_SAMPLES * SOC_ADC_DIGI_RESULT_BYTES] = {0}; // Buffer that will store ADC Results

    memset(result, 0xcc, ADC_NUM_SAMPLES * SOC_ADC_DIGI_RESULT_BYTES); // Fills result array with 0xCC at every index

    recent_max = 0;

    while (1)
    {
        // Waits until ADC is ready to read from
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        // Reads until there is no available data
        while (1)
        {
            ret = adc_continuous_read(handle, result, ADC_NUM_SAMPLES * SOC_ADC_DIGI_RESULT_BYTES, &ret_num, 0);

            if (ret == ESP_ERR_TIMEOUT)
            {
                break;
            }
            else if (ret != ESP_OK)
            {
                ESP_LOGW(TAG, "ADC Continuous Read did not return OK");
                continue;
            }

            // Buffer for samples to send;
            uint16_t *data = new uint16_t[ADC_NUM_SAMPLES];

            for (int i = 0; i < ret_num; i += SOC_ADC_DIGI_RESULT_BYTES)
            {
                adc_digi_output_data_t *p = (adc_digi_output_data_t *)&result[i];
                uint32_t chan_num = DECIBILITY_ADC_GET_CHANNEL(p);
                uint32_t datum = DECIBILITY_ADC_GET_DATA(p);
                /* Check the channel number validation, the data is invalid if the channel num exceed the maximum channel */
                if (chan_num < SOC_ADC_CHANNEL_NUM(DECIBILITY_ADC_UNIT))
                {
                    data[i / SOC_ADC_DIGI_RESULT_BYTES] = (uint16_t)datum;
                }
                else
                {
                    data[i / SOC_ADC_DIGI_RESULT_BYTES] = 0;
                    ESP_LOGW(TAG, "Invalid data");
                }
            }

            // If missing some samples, fill them in with 0
            for (int i = ret_num; i < ADC_NUM_SAMPLES; i++)
            {
                data[i] = 0;
            }

            // Transmits the data over bluetooth
            decibility_bt_send_audio_burst(data);

            vTaskDelay(AUDIO_SEND_DELAY_MS);

            delete[] data;
        }
    }
}

//***** Periodic Threads *****//

// Updates LEDs regularly
void update_LEDs(void *pvParameters)
{
    led_strip_handle_t freq_led_strip, volume_led_strip;
    decibility_led_init(&freq_led_strip, &volume_led_strip);

    // Set up buffers for LED colors
    uint8_t **volBuffer = new uint8_t *[VOLUME_STRIP_LEDS];
    for (int i = 0; i < VOLUME_STRIP_LEDS; i++)
    {
        volBuffer[i] = new uint8_t[NUM_COLORS];
        for (int j = 0; j < NUM_COLORS; j++)
        {
            volBuffer[i][j] = 0;
        }
    }

    uint8_t **freqBuffer = new uint8_t *[FREQ_STRIP_LEDS];
    for (int i = 0; i < FREQ_STRIP_LEDS; i++)
    {
        freqBuffer[i] = new uint8_t[NUM_COLORS];
        for (int j = 0; j < NUM_COLORS; j++)
        {
            freqBuffer[i][j] = 0;
        }
    }

    while (1)
    {
        while (!SerialBT.available())
        {
            vTaskDelay(pdMS_TO_TICKS(LED_UPDATE_PERIOD_MS));
        }

        decibility_bt_recive_led_command(volBuffer, freqBuffer);

        led_strip_set_pixel(volume_led_strip, DOWN_ARROW, volBuffer[DOWN_ARROW][0], volBuffer[DOWN_ARROW][1], volBuffer[DOWN_ARROW][2]);
        led_strip_set_pixel(volume_led_strip, CENTER, volBuffer[CENTER][0], volBuffer[CENTER][1], volBuffer[CENTER][2]);
        led_strip_set_pixel(volume_led_strip, UP_ARROW, volBuffer[UP_ARROW][0], volBuffer[UP_ARROW][1], volBuffer[UP_ARROW][2]);
        led_strip_refresh(volume_led_strip);

        led_strip_set_pixel(freq_led_strip, DOWN_ARROW, freqBuffer[DOWN_ARROW][0], freqBuffer[DOWN_ARROW][1], freqBuffer[DOWN_ARROW][2]);
        led_strip_set_pixel(freq_led_strip, CENTER, freqBuffer[CENTER][0], freqBuffer[CENTER][1], freqBuffer[CENTER][2]);
        led_strip_set_pixel(freq_led_strip, UP_ARROW, freqBuffer[UP_ARROW][0], freqBuffer[UP_ARROW][1], freqBuffer[UP_ARROW][2]);
        led_strip_refresh(freq_led_strip);

        // Wait for the next period
        vTaskDelay(pdMS_TO_TICKS(LED_UPDATE_PERIOD_MS));
    }
}