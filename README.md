### Decibility Wearable

Repo for the drivers for the Decibility Wearable

Build Instructions:
- The target for this repo is the [ESP32-C3-DevKitM-1](https://docs.espressif.com/projects/esp-idf/en/latest/esp32c3/hw-reference/esp32c3/user-guide-devkitm-1.html)
- Once the project is configured, connect the decibility wearable device to your PC using a usb cable
  - Diagrams for the decibility wearable are available in the top level "diagrams" folder
- To build:
  - Install and Configure the [ESP IDF for VSCode](https://docs.espressif.com/projects/esp-idf/en/v4.2.3/esp32/get-started/vscode-setup.html)
  - Clone the repo and open in it VSCode
  - Commands:
    - Ctrl-E + B to build
    - Ctrl-E + F to flash
    - Ctrl Ctrl-E + M to start monitoiring

Work that was completed in this Milestone
- All the driver code in this repo was completed during this module
- The drivers were entirely rewritten thoughout this module
- The only thing that was not done during this module was the initial setup and configuration

Known Bugs/Issues:
- No bugs are currenlty known, the code performs as expected
- Bluetooth communication is not configured yet
- More configuration options will be added in the final verison
- LED update logic is a placeholder for bluetooth communication
