/** 
 * @file
 * @author Jeff Lau
 *
 * General purpose timeout utilities, for implementing general timing/delays
 * that do not need exact precision.
 */

#include "timeout.h"

void TIMEOUT_Timer_event(timeout_t* timeout) {
  if (!timeout->_timerExpired && timeout->_timer != 0) {
    if (--timeout->_timer == 0) {
      timeout->_timerExpired = true;
    }
  }
}

bool TIMEOUT_Task(timeout_t* timeout) {
  if (timeout->_timerExpired) {
    timeout->_timerExpired = false;
    timeout->_isPending = false;
    return true;
  }
  
  return false;
}

void TIMEOUT_Start(timeout_t* timeout,  uint16_t duration) {
  // First mark the timer as expired. This guarantees that execution of
  // TIMEOUT_Timer_event from within a timer interrupt will not interfere 
  // with writing any other variables.
  timeout->_timerExpired = true;

  timeout->_timer = duration;
  timeout->_isPending = true;
  // Finally set the true "expired" state of the timer.
  // It is immediately expired if the duration is zero.
  timeout->_timerExpired = (duration == 0);
}

void TIMEOUT_Cancel(timeout_t* timeout) {
  // First mark the timer as expired. This guarantees that execution of
  // TIMEOUT_Timer_event from within a timer interrupt will not interfere 
  // with writing any other variables.
  timeout->_timerExpired = true;

  timeout->_timer = 0;
  timeout->_isPending = false;
  timeout->_timerExpired = false;
}

bool TIMEOUT_IsPending(timeout_t const* timeout) {
  return timeout->_isPending;
}
