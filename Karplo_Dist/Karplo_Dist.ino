// Karplus distortion with sine fold and reverb //

#include "FspTimer.h"

FspTimer audio_timer;

#define SAMPLERATE  22050
#define MAXTEMPO    14    // 350 BPM 16th note
#define MINTEMPO    111   // 45 BPM 16th note
#define SIZE        256   // audio buffer

float randomf(float minf, float maxf) { return minf + (rand()%(1UL << 31))*(maxf - minf) / (1UL << 31); }
float hardlim(float n) { float a; if(n>= 0) a = 1.0; else if(n<0) a = -1.0; return a; }
float fold(float x, float lim) { float out = x; while (out > lim || out < -lim) { if (out > lim) out = lim - (out - lim); else if (out < -lim) out = -lim + (-out-lim); } return out; }

  float r = 3.5699456f;
  float x = 0.1f;
  float b = 0.0f;
  float out = 0.0f;
  float net = 0.0f;
  float last = 0.0f;
  float curr = 0.0f;
  float delaymem[SIZE];
  uint16_t locat = 0;
  uint16_t bound = SIZE;
  float accum = 0;
  float lowpass = 0.99f; // 0 ... 1.0

  float p1 = 0.51;
  float p2 = 0.59;
  float p3 = -0.15;
  float w11 = 0.57;
  float w12 = -0.79;
  float w13 = 0.75;

  float sinelut[SIZE];
  uint8_t loc = 0;
  uint8_t add = 0;

const int FIXED_BITS = 14;
const int FIXED_SCALE = (1 << FIXED_BITS);
const int REVERB_SIZE = 0x1200;
const int REVERB_LENGTH = (int)(REVERB_SIZE * 0.9f);
const int REVERB_DECAY = (int)(FIXED_SCALE * 0.7f);

int16_t reverbAddr;
int reverbBuffer[REVERB_SIZE];


void timer_callback(timer_callback_args_t __attribute((unused)) *p_args) {

  delaymem[locat++] = hardlim(net);
  if (locat >= bound) locat = 0;
  curr = delaymem[locat];
  out = 0.5f * accum;
  accum = accum - (b * (accum - (last + curr))); 
  last = curr;
  net = (w11 * p1) + (w12 * p2) + (w13 * p3) + (1.0f / out);

  float sine = sinelut[loc];
  loc = loc + add;

  int16_t sample = 16384 * (fold(sine, 0.5f) + out);

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

  for (int i = 0; i < SIZE; i++) sinelut[i] = sinf(TWO_PI * (i / (float) SIZE));

}

void loop() {

  float nx = x;
  x = r * nx * (1.0f - nx);

  b = randomf(0.049f, x);

  for (int i = 0; i < SIZE; i++) delaymem[i] = randomf(-1.0f, 1.0f);
  bound = map(SIZE * x, 0, SIZE, 16, SIZE);

  add = 1 + (32.0f * x);

  delay(MINTEMPO);

}