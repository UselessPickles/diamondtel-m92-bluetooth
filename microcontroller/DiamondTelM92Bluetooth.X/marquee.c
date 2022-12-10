#include "marquee.h"
#include "interval.h"
#include "timeout.h"
#include "handset.h"
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#define MAX_TEXT_LENGTH (128)
#define SCROLL_INTERVAL (50)
#define SCROLL_DELAY (150)

static struct {
  char text[MAX_TEXT_LENGTH + 1];
  size_t textLen;
  MARQUEE_Row row;
  interval_t scrollInterval;
  timeout_t scrollDelayTimeout;
  size_t offset;
} state;

static void printMarqueeText(void) {
  char textToPrint[8];
  size_t len = state.textLen - state.offset;

  if (len < 7) {
    memset(textToPrint + len, ' ', 7 - len);
  } else {
    len = 7;
  }
  
  memcpy(textToPrint, state.text + state.offset, len);

  textToPrint[7] = 0;

  HANDSET_PrintStringAt(textToPrint, state.row * 7 + 6);  
}

void MARQUEE_Initialize(void) {
  INTERVAL_Init(&state.scrollInterval, SCROLL_INTERVAL);
  memset(state.text, ' ', 6);
  state.text[MAX_TEXT_LENGTH] = 0;
}

void MARQUEE_Timer10MS_event(void) {
  INTERVAL_Timer_event(&state.scrollInterval);
  TIMEOUT_Timer_event(&state.scrollDelayTimeout);
}

void MARQUEE_Task(void) {
  if (TIMEOUT_Task(&state.scrollDelayTimeout)) {
    INTERVAL_Start(&state.scrollInterval, true);
  }
  
  if (INTERVAL_Task(&state.scrollInterval)) {
    if (++state.offset >= state.textLen) {
      state.offset = 0;
    }

    printMarqueeText();
  }
}

void MARQUEE_Start(char const* text, MARQUEE_Row row) {
  if (MARQUEE_IsRunning(text, row)) {
    return;
  }

  size_t textLen = strlen(text);
  
  if (textLen > (MAX_TEXT_LENGTH - 6)) {
    textLen = MAX_TEXT_LENGTH - 6;
  }
  
  memcpy(state.text + 6, text, textLen);
  state.text[textLen + 6] = 0;
  state.textLen = textLen + 6;
  state.row = row;
  state.offset = 6;
  
  printMarqueeText();
  
  if (textLen > 7) {
    INTERVAL_Cancel(&state.scrollInterval);
    TIMEOUT_Start(&state.scrollDelayTimeout, SCROLL_DELAY);
  } else {
    MARQUEE_Stop();
  }
}

void MARQUEE_Stop(void) {
  TIMEOUT_Cancel(&state.scrollDelayTimeout);
  INTERVAL_Cancel(&state.scrollInterval);
}

bool MARQUEE_IsRunning(char const* text, MARQUEE_Row row) {
  return (TIMEOUT_IsPending(&state.scrollDelayTimeout) || INTERVAL_IsRunning(&state.scrollInterval))
      && (state.row == row)
      && (strncmp(state.text + 6, text, MAX_TEXT_LENGTH) == 0);
}
