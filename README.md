### Decibility Wearable

Repo for the drivers for the Decibility Wearable. The decibility wearable is a device that is intended to help people who are hard of hearing by letting others know when they are speaking in a way that the person who is hard of hearing cannot hear them. The wearable must be connected to an android phone with the Decibility Application installed.

Build Instructions:
- The target for this repo is the [ESP32-DevKitM-1](https://docs.espressif.com/projects/esp-idf/en/latest/esp32c3/hw-reference/esp32c3/user-guide-devkitm-1.html)
- Once the project is configured, connect the decibility wearable device to your PC using a usb cable
  - Diagrams for the decibility wearable are available in the top level "diagrams" folder
- To build:
  - Install and Configure the [ESP IDF for VSCode](https://docs.espressif.com/projects/esp-idf/en/v4.2.3/esp32/get-started/vscode-setup.html)
  - Clone the repo and open in it VSCode
  - Open the command pallet and run the "Add Arduino ESP32 as ESP-IDF Component Command"
  - Commands:
    - Ctrl-E + B to build
    - Ctrl-E + F to flash
    - Ctrl Ctrl-E + M to start monitoiring

Work that was completed in Alpha Build Milestone
- Bluetooth communication was implemented
- Protocols for sending audio and receiving LED commands were implemented
- Project was configured for ESP-32 instead of the ESP-32c3

Work that was completed in Pre-Alpha Milestone
- All the driver code in this repo was completed during this module
- The drivers were entirely rewritten thoughout this module
- The only thing that was not done during this module was the initial setup and configuration

Known Bugs/Issues:
- No bugs are currenlty known, the code performs as expected
- More configuration options will be added in the final verison
