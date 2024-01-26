#include "decibility_bluetooth.h"
#include "../decibility_leds/decibility_leds.h"
#include "../decibility_adc/decibility_adc.h"

#include "esp_log.h"

#include "nvs.h"
#include "nvs_flash.h"

bool initComplete = false;
BluetoothSerial SerialBT;

void decibility_nvs_init()
{
    esp_err_t ret = nvs_flash_init();

    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }

    ESP_ERROR_CHECK(ret);
}

void decibility_bt_init()
{
    if (initComplete)
        return;

    decibility_nvs_init();

    if (!SerialBT.begin(BT_DEVICE_NAME))
        return;

    initComplete = true;
}

bool decibility_bt_recive_led_command(uint8_t **volBuffer, uint8_t **freqBuffer)
{
    if (SerialBT.read() != LED_COMMAND_START)
        return false;

    for (int led = 0; led < VOLUME_STRIP_LEDS; led++)
    {
        for (int color = 0; color < NUM_COLORS; color++)
        {
            volBuffer[led][color] = SerialBT.read();
        }
    }

    for (int led = 0; led < FREQ_STRIP_LEDS; led++)
    {
        for (int color = 0; color < NUM_COLORS; color++)
        {
            freqBuffer[led][color] = SerialBT.read();
        }
    }

    if (SerialBT.read() != LED_COMMAND_END)
        return false;
    else
        return true;
}

void decibility_bt_send_audio_burst(uint16_t *data)
{
    size_t msg_size = 2 + ADC_NUM_SAMPLES * 2;
    uint8_t *msg = new uint8_t[msg_size];

    msg[0] = AUDIO_START;

    for (int i = 0; i < ADC_NUM_SAMPLES; i++)
    {
        msg[i * 2 + 1] = (uint8_t)data[i];        // LSB
        msg[i * 2 + 2] = (uint8_t)(data[i] >> 8); // MSB
    }

    msg[msg_size - 1] = AUDIO_END;

    SerialBT.write(msg, msg_size);

    delete[] msg;

    return;
}