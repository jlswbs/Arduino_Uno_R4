// Star-Wars 2D cellular automata //

#include "Arduino_LED_Matrix.h"

ArduinoLEDMatrix matrix;

#define WIDTH   12
#define HEIGHT  8
#define SCR     (WIDTH * HEIGHT)
#define ALIVE   3
#define DEATH_1 2
#define DEATH_2 1
#define DEAD    0

  uint8_t current[SCR] = {
    0, 0, 3, 3, 3, 0, 0, 3, 3, 3, 0, 0, 
    3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3,
    3, 0, 0, 3, 3, 3, 3, 3, 3, 0, 0, 3, 
    0, 0, 0, 0, 3, 3, 3, 3, 0, 0, 0, 0,
    0, 0, 0, 0, 3, 3, 3, 3, 0, 0, 0, 0, 
    3, 0, 0, 3, 3, 3, 3, 3, 3, 0, 0, 3,
    3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3,
    0, 0, 3, 3, 3, 0, 0, 3, 3, 3, 0, 0,
  };
  uint8_t next[SCR];
  uint8_t alive_counts[SCR];
  uint8_t temp[SCR];  
  uint8_t grid[HEIGHT][WIDTH];

void step(){

  for (int y = 0; y < HEIGHT; y++) {
  
    for (int x = 0; x < WIDTH; x++) {  
    
      int count = 0;
      int next_val;
    
      int mx = WIDTH-1;
      int my = HEIGHT-1;
    
      int self = current[x+y*WIDTH];
    
      for (int nx = x-1; nx <= x+1; nx++) {
  
        for (int ny = y-1; ny <= y+1; ny++) {
    
          if (nx == x && ny == y) continue;     
          if (current[(wrap(nx, mx))+(wrap(ny, my))*WIDTH] == ALIVE) count++;
      
        }   
      }  

    int neighbors = count;
    
    if (self == ALIVE) next_val = ((neighbors == 3) || (neighbors == 4) || (neighbors == 5)) ? ALIVE : DEATH_1;
  
    else if (self == DEAD) next_val = (neighbors == 2) ? ALIVE : DEAD;
  
    else next_val = self-1;
   
    next[x+y*WIDTH] = next_val;
  
    if (next_val == ALIVE) alive_counts[x+y*WIDTH] = min(alive_counts[x+y*WIDTH]+1, 100);
    else if (next_val == DEAD) alive_counts[x+y*WIDTH] = 0;
    
    }
  }
  
  memcpy(temp, current, SCR);
  memcpy(current, next, SCR);
  memcpy(next, temp, SCR);

}
  
int wrap(int v, int m){

    if (v < 0) return v + m;
    else if (v >= m) return v - m;
    else return v;
}

void draw_type(int min_alive, int max_alive){
       
   for (int y = 0; y < HEIGHT; y++) {
  
    for (int x = 0; x < WIDTH; x++) { 
   
    int self = current[x+y*WIDTH];
    if (self == DEAD) continue;
    int alive = alive_counts[x+y*WIDTH];
    if (alive < min_alive || alive > max_alive) continue;
    if (self == ALIVE) grid[y][x] = 1;
    else if (self == DEATH_1) grid[y][x] = 1;
    else if (self == DEATH_2) grid[y][x] = 0;

  }

  }
}


void setup() {

  matrix.begin();

}

void loop() {

  step();

  draw_type(50, 100);
  draw_type(2, 49);
  draw_type(0, 1);
  
  matrix.renderBitmap(grid, 8, 12);

  delay(250);

}