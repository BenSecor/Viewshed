// Laura Toma
//
#ifndef __map_h
#define __map_h

#include "pixel_buffer.h"
#include "grid.h"


#include <stdlib.h>
#include <string.h>
#include <stdio.h> 
#include <time.h>
#include <math.h>
#include <assert.h>

#define PI 3.1415


typedef float Color[3];



////////////////////////////////////////////////////////////
/*
a b c
d e f
g h i 
return nodata if e is nodata, or on th eboundary, or has a nodata neighbor
else: return (c + 2f + i - a - 2d -g)/8
 */
float dzdx(const Grid* grid, int r, int c);


////////////////////////////////////////////////////////////
/*
a b c
d e f
g h i 
return nodata if e is nodata, or on th eboundary, or has a nodata neighbor
else: return (g + 2h + i - a - 2b -c)/8
 */
float dzdy(const Grid* grid, int r, int c);

////////////////////////////////////////////////////////////
float slope_rad(const Grid* grid, float dzdx, float dzdy);

////////////////////////////////////////////////////////////
float aspect_rad(const Grid* grid, float dzdx, float dzdy);




#endif
