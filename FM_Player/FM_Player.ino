// FM polyphonic player //

#include "FspTimer.h"
#include "playtune.h"
#include "wavetable.h"
#include "songdata.h"
#include "tuningwords.h"
#include "envelope.h"

FspTimer audio_timer;

#define SAMPLERATE    22050

extern FMchannel ch[NUM_OF_CHANNELS];

  unsigned int timePlay = 0;
  unsigned int timePlayCount = 0;
  unsigned char isPlaying = 1;
  unsigned int songIndex = 0;
  float speed = 1.0f;


void timer_callback(timer_callback_args_t __attribute((unused)) *p_args) {

  int16_t sample = generateFModTask();
  analogWrite(DAC, 2048 + (sample >> 4));

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

  for (int i = 0; i < NUM_OF_CHANNELS; i++){
    ch[i].setFreqMult_c(4.000f);
    ch[i].setFreqMult_m(1.000f);
    ch[i].setModMultiplier(4096);
    ch[i].setADSR(0.05f, 0.15f, 0.15f, 0.25f);
  }

}

void loop() {

  if (timePlayCount > timePlay){

    timePlayCount = 0;
    updateNote(isPlaying, timePlay, timePlayCount, songIndex, speed);

  } else timePlayCount++;
  
  delayMicroseconds(480); 

}