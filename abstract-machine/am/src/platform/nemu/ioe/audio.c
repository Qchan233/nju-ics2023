#include <am.h>
#include <nemu.h>

#define AUDIO_FREQ_ADDR      (AUDIO_ADDR + 0x00)
#define AUDIO_CHANNELS_ADDR  (AUDIO_ADDR + 0x04)
#define AUDIO_SAMPLES_ADDR   (AUDIO_ADDR + 0x08)
#define AUDIO_SBUF_SIZE_ADDR (AUDIO_ADDR + 0x0c)
#define AUDIO_INIT_ADDR      (AUDIO_ADDR + 0x10)
#define AUDIO_COUNT_ADDR     (AUDIO_ADDR + 0x14)

static char* buffer_end;
void __am_audio_init() {
}

void __am_audio_config(AM_AUDIO_CONFIG_T *cfg) {
  cfg->present = false;
  cfg->bufsize = inl(AUDIO_SBUF_SIZE_ADDR); //in bytes
  buffer_end = (char*) AUDIO_SBUF_ADDR + cfg->bufsize;
}

void __am_audio_ctrl(AM_AUDIO_CTRL_T *ctrl) {
  ctrl->channels = inl(AUDIO_CHANNELS_ADDR);
  ctrl->freq = inl(AUDIO_FREQ_ADDR);
  ctrl->samples = inl(AUDIO_SAMPLES_ADDR);
}

void __am_audio_status(AM_AUDIO_STATUS_T *stat) {
  stat->count = inl(AUDIO_COUNT_ADDR);
}

void __am_audio_play(AM_AUDIO_PLAY_T *ctl) {
  uint32_t write_size = (char*) ctl->buf.end - (char*) ctl->buf.start;
  AM_AUDIO_STATUS_T stat;
  while (__am_audio_status(&stat), stat.count < write_size)
  {
    /*Wait for enough space*/
  }
  uint16_t* i;
  uint16_t* write_pos = (uint16_t*) ((char*) buffer_end - stat.count);
  for(i = (uint16_t*) ctl->buf.start; i < (uint16_t*)ctl->buf.end; i++){
    outw((uintptr_t) write_pos, *i);
    write_pos += 2;
  }
  
}
