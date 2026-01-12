# About this project

This repository features firmware for an ESP32S3 Wroom, developed in the ESP-IDF.
This firmware, installed on a compatible replacement mainboard allows the user
to send prompts to the OpenAI API via a keypad matrix controlled by the TCA8418 GPIO Expander from Texas Instruments.
The answers of ChatGPT can be displayed on a DOGM204 LCD Display by Display Visions.
The mainboard is designed to fit inside the (modified) case of Casio FX-series calculators.

This is an independent modification, I am not affiliated with Casio Computer Co.,Ltd. or Casio Europe GmbH in any way.

For a more detailed explanation of the project you can visit my Youtube Channel where I give an overview of the device.

(Youtube video not uploaded yet)

# Hardware description

Complementary to the firmware, Kicad schematics, Kicad board layouts (+ footprints), a bill of materials and additional hardware documentation are provided in the 'Documentation' and 'Hardware' folder. The board layout is described in the infographic 'Board stackup description'. The file 'Hardware function principle' shows how functional blocks of the hardware work in relation to each other.

To get the hardware you will need to order the correct parts and send the gerber files to a manufacturer. Please note that QFN packages with a pitch of just 0.5mm are used in this project, this makes hand soldering very difficult and requires special equipment like a hot air solder station or hot plate, flux, solder paste and ideally a solder stencil (order from PCB manufacturer).

# Firmware description

The code is written in C in the ESP-IDF extension for VSCode, for connecting to OpenAI I use the official Espressif OpenAI component. To understand the code, use my comments, the file 'File structure description' and the youtube video.

# Device setup, programming and UI

To set up the device a handful of steps are required (watch the Youtube Video):

- Opening the calculator and removing the original electronics
- Removing plastic bits from the case to adapt to the new electronics (see file 'Plastic parts removal guide')
- Soldering components to the board (see Silkscreen marks and schematic)
- Attaching the new mainboard to the front case
- Programming the device
- Adding your Wifi(s) and API Key

Additionally some 3D printed parts are used, which are however not strictly required for the device to work.
You can find the .3mf - files in the folder '3D Prints'.

For information on how to program the device you can watch my Youtube Video, where I also describe how to use the UI. You may also find the 'Keyboard Layout' file helpful, which shows the altered keypad layout of the device.

# The future of this project

I do not plan on adding any new features to the device in the near future, if that changes however I will add the new features or bug fixes here.

If you find bugs in the firmware or you need help setting the device up you can contact me on Youtube or instagram, feel free to build upon my work and add features, the ESP32 is powerful enough to run more complex code, there is also plenty of space left on the PCB and some free GPIOs for add-ons.

# Inspiration for add-ons and missing features

- NVS encryption for API Keys and Wifi credentials
- Adding bluetooth capabilities
- Ability to change GPT Model
- Adding an auto-off timer
- Ability to set the contrast of the display
- Ability for 'over the Air' firmware updates
- Adding transceivers for different communication protocols to the PCB
- Adding games to the calculator
- Adding a camera
- Adding more storage

© 2026 Jonas Heselschwerdt
Free for personal, research and educational use
Commercial use requires written permission








