/** 
 * @file
 * @author Jeff Lau
 * 
 * Tools for monitoring the DiamondTel Model 92's Transceiver battery state.
 * 
 * The MCU communicates to the Transceiver via UART4 as if the MCU was a Handset.
 * 
 * When we want to poll the battery level, we send commands as if the user is
 * requesting the Transceiver to display battery level on the Handset display,
 * then we parse the commands from the Transceiver that print the battery level.
 * 
 * To monitor whether the battery level is "low" or not, we monitor whether
 * the Transceiver is currently flashing the PWR indicator off/on (indicates
 * low battery).
 */

#include "transceiver.h"
#include "handset.h"
#include "../ui/indicator.h"
#include "../../mcc_generated_files/uart4.h"
#include "../util/timeout.h"
#include "../util/interval.h"
#include "../sound/sound.h"
#include <stdio.h>

/**
 * Amount of time (hundredths of a second) to wait for indication that the 
 * Transceiver is ready to process simulated handset button presses.
 * 
 * After this timeout, we assume that the Transceiver had already previously 
 * powered on and we missed the "ready" indication. This happens during 
 * development when restarting/rewriting the program to the MCU.
 */
#define TRANSCEIVER_READY_TIMOUT (400)

/**
 * Amount of time (hundredths of a second) between automatic polling
 * of the Transceiver battery level.
 */
#define BATTERY_LEVEL_REQUEST_INTERVAL (6000)

/**
 * Amount of time (hundredths of a second) to wait after requesting the 
 * Transceiver battery level before giving up.
 */
#define BATTERY_LEVEL_REQUEST_TIMEOUT (200)

/**
 * Amount of time (hundredths of a second) that the Transceiver's PWR indicator 
 * must remain on solid before we assume that the battery level is no longer low.
 * 
 * NOTE: The PWR indicator flashes on/off once per second when the battery is low.
 */
#define DEFER_BATTERY_LEVEL_OK_EVENT_TIMEOUT (200)

/**
 * Module state.
 */
static struct {
  /**
   * Event handler function pointer.
   */
  TRANSCEIVER_EventHandler eventHandler;
  /**
   * Timeout used to give up on waiting for indication that the Transceiver is ready.
   */
  timeout_t transceiverReadyTimeout;
  /**
   * Interval used for automatic periodic polling of the battery level.
   */
  interval_t batteryLevelRequestInterval;
  /**
   * Timeout used to give up waiting for a battery level response.
   */
  timeout_t batteryLevelRequestTimeout;
  /**
   * Used to accumulate the battery level value from the Transceiver's response.
   */
  uint8_t pendingBatteryLevel;
  /**
   * The current battery level.
   * 
   * Normal range is 1-5, or 0 if unknown.
   */
  uint8_t batteryLevel;
  /**
   * Timeout used to confirm that the PWR indicator is staying on solid,
   * meaning that the battery level is no longer low.
   */
  timeout_t deferBatteryLevelOkEventTimeout;
  /**
   * True if the battery level is currently "low".
   */
  bool isBatteryLevelLow;
  /**
   * This timeout is used to keep track of when a simulated button press was 
   * recently sent to the transceiver so that certain commands from the 
   * transceiver may be assumed to be a result of the button press, rather
   * than an unsolicited "event".
   */
  timeout_t recentSimulatedButtonPressTimeout;
  /**
   * True if the transceiver is connected to external power.
   * 
   * Assumed to be true during initialization until evidence of no external 
   * power is received.
   */
  bool isConnectedToExternalPower;
  /**
   * True if the PWR button is currently pressed.
   * 
   * NOTE: This is maintained by handling HANDSET button events, which means
   * that this is guaranteed to be false during power-on when the PWR button
   * is still held from powering on. This is intentional and necessary to 
   * ensure that we only handle the PWR button being pressed AFTER power-on.
   */
  bool isPowerButtonPressed;
  /**
   * True if the power-off process has begun (PWR button has been held long 
   * enough that the phone will now power off upon release).
   */
  bool isPoweringOff;
} module;

/**
 * Send Handset events to the Transceiver to simulate a complete press and
 * release of a Handset button.
 * 
 * Also starts (or restarts) the `recentSimulatedButtonPressTimeout` so 
 * certain subsequent commands from the Transceiver can be ignored as being a 
 * direct consequence of the simulated button press.
 * 
 * @param button - The button to simulate.
 */
static void simulateButtonPress(HANDSET_Button button) {
  UART4_Write(button);
  UART4_Write(HANDSET_UartEvent_RELEASE);
  TIMEOUT_Start(&module.recentSimulatedButtonPressTimeout, 50);
}

/**
 * Check if a button press was recently simulated.
 * @return True if a button press was recently simulated.
 */
static bool wasButtonPressRecentlySimulated(void) {
  return TIMEOUT_IsPending(&module.recentSimulatedButtonPressTimeout);
}

void TRANSCEIVER_Initialize(TRANSCEIVER_EventHandler eventHandler) {
  module.eventHandler = eventHandler;
  TIMEOUT_Start(&module.transceiverReadyTimeout, TRANSCEIVER_READY_TIMOUT);
  INTERVAL_Initialize(&module.batteryLevelRequestInterval, BATTERY_LEVEL_REQUEST_INTERVAL);
  TIMEOUT_Cancel(&module.batteryLevelRequestTimeout);
  module.pendingBatteryLevel = 0;
  module.batteryLevel = 0;
  TIMEOUT_Cancel(&module.deferBatteryLevelOkEventTimeout);
  module.isBatteryLevelLow = false;
  TIMEOUT_Cancel(&module.recentSimulatedButtonPressTimeout);
  module.isConnectedToExternalPower = true;
  module.isPowerButtonPressed = false;
  module.isPoweringOff = false;
}

void TRANSCEIVER_Task(void) {
  if (UART4_is_rx_ready()) {
    HANDSET_UartCmd cmd = UART4_Read();
    
    if (cmd == HANDSET_UartCmd_INDICATOR_PWR_OFF) {
      // The Transceiver flashes the PWR indicator off/on while the battery is low.
      // As soon as the PWR indicator is turned off, we know the battery level is
      // now low.
      if (!module.isBatteryLevelLow) {
        printf("[TSCVR] Battery Level LOW!\r\n");
        module.isBatteryLevelLow = true;
        module.eventHandler(TRANSCEIVER_EventType_BATTERY_LEVEL_IS_LOW);
        TRANSCEIVER_PollBatteryLevelNow();
      }
      
      // Don't interpret the previous PWR on command to mean that the battery 
      // level is OK.
      TIMEOUT_Cancel(&module.deferBatteryLevelOkEventTimeout);
    } else if (cmd == HANDSET_UartCmd_INDICATOR_PWR_ON) {
      // If the Transceiver turns the PWR indicator on while the battery low,
      // it may mean that the battery level is now OK (if the PWR indicator 
      // remains on), or it may jut be continuing to flash because the battery
      // is low. Start a timeout to assume the battery level is OK if we don't
      // receive another PWR off command.
      if (module.isBatteryLevelLow) {
        TIMEOUT_Start(&module.deferBatteryLevelOkEventTimeout, DEFER_BATTERY_LEVEL_OK_EVENT_TIMEOUT);
      }
    } else if ((cmd == HANDSET_UartCmd_BACKLIGHT_OFF) && module.isConnectedToExternalPower) {
      printf("[TSCVR] Disconnected from external power.\r\n");
      module.isConnectedToExternalPower = false;
      module.eventHandler(TRANSCEIVER_EventType_DISCONNECTED_FROM_EXTERNAL_POWER);
    } else if (
        (cmd == HANDSET_UartCmd_BACKLIGHT_ON) && 
        !module.isConnectedToExternalPower &&
        !wasButtonPressRecentlySimulated()
        ) {
      printf("[TSCVR] Connected to external power.\r\n");
      module.isConnectedToExternalPower = true;
      module.eventHandler(TRANSCEIVER_EventType_CONNECTED_TO_EXTERNAL_POWER);
    } else if ((cmd == HANDSET_UartCmd_SET_SIGNAL_STRENGTH_0) && TIMEOUT_IsPending(&module.transceiverReadyTimeout)) {
      // SET_SIGNAL_STRENGTH_0 is one of the final commands sent by the
      // Transceiver during its power-on sequence, after which the Transceiver
      // is ready to start processing input from the handset. So start polling 
      // the battery level now.
      // NOTE: Ignore this if the transceiverReadyTimeout has already timed out.
      printf("[TSCVR] Transceiver Ready\r\n");
      INTERVAL_Start(&module.batteryLevelRequestInterval, true);
      TIMEOUT_Cancel(&module.transceiverReadyTimeout);
    } else if ((cmd == HANDSET_UartCmd_LOUD_SPEAKER_ON) && module.isPowerButtonPressed) {
      // If the transceiver is turning the speaker on while the PWR button is 
      // pressed, then assume that it is for the purpose of playing the power-off
      // beep. We now know that the phone is powering off, and we should also
      // play the power-off beep.
      module.isPoweringOff = true;

      SOUND_PlaySingleTone(
          SOUND_Channel_FOREGROUND,
          SOUND_Target_SPEAKER,
          VOLUME_Mode_TONE,
          TONE_HIGH,
          0
      );
      
      printf("[TSCVR] Powering Off (upon PWR release)\r\n");
      module.eventHandler(TRANSCEIVER_EventType_POWERING_OFF);
    } else if ((cmd == HANDSET_UartCmd_LOUD_SPEAKER_OFF) && module.isPoweringOff) {
      // If the transceiver turns the speaker off while powering off, then we
      // know it's time to stop playing the power-off beep.
      SOUND_Stop(SOUND_Channel_FOREGROUND);
    }
    
    if (TIMEOUT_IsPending(&module.batteryLevelRequestTimeout)) {
      // We're waiting for a battery level response, so process commands from
      // the Transceiver to extract out the battery level it is trying to print
      // to the handset.
      if ((cmd == HANDSET_UartCmd_TEXT_DISPLAY_ON) && (module.pendingBatteryLevel > 0)) {
        // The Transceiver is turning text display back on, presumably after 
        // printing the battery level to the display, so 
        // transceiver.pendingBatteryLevel should now contain the battery level.
        // NOTE: If transceiver.pendingBatteryLevel is zero, then something went 
        //       wrong and we failed to extract a battery level, so we
        //       we won't update the battery level. The timeout will eventually
        //       expire and attempt to retry.
        printf("[TSCVR] Battery Level Received: %d\r\n", (uint16_t)module.pendingBatteryLevel);
        if (module.pendingBatteryLevel != module.batteryLevel) {
          module.batteryLevel = module.pendingBatteryLevel;
          module.eventHandler(TRANSCEIVER_EventType_BATTERY_LEVEL_CHANGED);
        }
        TIMEOUT_Cancel(&module.batteryLevelRequestTimeout);

        // Clear the battery level display so we're ready to request battery 
        // voltage again later
        simulateButtonPress(HANDSET_Button_CLR);
      } else if (cmd == HANDSET_UartCmd_PRINT_HYPHEN) {
        // When reporting the battery level, the Transceiver prints "BATERYV:"
        // to the handset, followed by 1-5 hyphens to represent the battery level.
        // So we count how many hyphens the Transceiver is printing.
        ++module.pendingBatteryLevel;
      }
    }    
  }
  
  TIMEOUT_Task(&module.recentSimulatedButtonPressTimeout);
  
  if (TIMEOUT_Task(&module.transceiverReadyTimeout)) {
    // We didn't get indication from the Transceiver that it is "ready" within 
    // the expected amount of time. Assume it must be ready and we just missed
    // the indicator because the Transceiver was already previously powered on.
    // Start polling the battery level now.
    printf("[TSCVR] Transceiver Ready Timed Out; Assuming Ready\r\n");
    INTERVAL_Start(&module.batteryLevelRequestInterval, true);

    // Simulate an innocuous button press to ensure that we get a BACKLIGHT_OFF
    // command after initialization if the transceiver is not connected to external
    // power.
    simulateButtonPress(HANDSET_Button_CLR);
  }
    
  if (TIMEOUT_Task(&module.deferBatteryLevelOkEventTimeout)) {
    // The battery level was low, but then the PWR indicator was turned on 
    // and remained long enough that it must not be flashing any more
    // to indicate low battery. So the battery level must be OK now.
    printf("[TSCVR] Battery Level OK!\r\n");
    module.isBatteryLevelLow = false;
    module.eventHandler(TRANSCEIVER_EventType_BATTERY_LEVEL_IS_OK);
    TRANSCEIVER_PollBatteryLevelNow();
  }
  
  if (INTERVAL_Task(&module.batteryLevelRequestInterval)) {
    // Time for a periodic polling of the battery level, but don't do
    // anything if there's still a pending battery level request.
    if (!TIMEOUT_IsPending(&module.batteryLevelRequestTimeout)) {
      printf("[TSCVR] Requesting Battery Voltage\r\n");
      // Reset the pending battery level so we're ready to increment it
      // as we process the response from the Transceiver.
      module.pendingBatteryLevel = 0;

      // Send button commands to display battery level: FCN * 5
      simulateButtonPress(HANDSET_Button_FCN);
      simulateButtonPress(HANDSET_Button_ASTERISK);
      simulateButtonPress(HANDSET_Button_5);

      TIMEOUT_Start(&module.batteryLevelRequestTimeout, BATTERY_LEVEL_REQUEST_TIMEOUT);
    }
  }

  if (TIMEOUT_Task(&module.batteryLevelRequestTimeout)) {
    // The Transceiver has not responded with battery level display within 
    // the expected time. 
    printf("[TSCVR] Battery Request Timed Out!\r\n");
    
    // Try clearing out of whatever may have interfered with displaying 
    // the battery level and hope the next attempt will succeed;
    simulateButtonPress(HANDSET_Button_CLR);
    // Retry polling the battery level now.
    TRANSCEIVER_PollBatteryLevelNow();
  }  
}

void TRANSCEIVER_HANDSET_EventHandler(HANDSET_Event const* event) {
  if (event->button == HANDSET_Button_PWR) {
    if (event->type == HANDSET_EventType_BUTTON_DOWN) {
      module.isPowerButtonPressed = true;
    } else if (event->type == HANDSET_EventType_BUTTON_UP) {
      module.isPowerButtonPressed = false;
    }
  }
}

void TRANSCEIVER_Timer10MS_Interrupt(void) {
  TIMEOUT_Timer_Interrupt(&module.transceiverReadyTimeout);
  TIMEOUT_Timer_Interrupt(&module.batteryLevelRequestTimeout);
  TIMEOUT_Timer_Interrupt(&module.deferBatteryLevelOkEventTimeout);
  TIMEOUT_Timer_Interrupt(&module.recentSimulatedButtonPressTimeout);
  INTERVAL_Timer_Interrupt(&module.batteryLevelRequestInterval);  
}

void TRANSCEIVER_PollBatteryLevelNow(void) {
  printf("[TSCVR] Poll Battery Level Now\r\n");
  INTERVAL_SkipAhead(&module.batteryLevelRequestInterval);
}

uint8_t TRANSCEIVER_GetBatteryLevel(void) {
  return module.batteryLevel;
}

bool TRANSCEIVER_IsBatteryLevelLow(void) {
  return module.isBatteryLevelLow;
}

bool TRANSCEIVER_IsConnectedToExternalPower(void) {
  return module.isConnectedToExternalPower;
}