/** 
 * @file
 * @author Jeff Lau
 * 
 * See header file for module description.
 */

#include "external_power.h"
#include "../util/timeout.h"
#include "../../mcc_generated_files/pin_manager.h"
#include <stdio.h>

/**
 * Module state.
 */
static struct {
  /**
   * Event handler. May be null.
   */
  EXTERNAL_POWER_EventHandler eventHandler;
  /**
   * True if external power is currently connected.
   */
  bool isConnected;
  /**
   * True if a change to the external power detect input pin has been detected.
   * 
   * This is set by the pin IOC interrupt handler, and cleared out by the 
   * main task function.
   */
  volatile bool isInputPinChangeDetected;
  /**
   * A timeout used for de-bouncing changes to the external power detect input pin.
   */
  timeout_t debounceTimeout;
} module;

/**
 * IOC interrupt handler for the MCU input pin that is used to detect external
 * power connection.
 */
static void inputPinChangeHandler(void) {
  module.isInputPinChangeDetected = true;
}

void EXTERNAL_POWER_Initialize(EXTERNAL_POWER_EventHandler eventHandler) {
  module.eventHandler = eventHandler;
  module.isInputPinChangeDetected = false;
  module.isConnected = IO_EXT_PWR_DETECT_GetValue();
  printf("[EXTERNAL POWER] Initial: %s\r\n", module.isConnected ? "Connected" : "Disconnected");
  TIMEOUT_Cancel(&module.debounceTimeout);
  IOCBF3_SetInterruptHandler(inputPinChangeHandler);
}

void EXTERNAL_POWER_Task(void) {
  if (module.isInputPinChangeDetected) {
    module.isInputPinChangeDetected = false;
    TIMEOUT_Start(&module.debounceTimeout, 15);
  }
  
  if (TIMEOUT_Task(&module.debounceTimeout)) {
    bool const wasConnected = module.isConnected;
    module.isConnected = IO_EXT_PWR_DETECT_GetValue();

    if (module.isConnected != wasConnected) {
      printf("[EXTERNAL POWER] %s\r\n", module.isConnected ? "Connected" : "Disconnected");
      
      if (module.eventHandler) {
        module.eventHandler(
            module.isConnected ? EXTERNAL_POWER_EventType_CONNECTED : EXTERNAL_POWER_EventType_DISCONNECTED
        );
      }
    }
  }
}

void EXTERNAL_POWER_Timer10MS_Interrupt(void) {
  TIMEOUT_Timer_Interrupt(&module.debounceTimeout);
}

bool EXTERNAL_POWER_IsConnected(void) {
  return module.isConnected;
}

