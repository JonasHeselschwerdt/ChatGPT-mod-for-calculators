/*

ChatGPT Hardware Hack for calculators: Software V2

© 2026 Jonas Heselschwerdt
Licensed under GPLv3 

user_extension.c: Write code for user extensions here

*/

/*

How to setup user extensions:

1) Make your hardware changes (for example via the Kicad User Extension Board template project)
2) Setup the FreeGPIOs (FREEGPIO_5...1 in device.h and device.c and FREEGPIO_0 in keypad.c and keypad.h)
   (use FREEGPIO_0 for slower signals ideally)
3) Configure your custom software module in this file
4) Integrate it into the user interface in UI.c or in device.c

*/