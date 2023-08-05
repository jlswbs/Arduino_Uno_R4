#include "math.h"
#include "playtune.h"
#include "wavetable.h"
#include "songdata.h"
#include "tuningwords.h"
#include "envelope.h"

FMchannel ch[NUM_OF_CHANNELS];

void FMchannel::resetEnvelopePointers() {
  adsrChannel.envelope_ptr1 = 0;
  adsrChannel.decayDurationCount = 0;
  adsrChannel.attackDurationCount = 0;
  adsrChannel.releaseDurationCount = 0;
}

void FMchannel::setADSR(float inputAttackS, float inputDecayS, float inputSustainS, float inputReleaseS) {

  inputAttackS = fabs(inputAttackS);
  inputDecayS = fabs(inputDecayS);
  inputSustainS = fabs(inputSustainS);
  inputReleaseS = fabs(inputReleaseS);

  adsrChannel.attackDuration = (unsigned int) ( (float)inputAttackS / (1.0f/(float)SAMPLE_RATE) );
  adsrChannel.attackDurationSlope = 1.00f / (inputAttackS / (1.0f/(float)SAMPLE_RATE) );

  adsrChannel.decayDuration = (unsigned int) ( ((float)inputDecayS/256.0f) / (1.0f/(float)SAMPLE_RATE) );

  adsrChannel.releaseDuration = (unsigned int) ( (float)inputReleaseS / (1.0f/(float)SAMPLE_RATE) );
  adsrChannel.releaseDurationSlope = -(1.00f / (inputReleaseS / (1.0f/(float)SAMPLE_RATE) ));

}


int generateFModTask() {

      for (auto i = 0; i < NUM_OF_CHANNELS; i++) {
        ch[i].generateFMsample();

        if( ch[i].getStateADSR() == stateAttack ) {
          ch[i].calculateAttackSample();
        }

        if( ch[i].getStateADSR() == stateDecay ) {
          ch[i].calculateDecaySample();
        }

        if( ch[i].getStateADSR() == stateRelease ) {
          ch[i].calculateReleaseSample();
        }

        if( ch[i].getStateADSR() == stateOff ) {
          ch[i].setOutput(0);
        }

      }
      // Temporary: only six channels are manually added!
      return ( (short)ch[0].getOutput() + (short)ch[1].getOutput() + (short)ch[2].getOutput() + (short)ch[3].getOutput() + (short)ch[4].getOutput() + (short)ch[5].getOutput() ) / NUM_OF_CHANNELS;
  
 
}

void updateNote(unsigned char& isPlaying, unsigned int& timePlay, unsigned int& timePlayCount, unsigned int& songIndex, float& speed) {

  unsigned char cmd = songData1[songIndex];
  unsigned char opcode = cmd & 0xf0;
  unsigned char chan = cmd & 0x0f;

  if (opcode == 0x90) {
    while (opcode == 0x90) {
      // Play a note here! Reset accumulators and set tuning words from the next note!
      // Also reset decay and attack states!
      ch[chan].setTuningWord_c( (unsigned int) ( (float)tuningWords[songData1[songIndex + 1]] * ch[chan].getFreqMult_m() ) ); // convert and put next notes!
      ch[chan].setTuningWord_m( (unsigned int) ( (float)tuningWords[songData1[songIndex + 1]] * ch[chan].getFreqMult_c() ) ); // same as previous.

      ch[chan].resetEnvelopePointers();
      ch[chan].setStateADSR(stateAttack);

      songIndex += 2;
      cmd = songData1[songIndex];
      opcode = cmd & 0xf0;
      chan = cmd & 0x0f;
    }
  }

  if (opcode == 0x80) {
    while (opcode == 0x80) {
      // Stop note: the note is dampened immediately in order to prevent the click
      // once the next note is played. Not all the clicks in the sound can be removed: this is currently
      // being investigated.
      ch[chan].setStateADSR(stateRelease);
      songIndex += 1;
      cmd = songData1[songIndex];
      opcode = cmd & 0xf0;
      chan = cmd & 0x0f;
      timePlay = 20;
    }
    return;
  }

  if (opcode == 0xf0) { // stop playing score!
    isPlaying = 0;
    return;
  }

  if (opcode == 0xe0) { // start playing from beginning!
    songIndex = 0;
    timePlay = 0;
    timePlayCount = 0;
    return;
  }

  if ( ((opcode & 0x80) == 0) ||  ((opcode & 0x90) == 0) ) {
    timePlay = (unsigned int)( ((songData1[songIndex] << 8) | songData1[songIndex + 1]) * (1/speed) );
    songIndex += 2;
  }
}