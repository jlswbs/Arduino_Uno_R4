// Lorenz chaotic oscillator //

#include "FspTimer.h"

FspTimer audio_timer;

#define SAMPLERATE  22050

class Synth {
public:

  int32_t X = 1024;
	int32_t Y = 0;
	int32_t Z = 0;
  int dt = 20; // 0.02 * 1024
  int A = 768; // 0.75 * 1024
  
  void calculate();

};

void Synth::calculate() {

  int32_t nx = X;
	int32_t ny = Y;
	int32_t nz = Z;

  X = nx + ((dt * ((ny * nz) >> 10)) >> 10);
	Y = ny + ((dt * (nx - ny)) >> 10);
	Z = nz + ((dt * (A - ((nx * ny) >> 10))) >> 10);

}

Synth lorenz;

void timer_callback(timer_callback_args_t __attribute((unused)) *p_args) {

  lorenz.calculate();

  int16_t sample = map(lorenz.X, -4500, 4500, 0, 4095);
  analogWrite(DAC, sample);

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

}

void loop() {

}