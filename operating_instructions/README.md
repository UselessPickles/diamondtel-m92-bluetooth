# Operating Instructions

- [Operating Instructions](#operating-instructions)
  - [Introduction](#introduction)
    - [Terminology](#terminology)
  - [Original DiamondTel Model 92 Functionality](#original-diamondtel-model-92-functionality)
  - [Enhanced Bluetooth Adapter Functionality](#enhanced-bluetooth-adapter-functionality)
    - [Bluetooth Connection Status Indicator](#bluetooth-connection-status-indicator)
    - [Bluetooth Connection Status Change Beep](#bluetooth-connection-status-change-beep)
    - [Bluetooth Automatic Reconnect](#bluetooth-automatic-reconnect)
    - [Bluetooth Pairing (FCN / \* / \*)](#bluetooth-pairing-fcn----)
    - [View Bluetooth Device Name (RCL / \* / \*)](#view-bluetooth-device-name-rcl----)
    - [Change Bluetooth Device Name (RCL / \* / \* / STO)](#change-bluetooth-device-name-rcl------sto)
    - [View Paired Host Phone Name (RCL / \* / #)](#view-paired-host-phone-name-rcl----)
    - [Reset Bluetooth Pairing Memory (RCL / \* / # / STO)](#reset-bluetooth-pairing-memory-rcl------sto)
    - [Ringtone Selection (FCN / 3)](#ringtone-selection-fcn--3)
    - [Alphanumeric Input](#alphanumeric-input)
    - [Caller ID Display](#caller-id-display)
    - [Call Waiting](#call-waiting)
  - [Programming / Setup](#programming--setup)
    - [Entering Programming Mode](#entering-programming-mode)
    - [Viewing/Editing Programming Options](#viewingediting-programming-options)
    - [Exiting Programming Mode](#exiting-programming-mode)
    - [Programming Options](#programming-options)
      - [DUAL NO](#dual-no)
      - [NO1](#no1)
      - [NO2](#no2)
      - [SECURITY](#security)
      - [DIS CU RESET](#dis-cu-reset)
      - [DIS OWN TEL](#dis-own-tel)
      - [CALLER ID](#caller-id)
      - [OEM HF UNIT](#oem-hf-unit)
    
## Introduction

The DiamondTel Model 92 Bluetooth Adapter connects in between the handset and
transceiver of an original DiamondTel Model 92 Portable Cellular Telephone to
enable the vintage phone to connect to a modern cell phone via Bluetooth and
make/receive calls.

A primary goal during the development of this Bluetooth Adapter was to
retain/replicate much of the original phone's functionality as much as
reasonably possible. Some original features were dropped for various reasons
(irrelevant to the Bluetooth-based capabilities, high complexity:value ratio,
etc.), some original features were altered for various reasons (necessary for
proper integration of Bluetooth capabilities, technical limitations, etc.), and
some completely new features were added for various reasons (necessary for
proper integration of Bluetooth capabilities, adding modern features, or just
for fun).

### Terminology

The following terminology is used throughout these instructions to distinguish
between the different components of the system, which can become confusing due
to the involvement of two phones.

- `"the phone"` - In general, any mention of "the phone" is referring to the
DiamondTel Model 92 phone.
- `host phone` - The "host phone" is the modern cell phone that is connected via
Bluetooth to the DiamondTel Model 92 via Bluetooth Adapter.

## Original DiamondTel Model 92 Functionality

This section clarifies which of the DiamondTel Model 92's original features are
supported. What follows is a replica of the much of the table of contents from
the original `DiamondTel Model 92 Operating Instructions` with indication of
which features are replicated/supported by the Bluetooth Adapter (✔️), and which features are not replicated/supported (❌). Supported features may have
additional notes about differences compared to the original phone. Otherwise,
refer to the original operating instructions (found in the `telephone` directory
of this project) for full details.

 - ✔️ STATUS INDICATORS
    - The `||||||` signal strength indicator has additional meanings for the
    Bluetooth Adapter:
      - All bars flashing: No Bluetooth connection.
      - One bar sweeping back and forth: Bluetooth pairing.
    - The `HORN` indicator is not used.
    - All other indicators work as expected. For example, the `NO SVC` and `ROAM` indicators accurately represent whether the host phone has no service or is roaming, and the `||||||` signal strength indicator represents the host phone's signal strength.    
 - ✔️ CONTROL KEYS
    - `STO`/`RCL`: Due to limited storage on the Bluetooth Adapter, only 24
    digits per phone number and 29 storage locations are supported (from 01 to 29).
 - EXTENDED BASIC FEATURES
    - See the EXTENDED FEATURE OPERATIONS section for per-feature details.
 - ✔️ BASIC OPERATION
    - ✔️ TURNING ON THE PHONE
      - After the initial power-on sequence, the `NO SVC` indicator will remain
      on and the `||||||`  signal strength bars will flash while the Bluetooth
      Adapter attempts to reconnect to the most recently connected host phone.
    - ✔️ PLACING A CALL
    - ✔️ RECEIVING A CALL
      - If Caller ID is enabled, then the Caller ID text will be displayed on
      the top row.
    - ✔️ ENDING A CALL
    - ✔️ TURNING OFF THE PHONE
      - Due to technical limitations, the `PWR` key is NOT disabled during a
      call, and the phone will NOT stay on during a call in vehicular mode when
      the ignition is turned off.
      - If the phone turns off during a call, then the call audio is simply
      transferred to the host phone.
      - If in-vehicle use with the ignition off is desired, consider setting the `DIS IGN SENSE` programming option on the original car phone to prevent the original transceiver from powering on/off automatically with the vehicle ignition. However, be aware that you must always manually turn the phone on/off.
- ✔️ OTHER BASIC OPERATION
    - ✔️ VOLUME ADJUSTMENT
      - There is a 6th volume adjustment supported by the Bluetooth Adapter:
      Game Music volume. Press the `↑`/`↓` keys while in a game to adjust Game
      Music volume.
    - ✔️ STORING OFTEN-USED NUMBERS
      - Due to limited storage on the Bluetooth Adapter, only 24
      digits per phone number and 29 storage locations are supported (from 01 to 29).
      - NOTE: Original instructions for erasing an entry are misleading. There's
      no specific need to press the `CLR` key for 0.5 seconds. That is only
      needed to clear the current number if the display is not empty.
    - ✔️ RECALLING OFTEN-USED NUMBERS
      - Due to limited storage on the Bluetooth Adapter, only 24
      digits per phone number and 29 storage locations are supported (from 01 to 29).
    - ✔️ HOOK-FLASH REQUEST
      - Modern cell phone service has no concept of a "hook-flash request".
      Instead, this is now used place a call on hold or take a call off hold.
      Unfortunately, technical limitations prevent the Bluetooth Adapter from
      indicating when the call is on hold. 
      - Also used two swap between two calls (Call Waiting).
    - ✔️ LAST NUMBER REDIAL
      - NOTE: This recalls the last number called from the phone, which may not
      be the last number called from the host phone.
    - ✔️ DIALED NUMBER CHECK
      - For consistency with the reduced maximum size of stored phone numbers,
      only 24 digits can be typed. Pressing and holding `RCL` will show the ten
      most signifigant digits for as long as the `RCL` key is held.
    - ✔️ CALL IN ABSENCE INDICATOR
      - NOTE: Only indicates a missed call that occurs while the Bluetooth
      Adapter is connected to the modern phone that received the missed call.
      - If Caller ID is enabled, then the missed call's Caller ID text will be
      displayed on the top row.
    - ✔️ LOW BATTERY INDICATOR
    - ✔️ BACKLIGHTING
- EXTENDED FEATURE OPERATIONS
    - ❌ ~~SYSTEM A/B SELECTION (FCN / 1)~~
      - Irrelevant for modern phone service/tech.
    - ❌ ~~FORCING SYSTEM A/B OPERATION~~
      - Irrelevant for modern phone service/tech.
    - ❌ ~~LOCKING/UNLOCKING THE PHONE (FCN / 2)~~
      - Chose not to replicate (high complexity, low value).
    - ❌ ~~EXTERNAL HORN ALERT (FCN / 3)~~
      - Technical limitations and complexity make this impractical to replicate.
    - ✔️ ACCUMULATED TALK TIME (FCN / 4)
    - ❌ ~~DISCONTINUOUS TRANSMISSION MODE (FCN / 5)~~
      - Irrelevant for modern phone service/tech.
    - ❌ ~~CALL RESTRICTION (FCN / 6)~~
      - Chose not to replicate (high complexity, low value).
    - ❌ ~~PROGRAMMING OF UNLOCK CODE (FCN / 7)~~
      - Chose not to replicate (high complexity, low value).
    - ✔️ ALPHABET NAME AND NUMBER STORE (FCN / 8)
      - See the [Alphanumeric Input](#alphanumeric-input) section for details about how name input is improved.
    - ✔️ ALPHABET SCAN (FCN / 9)
    - ✔️ PAUSE FUNCTION (FCN / 0)
    - ❌ ~~AUTOMATIC RETRY ON/OFF (FCN / #)~~
      - Chose not to replicate (unlikely to ever be used).
    - ✔️ CREDIT CARD NUMBER SECURITY (FCN / RCL)
    - ✔️ ANNOUNCE BEEP ON/OFF (FCN / \* / 1)
    - ✔️ STATUS CHANGE BEEP ON/OFF (FCN / \* / 2)
    - ❌ ~~STATUS MONITOR ON/OFF (FCN / \* / 3)~~
      - Irrelevant for modern phone service/tech.
    - ❌ ~~USER PROGRAMMABLE AREA CODE (FCN / \* / 4)~~
      - Irrelevant for modern phone service/tech (always dial full phone numbers
      with area code).
    - ✔️ BATTERY VOLTAGE DISPLAY (FCN / \* / 5)
      - While viewing battery voltage, press `FCN` to toggle between viewing the
      phone's own battery voltage ("BATTERY") and viewing the host phone's
      approximate battery level ("CELLBAT").
    - ❌ ~~RF OUTPUT POWER CHANGE (FCN / \* / 6)~~
      - Irrelevant for modern phone service/tech.
    - ✔️ AUTOMATIC ANSWER (FCN / \* / 7)
    - ❌ ~~DATA INTERFACE (RJ-11) (FCN / \* / 8)~~
      - Technical limitations and complexity make this impractical to replicate.
    - ✔️ SILENT SCRATCH PAD (FCN / MUTE)
    - ✔️ AUTO STORE (FCN / STO)
    - ✔️ ONE TOUCH DIAL NUMBER STORE/RECALL (STO, RCL P1/P2/P3)
    - ✔️ AUTOMATIC DTMF (FCN / SEND)
    - ✔️ RESET CUMULATIVE TIMER (FCN / CLR)
    - ✔️ LCD VIEW ANGLE CONTROL (FCN / 2sec)
    - ✔️ OWN TELEPHONE NUMBER DISPLAY/DUAL NAM (RCL / #)
    - ✔️ MEMORY SCAN (RCL / ↑/↓)
    - ✔️ DTMF OVERDIALLING FROM MEMORY
      - Original instructions seem incorrect and did not work on the original
      phone when simulating a call with a Mobile Service Tester. Recalling a
      memory address puts the phone into "memory scan mode", where the `FCN`
      key switches to "alphabet scan mode" if the entry has a stored name.
      - With the Bluetooth Adapter, follow the original instructions, but skip
      the `FCN` step. 
      - More generally: While in a call, and while viewing a a memory address
      entry (in either memory or alphabet scan mode), pressing the `SEND` key
      will send the phone number of that entry as DTMF tones. 
    - ✔️ VOICE MUTE
    - ✔️ TALK TIME
      - `CLR` key behavior differs slightly from original instructions, but
      seems to match actual original behavior: 
        - Press `CLR` to hide the call timer and view the dialed number (not the
        memory address location, and regardless of whether the dialed number is
        in the directory).
        - After clearing out the number display, the call timer can be toggled
        with the `CLR` key.
    - ❌ ~~CUMULATIVE TIMER INCREMENT~~
      - Chose to simplify by removing configurability. Cumulative timer always
      counts in 1-minute increments.
    - ❌ ~~DUAL HANDSET~~
      - Not enough information to replicate. Probably requires additional
      hardware that is not described anywhere.
    - ❌ ~~ROAM INHIBIT~~
      - Irrelevant for modern phone service/tech.

## Enhanced Bluetooth Adapter Functionality

This section explains all of the new functionality provided by the Bluetooth adapter that is not a replica of original DiamondTel Model 92 functionality.

### Bluetooth Connection Status Indicator

When the Bluetooth adapter is not connected to a host phone, all 6 bars of the `||||||` signal strength indicator will flash, and the `NO SVC` indicator will also be displayed.

If the `||||||` signal strength indicator is not flashing, then the Bluetooth adapter is connected to a host phone. The `||||||` and `NO SVC` indicators now represent the true status of the host phone.

### Bluetooth Connection Status Change Beep

If the `STATUS CHANGE BEEP` feature is enabled (refer to the original `DiamondTel Model 92 Operating Instructions`), then a sequence of 2 tones will indicate when the Bluetooth adapter connects to or disconnects from a host phone:
- Low tone, high tone: Bluetooth connected.
- High tone, low tone: Bluetooth disconnected.

### Bluetooth Automatic Reconnect

In the following situations, the Bluetooth adapter will automatically attempt to reconnect to the host phone that it was most recently paired to:
- Power on.
- Bluetooth connection lost (e.g., if you move the host phone out of range, then it will attempt to reconnect when brough back in range).
- Bluetooth pairing is canceled or timed out.

### Bluetooth Pairing (FCN / \* / \*)

Press `FCN`, `*`, `*` to enter Bluetooth pairing mode and make the Bluetooth adapter discoverable to modern cell phones as a Bluetooth device.

The handset will display "PAIRING" and the Bluetooth adapter will disconnect from any host phone that it is currently connected to.

When a single bar of the `||||||` signal strength indicator begins sweeping back and forth, the Bluetooth adapter has successfully entered pairing mode, and you should be able to soon find the Bluetooth adapter on your modern cell phone's Bluetooth menu.

If you are unsure of the Bluetooth device name you should be looking for on the modern cell phone, then you may [view](#view-bluetooth-device-name-rcl----), or even [change](#change-bluetooth-device-name-rcl------sto), the Bluetooth adapter's device name before entering pairing mode.

You may cancel pairing by pressing the `CLR` button.

If pairing is not successful within about 1 minute, then the Bluetooth adapter will automaticall cancel pairing and the handset will display "PAIRING TIMEOUT".

When pairing is successful, the handset will display "PAIRED:", followed by the name of the paired host phone.

### View Bluetooth Device Name (RCL / \* / \*)

Press `RCL`, `*`, `*` to view the current name of the Bluetooth adapter. This is the name that will appear on modern cell phones when pairing or connected to the Bluetooth adapter.

The handset will display "NAME:", followed by the Bluetooth device name.

NOTE: The default name is "DiamondTel Model 92".

### Change Bluetooth Device Name (RCL / \* / \* / STO)

While [viewing the Bluetooth device name](#view-bluetooth-device-name-rcl----), press `STO` to begin entering a new name.

The handset will display "INPUT CODE". Enter your 4-digit security code (as configured during [Programming / Setup](#programming--setup)) to confirm and proceed.

The handset will now display "BT NAME ?" and you may begin [Alphanumeric Input](#alphanumeric-input) of the new Bluetooth device name, up to 32 characters.

Press `STO` to save the new name.

Clear out the entered name to return to the input ptompt, then press `CLR` again to cancel the name change.

### View Paired Host Phone Name (RCL / \* / #)

Press `RCL`, `*`, `#` to view the name of the most recently paired host phone. 

NOTE: The Bluetooth adapter does not need to be currently connected to a host phone. 

The handset will display "PAIRED:" followed by the name of the paired host phone, or "[none]" if the Bluetooth adapter is not currently.

Press `CLR` to exit.

### Reset Bluetooth Pairing Memory (RCL / \* / # / STO)

While [viewing the paired host phone name](#view-paired-host-phone-name-rcl----), press `STO` to reset Bluetooth pairing memory. This causes the Bluetooth adapter to "forget" the most recently aired host phone so that it will no longer try to automatically reconnect to that host phone.

The handset will display "INPUT CODE". Enter your 4-digit security code (as configured during [Programming / Setup](#programming--setup)) to confirm and proceed.

### Ringtone Selection (FCN / 3)

Press `FCN`, `3` to select your preferred ringtone.

The handset will display "RINGER" followed by the name of the currently selected ringtone.

Press a number to select a ringtone as follows:
- `1`: `CLASSIC` (default) - A replica of the original DiamondTel Model 92 ringtone.
- `2`: `SMOOTH` - A "smoother" alternative to the `CLASSIC` ringtone.
- `3`: `NOKIA` - The classic [Nokia ringtone](https://www.youtube.com/watch?v=Vk4KK-gh0FM).
- `4`: `AXEL F` - The intro to the song ["Axel F" by Harold Faltermeyer](https://www.youtube.com/watch?v=Qx2gvHjNhQ0), made famous by the 1985 movie "Beverly Hills Cop".
- `5`: `SANS` - The catchy part of the song ["Megalovania" from the game "Undertale"](https://www.youtube.com/watch?v=0FCvzsVlXpQ) (music for the boss fight with Sans).
- `6`: `C.PHONE` - The chorus of the song ["Car Phone" by Julian Smith](https://www.youtube.com/watch?v=crm79j1qeqA).
- `7`: `TETRIS` - The [theme song from the game "Tetris"](https://www.youtube.com/watch?v=NmCCQxVBfyM).


While in the ringtone selection mode:
- Press `RCL` to toggle a preview of the selected ringtone.
- Press `↑`/`↓` to adjust the `ALERT` (ringer) volume.

Press `CLR` to exit ringtone selection.

### Alphanumeric Input

See the `ALPHABET NAME AND NUMBER STORE` section of the original `DiamondTel Model 92 Operating Instructions` (specifically the section about entering a name) for a complete explanation of how alphanumeric input generally works.

The Bluetooth adapter extends this functionality by allowing lowercase letters to be entered. Each button now cycles through 6 possibilities instead of only 3.

For example, the `2` button now cycles through `A`, `B`, `C`, `a`, `b`, `c`.

### Caller ID Display

If [enabled during Programming / Setup](#programming--setup), then caller ID text (either a name or phone number) will be displayed on the handset above the "CALL" text during an incoming call, or when a missed call is displayed.

### Call Waiting

If a second call is received while you are already in a call, then you will hear your host phone's standard "call waiting" tones through the call audio, and the handset will display "CALL" just like a normal incoming call (and [Caller ID](#caller-id-display), if enabled), but the `IN USE` indicator will be displayed due to the current active call.

You have 4 options to deal with an incoming call waiting:
1. Panic and do nothing, because Call Waiting is confusing. The second caller will eventually give up or be sent to voice mail.
1. Press `SEND` to place the current call on hold and answer the incoming call.
1. Press `END` to end the current call and answer the incoming call.
1. Press `FCN`, `END` to remain in the current call and reject the incoming call.

If you chose to place the current call on hold and answer the incoming call, then the letter `W` will now be displayed to the left of the call timer display, indicating that there is another call waiting on hold.

While in a call with a second call on hold, you can manage the two calls as follows:
1. Press `SEND` to swap between the two calls.
1. Press `END` to end the current call and swap to the on-hold call.
1. Press `FCN`, `END` to end the on-hold call.

If the current call is ended by the other particpant, then the on-hold call will remain on hold. Press `SEND` to take the call off hold.

Tips for remembering how to deal with call waiting:
- The `SEND` button consistently always swaps calls without ending either call.
- The `END` button consistently always ends the current call.
- The `FCN`, `END` button sequence consistently always ends the "other" call.

## Programming / Setup

The Bluetooth Adapter supports a "programming mode" for configuring core options
of the phone, much like the original DiamondTel Model 92. However, it is much
simpler than the original programming mode because there is no need to setup
cell phone carrier settings.

### Entering Programming Mode

Entering programming mode is done the same as the original phone:

1. Turn the phone on.
1. After the initial startup sequence (as soon as the `||||||` signal strength indicators start,
flashing), Press and hold the `CLR` key while entering the code `1591426`. This
must be completed within 10 seconds of startup.
    - NOTE: This code can only be used to enter programming mode up to 3 times.
    An alternate "programming reset" code (`8291112`) can ALWAYS be used to
    enter programming mode, and will also reset the ability to use the `1591426`
    code. This mimics the original phone behavior for maximum authenticity.
    The standard (3-times-only) code is included in the original Operating
    Instructions, while the "programming reset" code was a "secret" code known to
    "authorized" personnel only. The "programming reset" code is therefore the 
    more convenient code to use for programming.
1. If successful, the Bluetooth Adapter will disconnect from the host phone, and 
you will see the first programming option flashing on the screen.

### Viewing/Editing Programming Options

When first entering programming programming mode, you will see the first
option's label and value flashing on the screen. Each programming option has a 
simple numeric value. From here, you can either advance to the next option by
pressing `SEND`, or enter a new value for the current option.

While a programming option is flashing on the screen, begin entering a
numeric value to edit its value. The display will clear and stop flashing,
showing only the value you are entering.

Press `CLR` to cancel editing and return to viewing the current value of the
programming option.

Press `SEND` to save the new value. If the entered value is valid, it will be 
saved and programming will advance to the next option. If the entered value is
invalid, you will remain in editing mode. From here, you can either continue 
typing more digits if necessary to make the value valid, or cancel editing and 
start over.

### Exiting Programming Mode

While viewing (not editing) a programming option, press the `END` key. The
phone will restart with the new programming options applied.

NOTE: Each change to a programming option is saved immediately. There is no way
to cancel/exit programming mode without applying the changes you have already
made.

### Programming Options

#### DUAL NO

If enabled, then two phone numbers are associated with this phone instead of 
only one. See the `OWN TELEPHONE NUMBER DISPLAY/DUAL NAM` section of the
original `DiamondTel Model 92 Operating Instructions` for details about
switching between between dual numbers.

**Valid Values**:
- `0`: Dual Number disabled.
- `1`: Dual Number enabled.

**Default Value**: `0`

#### NO1

This is the primary phone number associated with this phone. This is entirely
cosmetic and has no impact on functionality (it does not need to match your
actual phone number). This number may be displayed during the power-on sequence,
or manually recalled (See the `OWN TELEPHONE NUMBER DISPLAY/DUAL NAM` section of
the original `DiamondTel Model 92 Operating Instructions`).

**Valid Values**: Any 7-digit or 10-digit number.

**Default Value**: `0000000000`

#### NO2

Only available if the `DUAL NO` programming option is enabled.

This is the secondary phone number associated with this phone. This is entirely
cosmetic and has no impact on functionality (it does not need to match your
actual phone number). This number may be displayed during the power-on sequence,
or manually recalled (See the `OWN TELEPHONE NUMBER DISPLAY/DUAL NAM` section of
the original `DiamondTel Model 92 Operating Instructions`).

**Valid Values**: Any 7-digit or 10-digit number.

**Default Value**: `0000000000`

#### SECURITY

This is a 4-digit security code that must be entered to access certain features
of the phone.

**Valid Values**: Any 4-digit number.

**Default Value**: `0000`

#### DIS CU RESET

Disables the ability to reset the cumulative talk timer. See the 
`RESET CUMULATIVE TIMER` section of the original
`DiamondTel Model 92 Operating Instructions`.

**Valid Values**:
- `0`: Cumulative timer reset is enabled.
- `1`: Cumulative timer reset is disabled.

**Default Value**: `0`

#### DIS OWN TEL

Disables the display of the phone's own configured phone number (`NO1` or `NO2`
programming option) during the phone's power-on sequence.

**Valid Values**:
- `0`: Own telephone number display is enabled.
- `1`: Own telephone number display is disabled.

**Default Value**: `0`

#### CALLER ID

Enables Caller ID for incoming calls.

**Valid Values**:
- `0`: Caller ID is disabled (authentic original behavior).
- `1`: Caller ID is enabled: display name (phone number is displayed if name is
unavailable).
- `2`: Caller ID is enabled: display phone number.

**Default Value**: `0`

#### OEM HF UNIT

Adjusts some behavior for compatibility with the OEM Hands-Free Controller Unit
that was available for some Mitsubishi vehicles as part of a fully integrated
car phone accessory. When enabled, the adjusted behavior is only applied when
an external microphone connection is detected. For example, Caller ID (if 
enabled) is automatically disabled to avoid interfering with the Hands-Free
Controller's ability to properly detect the start and end of an incoming call.

This option should be enabled only if you are using the Bluetooth Adapter
together with the OEM Hands-Free Controller Unit.

**Valid Values**:
- `0`: OEM Hands-Free Controller compatibility is disabled.
- `1`: OEM Hands-Free Controller compatibility is enabled.

**Default Value**: `0`
