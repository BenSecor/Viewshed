// your name 
// oct 2023

#ifndef __vis_hpp
#define __vis_hpp


#include "grid.h"

extern int VIEWSHED_SIZE;

typedef struct _point{
  int x,y;
} point;



inline float max(float a, float b) {
  return (a>=b)? a : b; 
}

void computeViewshed(Grid* elev_grid, Grid* view_grid, int r, int c, int elevation);


int isVisible(Grid* grid, int source_r, int source_c, int target_r, int target_c);

#endif 



