// PÚCA_DSP - M16 DSP phase modulation //
// https://github.com/algomusic/M16 //

#include "FspTimer.h"
#include "M16.h"
#include "Osc.h"

#define SAMPLE_RATE   22050
#define BPM           120

int16_t sineTable[TABLE_SIZE];

Osc aOsc1(sineTable);
Osc aOsc2(sineTable);

float modIndex, ratio;

FspTimer audio_timer;

void timer_callback(timer_callback_args_t __attribute((unused)) *p_args) {

  int16_t sample = aOsc1.phMod(aOsc2.next(), modIndex);
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
  beginTimer(SAMPLE_RATE);
  
  Osc::sinGen(sineTable);

  ratio = 0.5f;

}

void loop() {

  float pitch = 24 + rand(48);
  aOsc1.setPitch(pitch);
  aOsc2.setFreq(mtof(pitch) * ratio);
  modIndex = 1.0f + chaosRand(24.0f);

  int tempo = 60000 / BPM;
  delay(tempo / 4);

}