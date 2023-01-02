/** 
 * @file
 * @author Jeff Lau
 */

#include "level_select.h"
#include "../sound/sound.h"

static struct {
  LEVEL_SELECT_Callback callback;
  uint8_t levels;
  bool isLevelSelectButtonDown;
} module;

void LEVEL_SELECT_Prompt(uint8_t levels, LEVEL_SELECT_Callback callback) {
  module.callback = callback;
  module.levels = levels > 9 ? 9 : levels;
  module.isLevelSelectButtonDown = false;
  
  HANDSET_DisableTextDisplay();
  HANDSET_PrintString(" Level  1-");
  HANDSET_PrintChar('0' + levels);
  HANDSET_PrintString(":  ");
  HANDSET_EnableTextDisplay();
  HANDSET_ShowFlashingCursorAt(1);
}

void LEVEL_SELECT_HANDSET_EventHandler(HANDSET_Event const* event) {
  HANDSET_Button const button = event->button;
  bool const isButtonDown = (event->type == HANDSET_EventType_BUTTON_DOWN);
  
  if (isButtonDown && (button == HANDSET_Button_CLR)) {
    SOUND_PlayButtonBeep(button, false);
    module.callback(-1);
    return;
  }
  
  int8_t const level = button - '1';
  
  if ((level >= 0) && (level < module.levels)) {
    if (isButtonDown) {
      SOUND_PlayButtonBeep(button, false);
      HANDSET_PrintCharAt(button, 1);
      module.isLevelSelectButtonDown = true;
    } else if (module.isLevelSelectButtonDown && (event->type == HANDSET_EventType_BUTTON_UP)) {
      module.callback(level);
    }
  }
}
