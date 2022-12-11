# PIC18F27Q43 Microcontroller 

This directory contains information and software for the Microchip PIC18F27Q43 microcontroller.

The microcontroller software implements the majority of the functionality of this project:

- Interfaces with the DiamondTel Model 92 handset for user I/O.
- Interfaces with the DiamondTel Model 92 transceiver to monitor the battery level.
- Interfaces with the BM62 Bluetooth Module to pair with a modern cell phone as a "Hands Free" device, sending/receiving calls, etc.
- Interfaces with a digital potentiometer and digital switch IC to manage sound volume and routing.
- Implements all the behavior that makes the phone act like a phone.

## Software Source Code

The `DiamondTelM92Bluetooth.X` directory is a complete [MPLAB X](https://www.microchip.com/en-us/tools-resources/develop/mplab-x-ide) project containing all source code and configuration for the PIC18F27Q43 software.

See the README in that directory for more details on the project organization.