/** 
 * @file
 * @author Jeff Lau
 */

#include "tetris_game.h"
#include "level_select.h"
#include "../sound/sound.h"
#include "../ui/volume_adjust.h"
#include "../storage/storage.h"
#include "../../mcc_generated_files/tmr6.h"
#include "../../mcc_generated_files/tmr4.h"
#include "../util/string.h"
#include "../util/timeout.h"
#include "../util/interval.h"
#include "../ui/security_code.h"
#include "../ui/string_input.h"
#include <stdlib.h>
#include <string.h>

typedef enum State {
  State_TITLE,
  State_HIGH_SCORE,
  State_MENU,
  State_SELECT_LEVEL,
  State_PLAYING,
  State_ENTER_INITIALS,
  State_GAME_OVER_1,
  State_GAME_OVER_2,
  State_GAME_OVER_3,
  State_GAME_OVER_4,
  State_GAME_OVER_5,
  State_GAME_OVER_6,
  State_VOLUME_ADJUST,
  State_ENTER_SECURITY_CODE
} State;

typedef enum Shape {
  Shape_DOT,
  Shape_I3,
  Shape_L3
} Shape;

#define SHAPE_COUNT (3)

static uint8_t const SHAPE_SIZES[] = {
  1,
  2,
  2
};

static uint8_t const SHAPE_ORIENTATIONS[] = {
  1,
  4,
  4
};

static bool const SHAPE_DATA_DOT[] = {
  1
};

static bool const SHAPE_DATA_I3[] = {
  0, 0,
  1, 1,

  0, 1,
  0, 1,

  1, 1,
  0, 0,

  1, 0,
  1, 0
};

static bool const SHAPE_DATA_L3
[] = {
  1, 0,
  1, 1,

  0, 1,
  1, 1,

  1, 1,
  0, 1,

  1, 1,
  1, 0,
};

static bool const* const SHAPE_DATA[] = {
  SHAPE_DATA_DOT,
  SHAPE_DATA_I3,
  SHAPE_DATA_L3
};

#define BOARD_WIDTH (HANDSET_TEXT_DISPLAY_ROWS)
#define BOARD_HEIGHT (HANDSET_TEXT_DISPLAY_COLUMNS)
#define BOARD_SIZE (BOARD_WIDTH * BOARD_HEIGHT)

#define LEVEL_COUNT (9)

static uint8_t const INTERVALS_BY_LEVEL_INDEX[] = {
  100, 80, 64, 51, 41, 33, 26, 21, 17
};

#define LINE_FLASH_INTERVAL (25)
#define LINE_FLASH_COUNT (5)

#define FCN_TIMEOUT (400)

#define FAST_DROP_INTERVAL (10)

#define MAX_SCORE (65535)

static struct {
  bool isGameStarted;
  bool isMusicPlaying;
  TETRIS_GAME_ReturnCallback returnCallback;
  State state;
  interval_t stateInterval;
  timeout_t fcnTimeout;
  uint8_t startLevel;
  uint8_t level;
  bool boardData[BOARD_SIZE];
  Shape pieceShape;
  uint8_t pieceOrientation;
  int8_t pieceX;
  int8_t pieceY;
  int8_t lineClearOffset;
  int8_t lineClearCount;
  uint8_t lineFlashCount;
  uint16_t score;
  uint16_t totalLinesCleared;
  bool isFastDrop;
  bool isHighScore;
  bool isHighScoreInitialsEntered;
  char highScoreInitialsBuffer[4];
} module;

static void startMusic(void) {
  if (!module.isMusicPlaying) {
    SOUND_PlayEffect(
        SOUND_Channel_BACKGROUND, 
        SOUND_Target_SPEAKER, 
        VOLUME_Mode_GAME_MUSIC, 
        SOUND_Effect_TETRIS_MUSIC, 
        true
        );
    
    module.isMusicPlaying = true;
  }
}

static void stopMusic(void) {
  module.isMusicPlaying = false;
  SOUND_Stop(SOUND_Channel_BACKGROUND);
}

static void exitGame(void) {
  stopMusic();
  HANDSET_CancelCurrentButtonHoldEvents();
  module.returnCallback();
}

static void displayTitle(void) {
  stopMusic();
  HANDSET_DisableTextDisplay();
  HANDSET_PrintString("Tetris!Press #");
  HANDSET_EnableTextDisplay();
  module.state = State_TITLE;
}

static void printHighScore(void) {
  char scoreStr[5];

  uint2str(scoreStr, STORAGE_GetTetrisHighScore(), 5, 3);
  STORAGE_GetTetrisHighScoreInitials(module.highScoreInitialsBuffer);

  HANDSET_DisableTextDisplay();
  HANDSET_PrintString("#1: ");
  HANDSET_PrintString(module.highScoreInitialsBuffer);
  HANDSET_PrintCharN(' ', 3 - strlen(module.highScoreInitialsBuffer));
  HANDSET_PrintStringN(scoreStr, 5);
  HANDSET_PrintCharN(' ', 2);
  HANDSET_EnableTextDisplay();
}

static void displayHighScore(void) {
  printHighScore();
  module.state = State_HIGH_SCORE;
}

static void resetHighScore(void) {
  STORAGE_SetTetrisHighScore(0);
  STORAGE_SetTetrisHighScoreInitials("???");
  displayHighScore();
}

static void displayMenu(void) {
  startMusic();
  HANDSET_DisableTextDisplay();
  HANDSET_PrintString("1:Cont.2:New  ");
  HANDSET_EnableTextDisplay();
  module.state = State_MENU;
}

static void handle_LEVEL_SELECT_Callback(int8_t level);

static void displayLevelSelect(void) {
  startMusic();
  INTERVAL_Cancel(&module.stateInterval);

  LEVEL_SELECT_Prompt(LEVEL_COUNT, handle_LEVEL_SELECT_Callback);
  module.state = State_SELECT_LEVEL;
}

static uint8_t getDisplayPos(int8_t x, int8_t y) {
  return HANDSET_GetDisplayPos(y, x);
}

static uint8_t getBoardPos(int8_t x, int8_t y) {
  if (x < 0 || x >= BOARD_WIDTH) {
    return 0xFF;
  }
  
  if (y < 0 || y >= BOARD_HEIGHT) {
    return 0xFF;
  }
  
  return (uint8_t)(y * BOARD_WIDTH + x);
}

static void drawPiece(void) {
  uint8_t const shapeSize = SHAPE_SIZES[module.pieceShape];
  bool const* const shapeData = SHAPE_DATA[module.pieceShape] + module.pieceOrientation * shapeSize * shapeSize;
  
  for (int8_t x = 0; x < shapeSize; ++x) {
    for (int8_t y = 0; y < shapeSize; ++y) {
      if (shapeData[y * shapeSize + x]) {
        uint8_t const pos = getDisplayPos(x + module.pieceX, y + module.pieceY);
        HANDSET_PrintCharAt(HANDSET_Symbol_RECTANGLE, pos);
      }
    }
  }
}

static void updateDisplay(bool const* const positionsToErase, bool const* const positionsToDraw, int8_t const xDelta) {
  if (xDelta == 0) {
    // Update from left to right (secondarily bottom to top))
    for (uint8_t i = 0; i < HANDSET_TEXT_DISPLAY_COLUMNS; ++i) {
      for (uint8_t j = 0; j < HANDSET_TEXT_DISPLAY_ROWS; ++j) {
        uint8_t const pos = ((j + 1) * HANDSET_TEXT_DISPLAY_COLUMNS - 1) - i;

        if (positionsToDraw[pos]) {
          HANDSET_PrintCharAt(HANDSET_Symbol_RECTANGLE, pos);
        } else if (positionsToErase[pos]) {
          HANDSET_PrintCharAt(' ', pos);
        }
      }
    }
  } else if (xDelta < 0) {
    // Update from top to bottom (secondarily left to right)
    for (uint8_t j = 0; j < HANDSET_TEXT_DISPLAY_ROWS; ++j) {
      for (uint8_t i = 0; i < HANDSET_TEXT_DISPLAY_COLUMNS; ++i) {
        uint8_t const pos = ((2 - j) * HANDSET_TEXT_DISPLAY_COLUMNS - 1) - i;

        if (positionsToDraw[pos]) {
          HANDSET_PrintCharAt(HANDSET_Symbol_RECTANGLE, pos);
        } else if (positionsToErase[pos]) {
          HANDSET_PrintCharAt(' ', pos);
        }
      }
    }
  } else {
    // Update from bottom to top (secondarily left to right)
    for (uint8_t j = 0; j < HANDSET_TEXT_DISPLAY_ROWS; ++j) {
      for (uint8_t i = 0; i < HANDSET_TEXT_DISPLAY_COLUMNS; ++i) {
        uint8_t const pos = ((j + 1) * HANDSET_TEXT_DISPLAY_COLUMNS - 1) - i;

        if (positionsToDraw[pos]) {
          HANDSET_PrintCharAt(HANDSET_Symbol_RECTANGLE, pos);
        }

        if (positionsToErase[pos]) {
          HANDSET_PrintCharAt(' ', pos);
        }
      }
    }
  }
}

static void updatePiece(uint8_t newOrientation, int8_t newX, int8_t newY) {
  bool positionsToErase[HANDSET_TEXT_DISPLAY_LENGTH];
  bool positionsToDraw[HANDSET_TEXT_DISPLAY_LENGTH];
  
  memset(positionsToErase, false, HANDSET_TEXT_DISPLAY_LENGTH);
  memset(positionsToDraw, false, HANDSET_TEXT_DISPLAY_LENGTH);
  
  uint8_t const shapeSize = SHAPE_SIZES[module.pieceShape];
  bool const* const oldShapeData = SHAPE_DATA[module.pieceShape] + module.pieceOrientation * shapeSize * shapeSize;
  bool const* const newShapeData = SHAPE_DATA[module.pieceShape] + newOrientation * shapeSize * shapeSize;
  
  for (int8_t x = 0; x < shapeSize; ++x) {
    for (int8_t y = 0; y < shapeSize; ++y) {
      if (oldShapeData[y * shapeSize + x]) {
        uint8_t const pos = getDisplayPos(x + module.pieceX, y + module.pieceY);
        if (pos != 0xFF) {
          positionsToErase[pos] = true;
        }
      }
    }
  }
  
  for (int8_t x = 0; x < shapeSize; ++x) {
    for (int8_t y = 0; y < shapeSize; ++y) {
      if (newShapeData[y * shapeSize + x]) {
        uint8_t const pos = getDisplayPos(x + newX, y + newY);
        if (pos != 0xFF) {
          if (positionsToErase[pos]) {
            positionsToErase[pos] = false;
          } else {
            positionsToDraw[pos] = true;
          }
        }
      }
    }
  }

  updateDisplay(positionsToErase, positionsToDraw, newX - module.pieceX);
  
  module.pieceOrientation = newOrientation;
  module.pieceX = newX;
  module.pieceY = newY;
}

static bool canPositionPiece(uint8_t newOrientation, int8_t newX, int8_t newY) {
  uint8_t const shapeSize = SHAPE_SIZES[module.pieceShape];
  bool const* const shapeData = SHAPE_DATA[module.pieceShape] + newOrientation * shapeSize * shapeSize;
  
  for (int8_t x = 0; x < shapeSize; ++x) {
    for (int8_t y = 0; y < shapeSize; ++y) {
      if (shapeData[y * shapeSize + x]) {
        uint8_t const boardPos = getBoardPos(x + newX, y + newY); 
        
        if ((boardPos == 0xFF) || module.boardData[boardPos]) {
          return false;
        }
      }
    }
  }
  
  return true;
}

static void playSoundEffect(SOUND_Effect effect) {
    SOUND_PlayEffect(
        SOUND_Channel_FOREGROUND, 
        SOUND_Target_SPEAKER, 
        VOLUME_Mode_SPEAKER, 
        effect, 
        false
        );
}

static void rotatePiece(int8_t amount) {
  const uint8_t orientations = SHAPE_ORIENTATIONS[module.pieceShape];
  uint8_t const newOrientation = (orientations + module.pieceOrientation + amount) % orientations;
  bool didRotate = false;

  if (newOrientation == module.pieceOrientation) {
    return;
  }
  
  if (canPositionPiece(newOrientation, module.pieceX, module.pieceY)) {
    updatePiece(newOrientation, module.pieceX, module.pieceY);
    didRotate = true;
  } else if (
      (module.pieceX != 0) &&
      canPositionPiece(newOrientation, 0, module.pieceY)
      ) {
    updatePiece(newOrientation, 0, module.pieceY);
    didRotate = true;
  }  
  
  if (didRotate) {
    playSoundEffect(SOUND_Effect_TETRIS_USER_ROTATE);
  }
}

void movePieceX(int8_t amount) {
  int8_t newX = module.pieceX + amount;
  
  if (canPositionPiece(module.pieceOrientation, newX, module.pieceY)) {
    updatePiece(module.pieceOrientation, newX, module.pieceY);
    playSoundEffect(SOUND_Effect_TETRIS_USER_MOVE);
  }  
}

static void displayGameOver(void);

static void spawnPiece(void) {
  module.pieceShape = rand() % SHAPE_COUNT;
  module.pieceY = BOARD_HEIGHT - SHAPE_SIZES[module.pieceShape];

  switch (module.pieceShape) {
    case Shape_DOT:
      module.pieceOrientation = 0;
      break;
      
    case Shape_I3:
      // avoid ambiguous initial orientation
      module.pieceOrientation = ((rand() % 3) + 3) % 4;
      break;
      
    case Shape_L3:
      module.pieceOrientation = rand() % 4;
      break;
  }
  
  if (SHAPE_SIZES[module.pieceShape] == 1) {
    module.pieceX = rand() & 1;
  } else {
    module.pieceX = 0;
  }
  
  drawPiece();
  
  if (
      !canPositionPiece(module.pieceOrientation, module.pieceX, module.pieceY) ||
      (module.score == MAX_SCORE)
      ) {
    // Immediately save the high score with unknown initials, in case initials
    // entry is never completed.
    if (module.isHighScore) {
      STORAGE_SetTetrisHighScore(module.score);
      STORAGE_SetTetrisHighScoreInitials("???");
    }
    
    playSoundEffect(SOUND_Effect_TETRIS_LOSE);
    displayGameOver();
  }  
}

static void addPieceToBoard(void) {
  uint8_t const shapeSize = SHAPE_SIZES[module.pieceShape];
  bool const* const shapeData = SHAPE_DATA[module.pieceShape] + module.pieceOrientation * shapeSize * shapeSize;
  
  for (int8_t x = 0; x < shapeSize; ++x) {
    for (int8_t y = 0; y < shapeSize; ++y) {
      if (shapeData[y * shapeSize + x]) {
        module.boardData[getBoardPos(x + module.pieceX, y + module.pieceY)] = true;
      }
    }
  }
  
  module.lineClearOffset = -1;
  module.lineClearCount = 0;
  
  for (uint8_t y = 0; y < shapeSize + 1; ++ y) {
    bool isFull = true;
    int8_t const boardY = y + module.pieceY - 1;
    
    if (boardY < 0) {
      continue;
    }
    
    for (int8_t x = 0; x < BOARD_WIDTH; ++x) {
      if (!module.boardData[boardY * BOARD_WIDTH + x]) {
        isFull = false;
        break;
      }
    }
    
    if (isFull) {
      if (module.lineClearOffset == -1) {
        module.lineClearOffset = boardY;
      }
      
      ++module.lineClearCount;
    } else if (module.lineClearOffset != -1) {
      break;
    }
  }
  
  if (module.lineClearCount > 1) {
    module.totalLinesCleared += (uint8_t)module.lineClearCount;
    uint8_t const points = (uint8_t)((module.lineClearCount - 2) * 2 + 1) * (module.level + 1);
    
    if (points > (MAX_SCORE - module.score)) {
      module.score = MAX_SCORE;
    } else {
      module.score += points;
    }
    
    module.isHighScore = module.score > STORAGE_GetTetrisHighScore();
    
    module.lineFlashCount = 0;
    INTERVAL_Initialize(&module.stateInterval, LINE_FLASH_INTERVAL);
    
    playSoundEffect(
        module.lineClearCount == 2 
          ? SOUND_Effect_TETRIS_CLEAR_2_LINES 
          : SOUND_Effect_TETRIS_CLEAR_3_LINES
        );
    
    if (
        (module.totalLinesCleared >= ((uint16_t)(module.level + 1) * 20)) &&
        (module.level < LEVEL_COUNT - 1)
        ) {
      ++module.level;
    }
  } else {
    module.lineClearCount = 0;

    playSoundEffect(SOUND_Effect_TETRIS_PIECE_PLACED);
    INTERVAL_Initialize(&module.stateInterval, INTERVALS_BY_LEVEL_INDEX[module.level]);

    spawnPiece();
  }

  module.isFastDrop = false;
  INTERVAL_Start(&module.stateInterval, module.lineClearCount > 1);
}

static void updateBoardForLineClear(void) {
  bool positionsToErase[HANDSET_TEXT_DISPLAY_LENGTH];
  bool positionsToDraw[HANDSET_TEXT_DISPLAY_LENGTH];
  
  memset(positionsToErase, false, HANDSET_TEXT_DISPLAY_LENGTH);
  memset(positionsToDraw, false, HANDSET_TEXT_DISPLAY_LENGTH);
  
  for (int8_t y = module.lineClearOffset; y < BOARD_HEIGHT; ++y) {
    for (int8_t x = 0; x < BOARD_WIDTH; ++x) {
      uint8_t const boardPos = getBoardPos(x, y);
      uint8_t const offsetBoardPos = getBoardPos(x, y + module.lineClearCount);
      bool const isDrawn = module.boardData[boardPos];
      bool const newDrawn = (offsetBoardPos == 0xFF) ? false : module.boardData[offsetBoardPos];
      
      if (newDrawn != isDrawn) {
        uint8_t const displayPos = getDisplayPos(x, y);
        
        if (displayPos != 0xFF) {
          if (newDrawn) {
            positionsToDraw[displayPos] = true;
          } else {
            positionsToErase[displayPos] = true;
          }
        }
      }
    }
  }
  
  updateDisplay(positionsToErase, positionsToDraw, 0);
  
  memmove(
      module.boardData + module.lineClearOffset * BOARD_WIDTH, 
      module.boardData + (module.lineClearOffset + module.lineClearCount) * BOARD_WIDTH, 
      (BOARD_HEIGHT - (size_t)(module.lineClearOffset + module.lineClearCount)) * BOARD_WIDTH
      );
  
  memset(
      module.boardData + (BOARD_HEIGHT - module.lineClearCount) * BOARD_WIDTH,
      false,
      (size_t)module.lineClearCount * BOARD_WIDTH
      );
  
  module.lineClearCount = 0;
}

static void drawFullGameBoard(void) {
  HANDSET_DisableTextDisplay();
  
  uint8_t i;
  bool pixel;
  
  for (i = 0; i < BOARD_HEIGHT; ++i) {
    pixel = module.boardData[BOARD_WIDTH * i];
    HANDSET_PrintChar(pixel ? HANDSET_Symbol_RECTANGLE : ' ');
  }

  for (i = 0; i < BOARD_HEIGHT; ++i) {
    pixel = module.boardData[BOARD_WIDTH * i + 1];
    HANDSET_PrintChar(pixel ? HANDSET_Symbol_RECTANGLE : ' ');
  }
  
  drawPiece();
  
  HANDSET_EnableTextDisplay();
}

static void displayGameOver(void) {
  stopMusic();
  
  INTERVAL_Initialize(&module.stateInterval, 150);
  INTERVAL_Start(&module.stateInterval, true);
  
  module.isGameStarted = false;
  module.state = State_GAME_OVER_1;
}

static void handleHighScoreInitialsInputResult(STRING_INPUT_Result result, char const* initials) {
  if (result == STRING_INPUT_Result_APPLY) {
    STORAGE_SetTetrisHighScoreInitials(initials);
  }
  
  module.isHighScoreInitialsEntered = true;
  displayGameOver();
}

static void promptHighScoreInitials(void) {
  module.highScoreInitialsBuffer[0] = 0;
  
  STRING_INPUT_Start(
      module.highScoreInitialsBuffer,
      3,
      "You #1!Name ? ",
      true,
      true,
      handleHighScoreInitialsInputResult
  );
  
  module.state = State_ENTER_INITIALS;
}

static void resumeGame(void) {
  startMusic();
  drawFullGameBoard();
  module.isFastDrop = false;
  INTERVAL_Initialize(&module.stateInterval, module.lineClearCount ? LINE_FLASH_INTERVAL : INTERVALS_BY_LEVEL_INDEX[module.level]);
  INTERVAL_Start(&module.stateInterval, false);
  module.state = State_PLAYING;
}

static void startNewGame(uint8_t level) {
  srand(((uint16_t)TMR6_ReadTimer() << 8) | TMR4_ReadTimer());

  module.isGameStarted = true;
  module.startLevel = module.level = level;
  memset(module.boardData, 0, BOARD_SIZE);
  module.lineClearCount = 0;
  module.totalLinesCleared = 0;
  module.score = 0;
  module.isHighScore = false;
  module.isHighScoreInitialsEntered = false;
  spawnPiece();
  resumeGame();
}

static void gameLoop(void) {
  if (module.lineClearCount) {
    if (++module.lineFlashCount == LINE_FLASH_COUNT) {
      updateBoardForLineClear();
      spawnPiece();
      INTERVAL_Initialize(&module.stateInterval, INTERVALS_BY_LEVEL_INDEX[module.level]);
      INTERVAL_Start(&module.stateInterval, false);
    } else {
      char c = (module.lineFlashCount & 1) ? ' ' : HANDSET_Symbol_RECTANGLE;
      
      for (int8_t y = 0; y < module.lineClearCount; ++y) {
        for (int8_t x = 0; x < BOARD_WIDTH; ++x) {
          HANDSET_PrintCharAt(c, getDisplayPos(x, y + module.lineClearOffset));
        }
      }
    }
    return;
  }
  
  if (canPositionPiece(module.pieceOrientation, module.pieceX, module.pieceY - 1)) {
    updatePiece(module.pieceOrientation, module.pieceX, module.pieceY - 1);
  } else {
    addPieceToBoard();
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

static VOLUME_ADJUST_ReturnCallback const VOLUME_ADJUST_RETURN_CALLBACKS[] = {
  displayTitle,
  displayHighScore,
  displayMenu,
  displayLevelSelect,
  resumeGame,
  displayGameOver,
  displayGameOver,
  displayGameOver,
  displayGameOver,
  displayGameOver,
  displayGameOver,
  displayGameOver
};

void TETRIS_GAME_Start(TETRIS_GAME_ReturnCallback returnCallback) {
  module.returnCallback = returnCallback;
  TIMEOUT_Cancel(&module.fcnTimeout);
  displayTitle();
}

void TETRIS_GAME_Task(void) {
  if (TIMEOUT_Task(&module.fcnTimeout)) {
    HANDSET_SetIndicator(HANDSET_Indicator_FCN, false);
  }
  
  switch (module.state) {
    case State_PLAYING:
      if (INTERVAL_Task(&module.stateInterval)) {
        gameLoop();
      }
      break;
    
    case State_GAME_OVER_1: 
      if (INTERVAL_Task(&module.stateInterval)) {
        if (module.isHighScore && !module.isHighScoreInitialsEntered) {
          promptHighScoreInitials();
        } else {
          HANDSET_DisableTextDisplay();
          HANDSET_ClearText();
          HANDSET_PrintString("Game   Over ");
          HANDSET_EnableTextDisplay();
          ++module.state;
        }
      }
      break;

    case State_GAME_OVER_2: 
      if (INTERVAL_Task(&module.stateInterval)) {
        HANDSET_DisableTextDisplay();
        HANDSET_PrintString(" Level   ");
        HANDSET_PrintChar('1' + module.startLevel);
        HANDSET_PrintChar(HANDSET_Symbol_RIGHT_ARROW);
        HANDSET_PrintChar('1' + module.level);
        HANDSET_PrintCharN(' ', 2);
        HANDSET_EnableTextDisplay();
        ++module.state;
      }
      break;

    case State_GAME_OVER_3: 
      if (INTERVAL_Task(&module.stateInterval)) {
        char scoreStr[5];
        
        uint2str(scoreStr, module.score, 5, 3);
        
        HANDSET_DisableTextDisplay();
        HANDSET_PrintString(" Score ");
        HANDSET_PrintStringN(scoreStr, 5);
        HANDSET_PrintCharN(' ', 2);
        HANDSET_EnableTextDisplay();
        ++module.state;
      }
      break;
      
    case State_GAME_OVER_4: 
      if (INTERVAL_Task(&module.stateInterval)) {
        printHighScore();
        ++module.state;
      }
      break;
      
    case State_GAME_OVER_5: 
      if (INTERVAL_Task(&module.stateInterval)) {
        HANDSET_DisableTextDisplay();
        HANDSET_PrintString("#:Again*:New  ");
        HANDSET_EnableTextDisplay();
        ++module.state;
      }
      break;
      
    case State_GAME_OVER_6: 
      if (INTERVAL_Task(&module.stateInterval)) {
        drawFullGameBoard();
        module.state = State_GAME_OVER_1;
      }
      break;
      
    case State_VOLUME_ADJUST:
      VOLUME_ADJUST_Task();
      break;
  }
}

void TETRIS_GAME_Timer10MS_Interrupt(void) {
  switch (module.state) {
    case State_VOLUME_ADJUST:
      VOLUME_ADJUST_Timer10MS_Interrupt();
      break;

    default:
      INTERVAL_Timer_Interrupt(&module.stateInterval);
      TIMEOUT_Timer_Interrupt(&module.fcnTimeout);
      break;
  }
}

void TETRIS_GAME_HANDSET_EventHandler(HANDSET_Event const* event) {
  HANDSET_Button const button = event->button;
  bool const isButtonDown = (event->type == HANDSET_EventType_BUTTON_DOWN);
  bool const isButtonUp = (event->type == HANDSET_EventType_BUTTON_UP);
  bool const isFcn = TIMEOUT_IsPending(&module.fcnTimeout);

  if (isFcn && isButtonDown) {
    TIMEOUT_Cancel(&module.fcnTimeout);
    HANDSET_SetIndicator(HANDSET_Indicator_FCN, false);
    
    if (button == HANDSET_Button_CLR) {
      HANDSET_CancelCurrentButtonHoldEvents();
    }
  }
  
  if (
      (button == HANDSET_Button_CLR) && 
      (event->type == HANDSET_EventType_BUTTON_HOLD) &&
      (event->holdDuration == HANDSET_HoldDuration_SHORT)
      ) {
    SOUND_StopButtonBeep();
    exitGame();
    return;
  }
  
  if (
      (module.state != State_VOLUME_ADJUST) && 
      (module.state != State_ENTER_SECURITY_CODE) &&
      (module.state != State_ENTER_INITIALS)
      ) {
    if (isButtonDown) {
      switch (button) {
        case HANDSET_Button_FCN:
          SOUND_PlayButtonBeep(button, false);

          if (!isFcn) {
            TIMEOUT_Start(&module.fcnTimeout, FCN_TIMEOUT);
            HANDSET_SetIndicator(HANDSET_Indicator_FCN, true);
          }
          return;
          
        case HANDSET_Button_UP:  
        case HANDSET_Button_DOWN:  
          VOLUME_ADJUST_Start(
              isFcn ? VOLUME_Mode_SPEAKER : VOLUME_Mode_GAME_MUSIC, 
              !isFcn && module.isMusicPlaying,
              button == HANDSET_Button_UP, 
              VOLUME_ADJUST_RETURN_CALLBACKS[module.state]
              );
          module.state = State_VOLUME_ADJUST;
          return;
      }
    }
  }
  
  switch (module.state) {
    case State_TITLE:
      if (isButtonDown && !isFcn) {
        switch(button) {
          case HANDSET_Button_CLR:
            SOUND_PlayButtonBeep(button, false);
            exitGame();
            break;
            
          case HANDSET_Button_RCL:
            SOUND_PlayButtonBeep(button, false);
            displayHighScore();
            break;
            
          case HANDSET_Button_POUND:
            SOUND_PlayButtonBeep(button, false);
            if (module.isGameStarted) {
              displayMenu();
            } else {
              displayLevelSelect();
            }
            break;
        }
      }
      break;
      
    case State_HIGH_SCORE:
      if (isButtonDown && (button == HANDSET_Button_CLR)) {
        SOUND_PlayButtonBeep(button, false);

        if (isFcn) {
          SECURITY_CODE_Prompt(resetHighScore, displayHighScore);
          module.state = State_ENTER_SECURITY_CODE;
        } else {
          displayTitle();
        }
      }
      break;
      
    case State_MENU: 
      if (isButtonDown && !isFcn) {
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
        }
      }
      break;
    
    case State_SELECT_LEVEL:
      LEVEL_SELECT_HANDSET_EventHandler(event);
      break;
      
    case State_PLAYING:
      if (isButtonDown && !isFcn) {
        switch (button) {
          case HANDSET_Button_CLR:
            SOUND_PlayButtonBeep(button, false);
            displayMenu();
            break;
            
          case HANDSET_Button_1: {
            rotatePiece(-1);
            break;
          }
            
          case HANDSET_Button_3: {
            rotatePiece(1);
            break;
          }
            
          case HANDSET_Button_2: 
            movePieceX(-1);
            break;
          
          case HANDSET_Button_5: 
            movePieceX(1);
            break;

          case HANDSET_Button_4: 
            if (!module.lineClearCount) {
              module.isFastDrop = true;
              INTERVAL_Initialize(&module.stateInterval, FAST_DROP_INTERVAL);
              INTERVAL_Start(&module.stateInterval, true);
            }
            break;
        }
      } else if (isButtonUp) {
        if ((button == HANDSET_Button_4) && module.isFastDrop) {
          INTERVAL_Initialize(&module.stateInterval, INTERVALS_BY_LEVEL_INDEX[module.level]);
          INTERVAL_Start(&module.stateInterval, false);
        }
      }
      break;
      
    case State_ENTER_INITIALS:
      STRING_INPUT_HANDSET_EventHandler(event);
      break;
      
    case State_GAME_OVER_1:
    case State_GAME_OVER_2:
    case State_GAME_OVER_3:
    case State_GAME_OVER_4:
    case State_GAME_OVER_5:
    case State_GAME_OVER_6:
      if (isButtonDown && !isFcn) {
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
            startNewGame(module.startLevel);
            break;
        }
      }
      break;

    case State_VOLUME_ADJUST:
      VOLUME_ADJUST_HANDSET_EventHandler(event);
      break;
      
    case State_ENTER_SECURITY_CODE:
      SECURITY_CODE_HANDSET_EventHandler(event);
      break;
  }
}

