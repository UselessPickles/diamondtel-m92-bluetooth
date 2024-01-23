/** 
 * @file
 * @author Jeff Lau
 *
 * General purpose timeout utilities, for implementing general timing/delays
 * that do not need exact timing precision.
 */

#ifndef TIMEOUT_H
#define	TIMEOUT_H

#include <stdint.h>
#include <stdbool.h>

#ifdef	__cplusplus
extern "C" {
#endif

/**
 * @brief Contains the state of a single timeout.
 * 
 * The contents of this struct should not be accessed/manipulated directly.
 * Instead, pass a pointer to the various `TIMEOUT_*` functions.
 * 
 * The general pattern of usage:
 * -# From within a timer event that triggers at regular intervals, use
 *    TIMEOUT_Timer_Interrupt() to advance your timeout.
 * -# From within a main task loop, use TIMEOUT_Task() to process completion
 *    of the timeout. The function returns true if the timeout is completed,
 *    and therefore should trigger execution of code you want to run on 
 *    completion of the timeout.
 * -# Use TIMEOUT_Start() and TIMEOUT_Cancel() to start/cancel a timeout.
 * 
 * 
 * NOTE: This is not exactly precise. The amount of error in time between
 * starting a timeout and when your code will process the timeout expiration
 * will vary based:
 * - The time between hardware timer interrupts used to control your timeout.
 * - How long until the next timer event from the time that you start the timeout
 * - The delay between the timeout expiration via a timer event and the 
 *   execution of your main task loop code that will handle the expiration.
 * 
 * In general, using timer that triggers more frequently will increase the 
 * precision of the timeout.
 */
typedef struct timeout_t {
  /**
   * The number of timer interrupts remaining before this timeout will complete.
   */
  volatile uint16_t _timer;
  /**
   * True if the timeout was pending and the timer has now expired, 
   * but completion of the timeout has not yet been processed.
   */
  volatile bool _timerExpired;
  /**
   * True if this timeout has been started and has not yet completed.
   */
  bool _isPending;
} timeout_t;

/**
 * Performs periodic advancement of the timeout and detecting if its timer 
 * has timed out.
 * 
 * This function must be called for all potentially-running timeouts at a 
 * regular interval of time, typically from the interrupt handler of a 
 * hardware timer that is setup to trigger periodically.
 * 
 * The period of the timer interrupt determines how quickly the timeout
 * timer runs.
 * 
 * @param timeout - Pointer to a timeout.
 */
void TIMEOUT_Timer_Interrupt(timeout_t* timeout);

/**
 * Performs main task loop processing of a timeout.
 * 
 * This function must be called from a main task loop consistently so that you 
 * can properly detect and respond to the completion of the timeout.
 * 
 * NOTE: An expired timeout will still be considered to be "pending" until it 
 *       is  subsequently processed by TIMEOUT_Task().
 * 
 * @param timeout - Pointer to a timeout.
 * @return True if the timeout has just now completed. Use this result to 
 *         trigger execution of behavior that should occur upon completion
 *         of the timeout.
 */
bool TIMEOUT_Task(timeout_t* timeout);

/**
 * Starts a timeout.
 * 
 * If the timeout was already pending, then it will be restarted with the 
 * specified duration.
 * 
 * NOTE: If a timeout is started with a duration of zero (0), then it will
 *       start as pending, but already timed out. The next execution of 
 *       TIMEOUT_Task() will complete the timer.
 * 
 * @param timeout - Pointer to a timeout.
 * @param duration - Duration of the timeout, in terms of "count" of timer 
 *        interrupts. To target a specific amount of time, divide the amount 
 *        of time by the period of the timer whose interrupt is advancing this 
 *        timeout.
 */
void TIMEOUT_Start(timeout_t* timeout, uint16_t duration);

/**
 * Starts a timeout, or allows it to continue running as-is if the current 
 * remaining duration is longer than the new specified duration.
 * 
 * NOTE: If a timeout is started with a duration of zero (0), then it will
 *       start as pending, but already timed out. The next execution of 
 *       TIMEOUT_Task() will complete the timer.
 * 
 * @param timeout - Pointer to a timeout.
 * @param duration - Duration of the timeout, in terms of "count" of timer 
 *        interrupts. To target a specific amount of time, divide the amount 
 *        of time by the period of the timer whose interrupt is advancing this 
 *        timeout.
 */
void TIMEOUT_StartOrContinue(timeout_t* timeout, uint16_t duration);

/**
 * Cancels/stops a timeout.
 * 
 * @param timeout - Pointer to a timeout.
 */
void TIMEOUT_Cancel(timeout_t* timeout);

/**
 * Tests if a timeout is pending.
 * 
 * A timeout is considered to be pending if it has been started, but has not
 * yet completed.
 * 
 * NOTE: A timeout is still considered to be "pending" if its timer has expired,
 *       but its completion has not yet been processed by TIMEOUT_Task().
 * 
 * @param timeout - Pointer to a timeout.
 * @return True if the timeout is pending.
 */
bool TIMEOUT_IsPending(timeout_t const* timeout);

/**
 * Tests if a timeout is expired.
 * 
 * A timeout is considered to be expired if it has been started, then has
 * completed, but has not yet been processed by TIMEOUT_Task().
 * 
 * @param timeout - Pointer to a timeout.
 * @return True if the timeout is pending.
 */
bool TIMEOUT_IsExpired(timeout_t const* timeout);

#ifdef	__cplusplus
}
#endif

#endif	/* TIMEOUT_H */

