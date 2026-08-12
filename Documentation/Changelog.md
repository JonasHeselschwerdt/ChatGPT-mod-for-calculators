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

# Changelog - Hardware

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

## Version 2.0.0

- Split electronics into Mainboard and UI-Board, connected by 24 Pin FPC connector
- Battery and USB-C connector connected via Molex Picoblade Connectors (instead of soldered on)
- Replaced programming pads with a SMD low profile DIP switch
- Made bypassing the on-board battery protection easier (see v.1.0.1)
- Improved ESD-Protection of USB-Connector
- Added fuel gauge IC to monitor battery charge state via I2C
- Added a switch to manually turn of the main 3.3V rail
- Added an interface for a OV5640 camera module including power supply and clock source
- Switched 20x4 text LCD interface to I2C
- Improved power latch circuit
- Rerouted output signals of BMS to TCA8418 inputs
- Added an user extension interface offering up to 10 free GPIOs + I2C
- Added a second graphic display (128x64 OLED screen)
- Reactivated the 4 previously missing buttons
- Added testpoints and solder jumpers for easy modifications/testing
- Improved keypad button footprints
- Added on-board LED for use during debugging (disconnected by default)
- Added support for an external NTC-Thermistor for battery temperature monitoring

## Version 2.0.1

- Repurposed FreeGPIO9 to NTC-Voltage Divider activation Pin (FreeGPIO9 not available anymore)
- Added Ability to monitor the battery cell temperature with the ESP32S3 ADC1 Channel 2 (GPIO 3)
- Added a 100nF Capacitor from 3V3_Main_disable to make it less susceptible to noise
- Added copper pours at input pins of TPS63070 and MCP73871 to decrease thermal resistance to the PCB
- Disabled VPCC function of MCP73871 as it led to problems when the device is only powered through USB
- More infos: fixes_v2_0_1_documentation.md
- Replaced FPC connector link in BOM

# Future plans for the device (To-Do List)

- Cleaning up code
- Adding the camera into the UI
- Adding the side display into the UI
- Changing the key mapping to be more similar to normal PC keyboards