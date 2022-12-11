# PIC18F27Q43 Microcontroller Software Project

This directory is a complete [MPLAB X](https://www.microchip.com/en-us/tools-resources/develop/mplab-x-ide) project containing all source code and configuration for the PIC18F27Q43 microcontroller software.

## Sloppy Prototype Under Construction

Much of the code is not up to my personal standards, so don't judge me too harshly. This project is still in the transition phase from a quick, sloppy, and incrementally updated proof-of-concept to a more deliberately designed and implemented solution.

In particular, the bulk of the phone functionality in `app.c` is a big confusing mess that needs to be split out separate more manageable files, and the sound engine in `sound.c` is in need of a redesign. Many of the other files still require cleanup and documentation.

## MPLAB Code Configurator

This project uses the MPLAB Code Configurator to generate code for most of the initialization/configuration of various peripherals, pin assignments, etc. 

The `mcc_generated_files` directory contains the generated output of the Code Configurator. Use the Code Configurator to make changes to the peripheral/pin configurations rather than updating these files directly.

NOTE: Some generated files have minor customizations that will need to be preserved if new files are generated.

## Adapted from Microchip's BM62 Demo Software

The demo evaluation board software from Microchip's `BM64 DSPK v2.1.3` software/tools package was used as a starting point for implementing UART communications with the BM62 Bluetooth Module. The `bt_command_send` and `bt_command_decode` source files started as an exact copy/paste from the demo project, then modified/simplified as needed. Otherwise, all other (non-generated) source code in this project is original code for this project.