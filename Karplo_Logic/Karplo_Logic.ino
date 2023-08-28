// Karplus strong with distortion and reverb //

#include "FspTimer.h"

FspTimer audio_timer;

#define SAMPLERATE  22050
#define MAXTEMPO    14    // 350 BPM 16th note
#define MINTEMPO    111   // 45 BPM 16th note
#define SIZE        256   // audio buffer

float randomf(float minf, float maxf) { return minf + (rand()%(1UL << 31))*(maxf - minf) / (1UL << 31); }
float dist_filter(float input, float peak){ input = input * 4.0f / 3.0f; return clip(input, peak); }
float clip(float cval, float peak){ return (cval > 0) ? ((cval < peak) ? cval : peak) : ((cval > (-1.0f * peak)) ? cval : (-1.0f * peak)); }

  float r = 3.7f;
  float x = 0.1f;
  float b = 0.0f;
  float out = 0;
  float last = 0.0f;
  float curr = 0.0f;
  float delaymem[SIZE];
  uint16_t locat = 0;
  uint16_t bound = SIZE;
  float accum = 0;
  float lowpass = 0.99f; // 0 ... 1.0

const int FIXED_BITS = 14;
const int FIXED_SCALE = (1 << FIXED_BITS);
const int REVERB_SIZE = 0x1200;
const int REVERB_LENGTH = (int)(REVERB_SIZE * 0.9f);
const int REVERB_DECAY = (int)(FIXED_SCALE * 0.7f);

int16_t reverbAddr;
int reverbBuffer[REVERB_SIZE];


void timer_callback(timer_callback_args_t __attribute((unused)) *p_args) {

  delaymem[locat++] = out;
  if (locat >= bound) locat = 0;
  curr = delaymem[locat];
  out = 0.5f * accum;
  accum = accum - (b * (accum - (last + curr))); 
  last = curr;

  int16_t sample = 32768 * dist_filter(out, 0.1f);

  int reverb = ((int)reverbBuffer[reverbAddr] * REVERB_DECAY) >> FIXED_BITS;
  reverb += sample;
  reverbBuffer[reverbAddr] = reverb;
  reverbAddr++;
  if (reverbAddr > REVERB_LENGTH) reverbAddr = 0;
  int16_t output = sample + (reverbBuffer[reverbAddr]>>3);

  analogWrite(DAC, 2048 + (output>>4));

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

  float nx = x;
  x = r * nx * (1.0f - nx);

  b = randomf(0.049, x);

  for (int i = 0; i < SIZE; i++) delaymem[i] = randomf(-1.0f, 1.0f);
  bound = map(SIZE * x, 0, SIZE, 16, SIZE);

  delay(2 * MINTEMPO);

}