// Conway's Game of Life cellular automata //

#include "Arduino_LED_Matrix.h"

ArduinoLEDMatrix matrix;

#define WIDTH         12
#define HEIGHT        8
#define SCR           (WIDTH * HEIGHT)
#define crNum(x,y,z)  ((x)+(y))%z
#define speed         100
#define generation    22

  uint8_t grid[HEIGHT][WIDTH];
  bool newCells[SCR];
  bool cells[SCR];
  int counter;
  bool world[SCR] = {
    0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0,
    0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 
    0, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 0,
    0, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 0, 
    0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0,
    0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0,
  };

void reset(){

  memset(cells, 0, SCR);
  memset(newCells, 0, SCR);

  for(int k = 0; k < SCR; k++) cells[k] = world[k];

}

void next(){

  memcpy(newCells, cells, SCR);

  for(int y = 0; y < HEIGHT; y++){

    for(int x = 0; x < WIDTH; x++){
    
      int surrounding = 0;
     
      bool isAlive = false;

      cells[x+y*WIDTH] ? isAlive = true : isAlive = false;
      
      for(int j = -1; j < 2; j++) {
        for(int i = -1; i < 2; i++) surrounding += cells[crNum(x,i,WIDTH)+crNum(y,j,HEIGHT)*WIDTH];
      }

      surrounding -= cells[x+y*WIDTH];
      if((surrounding < 2 || surrounding > 3) && isAlive) newCells[x+y*WIDTH] = 0;
      if(surrounding == 3 && !isAlive) newCells[x+y*WIDTH] = 1;
      
    }

  }

  memcpy(cells, newCells, SCR);

}

void setup() {

  matrix.begin();

  reset();

}

void loop() {
  
  for(int y = 0; y < HEIGHT; y++){ 
    
    for(int x = 0; x < WIDTH; x++) cells[x+y*WIDTH] ? grid[y][x] = 1 : grid[y][x] = 0;

  }

  matrix.renderBitmap(grid, 8, 12);
  
  delay(speed);

  next();

  if(counter == generation){

    reset();
    counter = 0;

  }

  counter++;

}