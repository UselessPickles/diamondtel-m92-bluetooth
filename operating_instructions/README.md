# Operating Instructions

- [Operating Instructions](#operating-instructions)
  - [Introduction](#introduction)
    - [Terminology](#terminology)
  - [Original DiamondTel Model 92 Functionality](#original-diamondtel-model-92-functionality)
  - [Enhanced Bluetooth Adapter Functionality](#enhanced-bluetooth-adapter-functionality)
    - [Bluetooth Connection Status Indicator](#bluetooth-connection-status-indicator)
    - [Bluetooth Connection Status Change Beep](#bluetooth-connection-status-change-beep)
    - [Bluetooth Automatic Reconnect](#bluetooth-automatic-reconnect)
    - [Alphanumeric Input](#alphanumeric-input)
    - [Caller ID Display](#caller-id-display)
    - [Call Waiting](#call-waiting)
    - [Bluetooth Pairing (FCN / \* / \*)](#bluetooth-pairing-fcn----)
    - [View Bluetooth Device Name (RCL / \* / \*)](#view-bluetooth-device-name-rcl----)
    - [Change Bluetooth Device Name (RCL / \* / \* / STO)](#change-bluetooth-device-name-rcl------sto)
    - [View Paired Host Phone Name (RCL / \* / #)](#view-paired-host-phone-name-rcl----)
    - [Reset Bluetooth Pairing Memory (RCL / \* / # / STO)](#reset-bluetooth-pairing-memory-rcl------sto)
    - [Voice Dialing/Assistant (END / 1sec)](#voice-dialingassistant-end--1sec)
      - [Alternate Voice Dialing Method (Call "0" or "411")](#alternate-voice-dialing-method-call-0-or-411)
    - [Ringtone Selection (FCN / 3)](#ringtone-selection-fcn--3)
    - [Games (FCN / P1/P2/P3)](#games-fcn--p1p2p3)
      - [Snake](#snake)
      - [Memory](#memory)
      - [Tetris](#tetris)
  - [Programming / Setup](#programming--setup)
    - [Entering Programming Mode](#entering-programming-mode)
    - [Viewing/Editing Programming Options](#viewingediting-programming-options)
    - [Exiting Programming Mode](#exiting-programming-mode)
    - [Programming Options](#programming-options)
      - [DUAL NO](#dual-no)
      - [NO1](#no1)
      - [NO2](#no2)
      - [SEC](#sec)
      - [DIS CU RESET](#dis-cu-reset)
      - [DIS IGN SENSE](#dis-ign-sense)
      - [DIS PWR LOCK](#dis-pwr-lock)
      - [DIS OWN TEL](#dis-own-tel)
      - [CALLER ID](#caller-id)
      - [OEM HF UNIT](#oem-hf-unit)
    
## Introduction

The DiamondTel Model 92 Bluetooth Adapter completely replaces the original electronics
inside the transceiver of the DiamondTel Model 92 Portable Cellular Telephone to
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
      - The `PWR` button is disabled during a call like the original phone, but it is also possible to disable this feature with the `DIS PWR LOCK` programming option. When disabled, and the phone is turned off
      during a call, the call will be transferred to the host phone.
- ✔️ OTHER BASIC OPERATION
    - ✔️ VOLUME ADJUSTMENT
      - There is a 6th volume adjustment supported by the Bluetooth Adapter:
      Game Music volume. Press `FCN` then the `↑`/`↓` keys while in a game with music to adjust Game
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
      - Chose not to replicate (low value or irrelevant, because you'll have the host phone with you when away from the car).
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
      - Due to limited storage on the Bluetooth Adapter, only 6 storage locations are supported (from \*1 to \*6).
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

### Bluetooth Pairing (FCN / \* / \*)

Press `FCN`, `*`, `*` to enter Bluetooth pairing mode and make the Bluetooth adapter discoverable to modern cell phones as a Bluetooth device.

The handset will display "PAIRING" and the Bluetooth adapter will disconnect from any host phone that it is currently connected to.

When a single bar of the `||||||` signal strength indicator begins sweeping back and forth, the Bluetooth adapter has successfully entered pairing mode, and you should be able to soon find the Bluetooth adapter on your modern cell phone's Bluetooth menu.

If you are unsure of the Bluetooth device name you should be looking for on the modern cell phone, then you may [view](#view-bluetooth-device-name-rcl----), or even [change](#change-bluetooth-device-name-rcl------sto), the Bluetooth adapter's device name before entering pairing mode.

You may cancel pairing by pressing the `CLR` button.

If pairing is not successful within about 1 minute, then the Bluetooth adapter will automatically cancel pairing and the handset will display "PAIRING TIMEOUT".

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

Clear out the entered name to return to the input prompt, then press `CLR` again to cancel the name change.

### View Paired Host Phone Name (RCL / \* / #)

Press `RCL`, `*`, `#` to view the name of the most recently paired host phone. 

NOTE: The Bluetooth adapter does not need to be currently connected to a host phone. 

The handset will display "PAIRED:" followed by the name of the paired host phone, or "[none]" if the Bluetooth adapter is not currently paired to any host phone.

Press `CLR` to exit.

### Reset Bluetooth Pairing Memory (RCL / \* / # / STO)

While [viewing the paired host phone name](#view-paired-host-phone-name-rcl----), press `STO` to reset Bluetooth pairing memory. This causes the Bluetooth adapter to "forget" the most recently paired host phone so that it will no longer try to automatically reconnect to that host phone.

The handset will display "INPUT CODE". Enter your 4-digit security code (as configured during [Programming / Setup](#programming--setup)) to confirm and proceed.

The handset will display the paired host phone name again (as "[none]") as confirmation that the Bluetooth adapter is no longer paired to any host phone.

### Voice Dialing/Assistant (END / 1sec)

When not in a call, press and hold `END` for 1 second to initiate the host phone's voice dialing/assistant (e.g., Siri on Apple phones, Google Assistant on Android phones).

The handset will display "VOICE COMMAND", then the `IN USE` indicator will display to indicate that the voice dialing/assistant is successfully activated.

You may now speak your command into the car phone's microphone. If the handset is "on hook", then an external microphone is necessary (connected to the microphone jack of the Bluetooth adapter).

Press `END` to cancel the voice dialing/assistant.

#### Alternate Voice Dialing Method (Call "0" or "411") 

Voice dialing/assistant can also be initiated by placing a call to the phone numbers "0" or "411" (traditional phone numbers that used to call an operator or directory assistance).

This alternate method allows for voice dialing/assistant to be initiated from the OEM Hands-Free system that was available for some Mitsubishi vehicles. The hands-free system can be used to speed dial one of the first 3 directory entries stored on the phone. Simply save the phone number "0" or "411" as one of the first 3 directory entries.

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

### Games (FCN / P1/P2/P3)

Press `FCN`, then `P1`, `P2`, or `P3` to play one of three games:
- `P1`: [Snake](https://en.wikipedia.org/wiki/Snake_(video_game_genre)) - Control a snake to eat food and grow without crashing into yourself or the edge of the screen.
- `P2`: [Memory](https://en.wikipedia.org/wiki/Concentration_(card_game)) - Find pairs of matching cards. Also known as "Concentration", "Matching Pairs", etc.
- `P3`: [Tetris](https://en.wikipedia.org/wiki/Tetris) - Do I really need to describe what Tetris is?

Common features/controls in all games:
- Press `#` on the title screen to begin, or press `CLR` to exit the game.
- In general, press `CLR` on any screen within the game to back out one level toward the title screen, or press and hold `CLR` to completely exit the game.
- When prompted to choose a level, press a numbered button within the specified range. Lower numbered levels are easier. Higher numbered levels are more difficult. 
- While playing a game, press `CLR` to pause the game and exit out to a menu that allows you to choose between continuing the game, or starting a new game. Pressing `CLR` again will back out to the game's title screen.
- When an in-progress game is exited, progress is remembered until the phone is powered off. When entering the game again later, you will be prompted to choose between continuing the current game, or starting a new game.
  - Press `1` to continue the in-progress game.
  - Press `2` to start a new game.
- Press `↑`/`↓` to adjust speaker volume, which controls the volume of sound effects. `Snake` and `Tetris` have sound effects.
- Press `FCN`, `↑`/`↓` to adjust background music in games that have background music. `Tetris` has background music.
  - Background music does not play while on the title screen. This gives you a chance to adjust music volume before it begins playing on the next screen.
- When the the game ends due to a winning or losing condition, a sequence of screens will repeat to show details about the end of the game, such as your score, as well as instructions for playing another game.
  - Press `#` to play the game again with the same difficulty level.
  - Press `*` to start a new game with a different difficulty level.
  - Press `CLR` to exit to the game's title screen.

#### Snake

See the main [Games](#games-fcn--p1p2p3) section for general information that is applicable to all games.

You are a snake, represented on the screen as one or more solid rectangular blocks. The goal is to eat food (`*`) and grow as long as possible. Each piece of food adds one block to the length the snake. The snake starts as only a single block at the beginning of the game.

The head of the snake continuously moves in the last direction it was heading, and the body follows the path of the head. Press one of the following buttons to change direction:
- `2`: Up
- `8`: Down
- `4`: Left
- `6`: Right

The speed of the snake is determined by the difficulty level that was selected when starting the game.

The game ends when:
- The snake crashes into the edge of the screen.
- The snake crashes into itself.
- The snake grows to its maximum possible length (filling the entire screen).

#### Memory

See the main [Games](#games-fcn--p1p2p3) section for general information that is applicable to all games.

Several "cards" are presented face-down (represented as solid rectangular blocks). The face of each card is a symbol, and there are pairs of cards with the same symbol randomly arranged. Your goal is to find each matching pair of cards by revealing only two cards at a time.

The flashing card indicates the position of your cursor.

Controls:
- `2`: Move Up
- `8`: Move Down
- `4`: Move Left
- `6`: Move Right
- `1`/`3`: Move Left/Right, but skip over any revealed cards to the next face-down card, and wrap around to the next/previous row. This allows you to move around more quickly.
- `5`: Reveal the selected face-down card.

The first revealed card will remain revealed until you reveal a second face-down card. The pair of revealed cards will then flash a couple times. If the pair is a match, then they will remain revealed. Otherwise, they will be turned face-down again.

The number of cards is determined by the difficulty level that was selected when starting the game.

The game ends when all pairs of cards have been matched/revealed.

The end-game screen will indicate how many moves (pairs of revealed cards) you made. A lower number of moves is better.

#### Tetris

See the main [Games](#games-fcn--p1p2p3) section for general information that is applicable to all games.

Due to display limitations on the phone, this game of Tetris is played sideways. Pieces "fall" from right to left. There is also a minimum of 2 consecutive lines that must be filled in order to clear lines, and a maximum of 3 consecutive lines can be filled/cleared at a time.

Controls:
- `1`: Rotate piece counter-clockwise.
- `3`: Rotate piece clockwise.
- `2`: Move piece up.
- `5`: Move piece down.
- `4`: "Drop" piece rapidly to the left, while held down.

The scoring and the speed at which pieces fall is determined by the difficulty level.

Earn points by clearing lines. The base point value is determined by the number of lines simultaneously cleared:
- `2 lines`: 1 point
- `3 lines`: 3 points

This base point value is multiplied by the current diffulty level.

The difficulty level that was selected when starting the game only determines the initial difficulty level. Difficulty will automatically increase while playing as you reach milestones of total number or lines cleared as follows:
- `Level 2`: After 20 lines are cleared.
- `Level 3`: After 40 lines are cleared.
- `Level 4`: After 60 lines are cleared.
- `Level 5`: After 80 lines are cleared.
- `Level 6`: After 100 lines are cleared.
- `Level 7`: After 120 lines are cleared.
- `Level 8`: After 140 lines are cleared.
- `Level 9`: After 160 lines are cleared.

For maximum scoring, it is best to start at a higher level, and try to clear 3 lines at a time as much as possible, to maximize points per line cleared.

The game ends when pieces have stacked up to the point that there is no room for the next piece to be placed on the board.

##### High Score

A single high score is stored in memory for `Tetris`, along with player initials.

When a game ends and your score is higher than the current highs core, you will be prompted to enter your initials. Enter up to 3 letters/numbers as [Alphanumeric input](#alphanumeric-input), then press `STO` to save the initials, or press `CLR` to skip entering initials (initials will default to `???`).

You can view the current high score at any time from the title screen of the game by pressing `RCL`. 

While viewing the high score, you can choose to reset the high score by pressing `FCN`, `CLR`, then entering your 4-digit security code (as configured during [Programming / Setup](#programming--setup)). 

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

#### SEC

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

#### DIS IGN SENSE

Disables vehicle ignition sensing.

When NOT disabled, and if external power is connected, then 
the phone cannot be powered on unless the vehicle ignition is on. Turning the vehicle ignition off
will power the phone off. If the phone was last powered off by vehicle ignition, then turning vehicle
ignition on will power the phone on.

When disabled, the vehicle ignition is ignored and the phone can be manually powered on/off at any
time. Vehicle ignition will not power the phone on or off.

**Valid Values**:
- `0`: Vehicle ignition sensing is enabled.
- `1`: Vehicle ignition sensing is disabled.

**Default Value**: `0`

#### DIS PWR LOCK

Disables power-off lock-out behavior when in a call.

When NOT disabled, the phone cannot be powered off during a call. The `PWR` button is disabled.
If mounted in a vehicle, and vehicle ignition is turned off, then the phone will automatically power off
after the call ends or the Bluetooth connection is lost.

When disabled, the phone can be powered during a call, and the call will be transferred to the host phone. 

**Valid Values**:
- `0`: Power-off lock-out is enabled.
- `1`: Power-off lock-out is disabled.

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
