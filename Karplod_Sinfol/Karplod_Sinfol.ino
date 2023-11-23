// Karplus distortion + sine fold + echoverb //

#include "FspTimer.h"

FspTimer audio_timer;

#define SAMPLERATE  22050
#define BPM         120
#define SIZE        256
#define BUFF_SIZE   2048

float randomf(float minf, float maxf) { return minf + (rand()%(1UL << 31))*(maxf - minf) / (1UL << 31); }
float hardlim(float n) { float a; if(n>= 0) a = 1.0; else if(n<0) a = -1.0; return a; }
float fold(float x, float lim) { float out = x; while (out > lim || out < -lim) { if (out > lim) out = lim - (out - lim); else if (out < -lim) out = -lim + (-out-lim); } return out; }

  unsigned int buff_pos = 0;
  float buff[BUFF_SIZE];

  float r = 3.5699456f;
  float x = 0.1f;
  float out = 0.0f;
  float last = 0.0f;
  float curr = 0.0f;
  float delaymem[SIZE];
  uint16_t locat = 0;
  uint16_t bound = SIZE;
  float accum = 0.0f;
  float lowpass = 0.0f;
  float sinelut[SIZE];
  uint8_t loc = 0;
  uint8_t add = 0;

float echo_verb(float sample, float decay) {

  float old_sample = buff[buff_pos];
  float new_sample = (sample + (old_sample * decay)) / 2.0f;
  buff[buff_pos] = new_sample;
  buff_pos = buff_pos + 1;
  if (buff_pos == BUFF_SIZE) buff_pos = 0; 
  return new_sample;

}


void timer_callback(timer_callback_args_t __attribute((unused)) *p_args) {

  float sine = sinelut[loc];

  delaymem[locat++] = hardlim(out);
  if (locat >= bound) locat = 0;
  curr = delaymem[locat];
  out = 0.5f * (accum + fold(sine, 0.5f));
  accum = accum - (lowpass * (accum - (last + curr))); 
  last = curr;
  loc = loc + add;

  int16_t sample = 1024.0f * echo_verb(out, sine);
  analogWrite(DAC, 2048 + sample);

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

  for (int i = 0; i < SIZE; i++) sinelut[i] = sinf(TWO_PI * (i / (float) SIZE));

}

void loop() {

  float nx = x;
  x = r * nx * (1.0f - nx);

  add = 1 + (16.0f * x);
  lowpass = randomf(0.049f, x);
  for (int i = 0; i < SIZE; i++) delaymem[i] = randomf(-1.0f, 1.0f);
  bound = map(SIZE * x, 0, SIZE, 16, SIZE);
  
  int tempo = 60000 / BPM;
  delay(tempo / 4);

}