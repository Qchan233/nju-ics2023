#include <common.h>

#if defined(MULTIPROGRAM) && !defined(TIME_SHARING)
# define MULTIPROGRAM_YIELD() yield()
#else
# define MULTIPROGRAM_YIELD()
#endif

#define NAME(key) \
  [AM_KEY_##key] = #key,

static const char *keyname[256] __attribute__((used)) = {
  [AM_KEY_NONE] = "NONE",
  AM_KEYS(NAME)
};

size_t serial_write(const void *buf, size_t offset, size_t len) {
    int i;
    for (i = 0; i < len; i++){
      putch(((char *)buf)[i]);
    }
    return 0;
}

size_t events_read(void *buf, size_t offset, size_t len) {
  AM_INPUT_KEYBRD_T ev = io_read(AM_INPUT_KEYBRD);
  printf("ev.keycode = %d\n", ev.keycode);
  if (ev.keycode == AM_KEY_NONE){
    return 0;
  }
  if (ev.keydown){
    printf(buf, "kd %s", keyname[ev.keycode]);
  } else {
    printf(buf, "ku %s", keyname[ev.keycode]);
  }
  return 3 + strlen(keyname[ev.keycode]);
}

size_t dispinfo_read(void *buf, size_t offset, size_t len) {
  return 0;
}

size_t fb_write(const void *buf, size_t offset, size_t len) {
  return 0;
}

void init_device() {
  Log("Initializing devices...");
  ioe_init();
}
