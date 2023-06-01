# BM62 Bluetooth Module Configuration

This directory contains configuration files for the Microchip BM62 Bluetooth Module for use in this project.

- [BM62 Bluetooth Module Configuration](#bm62-bluetooth-module-configuration)
  - [Contents](#contents)
  - [Configuration Summary](#configuration-summary)
  - [Tools](#tools)
  - [Prepare the BM62 for Firmware/Software Install](#prepare-the-bm62-for-firmwaresoftware-install)
  - [Firmware Install](#firmware-install)
  - [Software Install](#software-install)

## Contents

- `firmware`: BM62 firmware version 2.1.5 hex file.
- `src`: The individual sources for the full EEPROM software and configuration.
    - The two `.txt` files are the source config files that can be edited with the `UI Tool` and the `DSP Tool`.
    - The `IS206X_012_DUALMODESPK2.1_E1.0.4.1_1214.bin` is the "default BIN" file that was originally combined together with the `UI` and `DSP` config source files using the `MPET` tool to produce the output found in the `dist` directory.
- `dist`: The full "compiled" EEPROM software and configuration, ready to be installed onto the BM62.
    - The `.bin` file in this directory is for use as the "default BIN" in the `MPET` tool when building a new full EEPROM file with updates to the `UI` and/or `DSP` config source files.

## Configuration Summary

The BM62 configuration for this project is very simple and limited compared to what the BM62 is capable of:

- UART communication enabled at 115200 baud.
- HFP profile only (no audio streaming for music, etc.).
- "Power on directly" (no power button or UART power on/off support).
- No battery charging.
- No user interaction through buttons (all interactions will be done by the MCU via UART commands).
- No automatic linkback when powering on, automatic retries, timeouts, etc. (all linkback/pairing behavior is managed by the MCU via UART commands).

## Tools

Download the `BM64 DSPK 2.1.5` software/tools from [Microchip's product page for the BM62](https://www.microchip.com/en-us/product/BM62).

BEWARE: Various documentation, instructions, readme files, etc., from Microchip have incorrect information about updating firmware and software on the BM62. Follow the instructions in this README.

In addition to the software tools provided by Microchip, you will also need a USB to UART adapter to install the
firmware and configuration onto the BM62 module. I found the `DSD TECH SH-U09C5` to be a very good option (reputable chipset,
supports more baud rates than some other options, adjustable voltage).

## Prepare the BM62 for Firmware/Software Install

It's easiest to install firmware/software on the BM62 if it is completely separable from the rest of the hardware. When developing on a breadboard, this is as easy as temporarily removing some strategic connections on the board. If you're working with a soldered prototype/perf board, it's probably a good idea to use a breadboard adapter and female headers soldered to the perf board so that the BM62 can be transferred to a breadboard for updates. Otherwise, the circuit design will require more complexity to support in-circuit updates.

The following setup is needed for both Firmware and Software installation:
- 22uF cap between `VDD_IO` and ground.
    - See Errata document for more info.
- `EAN` pulled high to `VDD_IO` 
    - Datasheet incorrectly claims EAN has an internal pull-up resistor.
- `P2_0` pulled low.
- `RXD`, `TXD`, Power, ground, and `#RESET` disconnected from the rest of the circuit so that the MCU cannot interfere with the update and no pins are inadvertently pulled low.
- Connect a USB-UART adapter to the BM62.
    - Ground to ground.
    - +5V to `ADAPTER_IN`
    - `TXD` to `RXD`
    - `RXD` to `TXD`

## Firmware Install

This section will guide you through installing the correct version of the core firmware for the BM62. The BM62 very likely comes pre-installed with an older firmware version.

NOTE: If you have previously installed the DSPK 2.1.5 firmware and only need to install a new version of the software for this project, then skip to the [Software Install](#software-install) section.

Consider reviewing the following resources that guided the writing of the following instructions (these were both sent to me by Microchip tech support when asking how to install firmware 2.1.5 on a BM62):
- [A Microchip Knowledge Base article about installing firmware 2.1.5 on a BM64](https://microchipsupport.force.com/s/article/BM64-Firmware-update-for-DSPK-v2-1-5-package).
- [A YouTube video about installing firmware 2.1.5 on a BM62](https://www.youtube.com/watch?v=s2zi_sSPKU0).

Installation instructions:
1. [Prepare the BM62 for Firmware/Software Install](#prepare-the-bm62-for-firmwaresoftware-install)
1. Launch the `DSPK v2.1.5 Package\Tools\FlashUpdate Tool\isupdate.exe` tool.
    - The readme file for these tools says that `isupdate` should not be used for BM62, but there is no way to install firmware version 2.1.5 using the specified `isbtflash` tool for the BM62. I got these instructions from Microchip tech support as a "workaround".
1. Select the following options:
    - `port`: The port that your USB-UART adapter maps to.
    - `baudrate`: 115200
    - `image num`: 1
    - `Memory Type / type`: flash
    - `Memory Type / subtype`: Serial Flash
1. Click `Connect`.
1. Click the first `Browse` button in the `Flash Update/Dump` section and choose the `firmware/BT5506_SHS_FLASH_Rehex_0F95_Rehex_0F3A.HEX` file from here in this project.
1. Click the `Update` button next to the `Browse` button you used.
1. Click `Disconnect` when done.

## Software Install

This section will guide you through installing the customized software/configuration of the BM62 for use in this DiamondTel Model 92 Bluetooth Adapter project.

Installation instructions:
1. [Prepare the BM62 for Firmware/Software Install](#prepare-the-bm62-for-firmwaresoftware-install)
1. Launch the `DSPK v2.1.5 Package\Tools\FlashUpdate Tool\isupdate.exe` tool.
    - Again, the readme for these tools claims that `isupdate` is not for use with the BM62, but this seems to be the only way to install a full EEPROM file on to the BM62. These instructions are thanks to [Tomas Kovacik's blog](https://github.com/tomaskovacik/IS2020/wiki/Upgrading-firmware-on-BM62) (the second section of the instructions about installing the `MCHP_DSPKv2.1.3_BM62_StandAlone` file; but don't install that file!).
1. Select the following options:
    - `port`: The port that your USB-UART adapter maps to.
    - `baudrate`: 115200
    - `image num`: 16
        - This may be irrelevant for software install, but documentation is non-existant, so better safe than sorry?
    - `Memory Type / type`: eeprom
    - `Memory Type / subtype`: default
1. Click `Connect`.
    - NOTE: If doing this immediately after performing a [Firmware Install](#firmware-install), then reset the BM62 before connecting.
1. Click the `Browse` button in the `Flash/EEProm/MCU/AHB Access` section and choose the `dist/DiamondTelM92Bluetooth.txt` file from here in this project.
1. Click the `Write Table` button next to the `Browse` button you used.
1. Click `Disconnect` when done.
