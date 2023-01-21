/** 
 * @file
 * @author Jeff Lau
 */

#include "snake_game.h"
#include "level_select.h"
#include "../sound/sound.h"
#include "../ui/volume_adjust.h"
#include "../util/string.h"
#include "../util/timeout.h"
#include "../util/interval.h"
#include "../../mcc_generated_files/tmr6.h"
#include "../../mcc_generated_files/tmr4.h"
#include <string.h>
#include <stdlib.h>

typedef enum State {
  State_TITLE,
  State_MENU,
  State_SELECT_LEVEL,
  State_STARTING_1,
  State_STARTING_2,
  State_STARTING_3,
  State_PLAYING,
  State_GAME_OVER_1,
  State_GAME_OVER_2,
  State_GAME_OVER_3,
  State_GAME_OVER_4,
  State_VOLUME_ADJUST
} State;

typedef enum Direction {
  Direction_UP,
  Direction_RIGHT,
  Direction_DOWN,
  Direction_LEFT
} Direction;

#define LEVEL_COUNT (9)

static uint8_t const intervalsByLevelIndex[] = {
  100, 75, 56, 42, 32, 24, 18, 13, 10
};

#define CHAR_EMPTY (' ')
#define CHAR_SNAKE (HANDSET_Symbol_RECTANGLE)
#define CHAR_FOOD ('*')

static struct {
  bool isGameStarted;
  SNAKE_GAME_ReturnCallback returnCallback;
  State state;
  interval_t stateInterval;
  uint8_t level;
  uint16_t score;
  uint8_t snakeLength;
  uint8_t snakeHeadIndex;
  uint8_t snakeTailIndex;
  uint8_t snakePositions[HANDSET_TEXT_DISPLAY_LENGTH];
  uint8_t foodPosition;
  char tiles[HANDSET_TEXT_DISPLAY_LENGTH];
  Direction direction;
} module;

static void displayTitle(void) {
  INTERVAL_Cancel(&module.stateInterval);
  
  HANDSET_DisableTextDisplay();
  HANDSET_PrintString(" Snek! Press #");
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

static void placeFood(void) {
  if (module.snakeLength == HANDSET_TEXT_DISPLAY_LENGTH) {
    return;
  }
  
  uint8_t emptyTileCount = HANDSET_TEXT_DISPLAY_LENGTH - module.snakeLength;
  uint8_t nthAvailablePos = ((unsigned)rand()) % emptyTileCount;
  uint8_t foodPos = 0;
  
  while (true) {
    if (module.tiles[foodPos] == CHAR_EMPTY) {
      if (nthAvailablePos == 0) {
        break;
      } else {
        --nthAvailablePos;
      }
    }
    
    ++foodPos;
  }
  
  module.tiles[foodPos] = CHAR_FOOD;
  module.foodPosition = foodPos;
  HANDSET_PrintCharAt(CHAR_FOOD, foodPos);
}

static void displayGameTiles(void) {
  HANDSET_DisableTextDisplay();
  for (uint8_t i = 1; i <= HANDSET_TEXT_DISPLAY_LENGTH; ++i) {
    HANDSET_PrintChar(module.tiles[HANDSET_TEXT_DISPLAY_LENGTH - i]);
  }
  HANDSET_EnableTextDisplay();
}

static void resumeGame(void) {
  displayGameTiles();
  
  HANDSET_PrintCharAt('3', 3);
  
  INTERVAL_Initialize(&module.stateInterval, 75);
  INTERVAL_Start(&module.stateInterval, false);
  module.state = State_STARTING_1;
}

static void startNewGame(uint8_t level) {
  srand(((uint16_t)TMR6_ReadTimer() << 8) | TMR4_ReadTimer());
  
  module.isGameStarted = true;
  module.level = level;
  module.score = 0;
  module.snakeLength = 1;
  module.snakeHeadIndex = module.snakeTailIndex = 0;
  module.snakePositions[0] = 13;
  module.direction = Direction_RIGHT;
  module.foodPosition = 0xFF;
  
  memset(module.tiles, CHAR_EMPTY, HANDSET_TEXT_DISPLAY_LENGTH);
  module.tiles[13] = CHAR_SNAKE;
  
  resumeGame();
}

static void displayGameOver(void) {
  INTERVAL_Initialize(&module.stateInterval, 150);
  INTERVAL_Start(&module.stateInterval, true);
  
  module.isGameStarted = false;
  module.state = State_GAME_OVER_1;
}

static void gameLoop(void) {
  if (module.snakeLength == HANDSET_TEXT_DISPLAY_LENGTH) {
    displayGameOver();
    return;
  }
  
  uint8_t currentHeadPos = module.snakePositions[module.snakeHeadIndex];
  uint8_t currentTailPos = module.snakePositions[module.snakeTailIndex];
  uint8_t newHeadPos = 0xFF;

  switch (module.direction) {
    case Direction_UP:
      if (currentHeadPos < 7) {
        newHeadPos = currentHeadPos + 7;
      }
      break;

    case Direction_DOWN:
      if (currentHeadPos >= 7) {
        newHeadPos = currentHeadPos - 7;
      }
      break;

    case Direction_LEFT:
      if ((currentHeadPos % 7) < 6) {
        newHeadPos = currentHeadPos + 1;
      }
      break;

    case Direction_RIGHT:
      if ((currentHeadPos % 7) > 0) {
        newHeadPos = currentHeadPos - 1;
      }
      break;
  }

  if (newHeadPos == 0xFF) {
    displayGameOver();
    return;
  }
  
  char newHeadTile = module.tiles[newHeadPos];
  
  if ((newHeadTile == CHAR_SNAKE) && (newHeadPos != currentTailPos)) {
    displayGameOver();
    return;
  }
  
  if (++module.snakeHeadIndex == HANDSET_TEXT_DISPLAY_LENGTH) {
    module.snakeHeadIndex = 0;
  }
  
  module.snakePositions[module.snakeHeadIndex] = newHeadPos;
  module.tiles[newHeadPos] = CHAR_SNAKE;
  HANDSET_PrintCharAt(CHAR_SNAKE, newHeadPos);
  
  if (newHeadTile == CHAR_FOOD) {
    module.score += module.level + 1;

    SOUND_PlaySingleTone(
        SOUND_Channel_FOREGROUND,
        SOUND_Target_SPEAKER,
        VOLUME_Mode_SPEAKER,
        TONE_HIGH,
        50
      );
    
    if (++module.snakeLength == HANDSET_TEXT_DISPLAY_LENGTH) {
      module.score += 10;
    } else { 
      placeFood();
    }
  } else {
    uint8_t currentTailPos = module.snakePositions[module.snakeTailIndex];
        
    if (++module.snakeTailIndex == HANDSET_TEXT_DISPLAY_LENGTH) {
      module.snakeTailIndex = 0;
    }
    
    if (currentTailPos != newHeadPos) {
      module.tiles[currentTailPos] = CHAR_EMPTY;
      HANDSET_PrintCharAt(CHAR_EMPTY, currentTailPos);
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

void SNAKE_GAME_Start(SNAKE_GAME_ReturnCallback returnCallback) {
  module.returnCallback = returnCallback;
  displayTitle();
}

void SNAKE_GAME_Task(void) {
  switch (module.state) {
    case State_STARTING_1:
    case State_STARTING_2:
      if (INTERVAL_Task(&module.stateInterval)) {
        HANDSET_PrintCharAt('2' - (module.state - State_STARTING_1), 3);
        ++module.state;
      }
      break;

    case State_STARTING_3:
      if (INTERVAL_Task(&module.stateInterval)) {
        HANDSET_PrintCharAt(module.tiles[3], 3);
        if (module.foodPosition == 0xFF) {
          placeFood();
        }
        INTERVAL_Initialize(&module.stateInterval, intervalsByLevelIndex[module.level]);
        INTERVAL_Start(&module.stateInterval, true);
        module.state = State_PLAYING;
      }
      break;
    
    case State_PLAYING:
      if (INTERVAL_Task(&module.stateInterval)) {
        gameLoop();
      }
      break;
      
    case State_GAME_OVER_1: 
      if (INTERVAL_Task(&module.stateInterval)) {
        HANDSET_DisableTextDisplay();
        HANDSET_ClearText();
        HANDSET_PrintString(module.snakeLength == HANDSET_TEXT_DISPLAY_LENGTH ? "You    Win! " : "Game   Over ");
        HANDSET_EnableTextDisplay();
        ++module.state;
      }
      break;

    case State_GAME_OVER_2: 
      if (INTERVAL_Task(&module.stateInterval)) {
        char scoreStr[3];
        
        uint2str(scoreStr, module.score, 3, 3);
        
        HANDSET_DisableTextDisplay();
        HANDSET_PrintString(" Score   ");
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

void SNAKE_GAME_Timer10MS_Interrupt(void) {
  switch (module.state) {
    case State_VOLUME_ADJUST:
      VOLUME_ADJUST_Timer10MS_Interrupt();
      break;
    default:  
      INTERVAL_Timer_Interrupt(&module.stateInterval);
      break;
  }
}

void SNAKE_GAME_HANDSET_EventHandler(HANDSET_Event const* event) {
  HANDSET_Button const button = event->button;
  bool const isButtonDown = (event->type == HANDSET_EventType_BUTTON_DOWN);
  
  if (
      (button == HANDSET_Button_CLR) && 
      (event->type == HANDSET_EventType_BUTTON_HOLD) &&
      (event->holdDuration == HANDSET_HoldDuration_SHORT)
      ) {
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
            VOLUME_ADJUST_Start(VOLUME_Mode_SPEAKER, false, button == HANDSET_Button_UP, displayTitle);
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
            VOLUME_ADJUST_Start(VOLUME_Mode_SPEAKER, false, button == HANDSET_Button_UP, displayMenu);
            module.state = State_VOLUME_ADJUST;
            break;
        }
      }
      break;
    
    case State_SELECT_LEVEL:
      LEVEL_SELECT_HANDSET_EventHandler(event);
      break;
      
    case State_STARTING_1:
    case State_STARTING_2:
    case State_STARTING_3:
      if (isButtonDown && (button == HANDSET_Button_CLR)) {
        SOUND_PlayButtonBeep(button, false);
        displayMenu();        
      }
      break;
      
    case State_PLAYING:  
      if (isButtonDown) {
        switch (button) {
          case HANDSET_Button_CLR:
            SOUND_PlayButtonBeep(button, false);
            displayMenu();
            break;
            
          case HANDSET_Button_2:
            module.direction = Direction_UP;
            break;

          case HANDSET_Button_6:
            module.direction = Direction_RIGHT;
            break;

          case HANDSET_Button_8:
            module.direction = Direction_DOWN;
            break;

          case HANDSET_Button_4:
            module.direction = Direction_LEFT;
            break;

          case HANDSET_Button_1:
            module.direction = (module.direction + 3) % 4;
            break;

          case HANDSET_Button_3:
            module.direction = (module.direction + 1) % 4;
            break;

          case HANDSET_Button_UP:  
          case HANDSET_Button_DOWN:  
            VOLUME_ADJUST_Start(VOLUME_Mode_SPEAKER, false, button == HANDSET_Button_UP, resumeGame);
            module.state = State_VOLUME_ADJUST;
            break;
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
            VOLUME_ADJUST_Start(VOLUME_Mode_SPEAKER, false, button == HANDSET_Button_UP, displayGameOver);
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
