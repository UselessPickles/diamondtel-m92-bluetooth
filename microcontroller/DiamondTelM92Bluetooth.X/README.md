# PIC18F27Q43 Microcontroller Software Project

This directory is a complete [MPLAB X](https://www.microchip.com/en-us/tools-resources/develop/mplab-x-ide) project containing all source code and configuration for the PIC18F27Q43 microcontroller software.

- [PIC18F27Q43 Microcontroller Software Project](#pic18f27q43-microcontroller-software-project)
  - [General](#general)
    - [Sloppy Prototype Under Construction](#sloppy-prototype-under-construction)
    - [MPLAB Code Configurator](#mplab-code-configurator)
    - [Adapted from Microchip's BM62 Demo Software](#adapted-from-microchips-bm62-demo-software)
  - [Peripherals and I/O Pins](#peripherals-and-io-pins)
    - [TMR0 - Call Timer](#tmr0---call-timer)
    - [TMR2 - General Purpose 10ms Timer](#tmr2---general-purpose-10ms-timer)
    - [TMR4 - General Purpose 1ms Timer](#tmr4---general-purpose-1ms-timer)
    - [TMR6 - Sound Sample Output Timer](#tmr6---sound-sample-output-timer)
    - [FVR - Fixed Voltage Reference](#fvr---fixed-voltage-reference)
    - [DAC1 - Sound Sample Output Value](#dac1---sound-sample-output-value)
    - [SPI1, RC3 (SPI1_CS_DPOT) - Volume Control Communication](#spi1-rc3-spi1_cs_dpot---volume-control-communication)
    - [RA6 (IO_VOICE_IN) - Incoming Voice Audio Switching](#ra6-io_voice_in---incoming-voice-audio-switching)
    - [RA5 (IO_MIC_OUT_DISABLE) - Microphone Audio Disable](#ra5-io_mic_out_disable---microphone-audio-disable)
    - [RA7 (IO_MIC_HF_SELECT) - Microphone Audio Switching](#ra7-io_mic_hf_select---microphone-audio-switching)
    - [RA3 (IO_MIC_HF_DETECT) - External Hands-Free Microphone Detection](#ra3-io_mic_hf_detect---external-hands-free-microphone-detection)
    - [UART3 - Handset Communication](#uart3---handset-communication)
    - [UART2 - Bluetooth Module Communication](#uart2---bluetooth-module-communication)
    - [UART1 - STDIO Logging/Debugging](#uart1---stdio-loggingdebugging)
    - [UART4 - Transceiver Communication](#uart4---transceiver-communication)
    - [RC4 (IO_BT_RESET) - Bluetooth Module Reset](#rc4-io_bt_reset---bluetooth-module-reset)
    - [RC5 (IO_BT_MFB) - Bluetooth Module UART Rx Indicator](#rc5-io_bt_mfb---bluetooth-module-uart-rx-indicator)
    - [RB5 (IO_PWR) - Handset Power Button](#rb5-io_pwr---handset-power-button)
    
## General 

### Sloppy Prototype Under Construction

Much of the code is not up to my personal standards, so don't judge me too harshly. This project is still in the transition phase from a quick, sloppy, and incrementally updated proof-of-concept to a more deliberately designed and implemented solution.

In particular, the bulk of the phone functionality in `app.c` is a big confusing mess that needs to be split out separate more manageable files, and the sound engine in `sound.c` is in need of a redesign. Many of the other files still require cleanup and documentation.

### MPLAB Code Configurator

This project uses the MPLAB Code Configurator to generate code for most of the initialization/configuration of various peripherals, pin assignments, etc. 

The `mcc_generated_files` directory contains the generated output of the Code Configurator. Use the Code Configurator to make changes to the peripheral/pin configurations rather than updating these files directly.

NOTE: Some generated files have minor customizations that will need to be preserved if new files are generated.

### Adapted from Microchip's BM62 Demo Software

The demo evaluation board software from Microchip's `BM64 DSPK v2.1.3` software/tools package was used as a starting point for implementing UART communications with the BM62 Bluetooth Module. The `bt_command_send` and `bt_command_decode` source files started as an exact copy/paste from the demo project, then modified/simplified as needed. Otherwise, all other (non-generated) source code in this project is original code for this project.

## Peripherals and I/O Pins

This is a summary of what each peripheral and I/O pin is used for.

### TMR0 - Call Timer

This timer is used exclusively for counting seconds during a call to maintain the call timer (see `call_timer.c`). The timer is only running during a call. 

Because this timer is dedicated to this one purpose, and is started at the beginning of a call, it produces good accuracy of elapsed call time.

### TMR2 - General Purpose 10ms Timer

This timer is setup to trigger every 10ms and is always running. It is used for general purpose low-precision timing of timeouts/intervals throughout the project.

Because this timer is always running, and timing of timeouts/intervals is done in terms of timer interupt counts, be aware that actual amount of between starting a timeout/interval and when it ends/triggers can be short by up to 10ms (e.g., the first "count" can happen nearly immediately if the timer event was already about to trigger).

### TMR4 - General Purpose 1ms Timer

This timer is setup to trigger every 1ms and is always running. It is used for general purpose higher-precision timing of timeouts/intervals throughout the project.

Because this timer is always running, and timing of timeouts/intervals is done in terms of timer interupt counts, be aware that actual amount of between starting a timeout/interval and when it ends/triggers can be short by up to 1ms (e.g., the first "count" can happen nearly immediately if the timer event was already about to trigger).

### TMR6 - Sound Sample Output Timer

This timer is used exclusively for generating sound output (see `tone.c`). It is setup to trigger every 100us, for a sample rate of 10kHz. It is used together with [DAC1](#dac1---sound-sample-output-value) to output the sound samples.

This timer's interrupt is the only high-priority interrupt, to guarantee consistency of the sound output sample rate. 

### FVR - Fixed Voltage Reference

The FVR is setup to provide a 1.024V reference to [DAC1](#dac1---sound-sample-output-value). This conveniently limits the analog sound output to a level that produces the maximum desired audio volume level, and keeps the signal well within the full-swing range of the Op Amp that buffers the signal.

### DAC1 - Sound Sample Output Value

This is used together with [TMR6](#tmr6---sound-sample-output-timer) and [FVR](#fvr---fixed-voltage-reference) to output sound samples (see `tone.c`).

### SPI1, RC3 (SPI1_CS_DPOT) - Volume Control Communication

This is used to communicate with a digital potentiometer via SPI for sound volume control (see `volume.c`). The `RC0` pin is a digital output used for the Chip Select (CS) line.

### RA6 (IO_VOICE_IN) - Incoming Voice Audio Switching

This digital output pin is used to control an analog switch that chooses whether to use incoming voice audio from the Bluetooth Module, or generated sounds from the MCU, as the source of sound to be provided to the phone handset.

### RA5 (IO_MIC_OUT_DISABLE) - Microphone Audio Disable

This digital output pin is used to control an analog switch that disconnects the handset microphone output from the Bluetooth module. This is used to quickly mute the microphone when generating a sound (e.g., a button press beep) so that the generated sound does not feed back into the microphone.

### RA7 (IO_MIC_HF_SELECT) - Microphone Audio Switching

This digital output pin is used to control an analog switch that switches between using the handset microphone audio or the external hands-free mcirophone audio.

### RA3 (IO_MIC_HF_DETECT) - External Hands-Free Microphone Detection

This digital input pin is used to detect when an external hands-free microphone is connected to the audio jack.

### UART3 - Handset Communication

This UART is used to communicate with the DiamondTel Model 92 telephone handset (see `handset.c`). It runs at 800 baud.

### UART2 - Bluetooth Module Communication

This UART is used to communicate with the BM62 Bluetooth Module (see `bt_command.c` and `bt_command_decode.c`). It runs at 115,200 baud.

### UART1 - STDIO Logging/Debugging

This UART is used for general terminal logging/debugging. STDIO is redirected to this UART. It runs at 9600 baud.

I use a USB to UART adapter (e.g., `DSD TECH SH-U09C5`) and [RealTerm](https://realterm.sourceforge.io/) to monitor output on a PC.

### UART4 - Transceiver Communication

This UART is used to communicate with the DiamondTel Model 92 telephone transceiver (see `transceiver.c`). It runs at 800 baud.

### RC4 (IO_BT_RESET) - Bluetooth Module Reset

This digital output pin is used to turn the BM62 Bluetooth module on/off via the BM62 `#reset` pin.

### RC5 (IO_BT_MFB) - Bluetooth Module UART Rx Indicator

This digital output pin is used to toggle the BM62's `MFB` pin to to indicate that it should be prepared to receive UART commands.

### RB5 (IO_PWR) - Handset Power Button

This digital input pin detects when the DiamondTel Model 92's handset power button is pressed (see `handset.c`).