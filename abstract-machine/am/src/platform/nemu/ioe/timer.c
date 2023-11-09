#include <am.h>
#include <nemu.h>
#include <time.h>
#include <stdio.h>

static uint32_t previous_time;
void __am_timer_init() {
  // uint32_t lo = inl(RTC_ADDR);
  // uint32_t hi = inl(RTC_ADDR + 4);
  // base_time = ((uint64_t) hi) << 32 | lo; 
}

void __am_timer_uptime(AM_TIMER_UPTIME_T *uptime) {
  uint32_t lo = inl(RTC_ADDR);
  uint32_t hi = inl(RTC_ADDR + 4);
  if (previous_time != lo){
    previous_time = lo;
    printf("\nlo:%d\n",lo);
  }
  uint64_t now = ((uint64_t) hi) << 32 | lo; 
  uptime->us = now;
}

void __am_timer_rtc(AM_TIMER_RTC_T *rtc) {
  rtc->second = 0;
  rtc->minute = 0;
  rtc->hour   = 0;
  rtc->day    = 0;
  rtc->month  = 0;
  rtc->year   = 1900;
}
