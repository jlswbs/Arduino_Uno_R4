// XOR fractal //

#include "Arduino_LED_Matrix.h"

ArduinoLEDMatrix matrix;

#define WIDTH   12
#define HEIGHT  8
#define SCR     (WIDTH * HEIGHT)

  uint8_t grid[HEIGHT][WIDTH];
  int cnt;


void setup() {

  matrix.begin();

}

void loop() {

  for (int y = 0; y < HEIGHT; y++) {
  
    for (int x = 0; x < WIDTH; x++) { 
   
      uint8_t t = (x ^ y) + cnt;
  
      grid[y][x] = ! (t % 16);
 
    }

  }

  cnt++;
  
  matrix.renderBitmap(grid, 8, 12);

  delay(25);

}