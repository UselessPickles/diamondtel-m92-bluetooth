/** 
 * @file
 * @author Jeff Lau
 * 
 * General purpose interval utilities, for implementing general periodic 
 * repeating execution of code that does not need exact timing precision.
 */

#ifndef INTERVAL_H
#define	INTERVAL_H

#include <stdint.h>
#include <stdbool.h>

#ifdef	__cplusplus
extern "C" {
#endif

/**
 * @brief Contains the state of an interval.
 * 
 * The contents of this struct should not be accessed/manipulated directly.
 * Instead, pass a pointer to the various `INTERVAL_*` functions.
 * 
 * The general pattern of usage:
 * -# Perform a one-time initialization of the interval with 
 *    INTERVAL_Initialize() to set its interval duration.
 * -# From within a timer event that triggers at regular intervals, use
 *    INTERVAL_Timer_event() to advance your interval.
 * -# From within a main task loop, use INTERVAL_Task() to process completion
 *    of each occurrence of the interval. The function returns true if the 
 *    an occurrence of the interval is completed, and therefore should trigger 
 *    execution of code you want to run periodically.
 * -# Use INTERVAL_Start() and INTERVAL_Cancel() to start/cancel a the interval.
 * 
 * 
 * NOTE: This is not exactly precise. The amount of error in time between
 * starting a interval and when your code will process the first occurrence of
 * the interval (and timing between subsequent occurrences) will depend on:
 * - The time between hardware timer interrupts used to control your timeout.
 * - How long until the next timer event from the time that you start the interval.
 * - The delay between the interval expiration via a timer event and the 
 *   execution of your main task loop code that will handle the expiration.
 * 
 * In general, using timer that triggers more frequently will increase the 
 * precision of the interval.
 */
typedef struct interval_t {
  /**
   * True if this interval is currently running.
   */
  volatile bool _isRunning;
  /**
   * The number of timer interrupts remaining before the next occurrence of
   * this interval will trigger.
   */
  volatile uint16_t _timer;
  /**
   * True if the timer has expired and the next occurrence if this interval
   * is ready to be triggered.
   */
  volatile bool _timerExpired;
  /**
   * The duration between occurrences of this interval (used to reset #_timer).
   */
  uint16_t _duration;
} interval_t;

/**
 * Initialize an interval.
 * 
 * The initial state of the interval will be NOT running.
 * 
 * NOTE: If the interval is initialized with a duration of zero (0), then
 *       the interval will trigger with EVERY execution of INTERVAL_Task().
 * 
 * @param interval - Pointer to an interval.
 * @param duration - The desired duration between intervals, in terms of 
 *        "count" of timer interrupts. To target a specific amount of time, 
 *        divide the amount of time by the period of the timer whose interrupt 
 *        is advancing this interval.
 */
void INTERVAL_Initialize(interval_t* interval, uint16_t duration);

/**
 * Performs periodic advancement of the interval and detecting if its timer 
 * has timed out.
 * 
 * This function must be called for all potentially-running intervals at a 
 * regular interval of time, typically from the interrupt handler of a 
 * hardware timer that is setup to trigger periodically.
 * 
 * The period of the timer interrupt determines how quickly the interval
 * timer runs.
 * 
 * @param interval - Pointer to an interval.
 */
void INTERVAL_Timer_Tick(interval_t* interval);

/**
 * Performs main task loop processing of an interval.
 * 
 * This function must be called from a main task loop consistently so that you 
 * can properly detect and respond to the completion of each occurrence of the
 * interval.
 * 
 * @param interval - Pointer to an interval.
 * @return True if an occurrence of the interval has just now completed. Use 
 *         this result to trigger execution of behavior that should occur 
 *         periodically with this interval.
 */
bool INTERVAL_Task(interval_t* interval);

/**
 * Starts an interval.
 * 
 * If the interval is already running, then it will be restarted.
 * 
 * @param interval - Pointer to an interval.
 * @param triggerImmediately - If true, then the first occurrence of the 
 *        interval will trigger immediately upon the next execution of
 *        INTERVAL_Task(). If false, then the first occurrence of the interval
 *        is delayed by the interval's configured duration.
 */
void INTERVAL_Start(interval_t* interval, bool triggerImmediately);

/**
 * If the interval is currently running, then this skips waiting for the next
 * occurrence of the interval and will trigger immediately upon the next 
 * execution of INTERVAL_Task().
 * 
 * Does nothing if the interval is not currently running.
 * 
 * @param interval - Pointer to an interval.
 */
void INTERVAL_SkipAhead(interval_t* interval);

/**
 * Cancels/stops an interval.
 * 
 * @param interval - Pointer to an interval.
 */
void INTERVAL_Cancel(interval_t* interval);

/**
 * Tests if an interval is currently running.
 * 
 * @param interval - Pointer to an interval.
 * @return True if the interval is currently running.
 */
bool INTERVAL_IsRunning(interval_t* interval);

#ifdef	__cplusplus
}
#endif

#endif	/* INTERVAL_H */

