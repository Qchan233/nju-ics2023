#include <am.h>
#include <nemu.h>
#include <time.h>

void __am_timer_init() {
}

int printf(const char *fmt, ...);
void __am_timer_uptime(AM_TIMER_UPTIME_T *uptime) {
  // printf("Reading Time\n");
  uptime->us = io_read(AM_TIMER_UPTIME).us;
  // panic("Not implmented");
}

void __am_timer_rtc(AM_TIMER_RTC_T *rtc) {
  rtc->second = 0;
  rtc->minute = 0;
  rtc->hour   = 0;
  rtc->day    = 0;
  rtc->month  = 0;
  rtc->year   = 1900;
}
