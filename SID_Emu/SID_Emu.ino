// SID chip emulator with reverb - original code https://github.com/cybernesto/sid-arduino-lib

#include "FspTimer.h"
#include "SID.h"

FspTimer audio_timer;
SID mySid;

#define SAMPLERATE    22050
#define CHANNEL1      0
#define CHANNEL2      7
#define CHANNEL3      14
#define SETTRIANGLE_1	4, 0x11, 5, 0xBB, 6, 0xAA,
#define C4_1	        1, 0x11, 0, 0x25,

const int FIXED_BITS = 14;
const int FIXED_SCALE = (1 << FIXED_BITS);
const int REVERB_SIZE = 0x1200;
const int REVERB_LENGTH = (int)(REVERB_SIZE * 0.9f);
const int REVERB_DECAY = (int)(FIXED_SCALE * 0.7f);

int16_t reverbAddr;
int reverbBuffer[REVERB_SIZE];
int16_t sample;

void timer_callback(timer_callback_args_t __attribute((unused)) *p_args) {

  static uint8_t mscounter = 0;

  sample = mySid.waveforms();

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

void setwaveform_triangle(uint8_t channel) {

  uint8_t dataset[]={ SETTRIANGLE_1 C4_1 0xFF };
  uint8_t n=0; 
  
  while(dataset[n] != 0xFF) {
    mySid.set_register(channel+dataset[n], dataset[n+1]); 
    n+=2;
  }

}

void set_frequency(uint16_t pitch,uint8_t channel) {

    mySid.set_register(channel, pitch&0xFF); // low register adress
    mySid.set_register(channel+1, pitch>>8); // high register adress
    
}

uint8_t regproc() {

	static uint16_t start = 0x0AA;

	uint16_t temp;
	uint8_t n;
		
	for(n=1; n<8; n++) {

		temp = start;
		start = start << 1;
	
		temp ^= start;
		if ((temp & 0x4000) == 0x4000) start |= 1;

	}
	
	return (start);

}

void loop() {

  setwaveform_triangle(CHANNEL1);
    
  while(1) {

    uint16_t n = regproc() * 4;
    set_frequency(n * 17, CHANNEL1);

    delay(100);

  }

}