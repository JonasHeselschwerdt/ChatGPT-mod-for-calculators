# Hardware Version v2.0.1

In this newest version some small changes to the board are made. They allow measuring the temperature
of the battery even when the MCP73871 is not active with an ADC of the ESP32S3. Also the VPCC function of
the MCP73871 gets deactivated as it caused problems when powering the device with just USB and no battery.
The PCB gerber files in the Hardware-Folder (v2_0_1) have these changes implemented, as well as routing improvements
for the power electronics.

## Upgrading Hardware v2.0.0 to v2.0.1

If you already ordered the v2.0.0 board, it can be upgraded:

1) Connect TP29 to TP14 or Pin 1 of J2 with a 100k Resistor
2) Connect TP14 or J2 Pin 1 with a 47k Resistor to GPIO3 (Pin 15) of the ESP32
3) Connect GPIO3 (Pin 15) of the ESP32 to a 10nF capacitor to GND (for example JP2)
4) Connect Pin 1 of SW1 with a 100nF capacitor to GND (for example Pin 3 or 4 of SW1)
5) Replace R7 with a 0 Ohm bridge or a solder bridge
6) Remove R8

For the modifications I recommend some 32AWG or 30AWG wire wrapping wire, Software Version v2.0.0 will require all changes
to the hardware listed above to be made on your hardware.