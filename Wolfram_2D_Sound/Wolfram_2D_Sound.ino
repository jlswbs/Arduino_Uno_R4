// Wolfram 2D cellular automata with sound //

#include "Arduino_LED_Matrix.h"
#include "analogWave.h"

ArduinoLEDMatrix matrix;
analogWave wave(DAC);

#define WIDTH   12
#define HEIGHT  8
#define SCR     (WIDTH * HEIGHT)

  uint8_t grid[HEIGHT][WIDTH];
  bool state[SCR] = {
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
  };
  bool newst[SCR];
  bool rules[10] = {0,0,1,1,1,1,0,0,0,0};
  int freq = 110;
  

uint8_t neighbors(uint16_t x, uint16_t y){
  
  uint8_t result = 0;

  if(y > 0 && state[x+(y-1)*WIDTH] == 1) result = result + 1;
  if(x > 0 && state[(x-1)+y*WIDTH] == 1) result = result + 1;
  if(x < WIDTH-1 && state[(x+1)+y*WIDTH] == 1) result = result + 1;
  if(y < HEIGHT-1 && state[x+(y+1)*WIDTH] == 1) result = result + 1;
  
  return result;
 
}

void setup() {

  matrix.begin();
  wave.sine(freq);
  wave.amplitude(0.1);
  analogWriteResolution(12);

}

void loop() {

  for(int y = 0; y < HEIGHT; y++){
    
    for(int x = 0; x < WIDTH; x++){
           
      uint8_t totalNeighbors = neighbors(x, y);
            
      if(state[x+y*WIDTH] == 0 && totalNeighbors == 0) {newst[x+y*WIDTH] = rules[0]; grid[y][x] = 0; freq = 262; }
      else if(state[x+y*WIDTH] == 1 && totalNeighbors == 0) {newst[x+y*WIDTH] = rules[1]; grid[y][x] = 1; freq = 294; }
      else if(state[x+y*WIDTH] == 0 && totalNeighbors == 1) {newst[x+y*WIDTH] = rules[2]; grid[y][x] = 0; freq = 330; }
      else if(state[x+y*WIDTH] == 1 && totalNeighbors == 1) {newst[x+y*WIDTH] = rules[3]; grid[y][x] = 1; freq = 349; }
      else if(state[x+y*WIDTH] == 0 && totalNeighbors == 2) {newst[x+y*WIDTH] = rules[4]; grid[y][x] = 0; freq = 392; }
      else if(state[x+y*WIDTH] == 1 && totalNeighbors == 2) {newst[x+y*WIDTH] = rules[5]; grid[y][x] = 1; freq = 440; }
      else if(state[x+y*WIDTH] == 0 && totalNeighbors == 3) {newst[x+y*WIDTH] = rules[6]; grid[y][x] = 0; freq = 494; }
      else if(state[x+y*WIDTH] == 1 && totalNeighbors == 3) {newst[x+y*WIDTH] = rules[7]; grid[y][x] = 1; freq = 523; }
      else if(state[x+y*WIDTH] == 0 && totalNeighbors == 4) {newst[x+y*WIDTH] = rules[8]; grid[y][x] = 0; freq = 587; }
      else if(state[x+y*WIDTH] == 1 && totalNeighbors == 4) {newst[x+y*WIDTH] = rules[9]; grid[y][x] = 1; freq = 659; }

      wave.freq(freq);

      analogWrite(DAC, freq);

      delayMicroseconds(freq / 4);

    }
  }
 
  memcpy(state, newst, SCR);
  
  matrix.renderBitmap(grid, 8, 12);

  delay(240);

}