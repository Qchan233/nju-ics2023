#include <am.h>

void __am_input_keybrd(AM_INPUT_KEYBRD_T *kbd) {
  kbd->keycode = inl(KBD_ADDR);
  if (kbd->keycode==AM_KEY_NONE){
    kbd->keydown = false;
  }
  else{
    kbd->keydown = true;
  }
}
