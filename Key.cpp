/*
 * Key.cpp
 * 
 * Interface for Key
 * 
 * Created by:
 *      K.C.Y
 * Date:
 *      2017/07
 */

#include "Key.h"
#include <Arduino.h>
extern "C" {
  #include <user_interface.h>
}

typedef struct key_param {
  int pin;
  key_press_fn_t long_press_fn;
  int long_press_time_ms;
} key_param_t;

static os_timer_t s_Timer;
static key_param_t s_KeyParam;

static void key_timer_handler(void* arg)
{
  int value = digitalRead(s_KeyParam.pin);
  if (value == LOW)
  {
    if (s_KeyParam.long_press_fn)
      s_KeyParam.long_press_fn();
  }
}

static void key_intr_handler()
{
  int value = digitalRead(s_KeyParam.pin);
  if (value == LOW)
  {
    os_timer_setfn(&s_Timer, (os_timer_func_t*)key_timer_handler, NULL);
    os_timer_arm(&s_Timer, s_KeyParam.long_press_time_ms, 0);
  }
  else
    os_timer_disarm(&s_Timer);
}

void key_init(int key_pin, int long_press_time_ms, key_press_fn_t long_press_fn)
{
  memset(&s_KeyParam, 0, sizeof(key_param_t));
  s_KeyParam.pin = key_pin;
  s_KeyParam.long_press_fn = long_press_fn;
  s_KeyParam.long_press_time_ms = long_press_time_ms;

  pinMode(s_KeyParam.pin, INPUT);
  attachInterrupt(s_KeyParam.pin, key_intr_handler, CHANGE);
}

