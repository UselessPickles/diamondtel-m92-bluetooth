#include "memory_game.h"
#include "level_select.h"
#include "../sound/sound.h"
#include "../ui/volume_adjust.h"
#include "../util/string.h"
#include "../util/interval.h"
#include "../../mcc_generated_files/tmr6.h"
#include "../../mcc_generated_files/tmr4.h"
#include <stdlib.h>
#include <string.h>

typedef enum State {
  State_TITLE,
  State_MENU,
  State_SELECT_LEVEL,
  State_PLAYING,
  State_GAME_OVER_1,
  State_GAME_OVER_2,
  State_GAME_OVER_3,
  State_GAME_OVER_4,
  State_VOLUME_ADJUST
} State;

static struct {
  bool isGameStarted;
  MEMORY_GAME_ReturnCallback returnCallback;
  State state;
  interval_t stateInterval;
  uint8_t level;
  uint8_t cursorPos;
  uint8_t firstGuessPos;
  uint8_t secondGuessPos;
  bool showCursor;
  uint8_t showGuessPairCount;
  uint8_t remainingPairs;
  uint8_t moves;
  char tiles[HANDSET_TEXT_DISPLAY_LENGTH];
  char cards[HANDSET_TEXT_DISPLAY_LENGTH];
} module;

#define LEVEL_COUNT (6)

#define CHAR_HIDDEN (HANDSET_Symbol_RECTANGLE)
#define CHAR_CURSOR (' ')

static char const cardChars[] = {
  '$',
  HANDSET_Symbol_LARGE_UP_ARROW,
  HANDSET_Symbol_SMALL_DOWN_ARROW_OUTLINE,
  '*',
  HANDSET_Symbol_LARGE_DOWN_ARROW,
  HANDSET_Symbol_YEN_SIGN,
  HANDSET_Symbol_SMALL_UP_ARROW_OUTLINE,
};

static void displayTitle(void) {
  HANDSET_DisableTextDisplay();
  HANDSET_PrintString("Memory!Press #");
  HANDSET_EnableTextDisplay();
  module.state = State_TITLE;
}

static void displayMenu(void) {
  HANDSET_DisableTextDisplay();
  HANDSET_PrintString("1:Cont.2:New  ");
  HANDSET_EnableTextDisplay();
  module.state = State_MENU;
}

static void handle_LEVEL_SELECT_Callback(int8_t level);

static void displayLevelSelect(void) {
  INTERVAL_Cancel(&module.stateInterval);

  LEVEL_SELECT_Prompt(LEVEL_COUNT, handle_LEVEL_SELECT_Callback);
  module.state = State_SELECT_LEVEL;
}

static uint8_t getDisplayPosForTilePos(uint8_t tilePos) {
  uint8_t const tilesPerRow = module.level + 2;
  return (tilePos / tilesPerRow) * 7 + (tilePos % tilesPerRow) + ((7 - tilesPerRow) / 2);
}

static char getCharForTileAtPos(uint8_t pos) {
  return (pos == module.firstGuessPos || pos == module.secondGuessPos) 
      ? module.cards[pos]
      : module.tiles[pos];
}

static char getCharForTileAtDisplayPos(uint8_t displayPos) {
  uint8_t const tilesPerRow = module.level + 2;
  uint8_t const offset = (7 - tilesPerRow) / 2;
  uint8_t const x = displayPos % 7;
  uint8_t const y = displayPos / 7;
  
  if ((x < offset) || (x >= offset + tilesPerRow)) {
    return ' ';
  }
  
  return getCharForTileAtPos(y * tilesPerRow + x - offset);
}

static void displayGameTiles(void) {
  HANDSET_DisableTextDisplay();
  for (uint8_t i = 1; i <= HANDSET_TEXT_DISPLAY_LENGTH; ++i) {
    HANDSET_PrintChar(getCharForTileAtDisplayPos(HANDSET_TEXT_DISPLAY_LENGTH - i));
  }
  HANDSET_EnableTextDisplay();
}

static void initCards(void) {
  srand(((uint16_t)TMR6_ReadTimer() << 8) | TMR4_ReadTimer());
  
  char cards[HANDSET_TEXT_DISPLAY_LENGTH];
  uint8_t const totalCards = module.remainingPairs * 2;
  uint8_t remainingCards = totalCards;
  
  memcpy(cards, cardChars, module.remainingPairs);
  memcpy(cards + module.remainingPairs, cardChars, module.remainingPairs);
  
  while (remainingCards != 0) {
    uint8_t nthAvailableCard = rand() % remainingCards;
    uint8_t cardIndex = 0;
    
    while (true) {
      if (cards[cardIndex] != 0) {
        if (nthAvailableCard == 0) {
          break;
        } else {
          --nthAvailableCard;
        }
      }
      
      ++cardIndex;
    }
    
    module.cards[--remainingCards] = cards[cardIndex];
    cards[cardIndex] = 0;
  }
}

static void resumeGame(void) {
  displayGameTiles();
  module.showCursor = false;
  INTERVAL_Initialize(&module.stateInterval, 25);
  INTERVAL_Start(&module.stateInterval, false);
  module.state = State_PLAYING;
}

static void startNewGame(uint8_t level) {
  module.isGameStarted = true;
  module.level = level;
  module.moves = 0;
  module.remainingPairs = level + 2;
  module.cursorPos = (level + 1) / 2;
  module.firstGuessPos = module.secondGuessPos = 0xFF;
  memset(module.tiles, CHAR_HIDDEN, HANDSET_TEXT_DISPLAY_LENGTH);
  initCards();
  resumeGame();
}

static void displayGameOver(void) {
  INTERVAL_Initialize(&module.stateInterval, 150);
  INTERVAL_Start(&module.stateInterval, true);
  
  module.isGameStarted = false;
  module.state = State_GAME_OVER_1;
}

static void gameLoop(void) {
  module.showCursor = !module.showCursor;

  if (module.showCursor) {
    if (module.secondGuessPos != 0xFF) {
      char const cursorChar = module.cards[module.firstGuessPos] == module.cards[module.secondGuessPos] 
          ? CHAR_CURSOR 
          : CHAR_HIDDEN;

      HANDSET_PrintCharAt(cursorChar, getDisplayPosForTilePos(module.firstGuessPos));
      HANDSET_PrintCharAt(cursorChar, getDisplayPosForTilePos(module.secondGuessPos));
    } else if (module.remainingPairs == 0) {
      displayGameOver();
    } else {
      HANDSET_PrintCharAt(CHAR_CURSOR, getDisplayPosForTilePos(module.cursorPos));
    }
  } else {
    if (module.secondGuessPos != 0xFF) {
      if (--module.showGuessPairCount == 0) {
        char const firstGuessCard = module.cards[module.firstGuessPos];
        bool const isMatch = firstGuessCard == module.cards[module.secondGuessPos];

        if (isMatch) {
          module.tiles[module.firstGuessPos] = firstGuessCard;
          module.tiles[module.secondGuessPos] = firstGuessCard;
          --module.remainingPairs;
        }

        HANDSET_PrintCharAt(module.tiles[module.firstGuessPos], getDisplayPosForTilePos(module.firstGuessPos));
        HANDSET_PrintCharAt(isMatch ? module.tiles[module.secondGuessPos] : CHAR_CURSOR, getDisplayPosForTilePos(module.secondGuessPos));
        module.firstGuessPos = module.secondGuessPos = 0xFF;
        module.showCursor = !isMatch;
      } else {
        HANDSET_PrintCharAt(getCharForTileAtPos(module.firstGuessPos), getDisplayPosForTilePos(module.firstGuessPos));
        HANDSET_PrintCharAt(getCharForTileAtPos(module.secondGuessPos), getDisplayPosForTilePos(module.secondGuessPos));
      }
    } else {
      HANDSET_PrintCharAt(getCharForTileAtPos(module.cursorPos), getDisplayPosForTilePos(module.cursorPos));
    }
  }
}

static void handle_LEVEL_SELECT_Callback(int8_t level) {
  if (level < 0) {
    if (module.isGameStarted) {
      displayMenu();
    } else {
      displayTitle();
    }
  } else {
    startNewGame(level);
  }
}

void MEMORY_GAME_Start(MEMORY_GAME_ReturnCallback returnCallback) {
  module.returnCallback = returnCallback;
  displayTitle();
}

void MEMORY_GAME_Task(void) {
  switch (module.state) {
    case State_PLAYING:
      if (INTERVAL_Task(&module.stateInterval)) {
        gameLoop();
      }
      break;
    
    case State_GAME_OVER_1: 
      if (INTERVAL_Task(&module.stateInterval)) {
        HANDSET_DisableTextDisplay();
        HANDSET_ClearText();
        HANDSET_PrintString("You    Win! ");
        HANDSET_EnableTextDisplay();
        ++module.state;
      }
      break;

    case State_GAME_OVER_2: 
      if (INTERVAL_Task(&module.stateInterval)) {
        char scoreStr[3];
        
        uint2str(scoreStr, module.moves, 3, 3);
        
        HANDSET_DisableTextDisplay();
        HANDSET_PrintString(" Moves   ");
        HANDSET_PrintStringN(scoreStr, 3);
        HANDSET_PrintCharN(' ', 2);
        HANDSET_EnableTextDisplay();
        ++module.state;
      }
      break;
      
    case State_GAME_OVER_3: 
      if (INTERVAL_Task(&module.stateInterval)) {
        HANDSET_DisableTextDisplay();
        HANDSET_PrintString("#:Again*:New  ");
        HANDSET_EnableTextDisplay();
        ++module.state;
      }
      break;

    case State_GAME_OVER_4: 
      if (INTERVAL_Task(&module.stateInterval)) {
        displayGameTiles();
        module.state = State_GAME_OVER_1;
      }
      break;
      
    case State_VOLUME_ADJUST:
      VOLUME_ADJUST_Task();
      break;
  }
}

void MEMORY_GAME_Timer10MS_Interrupt(void) {
  switch (module.state) {
    case State_VOLUME_ADJUST:
      VOLUME_ADJUST_Timer10MS_Interrupt();
      break;

    default:
      INTERVAL_Timer_Interrupt(&module.stateInterval);
      break;
  }
}

void MEMORY_GAME_HANDSET_EventHandler(HANDSET_Event const* event) {
  HANDSET_Button const button = event->button;
  bool const isButtonDown = (event->type == HANDSET_EventType_BUTTON_DOWN);
  
  if (
      (button == HANDSET_Button_CLR) && 
      (event->type == HANDSET_EventType_BUTTON_HOLD) &&
      (event->holdDuration == HANDSET_HoldDuration_SHORT)
      ) {
    SOUND_StopButtonBeep();
    HANDSET_CancelCurrentButtonHoldEvents();
    module.returnCallback();
    return;
  }
  
  switch (module.state) {
    case State_TITLE:
      if (isButtonDown) {
        switch(button) {
          case HANDSET_Button_CLR:
            SOUND_PlayButtonBeep(button, false);
            module.returnCallback();
            break;
            
          case HANDSET_Button_POUND:
            SOUND_PlayButtonBeep(button, false);
            if (module.isGameStarted) {
              displayMenu();
            } else {
              displayLevelSelect();
            }
            break;
            
          case HANDSET_Button_UP:  
          case HANDSET_Button_DOWN:  
            VOLUME_ADJUST_Start(VOLUME_Mode_SPEAKER, button == HANDSET_Button_UP, displayTitle);
            module.state = State_VOLUME_ADJUST;
            break;
        }
      }
      break;
      
    case State_MENU: 
      if (isButtonDown) {
        switch(button) {
          case HANDSET_Button_CLR:
            SOUND_PlayButtonBeep(button, false);
            displayTitle();
            break;
            
          case HANDSET_Button_1:
            SOUND_PlayButtonBeep(button, false);
            resumeGame();
            break;
            
          case HANDSET_Button_2:
            SOUND_PlayButtonBeep(button, false);
            displayLevelSelect();
            break;
            
          case HANDSET_Button_UP:  
          case HANDSET_Button_DOWN:  
            VOLUME_ADJUST_Start(VOLUME_Mode_SPEAKER, button == HANDSET_Button_UP, displayMenu);
            module.state = State_VOLUME_ADJUST;
            break;
        }
      }
      break;
    
    case State_SELECT_LEVEL:
      LEVEL_SELECT_HANDSET_EventHandler(event);
      break;
      
    case State_PLAYING:
      if (isButtonDown) {
        uint8_t newCursorPos = module.cursorPos;
        uint8_t const tilesPerRow = module.level + 2;

        switch (button) {
          case HANDSET_Button_CLR:
            SOUND_PlayButtonBeep(button, false);
            displayMenu();
            break;
          
          case HANDSET_Button_2:
            if (newCursorPos < tilesPerRow) {
              newCursorPos += tilesPerRow;
            }
            break;

          case HANDSET_Button_8:
            if (newCursorPos >= tilesPerRow) {
              newCursorPos -= tilesPerRow;
            }
            break;

          case HANDSET_Button_4:
            if ((module.cursorPos % tilesPerRow) < (tilesPerRow - 1)) {
              ++newCursorPos;
            }
            break;

          case HANDSET_Button_6:
            if ((module.cursorPos % tilesPerRow) > 0) {
              --newCursorPos;
            }
            break;
            
          case HANDSET_Button_1:
            do {
              if (++newCursorPos == (tilesPerRow << 1)) {
                newCursorPos = 0;
              }
            } while ((newCursorPos == module.firstGuessPos) || (module.tiles[newCursorPos] != CHAR_HIDDEN));
            break;            
            
          case HANDSET_Button_3:
            do {
              if (--newCursorPos == 0xFF) {
                newCursorPos += (tilesPerRow << 1);
              }
            } while ((newCursorPos == module.firstGuessPos) || (module.tiles[newCursorPos] != CHAR_HIDDEN));
            break;            
            
          case HANDSET_Button_5:
            if (
                (module.cursorPos != module.firstGuessPos) && 
                (module.secondGuessPos == 0xFF) && 
                (module.tiles[module.cursorPos] == CHAR_HIDDEN)
                ) {
              if (module.firstGuessPos == 0xFF) {
                module.firstGuessPos = module.cursorPos;
              } else {
                module.secondGuessPos = module.cursorPos;
                module.showGuessPairCount = 2;
                ++module.moves;
              }
              HANDSET_PrintCharAt(getCharForTileAtPos(module.cursorPos), getDisplayPosForTilePos(module.cursorPos));
              module.showCursor = false;
              INTERVAL_Start(&module.stateInterval, false);
            }
            break;
            
          case HANDSET_Button_UP:  
          case HANDSET_Button_DOWN:  
            VOLUME_ADJUST_Start(VOLUME_Mode_SPEAKER, button == HANDSET_Button_UP, resumeGame);
            module.state = State_VOLUME_ADJUST;
            break;
        }
        
        if ((newCursorPos != module.cursorPos) && (module.secondGuessPos == 0xFF)) {
          HANDSET_PrintCharAt(getCharForTileAtPos(module.cursorPos), getDisplayPosForTilePos(module.cursorPos));
          HANDSET_PrintCharAt(CHAR_CURSOR, getDisplayPosForTilePos(newCursorPos));
          module.cursorPos = newCursorPos;
          module.showCursor = true;
          INTERVAL_Start(&module.stateInterval, false);
        }
      }
      break;
      
    case State_GAME_OVER_1:
    case State_GAME_OVER_2:
    case State_GAME_OVER_3:
    case State_GAME_OVER_4:
      if (isButtonDown) {
        switch(button) {
          case HANDSET_Button_CLR:
            SOUND_PlayButtonBeep(button, false);
            displayTitle();
            break;

          case HANDSET_Button_ASTERISK:
            SOUND_PlayButtonBeep(button, false);
            displayLevelSelect();
            break;

          case HANDSET_Button_POUND:
            SOUND_PlayButtonBeep(button, false);
            startNewGame(module.level);
            break;

          case HANDSET_Button_UP:  
          case HANDSET_Button_DOWN:  
            VOLUME_ADJUST_Start(VOLUME_Mode_SPEAKER, button == HANDSET_Button_UP, displayGameOver);
            module.state = State_VOLUME_ADJUST;
            break;
        }
      }
      break;

    case State_VOLUME_ADJUST:
      VOLUME_ADJUST_HANDSET_EventHandler(event);
      break;
  }
}

