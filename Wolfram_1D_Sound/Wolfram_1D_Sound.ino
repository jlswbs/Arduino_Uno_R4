// Wolfram 1D cellular automata with sound //

#include "Arduino_LED_Matrix.h"
#include "analogWave.h"

ArduinoLEDMatrix matrix;
analogWave wave(DAC);

#define WIDTH   12
#define HEIGHT  8

  uint8_t grid[HEIGHT][WIDTH];
  bool state[WIDTH] = {0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0};
  bool newst[WIDTH];
  bool rules[8] = {0, 1, 1, 1, 1, 0, 0, 0};
  int freq[WIDTH];


void setup() {

  matrix.begin();
  wave.sine(110);
  wave.amplitude(0.1);

}

void loop() {

  for (int y = 0; y < HEIGHT; y++) {

    memset (newst, 0, WIDTH);

    for (int x=0; x < WIDTH; x++) {     
      uint8_t k = 4 * state[(x - 1 + WIDTH) % WIDTH] + 2 * state[x] + state[(x + 1) % WIDTH];
      newst[x] = rules[k];
    }

    memcpy (state, newst, WIDTH);
  
    for (int x = 0; x < WIDTH; x++) {
      if (state[x] == 1) grid[y][x] = 1;       
      else grid[y][x] = 0;
    }

    if(state[0] == 1) freq[0] = 262; else freq[0] = 0;
    if(state[1] == 1) freq[1] = 294; else freq[1] = 0;
    if(state[2] == 1) freq[2] = 330; else freq[2] = 0;
    if(state[3] == 1) freq[3] = 349; else freq[3] = 0;
    if(state[4] == 1) freq[4] = 392; else freq[4] = 0;
    if(state[5] == 1) freq[5] = 440; else freq[5] = 0;
    if(state[6] == 1) freq[6] = 494; else freq[6] = 0;
    if(state[7] == 1) freq[7] = 523; else freq[7] = 0;
    if(state[8] == 1) freq[8] = 587; else freq[8] = 0;
    if(state[9] == 1) freq[9] = 659; else freq[9] = 0;
    if(state[10] == 1) freq[10] = 699; else freq[10] = 0;
    if(state[11] == 1) freq[11] = 784; else freq[11] = 0;

    int frequency = 0;

    for (int x = 0; x < WIDTH; x++) frequency = frequency + freq[x];

    wave.freq(frequency / 4);

    delay(120);

    matrix.renderBitmap(grid, 8, 12);

  }

}