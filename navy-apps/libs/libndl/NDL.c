#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>

static int evtdev = -1;
static int fbdev = -1;
static int screen_w = 0, screen_h = 0;
static int canvas_w = 0, canvas_h = 0;
static int x_offset = 0, y_offset = 0;

int get_canvas_w() {
  return canvas_w;
}

int get_canvas_h() {
  return canvas_h;
}

uint32_t NDL_GetTicks() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_usec / 1000;
}

int NDL_PollEvent(char *buf, int len) {
  int fd = open("/dev/events", 0, 0);
  int read_len = read(fd, buf, len);
  return read_len;
}

void NDL_OpenCanvas(int *w, int *h) {
  if (getenv("NWM_APP")) {
    int fbctl = 4;
    fbdev = 5;
    screen_w = *w; screen_h = *h;
    char buf[64];
    int len = sprintf(buf, "%d %d", screen_w, screen_h);
    // let NWM resize the window and create the frame buffer
    write(fbctl, buf, len);
    while (1) {
      // 3 = evtdev
      int nread = read(3, buf, sizeof(buf) - 1);
      if (nread <= 0) continue;
      buf[nread] = '\0';
      if (strcmp(buf, "mmap ok") == 0) break;
    }
    close(fbctl);
  }
  int fd = open("/proc/dispinfo", 0, 0);
  int buf[2];
  read(fd, buf, 2 * sizeof(int));
  screen_w = buf[0]; screen_h = buf[1];
  if (*w == 0) *w = screen_w;
  if (*h == 0) *h = screen_h;
  // place canvas in the middle
  canvas_h = *h; canvas_w = *w;
  x_offset = (screen_w - *w) / 2;
  y_offset = (screen_h - *h) / 2;
}

void NDL_DrawRect(uint32_t *pixels, int x, int y, int w, int h) {
  int i;
  int fd = open("/dev/fb", 0, 0);
  x += x_offset;
  y += y_offset;
  for (i = 0; i < h; i++){
    int result = lseek(fd, (y + i) * screen_w * sizeof(uint32_t) + x * sizeof(uint32_t), SEEK_SET);
    // printf("result: %d\n", result);
    write(fd, pixels + i * w  ,w *sizeof(uint32_t));
  }
}

void NDL_OpenAudio(int freq, int channels, int samples) {
}

void NDL_CloseAudio() {
}

int NDL_PlayAudio(void *buf, int len) {
  return 0;
}

int NDL_QueryAudio() {
  return 0;
}

int NDL_Init(uint32_t flags) {
  if (getenv("NWM_APP")) {
    evtdev = 3;
  }
  return 0;
}

void NDL_Quit() {
}
