# How to order the boards (from JLCPCB)

If you want to get your own copy of the mainboard and UI-Board I recommend JLCPCB
as PCB manufacturer.

To get the boards upload the Gerber files (one by one) as .zip on https://jlcpcb.com/pcb-assembly?from=JUOLXLQL -> Get instant quote

## After that you can set some settings:

- PCB Qty: Set how many boards you need
- PCB thickness: Set to 0.8mm (for both boards)
- Surface finish: ENIG heavily recommended for UI-board, mainboard can be HASL or ENIG
- For mainboard activate either 'PCB assembly' (let JLCPCB handle assembly) or 'Solder stencil' (assemble board yourself)
- Leave all other settings in their default state. When using PCB assembly services, the extra BOM and CPL files in the hardware folder are necessary (JLCPCB_Mainboard_V203_CPL.csv and JLCPCB_Mainboard_V203_BOM.csv)

## PCB Assembly files

The PCBA files (CPL and BOM) use some different parts than the ones shown in my videos (because of availability issues)
These files look good in the JLCPCB order preview, however **I have not validated them yet through a real order**.
Please keep this in mind before ordering, if you suspect an error in the files you can contact me via DM (contact info in main readme)

## Mainboard solder stencil

- Stencil side: Top only
- Polishing process: Electropolishing
- Leave all other settings in their default state

## Transparency note:

JLCPCB is the sponsor of some of my videos