# BM62 Bluetooth Module Configuration

This directory contains configuration files for the Microchip BM62 Bluetooth Module for use in this project.

## Contents

- `firmware`: BM62 firmware version 2.1.3 hex files.
- `src`: The individual sources for the full EEPROM software and configuration.
    - The two `.txt` files are the source config files that can be edited with the `UI Tool` and the `DSP Tool`.
    - The `IS206X_012_DUALMODESPK2.1_E1.0.4.1_1214.bin` is the "default BIN" file that was originally combined together with the `UI` and `DSP` config source files using the `MPET` tool to produce the output found in the `dist` directory.
- `dist`: The full "compiled" EEPROM software and configuration, ready to be installed onto the BM62.
    - The `.bin` file in this directory is for use as the "default BIN" in the `MPET` tool when building a new full EEPROM file with updates to the `UI` and/or `DSP` config source files.

## Firmware/Software Install

This software and configuration is for firmware version v2.1.3.

Download the `BM64 DSPK v2.1.3` software/tools from [Microchip's product page for the BM62](https://www.microchip.com/en-us/product/BM62).

Follow [these instructions](https://github.com/tomaskovacik/IS2020/wiki/Upgrading-firmware-on-BM62) to install the firmware and full EEPROM configuration, but with the following adjustments:
1. For the first step (using `isbtflash.exe`), you may use the firmware files found in this project for convenience.
1. For the second step (using `isupdate.exe`), select the `dist/DiamondTelM92Bluetooth.txt` file from this project, instead of the `MCHP_DSPKv2.1.3_BM62_StandAlone.txt` file.
1. Skip the third step (using `EEPROM_Tool.exe`), because the `dist/DiamondTelM92Bluetooth.txt` full EEPROM file already includes the UI and DSP config.

NOTE: If you have already previously installed the firmware (step 1), but need to install a new version of the full EEPROM config, you can skip step 1.

## Tools

In addition to the software tools provided by Microchip (see above), you will also need a USB to UART adapter to install the
firmware and configuration onto the BM62 module. I found the `DSD TECH SH-U09C5` to be a very good option (reputable chipset,
supports more baud rate options than some other options, adjustable voltage).

## Configuration Summary

The BM62 configuration for this project is very simple and limited compared to what the BM62 is capable of:

- UART communication enabled at 115200 baud.
- HFP profile only (no audio streaming for music, etc.).
- "Power on directly" (no power button or UART power on/off support).
- No battery charging.
- No user interaction through buttons (all interactions will be done by the MCU via UART commands).
- No automatic linkback when powering on, automatic retries, timeouts, etc. (all linkback/pairing behavior is managed by the MCU via UART commands).