// Vult DSP phase distortion + reverb //

#include "FspTimer.h"
#include "Phasedist.h"

Phasedist_process_type phasedist;

#define SAMPLERATE 22050

const int FIXED_BITS = 14;
const int FIXED_SCALE = (1 << FIXED_BITS);
const int REVERB_SIZE = 0x1000;
const int REVERB_LENGTH = (int)(REVERB_SIZE * 0.9f);
const int REVERB_DECAY = (int)(FIXED_SCALE * 0.7f);

int16_t reverbAddr;
int reverbBuffer[REVERB_SIZE];

  uint16_t cv;      // control voltage
  uint16_t detune;  // detune

FspTimer audio_timer;

void timer_callback(timer_callback_args_t __attribute((unused)) *p_args) {

  int16_t sample = Phasedist_process(phasedist, cv , detune) >> 2;

  int reverb = ((int)reverbBuffer[reverbAddr] * REVERB_DECAY) >> FIXED_BITS;
  reverb += sample;
  reverbBuffer[reverbAddr] = reverb;
  reverbAddr++;
  if (reverbAddr > REVERB_LENGTH) reverbAddr = 0;
  int16_t output = sample + (reverbBuffer[reverbAddr]>>3);

  analogWrite(DAC, 2048 + (output >> 4));

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
  
  Phasedist_process_init(phasedist);

}

void loop() {

  cv = 8192 + rand()%16384;
  detune = rand()%32768;

  delay(120);

}