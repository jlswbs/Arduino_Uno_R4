// Lo-Fi sample drums mangler with delay //

#include "FspTimer.h"
#include "data-bd.h"
#include "data-sd.h"
#include "data-oh.h"
#include "data-cp.h"

FspTimer audio_timer;

#define SAMPLERATE    44100

const int FIXED_BITS = 14;
const int FIXED_SCALE = (1 << FIXED_BITS);
const int REVERB_SIZE = 0x1200;
const int REVERB_LENGTH = (int)(REVERB_SIZE * 0.9f);
const int REVERB_DECAY = (int)(FIXED_SCALE * 0.8f);

int16_t reverbAddr;
int reverbBuffer[REVERB_SIZE];

class SamplePlayer
{ 
public:
  SamplePlayer(const int *data, size_t len)
    : sample(data)
    , size(len)
    , position(0)
    , pitch(127)
    , play(0)
  { }
  const int* sample;
  unsigned int size;
  unsigned int position;
  uint8_t pitch;
  uint8_t play;      
};

SamplePlayer samples[] =
{
  {sample_bd, sizeof(sample_bd)},
  {sample_sd, sizeof(sample_sd)},
  {sample_oh, sizeof(sample_oh)},
  {sample_cp, sizeof(sample_cp)},
};

const uint8_t NumSamples = sizeof(samples)/sizeof(samples[0]);
unsigned int lenght[NumSamples];


void timer_callback(timer_callback_args_t __attribute((unused)) *p_args) {

  int16_t sample = 0;

  for (int8_t i = NumSamples - 1; i >= 0; i--) {
        
    SamplePlayer* s = samples + i;
    if (s->play) {

      unsigned long k = s->position;
      k *= s->pitch;
      k >>= 7;
      lenght[i] = s->size;
      if (k >= s->size) {
        s->play = 0;
        continue;
      }
      int16_t v = (char)pgm_read_byte_near(s->sample + (unsigned int)k);
      v *= s->play;
      v >>= 8;
      sample += v;
      s->position++;
    }
  }

  sample = 128 * sample;
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

}

void loop() {

  uint8_t num = rand()%NumSamples;
  auto &sample = samples[num]; // 0 - NumSamples-1
  sample.position = rand()%(lenght[num]-256); // 0 - lenght-256
  sample.pitch = 12 + rand()%128; // 0 - 255
  sample.play  = 24 + rand()%232; // 0 - 255

  delay(1 + rand()%500);

}