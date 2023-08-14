// Hack-Match synth with reverb - original code https://github.com/Quillington/Hack-Match-Synthesizer //

#include "FspTimer.h"

FspTimer audio_timer;

#define SAMPLERATE    22050
#define MAX_AMP       32767

const int FIXED_BITS = 14;
const int FIXED_SCALE = (1 << FIXED_BITS);
const int REVERB_SIZE = 0x1200;
const int REVERB_LENGTH = (int)(REVERB_SIZE * 0.9f);
const int REVERB_DECAY = (int)(FIXED_SCALE * 0.8f);

int16_t reverbAddr;
int reverbBuffer[REVERB_SIZE];

  signed char sineTable[256], fm1, fm2;
  int indexFM = 0;
  const uint8_t SAMP_SIZE = 100;
  volatile bool readyToPlay = false;
  volatile int playCount = 0;
  int16_t sample;

typedef struct samp {
  uint8_t index;
  uint32_t duration;
  uint8_t volume;
  uint8_t stringLength;
  bool isLast;
  bool isLoop;
  int indexToLoop;
}Samp;
   
typedef struct synthSamp {
  int index;
  uint32_t duration;
  int volume;
  int fc;
  int fm;
  int decayFC;
  int riseFC;
  int depthFM;
  int decayFM;
  bool isLast;
  bool isLoop;
  int indexToLoop;
}SynthSamp;

Samp sampArray[] = {
  {0, 8000, 3, 97, 0, 0, 0},
  {1, 8000, 3, 97, 0, 0, 0},
  {2, 2000, 3, 97, 0, 0, 0},
  {3, 6000, 3, 97, 0, 0, 0},
  {4, 8000, 3, 97, 0, 1, 0},

  {5, 8000, 1, 41, 0, 0, 5},
  {6, 2000, 1, 36, 0, 0, 6},
  {7, 14000, 1, 32, 0, 0, 5},
  {8, 2000, 1, 36, 0, 0, 5},
  {9, 6000, 1, 41, 0, 0, 5},
  {10, 2000, 1, 32, 0, 0, 5},
  {11, 30000, 1, 36, 0, 0, 5},

  {12, 8000, 1, 41, 0, 0, 5},
  {13, 2000, 1, 36, 0, 0, 6},
  {14, 14000, 1, 32, 0, 0, 5},
  {15, 2000, 1, 27, 0, 0, 5},
  {16, 6000, 1, 32, 0, 0, 5},
  {17, 2000, 1, 36, 0, 0, 5},
  {18, 30000, 1, 32, 0, 1, -1},
};

SynthSamp sampArraySynth[] = {
  {0, 8000, 0, (int)(8.0 * 164.81), (int)(8.0 * (164.81)), 5, 4, 8, 4, 0, 0, 0},
  {1, 2000, 0, (int)(8.0 * 146.83), (int)(8.0 * (146.83)), 5, 4, 8, 4, 0, 0, 0},
  {2, 14000, 0, (int)(8.0 * 164.81), (int)(8.0 * (164.81)), 5, 4, 8, 4, 0, 0, 0},
  {3, 6000, 0, (int)(8.0 * 174.61), (int)(8.0 * (174.61)), 5, 4, 8, 4, 0, 0, 0},
  {4, 2000, 0, (int)(8.0 * 174.61), (int)(8.0 * (174.61)), 5, 4, 8, 4, 0, 0, 0},
  {5, 6000, 0, (int)(8.0 * 164.81), (int)(8.0 * (164.81)), 5, 4, 8, 4, 0, 0, 0},
  {6, 2000, 0, (int)(8.0 * 146.83), (int)(8.0 * (146.83)), 5, 4, 8, 4, 0, 0, 0},
  {7, 24000, 0, (int)(8.0 * 164.81), (int)(8.0 * (164.81)), 5, 4, 8, 4, 0, 0, 0},

  {8, 8000, 0, (int)(8.0 * 164.81), (int)(8.0 * (164.81)), 5, 4, 8, 4, 0, 0, 0},
  {9, 2000, 0, (int)(8.0 * 146.83), (int)(8.0 * (146.83)), 5, 4, 8, 4, 0, 0, 0},
  {10, 14000, 0, (int)(8.0 * 164.81), (int)(8.0 * (164.81)), 5, 4, 8, 4, 0, 0, 0},
  {11, 6000, 0, (int)(8.0 * 174.61), (int)(8.0 * (174.61)), 5, 4, 8, 4, 0, 0, 0},
  {12, 2000, 0, (int)(8.0 * 174.61), (int)(8.0 * (174.61)), 5, 4, 8, 4, 0, 0, 0},
  {13, 6000, 0, (int)(8.0 * 164.81), (int)(8.0 * (164.81)), 5, 4, 8, 4, 0, 0, 0},
  {14, 2000, 0, (int)(8.0 * 146.83), (int)(8.0 * (146.83)), 5, 4, 8, 4, 0, 0, 0},
  {15, 24000, 0, (int)(8.0 * 164.81), (int)(8.0 * (164.81)), 5, 4, 8, 4, 0, 1, -1},
};

typedef struct stringChannel {
  bool isOn;
  uint16_t index;
  int channelArray[SAMP_SIZE];
  Samp sample;
}StringChannel;

typedef struct SynthChannel {
  bool isOn;
  uint16_t index;
  uint16_t mainAmpFall;
  uint16_t mainRisePhase;
  uint16_t mainAmpRise;
  uint16_t FMamp;
  uint16_t FMAcc;
  uint16_t mainAcc;
  SynthSamp sample;
}SynthChannel;

StringChannel channelArray[] = {
  {1, 0, {0}, sampArray[0]},
  {1, 0, {0}, sampArray[5]},
};

SynthChannel channelArraySynth[] = {
  {0, 0, MAX_AMP, MAX_AMP, 0, MAX_AMP, 0, 0, sampArraySynth[0]}
};

int FMSynth(SynthChannel *chan) {

  SynthSamp *sample = &chan->sample;
  if ((chan->index & 0x0ff) == 0) {
    chan->mainAmpFall = chan->mainAmpFall - (chan->mainAmpFall >> sample->decayFC) ;
    chan->mainRisePhase = chan->mainRisePhase - (chan->mainRisePhase >> sample->riseFC);
    chan->FMamp = chan->FMamp - (chan->FMamp >> sample->decayFM) ;
  }

  chan->mainAmpRise =  MAX_AMP - chan->mainRisePhase;
  uint32_t FCamp = (chan->mainAmpRise >> 8) * (chan->mainAmpFall >> 8) ;
  chan->FMAcc = chan->FMAcc + sample->fm ;
  uint8_t high_fm = (char)(chan->FMAcc >> 8) ;
  int8_t fm1 = sineTable[high_fm] ;
  chan->mainAcc = chan->mainAcc + (sample->fc + (fm1*(chan->FMamp >> sample->depthFM)));
  uint8_t high_main = (char)(chan->mainAcc >> 8) ;
  int num = 0;
  if ((int)sineTable[high_main] > 0) {
    num = 1;
  }
  if ((int)sineTable[high_main] < 0) {
    num = -1;
  }
  signed int output = 128 + ((FCamp >> 8) * (num)) ;
  
  chan->index = chan->index + 1;
  if (chan->index == sample->duration) {
    chan->index = 0;
    chan->mainAmpFall = MAX_AMP;
    chan->mainRisePhase = MAX_AMP;
    chan->mainAmpRise = 0;
    chan->FMamp = MAX_AMP;
    chan->FMAcc = 0;
    chan->mainAcc = 0;
    if (sample->isLoop) {
      if (sample->indexToLoop == -1) {
        chan->isOn = false;
        channelArray[1].isOn = true;
        channelArray[1].sample = sampArray[5];
      }
      else{
        chan->sample = sampArraySynth[sample->indexToLoop];
      }
    }
    else {
      if (!sample->isLast) {
        chan->sample = sampArraySynth[sample->index + 1];
      }
      else {
        chan->isOn = false;
      }
    }
  }
  return output;

}

int KSAlg(StringChannel *chan) {

  int output;
  Samp *sample = &chan->sample;
  int firstIndex = chan->index % sample->stringLength;
  int secondIndex;
  if (chan->index == sample->stringLength - 1) {
    secondIndex = 0;
  }
  else {
    secondIndex = firstIndex + 1;
  }
  if (chan->index >= sample->stringLength) {
    output = ((chan->channelArray[firstIndex] + chan->channelArray[secondIndex]) >> 1);
    chan->channelArray[firstIndex] = output;
  }
  else {
    output = (rand()%65536) >> sample->volume;
    chan->channelArray[firstIndex] = output;
  }
  chan->index = chan->index + 1;
  if (chan->index == sample->duration) {
    chan->index = 0;
    if (sample->isLoop) {
      if (sample->indexToLoop == -1) {
        chan->isOn = false;
        channelArraySynth[0].isOn = true;
        channelArraySynth[0].sample = sampArraySynth[0];
      }
      else {
        chan->sample = sampArray[sample->indexToLoop];
      }
    }
    else {
      if (!sample->isLast) {
        chan->sample = sampArray[sample->index + 1];
      }
      else {
        chan->isOn = false;
      }
    }
  }
  return output;

}

void KSAlgMaster() {

  long output;
  for (int i = 0; i < 2; i++){
    if (channelArray[i].isOn) {
      output += (KSAlg(&channelArray[i]) + (1024 >> channelArray[i].sample.volume));
    }
  }
  if (channelArraySynth[0].isOn) {
    output += (FMSynth(&channelArraySynth[0]) * 2);
  }
  while (!readyToPlay) {}
  sample = output>>1;
  readyToPlay = false;

}

void timer_callback(timer_callback_args_t __attribute((unused)) *p_args) {

  readyToPlay = true;
  playCount++;

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

  for (int i = 0; i < 256; i++) sineTable[i] = (char)(128.0f * sinf(TWO_PI * ((float)i) / 256.0f));

}

void loop() {

  KSAlgMaster();

}