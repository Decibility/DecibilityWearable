#pragma once

// Bluetooth Library
#include "BluetoothSerial.h"

// Check if Bluetooth is available
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

// Check Serial Port Profile
#if !defined(CONFIG_BT_SPP_ENABLED)
#error Serial Port Profile for Bluetooth is not available or not enabled. It is only available for the ESP32 chip.
#endif

// Library Object that handles SPP bluetooth communication
extern BluetoothSerial SerialBT;
extern bool initComplete;

#define BT_DEVICE_NAME "Decibility Wearable"

void decibility_nvs_init(); // It is necessary to initialize the NVS flash before using bluetooth
void decibility_bt_init();  // Initializes bluetooth with device name, avoids duplicate calls

/*
    Receives what the new state of the LEDs should be. Message must be in the format:

    LED_COMMAND_START, VOL_0_R, VOL_0_G, VOL_0_B, VOL_1_R, ... VOL_2_B, FREQ_0_R, ... FREQ_2_B, LED_COMMAND_END
*/
#define LED_COMMAND_START 'w'
#define LED_COMMAND_END '\n'
bool decibility_bt_recive_led_command(uint8_t **volBuffer, uint8_t **freqBuffer);

/*
    Sends Burst of Recorded Audio over Bluetooth for Processing.
    Format:

    AUDIO_START, SAMPLE_0_LOW, SAMPLE_0_HIGH, SAMPLE_1_LOW, ... SAMPLE_N_HIGH, AUDIO_END
*/
#define AUDIO_START 'a'
#define AUDIO_END '\n'
void decibility_bt_send_audio_burst(uint16_t *data);