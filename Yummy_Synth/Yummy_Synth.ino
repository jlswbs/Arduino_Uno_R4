// Yummy DSP synth with reverb example - original code https://github.com/garygru/yummyDSP //

#include "FspTimer.h"
#include "YummyDSP.h"

FspTimer audio_timer;

#define SAMPLERATE    22050
#define MAX_AMP       32767

const int FIXED_BITS = 14;
const int FIXED_SCALE = (1 << FIXED_BITS);
const int REVERB_SIZE = 0x1200;
const int REVERB_LENGTH = (int)(REVERB_SIZE * 0.9f);
const int REVERB_DECAY = (int)(FIXED_SCALE * 0.6f);

int16_t reverbAddr;
int reverbBuffer[REVERB_SIZE];

YummyDSP dsp;
WaveSynth synth;
FilterNode lp;
WaveShaperNode sat;

void timer_callback(timer_callback_args_t __attribute((unused)) *p_args) {

  float sampleMono = synth.getSample();
  sampleMono = dsp.process(sampleMono, 1);
  int16_t sample =  16384 * sampleMono;

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

  dsp.begin(SAMPLERATE);
  synth.begin(SAMPLERATE, 1024);
  synth.noteOff();
  synth.setWaveform(SAW);
  synth.setGlide(10);
  synth.setAttack(10);
  synth.setSustain(0.3);  

  lp.begin(SAMPLERATE, 1);
  lp.setupFilter(FilterNode::LPF, 1500, 1.7);

  sat.begin(SAMPLERATE, 1);
  sat.setDrive(0.3f);

  dsp.addNode(&lp);
  dsp.addNode(&sat);

}

void loop() {

  synth.note(36 + rand() % 36);
  lp.updateFilter(440 + rand() % 880);

  delay(1);

  synth.noteOff();

  delay(119); 

}