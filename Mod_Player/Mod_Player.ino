// Amiga MOD player //

#include "FspTimer.h"
#include "hxcmod.h"
#include "quake.h"

FspTimer audio_timer;

#define SAMPLERATE  44100

  volatile uint16_t buff[2];
  modcontext mcontext;
  void* tune = (void*) &song;

void timer_callback(timer_callback_args_t __attribute((unused)) *p_args) {

  hxcmod_fillbuffer(&mcontext,(msample*) &buff, 1, 0);
  analogWrite(DAC, buff[0] >> 4);

}

bool beginTimer(float rate) {

  uint8_t timer_type = GPT_TIMER;
  int8_t tindex = FspTimer::get_available_timer(timer_type);
  if (tindex==0){
    FspTimer::force_use_of_pwm_reserved_timer();
    tindex = FspTimer::get_available_timer(timer_type);
  }
  if (tindex==0){
    return false;
  }

  if(!audio_timer.begin(TIMER_MODE_PERIODIC, timer_type, tindex, rate, 0.0f, timer_callback)){
    return false;
  }

  if (!audio_timer.setup_overflow_irq()){
    return false;
  }

  if (!audio_timer.open()){
    return false;
  }

  if (!audio_timer.start()){
    return false;
  }

  return true;

}

void setup() {

  analogWriteResolution(12);
  beginTimer(SAMPLERATE);

  hxcmod_init(&mcontext);
  hxcmod_setcfg(&mcontext, SAMPLERATE, 0, 0);
  hxcmod_load(&mcontext,tune, sizeof(song));

}

void loop() {

}