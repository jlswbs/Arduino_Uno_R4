// AY-3-8910 chip emulator with reverb //

#include "FspTimer.h"
#include "ay_emu.h"

FspTimer audio_timer;

#define AY_CLOCK      1773400
#define SAMPLERATE    22050
#define FRAME_RATE    5

struct AYSongInfo{
  AYChipStruct chip0;
};

struct AYSongInfo AYInfo;

int interruptCnt;

const int FIXED_BITS = 14;
const int FIXED_SCALE = (1 << FIXED_BITS);
const int REVERB_SIZE = 0x1200;
const int REVERB_LENGTH = (int)(REVERB_SIZE * 0.9f);
const int REVERB_DECAY = (int)(FIXED_SCALE * 0.7f);

int16_t reverbAddr;
int reverbBuffer[REVERB_SIZE];


void timer_callback(timer_callback_args_t __attribute((unused)) *p_args) {

  int16_t sample = (AYInfo.chip0.out[0] + AYInfo.chip0.out[1] + AYInfo.chip0.out[2]) >> 1;

  int reverb = ((int)reverbBuffer[reverbAddr] * REVERB_DECAY) >> FIXED_BITS;
  reverb += sample;
  reverbBuffer[reverbAddr] = reverb;
  reverbAddr++;
  if (reverbAddr > REVERB_LENGTH) reverbAddr = 0;
  int16_t output = sample + (reverbBuffer[reverbAddr]>>3);

  analogWrite(DAC, output >> 4);

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

/* Registers */
enum{
  REG_FREQ_A_LO = 0,
  REG_FREQ_A_HI,
  REG_FREQ_B_LO,
  REG_FREQ_B_HI,
  REG_FREQ_C_LO,
  REG_FREQ_C_HI,
  REG_FREQ_NOISE,
  REG_IO_MIXER,
  REG_LVL_A,
  REG_LVL_B,
  REG_LVL_C,
  REG_FREQ_ENV_LO,
  REG_FREQ_ENV_HI,
  REG_ENV_SHAPE,
  REG_IOA,
  REG_IOB
};

uint16_t note[] = {//MIDI note number
  15289, 14431, 13621, 12856, 12135, 11454, 10811, 10204,//0-o7
  9631, 9091, 8581, 8099, 7645, 7215, 6810, 6428,//8-15
  6067, 5727, 5405, 5102, 4816, 4545, 4290, 4050,//16-23
  3822, 3608, 3405, 3214, 3034, 2863, 2703, 2551,//24-31
  2408, 2273, 2145, 2025, 1911, 1804, 1703, 1607,//32-39
  1517, 1432, 1351, 1276, 1204, 1136, 1073, 1012,//40-47
  956, 902, 851, 804, 758, 716, 676, 638,//48-55
  602, 568, 536, 506, 478, 451, 426, 402,//56-63
  379, 358, 338, 319, 301, 284, 268, 253,//64-71
  239, 225, 213, 201, 190, 179, 169, 159,//72-79
  150, 142, 134, 127, 119, 113, 106, 100,//80-87
  95, 89, 84, 80, 75, 71, 67, 63,//88-95
  60, 56, 53, 50, 47, 45, 42, 40,//96-103
  38, 36, 34, 32, 30, 28, 27, 25,//104-111
  24, 22, 21, 20, 19, 18, 17, 16,//112-119
  15, 14, 13, 13, 12, 11, 11, 10,//120-127
  0//off
};

void note_chan_A(uint8_t i){
  ay_out(&AYInfo.chip0,REG_FREQ_A_LO, note[i]&0xff);
  ay_out(&AYInfo.chip0,REG_FREQ_A_HI, (note[i] >> 8)&0x0f);    
}

void note_chan_B(uint8_t i){
  ay_out(&AYInfo.chip0,REG_FREQ_B_LO, note[i]&0xff);
  ay_out(&AYInfo.chip0,REG_FREQ_B_HI, (note[i] >> 8)&0x0f);
}

void note_chan_C(uint8_t i){
  ay_out(&AYInfo.chip0,REG_FREQ_C_LO, note[i]&0xff);
  ay_out(&AYInfo.chip0,REG_FREQ_C_HI, (note[i] >> 8)&0x0f);
}

void envelope(uint16_t freq){       
  ay_out(&AYInfo.chip0,REG_FREQ_ENV_LO, freq & 0xff);
  ay_out(&AYInfo.chip0,REG_FREQ_ENV_HI, (freq >> 8)& 0xff);  
}

void set_env( bool hold, bool alternate, bool attack, bool cont){
  ay_out(&AYInfo.chip0,REG_ENV_SHAPE, (hold == true ? 0 : 1)|(alternate == true? 0 : 2)|(attack == true ? 0 : 4)|(cont == true ? 0 : 8));
}

void amp_chan_A(uint8_t ampl, bool envset){
  ay_out(&AYInfo.chip0,REG_LVL_A, (ampl & 0xf)|(envset != true ? 0 : 0b00010000));
}

void amp_chan_B(uint8_t ampl, bool envset){
  ay_out(&AYInfo.chip0,REG_LVL_B, (ampl & 0xf)|(envset != true ? 0 : 0b00010000));
}

void amp_chan_C(uint8_t ampl, bool envset){
  ay_out(&AYInfo.chip0,REG_LVL_C, (ampl & 0xf)|(envset != true ? 0 : 0b00010000));
}

void noise(uint8_t freq){
  ay_out(&AYInfo.chip0,REG_FREQ_NOISE, freq&0x1f);
}

void set_mix(bool tone_A, bool tone_B, bool tone_C, bool noise_A, bool noise_B, bool noise_C){
  ay_out(&AYInfo.chip0,REG_IO_MIXER, 0b11000000|(noise_C == true ? 0 : 0b00100000)|(noise_B == true? 0 : 0b00010000)|(noise_A == true ? 0 : 0b00001000)|(tone_C == true ? 0 : 0b00000100)|(tone_B == true ? 0 : 0b00000010)|(tone_A == true ? 0 : 0b00000001));
}


void setup() {

  analogWriteResolution(12);
  beginTimer(SAMPLERATE);
  
  ay_init(&AYInfo.chip0);

}

void loop() {

  if (interruptCnt++ >= (SAMPLERATE / FRAME_RATE)) {

    set_mix(1, 1, 1, 0, 0, 1); // toneA, toneB, toneC, noiseA, noiseB, noiseC 0-1

    amp_chan_A(8, 0); // 0-15
    amp_chan_B(8, 0); // 0-15
    amp_chan_C(9, 1); // 0-15

    set_env(1, 0, 1, 0); // hold, alter, attack, cont 0-1
    envelope(rand() % 500); // 0-65535

    noise(rand() % 32); // 0-31
  
    note_chan_A(48 + rand() % 42); // note 0-127
    note_chan_B(48 + rand() % 48); // note 0-127
    note_chan_C(24 + rand() % 64); // note 0-127          
        
    interruptCnt = 0;
  
  }      

  ay_tick(&AYInfo.chip0, (AY_CLOCK / SAMPLERATE / 10));

}