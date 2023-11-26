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

static const char *TAG = "DecibilityWearable";

/***** ADC STUFF *****/

#define EXAMPLE_ADC_UNIT ADC_UNIT_1
#define _EXAMPLE_ADC_UNIT_STR(unit) #unit
#define EXAMPLE_ADC_UNIT_STR(unit) _EXAMPLE_ADC_UNIT_STR(unit)
#define EXAMPLE_ADC_CONV_MODE ADC_CONV_SINGLE_UNIT_1
#define EXAMPLE_ADC_ATTEN ADC_ATTEN_DB_0
#define EXAMPLE_ADC_BIT_WIDTH SOC_ADC_DIGI_MAX_BITWIDTH

#if CONFIG_IDF_TARGET_ESP32 || CONFIG_IDF_TARGET_ESP32S2
#define EXAMPLE_ADC_OUTPUT_TYPE ADC_DIGI_OUTPUT_FORMAT_TYPE1
#define EXAMPLE_ADC_GET_CHANNEL(p_data) ((p_data)->type1.channel)
#define EXAMPLE_ADC_GET_DATA(p_data) ((p_data)->type1.data)
#else
#define EXAMPLE_ADC_OUTPUT_TYPE ADC_DIGI_OUTPUT_FORMAT_TYPE2
#define EXAMPLE_ADC_GET_CHANNEL(p_data) ((p_data)->type2.channel)
#define EXAMPLE_ADC_GET_DATA(p_data) ((p_data)->type2.data)
#endif

#define NUM_SAMPLES 256

static adc_channel_t channel[1] = {ADC_CHANNEL_2};

static TaskHandle_t s_task_handle;

// Interrupt Handler for ADC conversion done
static bool IRAM_ATTR s_conv_done_cb(adc_continuous_handle_t handle, const adc_continuous_evt_data_t *edata, void *user_data)
{
    BaseType_t mustYield = pdFALSE;
    // Notify that ADC continuous driver has done enough number of conversions
    vTaskNotifyGiveFromISR(s_task_handle, &mustYield);

    return (mustYield == pdTRUE);
}

// Initializes Continuous ADC Conversions
static void continuous_adc_init(adc_channel_t *channel, uint8_t channel_num, adc_continuous_handle_t *out_handle)
{
    adc_continuous_handle_t handle = NULL;

    adc_continuous_handle_cfg_t adc_config = {
        .max_store_buf_size = 1024,
        .conv_frame_size = NUM_SAMPLES,
    };
    ESP_ERROR_CHECK(adc_continuous_new_handle(&adc_config, &handle));

    adc_continuous_config_t dig_cfg = {
        .sample_freq_hz = 44100,
        .conv_mode = EXAMPLE_ADC_CONV_MODE,
        .format = EXAMPLE_ADC_OUTPUT_TYPE,
    };

    adc_digi_pattern_config_t adc_pattern[SOC_ADC_PATT_LEN_MAX] = {0};
    dig_cfg.pattern_num = channel_num;
    for (int i = 0; i < channel_num; i++)
    {
        adc_pattern[i].atten = EXAMPLE_ADC_ATTEN;
        adc_pattern[i].channel = channel[i] & 0x7;
        adc_pattern[i].unit = EXAMPLE_ADC_UNIT;
        adc_pattern[i].bit_width = EXAMPLE_ADC_BIT_WIDTH;

        ESP_LOGI(TAG, "adc_pattern[%d].atten is :%" PRIx8, i, adc_pattern[i].atten);
        ESP_LOGI(TAG, "adc_pattern[%d].channel is :%" PRIx8, i, adc_pattern[i].channel);
        ESP_LOGI(TAG, "adc_pattern[%d].unit is :%" PRIx8, i, adc_pattern[i].unit);
    }
    dig_cfg.adc_pattern = adc_pattern;
    ESP_ERROR_CHECK(adc_continuous_config(handle, &dig_cfg));

    *out_handle = handle;
}

void app_main(void)
{
    // Initialize LED Strips
    led_strip_handle_t freq_led_strip, volume_led_strip;
    decibility_led_init(&freq_led_strip, &volume_led_strip);

    // ADC Variables
    esp_err_t ret;                     // Error Code that adc_continuous_read() will return
    uint32_t ret_num = 0;              // Will store number of data points that adc_continuous_read() returns
    uint8_t result[NUM_SAMPLES] = {0}; // Buffer that will store ADC Results

    memset(result, 0xcc, NUM_SAMPLES); // Fills result array with 0xCC at every index

    // Get handle to main
    s_task_handle = xTaskGetCurrentTaskHandle();

    // Initializes ADC and cereates a Task for it, returning the handle
    adc_continuous_handle_t handle = NULL;
    continuous_adc_init(channel, sizeof(channel) / sizeof(adc_channel_t), &handle);

    // Sets ISR for ADC Conversion Complete
    adc_continuous_evt_cbs_t cbs = {
        .on_conv_done = s_conv_done_cb,
    };
    ESP_ERROR_CHECK(adc_continuous_register_event_callbacks(handle, &cbs, NULL));

    // Starts Continuous ADC Conversion
    ESP_ERROR_CHECK(adc_continuous_start(handle));

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
                uint32_t chan_num = EXAMPLE_ADC_GET_CHANNEL(p);
                uint32_t data = EXAMPLE_ADC_GET_DATA(p);
                /* Check the channel number validation, the data is invalid if the channel num exceed the maximum channel */
                if (chan_num < SOC_ADC_CHANNEL_NUM(EXAMPLE_ADC_UNIT))
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