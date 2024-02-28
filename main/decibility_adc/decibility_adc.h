#pragma once

#include "esp_adc/adc_continuous.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// ESP IDF ADC Configuration
#define DECIBILITY_ADC_UNIT ADC_UNIT_1
#define _DECIBILITY_ADC_UNIT_STR(unit) #unit
#define DECIBILITY_ADC_UNIT_STR(unit) _DECIBILITY_ADC_UNIT_STR(unit)
#define DECIBILITY_ADC_CONV_MODE ADC_CONV_SINGLE_UNIT_1
#define DECIBILITY_ADC_ATTEN ADC_ATTEN_DB_0
#define DECIBILITY_ADC_BIT_WIDTH SOC_ADC_DIGI_MAX_BITWIDTH

#if CONFIG_IDF_TARGET_ESP32 || CONFIG_IDF_TARGET_ESP32S2
#define DECIBILITY_ADC_OUTPUT_TYPE ADC_DIGI_OUTPUT_FORMAT_TYPE1
#define DECIBILITY_ADC_GET_CHANNEL(p_data) ((p_data)->type1.channel)
#define DECIBILITY_ADC_GET_DATA(p_data) ((p_data)->type1.data)
#else
#define DECIBILITY_ADC_OUTPUT_TYPE ADC_DIGI_OUTPUT_FORMAT_TYPE2
#define DECIBILITY_ADC_GET_CHANNEL(p_data) ((p_data)->type2.channel)
#define DECIBILITY_ADC_GET_DATA(p_data) ((p_data)->type2.data)
#endif

#define ADC_NUM_SAMPLES 1024
#define ADC_SAMPLE_FREQ 44100

// Decibility ADC Constants
// ADC 1 Channel 6 maps to pin 34 but it actually maps to pin 39 on the original dev board
// ADC 1 Channel 7 maps to pin 35
#define DECIBILITY_ADC_CHANNEL ADC_CHANNEL_7

// Handle for Task that processes ADC Results
extern TaskHandle_t s_task_handle;

// Interrupt Handler for ADC conversion done
bool IRAM_ATTR s_conv_done_cb(adc_continuous_handle_t handle, const adc_continuous_evt_data_t *edata, void *user_data);

// Initializes Continuous ADC Conversions
void continuous_adc_init(adc_channel_t channel, adc_continuous_handle_t *out_handle);

// Initializes ADC for Decibility
void decibility_adc_init(adc_continuous_handle_t *out_handle);
