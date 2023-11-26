#include "decibility_leds.h"

#include "led_strip.h"
#include "driver/gpio.h"

void led_strip_init(led_strip_handle_t *led_strip, int gpio, uint32_t num_leds, uint32_t resolution_hz)
{
    /* LED strip initialization with the GPIO and pixels number*/
    led_strip_config_t strip_config = {
        .strip_gpio_num = gpio,
        .max_leds = num_leds,
    };
    led_strip_rmt_config_t rmt_config = {
        .resolution_hz = resolution_hz,
    };
    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, led_strip));

    /* Set all LED off to clear all pixels */
    led_strip_clear(*led_strip);

    return;
}

void decibility_led_init(led_strip_handle_t *freq_led_strip, led_strip_handle_t *volume_led_strip)
{
    led_strip_init(freq_led_strip, FREQ_STRIP_GPIO, FREQ_STRIP_LEDS, FREQ_STRIP_RES);
    led_strip_init(volume_led_strip, VOLUME_STRIP_GPIO, VOLUME_STRIP_LEDS, VOLUME_STRIP_RES);

    return;
}