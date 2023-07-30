// Brian's brain 2D cellular automata //

#include "Arduino_LED_Matrix.h"

ArduinoLEDMatrix matrix;

#define WIDTH   12
#define HEIGHT  8
#define SCR     (WIDTH * HEIGHT)

#define READY       0
#define REFRACTORY  1
#define FIRING      2

  uint8_t grid[HEIGHT][WIDTH];
  uint8_t world[SCR] = {
    0, 0, 0, 0, 0, 2, 2, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 2, 2, 0, 0, 0, 0, 0,
    0, 2, 0, 0, 2, 0, 0, 2, 0, 0, 2, 0, 
    0, 0, 2, 0, 0, 2, 2, 0, 0, 2, 0, 0,
    0, 0, 2, 0, 0, 2, 2, 0, 0, 2, 0, 0, 
    0, 2, 0, 0, 2, 0, 0, 2, 0, 0, 2, 0,
    0, 0, 0, 0, 0, 2, 2, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 2, 2, 0, 0, 0, 0, 0,
  };
  uint8_t newworld[SCR];

uint8_t neighbours(uint8_t world[SCR], int x_pos, int y_pos){
  
    int cell;
    int count = 0;

    for (int y = -1; y < 2; y++) {

        for (int x = -1; x < 2; x++) {

            int cx = x_pos + x;
            int cy = y_pos + y;
            if ( (0 <= cx && cx < WIDTH) && (0 <= cy && cy < HEIGHT)) {
                cell = world[(x_pos + x) + (y_pos + y) * WIDTH];
                if (cell == FIRING) count ++;           
            }
        
        }
    
    }
  
  return count;

}


void apply_rules(uint8_t world[SCR]){
  
  int cell;

  memcpy(newworld, world, SCR);

  for (int y = 0; y < HEIGHT; y++) {
    
    for (int x = 0; x < WIDTH; x++){
      
      cell = newworld[x+y*WIDTH];          
      if (cell == READY) {
        int neighbour = neighbours(newworld, x, y);
        if (neighbour == 2) world[x+y*WIDTH] = FIRING; }
      else if (cell == FIRING) world[x+y*WIDTH] = REFRACTORY;
      else world[x+y*WIDTH] = READY;
    
    }
  
  }

}


void setup() {

  matrix.begin();

}

void loop() {

  apply_rules(world);

  for(int y = 0; y < HEIGHT; y++){
    
    for(int x = 0; x < WIDTH; x++){
           
      if (world[x+y*WIDTH] == FIRING) grid[y][x] = 1;    
      else if (world[x+y*WIDTH] == REFRACTORY) grid[y][x] = 1;
      else grid[y][x] = 0; 

    }

  }
  
  matrix.renderBitmap(grid, 8, 12);

  delay(250);

}