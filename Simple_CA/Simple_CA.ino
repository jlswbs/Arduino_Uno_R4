// Simple 1D cellular automata //

#include "Arduino_LED_Matrix.h"

ArduinoLEDMatrix matrix;

#define WIDTH   12
#define HEIGHT  8

  uint8_t grid[HEIGHT][WIDTH];
  bool state[WIDTH] = {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  bool k;


void setup() {

  matrix.begin();

}

void loop() {

  for (int y = 0; y < HEIGHT; y++) {

    for (int x=0; x < WIDTH; x++) {

      k = k ^ state[x];	
      state[x] = k;

      if (state[x] == 1) grid[y][x] = 1;       
      else grid[y][x] = 0;

    }

  }
  
  matrix.renderBitmap(grid, 8, 12);

  delay(250);

}