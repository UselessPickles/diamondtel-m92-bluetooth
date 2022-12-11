# BM62 Bluetooth Module Configuration

This directory contains configuration files for the Microchip BM62 Bluetooth Module for use in this project.

- `src`: The individual source configurations that are combined into the final binary.
- `dist`: The combined configuration as a binary, ready to be installed onto the BM62.

## Software/Firmware 

This configuration is for firmware version v2.1.3.

Download the `BM64 DSPK v2.1.3` software/tools from [Microchip's product page for the BM62](https://www.microchip.com/en-us/product/BM62).

Follow [these instructions for updating your BM62 to v2.1.3](https://github.com/tomaskovacik/IS2020/wiki/Upgrading-firmware-on-BM62)
before attempting to install the configuration.

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