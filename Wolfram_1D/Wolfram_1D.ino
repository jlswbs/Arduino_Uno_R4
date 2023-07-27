// Wolfram 1D cellular automata //

#include "Arduino_LED_Matrix.h"

ArduinoLEDMatrix matrix;

#define WIDTH   12
#define HEIGHT  8

  uint8_t grid[HEIGHT][WIDTH];
  bool state[WIDTH] = {0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0};
  bool newst[WIDTH];
  bool rules[8] = {0, 1, 1, 1, 1, 0, 0, 0};


void setup() {

  matrix.begin();

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

  }
  
  matrix.renderBitmap(grid, 8, 12);

  delay(250);

}