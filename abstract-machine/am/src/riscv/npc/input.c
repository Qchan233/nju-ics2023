#include <am.h>

void __am_input_keybrd(AM_INPUT_KEYBRD_T *kbd) {
  kbd->keycode = AM_KEY_NONE;
  kbd->keydown = false;

}
