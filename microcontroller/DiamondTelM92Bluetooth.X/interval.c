/** 
 * @file
 * @author Jeff Lau
 * 
 * General purpose interval utilities, for implementing general periodic 
 * repeating execution of code that does not need exact timing precision.
 */

#include "interval.h"

void INTERVAL_Init(interval_t* interval, uint16_t duration) {
  // First mark the timer as expired. This guarantees that execution of
  // INTERVAL_Timer_event from within a timer interrupt will not interfere 
  // with writing any other variables.
  interval->_timerExpired = true;

  interval->_isRunning = false;
  interval->_duration = duration;
  interval->_timer = 0;
  interval->_timerExpired = false;
}

void INTERVAL_Timer_event(interval_t* interval) {
  if (!interval->_timerExpired && interval->_timer != 0) {
    if (--interval->_timer == 0) {
      interval->_timerExpired = true;
    }
  }
}

bool INTERVAL_Task(interval_t* interval) {
  if (interval->_timerExpired) {
    interval->_timer = interval->_duration;
    interval->_timerExpired = (interval->_timer == 0);

    return true;
  }
  
  return false;
}

void INTERVAL_Start(interval_t* interval, bool triggerImmediately) {
  // First mark the timer as expired. This guarantees that execution of
  // INTERVAL_Timer_event from within a timer interrupt will not interfere 
  // with writing any other variables.
  interval->_timerExpired = true;
  
  if (triggerImmediately) {
    interval->_timer = 0;
  } else {
    interval->_timer = interval->_duration;
  }

  interval->_isRunning = true;  
  interval->_timerExpired = (interval->_timer == 0);
}

void INTERVAL_SkipAhead(interval_t* interval) {
  if (interval->_isRunning) {
    interval->_timerExpired = true;
    interval->_timer = 0;
  }
}

void INTERVAL_Cancel(interval_t* interval) {
  // First mark the timer as expired. This guarantees that execution of
  // INTERVAL_Timer_event from within a timer interrupt will not interfere 
  // with writing any other variables.
  interval->_timerExpired = true;

  interval->_timer = 0;
  interval->_isRunning = false;
  interval->_timerExpired = false;
}

bool INTERVAL_IsRunning(interval_t* interval) {
  return interval->_isRunning;
}

