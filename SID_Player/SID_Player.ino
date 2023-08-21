// SID chip player with reverb //

#include "FspTimer.h"
#include "SID.h"
#include "Visitors.h"
//#include "Commando.h"

FspTimer audio_timer;
SID mySid;

#define SAMPLERATE    22050

const int FIXED_BITS = 14;
const int FIXED_SCALE = (1 << FIXED_BITS);
const int REVERB_SIZE = 0x1200;
const int REVERB_LENGTH = (int)(REVERB_SIZE * 0.9f);
const int REVERB_DECAY = (int)(FIXED_SCALE * 0.7f);

int16_t reverbAddr;
int reverbBuffer[REVERB_SIZE];


void timer_callback(timer_callback_args_t __attribute((unused)) *p_args) {

  static uint8_t mscounter = 0;

  int16_t sample = mySid.waveforms();

  int reverb = ((int)reverbBuffer[reverbAddr] * REVERB_DECAY) >> FIXED_BITS;
  reverb += sample;
  reverbBuffer[reverbAddr] = reverb;
  reverbAddr++;
  if (reverbAddr > REVERB_LENGTH) reverbAddr = 0;
  int16_t output = sample + (reverbBuffer[reverbAddr]>>3);

  analogWrite(DAC, 2048 + output);
	
	if(mscounter++ >= MSCOUNT) {
	  mySid.envelopes();
		mscounter = 0;
	}

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
  
  mySid.begin();

}

void loop() {

  for(uint16_t pointer = 0; pointer <= sidLength; pointer++){

    for(uint8_t sidReg = 0; sidReg <= 24; sidReg++) mySid.set_register(sidReg, (pgm_read_byte(&sidData[(pointer+sidReg)])));
    
    delay(8);

    pointer = pointer + 24;
 
  }

}