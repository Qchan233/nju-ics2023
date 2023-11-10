#include <am.h>
#include <nemu.h>
#include <stdio.h>

#define SYNC_ADDR (VGACTL_ADDR + 4)
static int WIDTH, HEIGHT;
#define POS(x,y) (x)*(WIDTH) + (y)
void __am_gpu_init() {
  WIDTH = io_read(AM_GPU_CONFIG).width;
  HEIGHT = io_read(AM_GPU_CONFIG).height;
  printf("%d %d\n", WIDTH, HEIGHT);
  int i;
  int w = io_read(AM_GPU_CONFIG).width;  // TODO: get the correct width
  int h = io_read(AM_GPU_CONFIG).height;  // TODO: get the correct height
  
  uint32_t *fb = (uint32_t *)(uintptr_t)FB_ADDR;
  for (i = 0; i < w * h; i ++) fb[i] = i;
  outl(SYNC_ADDR, 1);
}

#define HMASK 0xFFFF
void __am_gpu_config(AM_GPU_CONFIG_T *cfg) {
  uint32_t wh = inl(VGACTL_ADDR);
  *cfg = (AM_GPU_CONFIG_T) {
    .present = true, .has_accel = false,
    .width = wh >> 16, .height = HMASK & wh,
    .vmemsz = 0
  };
}

void __am_gpu_fbdraw(AM_GPU_FBDRAW_T *ctl) {
  if (ctl->sync) {
    outl(SYNC_ADDR, 1);
  }
  int i,j;
  for(i=0;i<ctl->w;i++){
    for(j=0;j<ctl->h;j++){
      outl(FB_ADDR + POS(ctl->x + j, ctl->y + i) * 4, ((uint32_t*)(ctl->pixels))[i*ctl->w + j]);
    }
  }
  
}

void __am_gpu_status(AM_GPU_STATUS_T *status) {
  status->ready = true;
}
