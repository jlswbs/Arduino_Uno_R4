// Pt3 chiptune player //

#include "FspTimer.h"
#include "ay_emu.h"
#include "acidgoa.h"

FspTimer audio_timer;

#define AY_CLOCK      1750000
#define SAMPLERATE    22050
#define FRAME_RATE    50

  int interruptCnt;

struct PT3_Channel_Parameters
{
  unsigned short Address_In_Pattern, OrnamentPointer, SamplePointer, Ton;
  unsigned char Loop_Ornament_Position, Ornament_Length, Position_In_Ornament, Loop_Sample_Position, Sample_Length, Position_In_Sample, Volume, Number_Of_Notes_To_Skip, Note, Slide_To_Note, Amplitude;
  bool Envelope_Enabled, Enabled, SimpleGliss;
  short Current_Amplitude_Sliding, Current_Noise_Sliding, Current_Envelope_Sliding, Ton_Slide_Count, Current_OnOff, OnOff_Delay, OffOn_Delay, Ton_Slide_Delay, Current_Ton_Sliding, Ton_Accumulator, Ton_Slide_Step, Ton_Delta;
  signed char Note_Skip_Counter;
};

struct PT3_Parameters
{
  unsigned char Env_Base_lo;
  unsigned char Env_Base_hi;
  short Cur_Env_Slide, Env_Slide_Add;
  signed char Cur_Env_Delay, Env_Delay;
  unsigned char Noise_Base, Delay, AddToNoise, DelayCounter, CurrentPosition;
  int Version;
};

struct PT3_SongInfo
{
  PT3_Parameters PT3;
  PT3_Channel_Parameters PT3_A, PT3_B, PT3_C;
};

struct AYSongInfo
{
  unsigned char* module;
  unsigned char* module1;
  int module_len;
  PT3_SongInfo data;
  PT3_SongInfo data1;
  bool is_ts;

  AYChipStruct chip0;
};

struct AYSongInfo AYInfo;

void ay_resetay(AYSongInfo* info, int chipnum)
{
  ay_init(&info->chip0);
}

void ay_writeay(AYSongInfo* info, int reg, int val, int chipnum)
{
  ay_out(&info->chip0, reg, val);
}

#include "PT3Play.h"

void timer_callback(timer_callback_args_t __attribute((unused)) *p_args) {

  if (interruptCnt++ >= (SAMPLERATE / FRAME_RATE)) {

    PT3_Play_Chip(AYInfo, 0);
    interruptCnt = 0;
  
  }

  ay_tick(&AYInfo.chip0, (AY_CLOCK / SAMPLERATE / 10));
  analogWrite(DAC, (AYInfo.chip0.out[0] + AYInfo.chip0.out[2] + AYInfo.chip0.out[1]) >> 5);

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

  memset(&AYInfo, 0, sizeof(AYInfo));
  ay_init(&AYInfo.chip0);
  AYInfo.module = music_data;
  AYInfo.module_len = music_data_size;
  PT3_Init(AYInfo);

}

void loop() {

}