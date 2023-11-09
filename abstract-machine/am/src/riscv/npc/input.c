#include <am.h>

void __am_input_keybrd(AM_INPUT_KEYBRD_T *kbd) {
  kbd->keycode = inl(KBD_ADDR);
  if (kbd->keycode==AM_KEY_NONE){
    kbd->keydown = true;
  }
  else{
    kbd->keydown = 0;
  }
  kbd->keydown = 0;
  kbd->keycode = AM_KEY_NONE;
}
