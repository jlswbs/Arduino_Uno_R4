// Poly karplus strong with distortion and reverb //

#include "FspTimer.h"

FspTimer audio_timer;

#define SAMPLERATE  22050
#define MAXTEMPO    14    // 350 BPM 16th note
#define MINTEMPO    111   // 45 BPM 16th note
#define SIZE        256   // audio buffer
#define POLY        2     // Two voice poly

float randomf(float minf, float maxf) { return minf + (rand()%(1UL << 31))*(maxf - minf) / (1UL << 31); }
float dist_filter(float input, float peak){ input = input * 4.0f / 3.0f; return clip(input, peak); }
float clip(float cval, float peak){ return (cval > 0) ? ((cval < peak) ? cval : peak) : ((cval > (-1.0f * peak)) ? cval : (-1.0f * peak)); }

  float r[POLY];
  float x[POLY];
  float b[POLY];
  float out[POLY];
  float last[POLY];
  float curr[POLY];
  float delaymem[POLY][SIZE];
  uint16_t locat[POLY];
  uint16_t bound[POLY];
  float accum[POLY];
  float lowpass[POLY]; // 0 ... 1.0
  float nx[POLY];

const int FIXED_BITS = 14;
const int FIXED_SCALE = (1 << FIXED_BITS);
const int REVERB_SIZE = 0x1200;
const int REVERB_LENGTH = (int)(REVERB_SIZE * 0.9f);
const int REVERB_DECAY = (int)(FIXED_SCALE * 0.7f);

int16_t reverbAddr;
int reverbBuffer[REVERB_SIZE];


void timer_callback(timer_callback_args_t __attribute((unused)) *p_args) {

  for (int j = 0; j < POLY; j++) {
  
    delaymem[j][locat[j]++] = out[j];
    if (locat[j] >= bound[j]) locat[j] = 0;
    curr[j] = delaymem[j][locat[j]];
    out[j] = 0.5f * accum[j];
    accum[j] = accum[j] - (b[j] * (accum[j] - (last[j] + curr[j]))); 
    last[j] = curr[j];

  }

  float outs = 0.0f;
  for (int l = 0; l < POLY; l++) outs += out[l];
  outs = outs / POLY;

  int16_t sample = 32768 * dist_filter(outs, 0.1f);

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

  x[0] = 0.1f;
  x[1] = 0.2f;
  r[0] = 3.7f;
  r[1] = 3.8f;

}

void loop() {

  for (int k = 0; k < POLY; k++) {

    nx[k] = x[k];
    x[k] = r[k] * nx[k] * (1.0f - nx[k]);

  }

  for (int i = 0; i < SIZE; i++) delaymem[0][i] = randomf(-1.0f, 1.0f);
  bound[0] = map(SIZE * x[0], 0, SIZE, 16, SIZE);
  b[0] = fabs(x[1]);

  delay(4 * MINTEMPO);

  for (int i = 0; i < SIZE; i++) delaymem[1][i] = randomf(-1.0f, 1.0f);
  bound[1] = map(SIZE * x[1], 0, SIZE, 16, SIZE);
  b[1] = fabs(x[0]);

  delay(2 * MINTEMPO);

}