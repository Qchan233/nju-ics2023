#include <am.h>
#include <nemu.h>
#include <stdio.h>

#define KEYDOWN_MASK 0x8000

void __am_input_keybrd(AM_INPUT_KEYBRD_T *kbd) {
  uint32_t kbd_info = inl(KBD_ADDR);
  kbd->keycode = kbd_info & ~KEYDOWN_MASK;
  kbd->keydown = kbd_info - kbd->keycode;
}
