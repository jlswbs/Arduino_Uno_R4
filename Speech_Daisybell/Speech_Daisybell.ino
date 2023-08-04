// Lo-Fi speech engine example //

#include "FspTimer.h"

FspTimer audio_timer;

#define SAMPLERATE    22050

int frameTime = 8; // ms
int formantScale = 64;

const int FIXED_BITS = 14;
const int FIXED_SCALE = (1 << FIXED_BITS);
const int REVERB_SIZE = 0x1200;
const int REVERB_LENGTH = (int)(REVERB_SIZE * 0.9f);
const int REVERB_DECAY = (int)(FIXED_SCALE * 0.7f);

int16_t reverbAddr;
int reverbBuffer[REVERB_SIZE];

uint16_t pitchPhase, form1Phase,form2Phase,form3Phase;
uint16_t pitchPhaseInc,form1PhaseInc,form2PhaseInc,form3PhaseInc;
uint8_t form1Amp,form2Amp,form3Amp;

int8_t sinCalc[256] PROGMEM = {
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,2,2,3,3,4,5,6,7,8,10,12,14,17,20,24,
  0,4,4,5,6,7,9,11,13,15,18,22,26,31,37,45,
  0,5,6,7,8,10,12,14,17,20,24,28,34,41,49,58,
  0,5,6,7,9,10,12,15,18,21,26,31,37,44,53,63,
  0,5,6,7,8,10,12,14,17,20,24,28,34,41,49,58,
  0,4,4,5,6,7,9,11,13,15,18,22,26,31,37,45,
  0,2,2,3,3,4,5,6,7,8,10,12,14,17,20,24,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,-2,-2,-3,-3,-4,-5,-6,-7,-8,-10,-12,-14,-17,-20,-24,
  0,-4,-4,-5,-6,-7,-9,-11,-13,-15,-18,-22,-26,-31,-37,-45,
  0,-5,-6,-7,-8,-10,-12,-14,-17,-20,-24,-28,-34,-41,-49,-58,
  0,-5,-6,-7,-9,-10,-12,-15,-18,-21,-26,-31,-37,-44,-53,-63,
  0,-5,-6,-7,-8,-10,-12,-14,-17,-20,-24,-28,-34,-41,-49,-58,
  0,-4,-4,-5,-6,-7,-9,-11,-13,-15,-18,-22,-26,-31,-37,-45,
  0,-2,-2,-3,-3,-4,-5,-6,-7,-8,-10,-12,-14,-17,-20,-24
};

int8_t sqrCalc[256] PROGMEM ={
  0,1,2,2,2,3,3,4,5,5,6,8,9,11,13,16,
  0,1,2,2,2,3,3,4,5,5,6,8,9,11,13,16,
  0,1,2,2,2,3,3,4,5,5,6,8,9,11,13,16,
  0,1,2,2,2,3,3,4,5,5,6,8,9,11,13,16,
  0,1,2,2,2,3,3,4,5,5,6,8,9,11,13,16,
  0,1,2,2,2,3,3,4,5,5,6,8,9,11,13,16,
  0,1,2,2,2,3,3,4,5,5,6,8,9,11,13,16,
  0,1,2,2,2,3,3,4,5,5,6,8,9,11,13,16,
  0,-1,-2,-2,-2,-3,-3,-4,-5,-5,-6,-8,-9,-11,-13,-16,
  0,-1,-2,-2,-2,-3,-3,-4,-5,-5,-6,-8,-9,-11,-13,-16,
  0,-1,-2,-2,-2,-3,-3,-4,-5,-5,-6,-8,-9,-11,-13,-16,
  0,-1,-2,-2,-2,-3,-3,-4,-5,-5,-6,-8,-9,-11,-13,-16,
  0,-1,-2,-2,-2,-3,-3,-4,-5,-5,-6,-8,-9,-11,-13,-16,
  0,-1,-2,-2,-2,-3,-3,-4,-5,-5,-6,-8,-9,-11,-13,-16,
  0,-1,-2,-2,-2,-3,-3,-4,-5,-5,-6,-8,-9,-11,-13,-16,
  0,-1,-2,-2,-2,-3,-3,-4,-5,-5,-6,-8,-9,-11,-13,-16
};

#define FORMANT_SZ 7

enum {
  SP,DOT,QM,COM,HYP,IY,IH,EH,AE,AA,
  AH,AO,UH,AX,IX,ER,UX,OH,RX,LX,
  WX,YX,WH,R,L,W,Y,M,N,NX,
  DX,Q,S,SH,FF,TH,H,X,Z,ZH,
  V,DH,CHa,CHb,Ja,Jb,Jc,Jd,EY,AY,
  OY,AW,OW,UW,Ba,Bb,Bc,Da,Db,Dc,
  Ga,Gb,Gc,GXa,GXb,GXc,Pa,Pb,Pc,Ta,
  Tb,Tc,Ka,Kb,Kc,KXa,KXb,KXc
};

uint8_t formantTable[] PROGMEM = {
   0x0, 0x0, 0x0,0x0,0x0,0x0,0x0,/*00 space*/ 0x13,0x43,0x5b,0x0,0x0,0x0,0x0,/*01 .*/
  0x13,0x43,0x5b,0x0,0x0,0x0,0x0,/*02 ?*/     0x13,0x43,0x5b,0x0,0x0,0x0,0x0,/*03 ,*/
  0x13,0x43,0x5b,0x0,0x0,0x0,0x0,/*04 -*/      0xa,0x54,0x6e,0xd,0xa,0x8,0x0,/*05 IY*/
   0xe,0x49,0x5d,0xd,0x8,0x7,0x0,/*06 IH*/    0x13,0x43,0x5b,0xe,0xd,0x8,0x0,/*07 EH*/
  0x18,0x3f,0x58,0xf,0xe,0x8,0x0,/*08 AE*/    0x1b,0x28,0x59,0xf,0xd,0x1,0x0,/*09 AA*/
  0x17,0x2c,0x57,0xf,0xc,0x1,0x0,/*10 AH*/    0x15,0x1f,0x58,0xf,0xc,0x0,0x0,/*11 AO*/
  0x10,0x25,0x52,0xf,0xb,0x1,0x0,/*12 UH*/    0x14,0x2c,0x57,0xe,0xb,0x0,0x0,/*13 AX*/
   0xe,0x49,0x5d,0xd,0xb,0x7,0x0,/*14 IX*/    0x12,0x31,0x3e,0xc,0xb,0x5,0x0,/*15 ER*/
   0xe,0x24,0x52,0xf,0xc,0x1,0x0,/*16 UX*/    0x12,0x1e,0x58,0xf,0xc,0x0,0x0,/*17 OH*/
  0x12,0x33,0x3e,0xd,0xc,0x6,0x0,/*18 RX*/    0x10,0x25,0x6e,0xd,0x8,0x1,0x0,/*19 LX*/
   0xd,0x1d,0x50,0xd,0x8,0x0,0x0,/*20 WX*/     0xf,0x45,0x5d,0xe,0xc,0x7,0x0,/*21 YX*/
   0xb,0x18,0x5a,0xd,0x8,0x0,0x0,/*22 WH*/    0x12,0x32,0x3c,0xc,0xa,0x5,0x0,/*23 R*/
   0xe,0x1e,0x6e,0xd,0x8,0x1,0x0,/*24 L*/      0xb,0x18,0x5a,0xd,0x8,0x0,0x0,/*25 W*/
   0x9,0x53,0x6e,0xd,0xa,0x8,0x0,/*26 Y*/      0x6,0x2e,0x51,0xc,0x3,0x0,0x0,/*27 M*/
   0x6,0x36,0x79,0x9,0x9,0x0,0x0,/*28 N*/      0x6,0x56,0x65,0x9,0x6,0x3,0x0,/*29 NX*/
   0x6,0x36,0x79,0x0,0x0,0x0,0x0,/*30 DX*/    0x11,0x43,0x5b,0x0,0x0,0x0,0x0,/*31 Q*/
   0x6,0x49,0x63,0x7,0xa,0xd,0xf,/*32 S*/      0x6,0x4f,0x6a,0x0,0x0,0x0,0x0,/*33 SH*/
   0x6,0x1a,0x51,0x3,0x3,0x3,0xf,/*34 F*/      0x6,0x42,0x79,0x0,0x0,0x0,0x0,/*35 TH*/
   0xe,0x49,0x5d,0x0,0x0,0x0,0x0,/*36 /H*/    0x10,0x25,0x52,0x0,0x0,0x0,0x0,/*37 /X*/
   0x9,0x33,0x5d,0xf,0x3,0x0,0x3,/*38 Z*/      0xa,0x42,0x67,0xb,0x5,0x1,0x0,/*39 ZH*/
   0x8,0x28,0x4c,0xb,0x3,0x0,0x0,/*40 V*/      0xa,0x2f,0x5d,0xb,0x4,0x0,0x0,/*41 DH*/
   0x6,0x4f,0x65,0x0,0x0,0x0,0x0,/*42 CHa*/    0x6,0x4f,0x65,0x0,0x0,0x0,0x0,/*43 CHb*/
   0x6,0x42,0x79,0x1,0x0,0x0,0x0,/*44 Ja*/     0x5,0x42,0x79,0x1,0x0,0x0,0x0,/*45 Jb*/
   0x6,0x6e,0x79,0x0,0xa,0xe,0x0,/*46 Jc*/     0x0, 0x0, 0x0,0x2,0x2,0x1,0x0,/*47 Jd*/
  0x13,0x48,0x5a,0xe,0xe,0x9,0x0,/*48 EY*/    0x1b,0x27,0x58,0xf,0xd,0x1,0x0,/*49 AY*/
  0x15,0x1f,0x58,0xf,0xc,0x0,0x0,/*50 OY*/    0x1b,0x2b,0x58,0xf,0xd,0x1,0x0,/*51 AW*/
  0x12,0x1e,0x58,0xf,0xc,0x0,0x0,/*52 OW*/     0xd,0x22,0x52,0xd,0x8,0x0,0x0,/*53 UW*/
   0x6,0x1a,0x51,0x2,0x0,0x0,0x0,/*54 Ba*/     0x6,0x1a,0x51,0x4,0x1,0x0,0xf,/*55 Bb*/
   0x6,0x1a,0x51,0x0,0x0,0x0,0x0,/*56 Bc*/     0x6,0x42,0x79,0x2,0x0,0x0,0x0,/*57 Da*/
   0x6,0x42,0x79,0x4,0x1,0x0,0xf,/*58 Db*/     0x6,0x42,0x79,0x0,0x0,0x0,0x0,/*59 Dc*/
   0x6,0x6e,0x70,0x1,0x0,0x0,0x0,/*60 Ga*/     0x6,0x6e,0x6e,0x4,0x1,0x0,0xf,/*61 Gb*/
   0x6,0x6e,0x6e,0x0,0x0,0x0,0x0,/*62 Gc*/     0x6,0x54,0x5e,0x1,0x0,0x0,0x0,/*63 GXa*/
   0x6,0x54,0x5e,0x4,0x1,0x0,0xf,/*64 GXb*/    0x6,0x54,0x5e,0x0,0x0,0x0,0x0,/*65 GXc*/
   0x6,0x1a,0x51,0x0,0x0,0x0,0x0,/*66 Pa*/     0x6,0x1a,0x51,0x0,0x0,0x0,0x0,/*67 Pb*/
   0x6,0x1a,0x51,0x0,0x0,0x0,0x0,/*68 Pc*/     0x6,0x42,0x79,0x0,0x0,0x0,0x0,/*69 Ta*/
   0x6,0x42,0x79,0x0,0x0,0x0,0x0,/*70 Tb*/     0x6,0x42,0x79,0x0,0x0,0x0,0x0,/*71 Tc*/
   0x6,0x6d,0x65,0x0,0x0,0x0,0x0,/*72 Ka*/     0xa,0x56,0x65,0xc,0xa,0x7,0x0,/*73 Kb*/
   0xa,0x6d,0x70,0x0,0x0,0x0,0x0,/*74 Kc*/     0x6,0x54,0x5e,0x0,0x0,0x0,0x0,/*75 KXa*/
   0x6,0x54,0x5e,0x0,0xa,0x5,0x0,/*76 KXb*/    0x6,0x54,0x5e,0x0,0x0,0x0,0x0 /*77 KXc*/
};

uint16_t pitchTable[64] = {
  // Covers A1 to C7
  58,61,65,69,73,77,82,86,92,97,
  103,109,115,122,129,137,145,154,163,173,
  183,194,206,218,231,244,259,274,291,308,
  326,346,366,388,411,435,461,489,518,549,
  581,616,652,691,732,776,822,871,923,978,
  1036,1097,1163,1232,1305,1383,1465,1552,1644,1742,
  1845,1955,2071,2195
};

  uint8_t frameList[] PROGMEM = {
  #if 1
  Da,3,0,39,Db,1,0,39,Dc,1,3,39,EY,8,6,39,YX,20,3,39, // Dai..
  Z,10,0,36,IY,35,3,36, // ..sy
  Da,3,0,32,Db,1,0,32,Dc,1,3,32,EY,8,6,32,YX,20,3,32, // Dai..
  Z,10,0,27,IY,35,3,27, // ..sy
  Ga,2,0,29,Gb,2,0,29,Gc,2,0,29,IH,10,3,29,V,5,0,29, // Give
  M,2,0,31,IY,10,3,31, // me
  YX,5,0,32,AO,10,0,32,RX,5,0,32, // your
  AH,25,0,29,NX,5,0,29, // an..
  S,2,0,32,ER,10,0,32,RX,3,0,32, // ..swer
  Da,3,0,27,Db,1,0,27,Dc,1,3,27,UX,80,3,27,WX,5,0,27, // do
  AY,5,20,34,YX,10,0,34,M,8,0,34, // I'm
  H,5,0,39,AX,30,0,39,FF,10,0,39, // half
  Ka,3,0,36,Kb,3,0,36,Kc,4,0,36,R,5,0,36,EY,30,0,36, // cra..
  Z,5,0,32,IY,40,0,32, // ..zy
  AO,10,0,29,LX,5,0,29, // all
  FF,5,0,31,AO,10,0,31, // for
  DH,5,0,32,AH,10,0,32, // the
  L,5,0,34,AH,20,0,34,V,5,0,34,// love
  AA,10,0,36,V,5,0,36,// of
  Y,10,0,34,UX,80,0,34, // you
  IH,10,0,36,Ta,2,0,36,Tb,1,0,36,Tc,2,0,36,// It
  W,2,0,37,OH,10,0,37,N,1,0,37,Ta,1,0,37,Tb,1,0,37,Tc,1,0,37,// won't
  Ba,2,0,36,Bb,1,0,36,Bc,2,0,36,IY,10,0,36,// be
  AH,15,0,34,// a
  S,2,0,39,Ta,2,0,39,Tb,2,0,39,Tc,2,0,39,AY,1,10,39,YX,10,0,39,// sty..
  L,3,0,36,IH,10,0,36,SH,2,0,36,// ..lish
  M,5,0,34,AE,10,0,34,// ma..
  R,5,0,32,IH,60,0,32,Ja,2,0,32,Jb,2,0,32,Jc,2,0,32,// ..rriage
  AY,5,10,34,YX,5,0,34,// I
  Ka,2,0,36,Kb,2,0,36,Kc,2,0,36,AH,20,0,36,N,2,0,36,Ta,2,0,36,Tb,2,0,26,Tc,2,0,36,// can't
  AX,15,0,32,// a..
  FF,5,0,29,AO,20,0,29,R,2,0,29,Da,1,0,29,Db,1,0,29,Dc,1,0,29,// ..fford
  AX,15,0,32,// a
  Ka,1,0,29,Kb,1,0,29,Kc,1,0,29,AE,12,0,29,// ca..
  R,5,0,27,IH,45,0,27,Ja,2,0,27,Jb,2,0,27,Jc,2,0,27,// ..rriage
  Ba,1,0,27,Bb,1,0,27,Bc,1,0,27,AH,10,0,27,Ta,1,0,27,Tb,1,0,27,Tc,1,0,27,// but
  Y,5,0,32,UH,10,10,32,L,5,0,32,// you'll
  L,3,0,36,UH,10,0,36,Ka,1,0,36,Kb,1,0,36,Kc,1,0,36,// look
  S,2,0,34,W,2,0,34,IY,20,0,34,Ta,2,0,34,Tb,2,0,34,Tc,2,0,34,// sweet
  AX,15,0,27,// a..
  Ka,2,0,32,Kb,2,0,32,Kc,2,0,32,R,2,0,32,AA,20,0,32,S,5,0,32,// ..cross
  DH,5,0,36,AH,10,0,36,// the
  S,2,0,34,IY,10,0,34,Ta,2,0,34,Tb,2,0,34,Tc,2,0,34,// seat
  AA,10,0,36,V,5,0,36,// of
  AE,15,0,37,// a
  Ba,2,0,39,Bb,2,0,39,Bc,2,0,39,AY,5,5,39,YX,5,0,39,// bi..
  S,5,0,36,IH,10,0,36,// ..cy..
  Ka,2,0,32,Kb,2,0,32,Kc,2,0,32,L,9,0,32,// ..cle
  M,2,0,34,EY,5,10,34,YX,10,0,34,Da,2,0,34,Db,2,0,34,Dc,2,0,34,// made
  FF,5,0,27,OY,1,5,27,RX,5,0,27,// for
  Ta,2,0,32,Tb,2,0,32,Tc,2,0,32,UX,50,0,32,// two
  #endif
  Ta,0,0,61
};

void timer_callback(timer_callback_args_t __attribute((unused)) *p_args) {

  int8_t value;
  form1Phase += form1PhaseInc;
  value = pgm_read_byte(sinCalc+(((form1Phase>>8) & 0xf0) | form1Amp));
  form2Phase += form2PhaseInc;
  value += pgm_read_byte(sinCalc+(((form2Phase>>8) & 0xf0) | form2Amp));
  form3Phase += form3PhaseInc;
  value += pgm_read_byte(sqrCalc+(((form3Phase>>8) & 0xf0) | form3Amp));
  value = (value * (0xff^(pitchPhase>>8)))>>8;
  pitchPhase += pitchPhaseInc;

  int sample = 127 * value;  

  int reverb = ((int)reverbBuffer[reverbAddr] * REVERB_DECAY) >> FIXED_BITS;
  reverb += sample;
  reverbBuffer[reverbAddr] = reverb;
  reverbAddr++;
  if (reverbAddr > REVERB_LENGTH) reverbAddr = 0;

  int16_t output = sample + (reverbBuffer[reverbAddr]>>2);
  analogWrite(DAC, 2048 + (output >> 4));   

  if ((pitchPhase) < pitchPhaseInc) {
    form1Phase = 0;
    form2Phase = 0;
    form3Phase = 0;
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

}

void loop() {

  uint8_t *framePos = frameList;
  
  while(1) {

    uint8_t startFormant,staticFrames,tweenFrames;
    uint16_t startPitch,nextPitch;
    uint8_t nextFormant;
    int16_t startForm1PhaseInc,startForm2PhaseInc,startForm3PhaseInc;
    uint8_t startForm1Amp,startForm2Amp,startForm3Amp;
    uint8_t *formantPos;

    startFormant = pgm_read_byte(framePos++);
    staticFrames = pgm_read_byte(framePos++);

    if (!staticFrames) break;

    tweenFrames = pgm_read_byte(framePos++);

    startPitch = pitchTable[pgm_read_byte(framePos++)];
    nextFormant = pgm_read_byte(framePos);
    nextPitch = pitchTable[pgm_read_byte(framePos+3)];
    pitchPhaseInc = startPitch;
    formantPos = formantTable + startFormant * FORMANT_SZ;
    form1PhaseInc = startForm1PhaseInc = pgm_read_byte(formantPos++)*formantScale;
    form2PhaseInc = startForm2PhaseInc = pgm_read_byte(formantPos++)*formantScale;
    form3PhaseInc = startForm3PhaseInc = pgm_read_byte(formantPos++)*formantScale;
    form1Amp = startForm1Amp = pgm_read_byte(formantPos++);
    form2Amp = startForm2Amp = pgm_read_byte(formantPos++);
    form3Amp = startForm3Amp = pgm_read_byte(formantPos++);

    for (;staticFrames--;) delay(frameTime);
    
    if (tweenFrames) {
      uint8_t* formantPos;
      int16_t deltaForm1PhaseInc,deltaForm2PhaseInc,deltaForm3PhaseInc;
      int8_t deltaForm1Amp,deltaForm2Amp,deltaForm3Amp;
      int16_t deltaPitch;
      tweenFrames--;
      formantPos = formantTable + nextFormant * FORMANT_SZ;
      deltaForm1PhaseInc = pgm_read_byte(formantPos++)*formantScale - startForm1PhaseInc;
      deltaForm2PhaseInc = pgm_read_byte(formantPos++)*formantScale - startForm2PhaseInc;
      deltaForm3PhaseInc = pgm_read_byte(formantPos++)*formantScale - startForm3PhaseInc;
      deltaForm1Amp = pgm_read_byte(formantPos++) - startForm1Amp;
      deltaForm2Amp = pgm_read_byte(formantPos++) - startForm2Amp;
      deltaForm3Amp = pgm_read_byte(formantPos++) - startForm3Amp;
      deltaPitch = nextPitch - startPitch;
      for (int i=1; i<=tweenFrames; i++) {
        form1PhaseInc = startForm1PhaseInc + (i*deltaForm1PhaseInc)/tweenFrames;
        form2PhaseInc = startForm2PhaseInc + (i*deltaForm2PhaseInc)/tweenFrames;
        form3PhaseInc = startForm3PhaseInc + (i*deltaForm3PhaseInc)/tweenFrames;
        form1Amp = startForm1Amp + (i*deltaForm1Amp)/tweenFrames;
        form2Amp = startForm2Amp + (i*deltaForm2Amp)/tweenFrames;
        form3Amp = startForm3Amp + (i*deltaForm3Amp)/tweenFrames;
        pitchPhaseInc = startPitch + (i*deltaPitch)/tweenFrames;
        delay(frameTime);
      }
    }
  }

}