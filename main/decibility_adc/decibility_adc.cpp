#include "decibility_adc.h"

#include "esp_adc/adc_continuous.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

// Interrupt Handler for ADC conversion done
bool s_conv_done_cb(adc_continuous_handle_t handle, const adc_continuous_evt_data_t *edata, void *user_data)
{
    BaseType_t mustYield = pdFALSE;
    // Notify that ADC continuous driver has done enough number of conversions
    vTaskNotifyGiveFromISR(s_task_handle, &mustYield);

    return (mustYield == pdTRUE);
}

// Initializes Continuous ADC Conversion
void continuous_adc_init(adc_channel_t channel, adc_continuous_handle_t *out_handle)
{
    adc_continuous_handle_t handle = NULL;

    adc_continuous_handle_cfg_t adc_config = {
        .max_store_buf_size = 1024,
        .conv_frame_size = ADC_NUM_SAMPLES,
    };
    ESP_ERROR_CHECK(adc_continuous_new_handle(&adc_config, &handle));

    adc_continuous_config_t dig_cfg = {
        .sample_freq_hz = 44100,
        .conv_mode = DECIBILITY_ADC_CONV_MODE,
        .format = DECIBILITY_ADC_OUTPUT_TYPE,
    };

    adc_digi_pattern_config_t adc_pattern;
    dig_cfg.pattern_num = 1;

    adc_pattern.atten = DECIBILITY_ADC_ATTEN;
    adc_pattern.channel = channel & 0x7;
    adc_pattern.unit = DECIBILITY_ADC_UNIT;
    adc_pattern.bit_width = DECIBILITY_ADC_BIT_WIDTH;

    dig_cfg.adc_pattern = &adc_pattern;
    ESP_ERROR_CHECK(adc_continuous_config(handle, &dig_cfg));

    *out_handle = handle;
}

void decibility_adc_init(adc_continuous_handle_t *out_handle)
{
    adc_continuous_handle_t handle = NULL;
    continuous_adc_init(DECIBILITY_ADC_CHANNEL, &handle);

    // Sets ISR for ADC Conversion Complete
    adc_continuous_evt_cbs_t cbs = {
        .on_conv_done = s_conv_done_cb,
    };
    ESP_ERROR_CHECK(adc_continuous_register_event_callbacks(handle, &cbs, NULL));

    *out_handle = handle;
}
