// Two operator FM synth with reverb //

#include "FspTimer.h"

FspTimer audio_timer;
#include "fmtg.h"
#include "notedefs.h"

#define BPM (125)
#define SAMPLERATE    22050

const int FIXED_BITS = 14;
const int FIXED_SCALE = (1 << FIXED_BITS);
const int REVERB_SIZE = 0x1200;
const int REVERB_LENGTH = (int)(REVERB_SIZE * 0.9f);
const int REVERB_DECAY = (int)(FIXED_SCALE * 0.7f);

int16_t reverbAddr;
int reverbBuffer[REVERB_SIZE];
int16_t samples;

int8_t seq_length  = 15;

int8_t notes[] = {M_C5, M_C5, M_G5, M_G5, 
                  M_A5, M_A5, M_G5, 
                  M_F5, M_F5, M_E5, M_E5, 
                  M_D5, M_D5, M_C5, 
                  M_REST};
                  
int8_t beats[] = {L_4,  L_4,  L_4,  L_4,
                  L_4,  L_4,  L_2, 
                  L_4,  L_4,  L_4,  L_4,
                  L_4,  L_4,  L_2,  
                  L_1};

int16_t unit_dur = (((4883/BPM)+1)>>1);
volatile uint8_t fsflag = 0;
FMop op[2];

#define NUM_INST (7)

const PROGMEM int8_t v_data[NUM_INST][6] = {
// +---- OP0 ----+  OP1 oct
// FB MULT  TL  DR  DR  shift
  { 5,  1,  32,  1,  2,  0}, // Acoustic Piano
  { 7,  5,  44,  5,  2,  0}, // Electric Piano
  { 5,  9,  32,  2,  2,  0}, // Tubular Bells
  { 0,  8,  34,  8,  7,  0}, // Marimba
  { 7,  3,  32,  1,  2,  0}, // Jazz Guitar
  { 4,  1,  16,  1,  2, -2}, // Finger Bass
  { 4,  1,   8,  3,  2, -2}, // Slap Bass
};

void program_change(uint8_t pno) 
{
  op[0].FB   = READ_BYTE(v_data[pno][0]);
  op[0].MULT = READ_BYTE(v_data[pno][1]);
  op[0].TL   = READ_BYTE(v_data[pno][2]);
  op[0].DR   = READ_BYTE(v_data[pno][3]);
  op[1].DR   = READ_BYTE(v_data[pno][4]);
}


void timer_callback(timer_callback_args_t __attribute((unused)) *p_args) {

  fsflag = 1;

  int reverb = ((int)reverbBuffer[reverbAddr] * REVERB_DECAY) >> FIXED_BITS;
  reverb += samples;
  reverbBuffer[reverbAddr] = reverb;
  reverbAddr++;
  if (reverbAddr > REVERB_LENGTH) reverbAddr = 0;

  int16_t output = samples + (reverbBuffer[reverbAddr]>>2); 
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

  static int16_t ww  = 0;
  static uint8_t seq = 0;
  static uint8_t pno = 0;
  static int8_t  nofs = 0;
  static int8_t  tick1ms = SEQTICK1ms-1;
  static int16_t dur_cnt = 0;
 
  if (fsflag) {
    samples = ww>>2;
    fsflag = 0;
    if (0 > (--tick1ms)) {
      tick1ms = SEQTICK1ms-1;
      op[0].eg_update();
      op[1].eg_update();
      if (0 > (--dur_cnt)){
        if (M_REST < notes[seq]) {
          if (0 == seq) {
            program_change(pno);
            nofs = 12 * READ_BYTE(v_data[pno][5]);
            if (NUM_INST <= (++pno)) pno = 0;
          } 
          for (uint8_t i = 0; i < 2; i++) {
            op[i].gate_on(notes[seq]+nofs, 127);
          }
        }
        dur_cnt = beats[seq] * unit_dur - 1;
        if (seq_length <= (++seq)) seq = 0;
      }
    }
    ww = op[0].calc(0);
    ww = op[1].calc((int32_t)ww << MOD_SHIFT);
  }

}