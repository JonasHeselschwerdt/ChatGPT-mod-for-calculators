# Changelog - Casio FX Series Hardware Mod User Interface

## Version 1.0.0 (non-public)

First version of the firmware.

Abilities:

- Save one WIFI SSID + Password (typed into source code)
- Save on API- Key (typed into source code)
- Prompt processing and displaying the answer from OpenAI API

## Version 1.0.1

Improvements:

- Added Menu Mode
- Save up to 8 WIFI credentials directly via the UI
- Save API Key directly via the UI

## Version 1.1.0

Improvements:

- Device can load HTML files from local http servers and saves them into flash memory (non volatile) via littleFS
- Added 'stealth mode' upon start (device works like regular calculator when starting)
- Added Auto off feature (user can change time limit)
- Added factory reset ability
- Added USB HID mode (device can act as keyboard)
- Added ability to change GPT models
- Increased maximal ChatGPT answer length from 480 ASCII chars to 4800

Note: A text transfer tool to upload texts to the device was added

# Changelog - HTML local network hosting tool (text transfer tool)

## Version 1.0.0

Initial version:

- Up to 9 different text sections (each up to 80.000 ASCII chars)
- Automatic formatting (removing non-ASCII signs, removing formatting signs (\\n, \\t, etc.))

# Changelog - Mainboard

## Version 1.0.0

Initial version:

- 3.7V LiPo Battery integrated
- Battery protection circuit
- Battery management with powerpath
- Buck boost converter (3.3V output)
- USB port on the side of the board
- ESP32S3 wroom module with onboard antenna
- Powerlatch circuit
- Membrane keypad controlled by GPIO expander
- 20x4 text LCD (no backlight)

## Version 1.0.1 (Hotfix)

- Fixed 'deadlock' issue of battery protection (see battery_prot_hotfix_v1_0_1.md)

# Future plans for the device (To-Do List)

- Adding a Data Logging mode for different sensors (with .csv export via WIFI)
- Adding a Bluetooth Low Energy (BLE) HID Keyboard mode
- Adding Camera Support
- Rerouting the USB Jack to where the battery holder used to be
- Switching from Mikro USB to USB-C
- Improving the ESD protection with bidirectional TVS diodes
- Adding JST connectors to the PCB to connect peripherals
- Reactivating the 4 non-operational buttons of the keypad
- Changing the key mapping to be more similar to normal PC keyboards
- Replacing the programming pads with small SMD dip switch
- Adapting the PCB so the Battery protection can be bypassed
  (only if battery already has a built-in battery protection)
- Adding the possibility to use an external thermistor on the PCB
  (if the battery does not have it built in)
- Creating a 3D Model for a slightly thicker backcase
  so the electronics fit in better
- Switching from a 4bit parallel interface to I2C for display
  communication to free up GPIOs