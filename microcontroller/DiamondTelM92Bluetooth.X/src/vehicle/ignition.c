/** 
 * @file
 * @author Jeff Lau
 * 
 * See header file for module description.
 */

#include "ignition.h"
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
  IGNITION_EventHandler eventHandler;
  /**
   * True if the vehicle ignition is currently ON.
   */
  bool isOn;
  /**
   * True if a change to the vehicle ignition detect input pin has been detected.
   * 
   * This is set by the pin IOC interrupt handler, and cleared out by the 
   * main task function.
   */
  volatile bool isInputPinChangeDetected;
  /**
   * A timeout used for de-bouncing changes to the vehicle ignition detect input pin.
   */
  timeout_t debounceTimeout;
} module;

/**
 * IOC interrupt handler for the MCU input pin that is used to vehicle ignition.
 */
static void inputPinChangeHandler(void) {
  module.isInputPinChangeDetected = true;
}

void IGNITION_Initialize(IGNITION_EventHandler eventHandler) {
  module.eventHandler = eventHandler;
  module.isInputPinChangeDetected = false;
  module.isOn = IO_IGN_DETECT_GetValue();
  printf("[IGNITION] Initial: %s\r\n", module.isOn ? "ON" : "OFF");
  TIMEOUT_Cancel(&module.debounceTimeout);
  IOCBF4_SetInterruptHandler(inputPinChangeHandler);
}

void IGNITION_Task(void) {
  if (module.isInputPinChangeDetected) {
    module.isInputPinChangeDetected = false;
    TIMEOUT_Start(&module.debounceTimeout, 20);
  }
  
  if (TIMEOUT_Task(&module.debounceTimeout)) {
    bool const wasOn = module.isOn;
    module.isOn = IO_IGN_DETECT_GetValue();

    if (module.isOn != wasOn) {
      printf("[IGNITION] %s\r\n", module.isOn ? "ON" : "OFF");
      
      if (module.eventHandler) {
        module.eventHandler(
            module.isOn ? IGNITION_EventType_ON : IGNITION_EventType_OFF
        );
      }
    }
  }
}

void IGNITION_Timer10MS_Interrupt(void) {
  TIMEOUT_Timer_Interrupt(&module.debounceTimeout);
}

bool IGNITION_IsOn(void) {
  return module.isOn;
}

