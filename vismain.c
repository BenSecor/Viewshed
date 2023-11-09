//
// Ben Secor and Liam Jachetta 
// oct 2023 
//
#include "vis.hpp"
#include "map.h"
#include "grid.h"
#include "pixel_buffer.h"

#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h> 
#include <time.h>
#include <math.h>
#include <getopt.h>
#include <assert.h>







/*
  Reads command line arguments
*/
#include <iostream>
#include <cstdlib>



static const u8 max_rgb_value = 0xffu;
static const float z = 2.5;

 // must convery her SUN_zenith_deg and azimuth as we skipped that step last time 
float sun_zenith_rad = 2.36;
float sun_azimuth_rad = 0.79;

 //for hillshading
 const int SUN_altitude_deg = 45;
 const int SUN_azimuth_deg = 315;

int VIEWSHED_SIZE = 0;

void createGrids(Grid* grid, Grid* hillshadeGrid) {
  float SUN_azimuth_math;
  float SUN_zenith_deg; 
  SUN_zenith_deg = 90-SUN_altitude_deg;
  sun_zenith_rad = SUN_zenith_deg * PI / 180;
  SUN_azimuth_math = ((SUN_azimuth_deg-90)+360) %360;
  sun_azimuth_rad = SUN_azimuth_math*PI / 180;

  for (int y=1; y < (grid->nrows)-1; y++) {
     for (int x=1; x < (grid->ncols)-1; x++) {
        if (grid_is_nodata(grid, y, x)){
          continue;
       } else {
        //compute slope
        float topLeft = grid_get(grid, y - 1, x - 1);
        float topRight = grid_get(grid, y - 1, x + 1);
        float topMiddle = grid_get(grid, y-1, x);
        float middleLeft = grid_get(grid, y, x-1);
        float middleRight = grid_get(grid, y, x+1);
        float bottomMiddle = grid_get(grid, y+1, x);
        float bottomLeft = grid_get(grid, y + 1, x - 1);
        float bottomRight = grid_get(grid, y + 1, x + 1);
        // Compute the slope using the formula
        float dx = grid->cellsize; // Horizontal distance between diagonal points
        float dy = grid->cellsize; // Vertical distance between diagonal points

        float slopeX = (topRight + 2 * middleRight + bottomRight  - topLeft - 2 * middleLeft - bottomLeft) / (dx*8);
        float slopeY = (topLeft + 2 * topMiddle+ topRight - bottomLeft - 2 * bottomMiddle - bottomRight) / (dy*8);
        float slope = atan(z*sqrt(slopeX*slopeX+slopeY*slopeY));
        //compute the aspect grid
        float azimuth_rad;
         if (slopeX != 0){
            azimuth_rad= atan2(slopeY, slopeX);
              if(azimuth_rad < 0){
               azimuth_rad += 2*PI;
              }
         } else {
          if (slopeY > 0) {
            azimuth_rad = PI/2;
          } else if ( slopeY < 0){
            azimuth_rad = 3*PI/2;
          } else{
            azimuth_rad = 0;
          }
        }
        float hillshade = cos(sun_zenith_rad)*cos(slope)+ sin(sun_zenith_rad)*sin(slope)*cos(sun_azimuth_rad - azimuth_rad);
        if (hillshade < 0){
          hillshade = 0.0;
        }
        grid_set(hillshadeGrid, y, x, hillshade);
      }
    }
  }
}


 void computeHillshadeBuffer(const PixelBuffer* pb, const Grid* grid, const Grid* hillshadeGrid){
    float color[3];
    for (u16 y=0; y < pb->height; y++) {
      for (u16 x=0; x< pb->width; x++) {
        if (grid_is_nodata(grid, y, x)) {
          color[0] = 1; color[1] = 1; color[2] = 0; //black if nodata 
        } else {
          // get aspect ratio and slope value of the current pixel
          float h = grid_get(hillshadeGrid, y, x); //aspect ration of this pixel 
          
            // set color value to the sum of that interpolated color based on slope/aspect 
            color[0] = color[1] = color[2] = h;
        }
        //bring it from [0,1] to [0, 255]
        u16 rgb_color[3] = {
        (u16) (color[0] * max_rgb_value),
        (u16) (color[1] * max_rgb_value),
        (u16) (color[2] * max_rgb_value)}; 
        
        write_pixel_to_buffer(pb, x, y, rgb_color[0], rgb_color[1], rgb_color[2]);
      }
    }
  }


void pixelbuffer_overlay(const PixelBuffer* pbout, const PixelBuffer* pb1, const PixelBuffer* pb2, float alpha) {
       // Iterate over each pixel in the buffers and perform the overlay operation
      for (u16 y = 0; y < pb1->height; y++) {
        for (u16 x = 0; x < pb1->width; x++) {
             u32 pixel1 = pb1->pixels[y * pb1->width + x];
            u32 pixel2 = pb2->pixels[y * pb2->width + x];

            // Extract the R, G, and B components of pixel1 and pixel2
            u8 b1 = (u8)((pixel1 >> 16) & 0xFF);
            u8 g1 = (u8)((pixel1 >> 8) & 0xFF);
            u8 r1 = (u8)(pixel1 & 0xFF);

            u8 b2 = (u8)((pixel2 >> 16) & 0xFF);
            u8 g2 = (u8)((pixel2 >> 8) & 0xFF);
            u8 r2 = (u8)(pixel2 & 0xFF);

            // Multiply the pixel colors by alpha
            u8 result_r = (u8)((alpha * r1) + ((1.0f - alpha) * r2));
            u8 result_g = (u8)((alpha * g1) + ((1.0f - alpha) * g2));
            u8 result_b = (u8)((alpha * b1) + ((1.0f - alpha) * b2));

            // Write the resulting R, G, and B values to pbout
            write_pixel_to_buffer(pbout, x, y, result_r, result_g, result_b);
        }
    }
}

void computeViewshedBuffer(const PixelBuffer* viewPB, const Grid* elev_grid, Grid* view_grid){
  float color[3];
  for (u16 y = 0; y < viewPB->height; y++) {
    for (u16 x = 0; x < viewPB->width; x++) {
    if (grid_is_nodata(view_grid, y, x)) {
        color[0] = 1; color[1] = 1; color[2] = 0 ; //black if nodata 
    } else {
        // get aspect ratio and slope value of the current pixel
        float h = grid_get(view_grid, y, x); //aspect ration of this pixel 
        if(h){
          VIEWSHED_SIZE++;
          color[1] = h;
          color[0] =  color[2] = 0;
        } else {
          // set color value to the sum of that interpolated color based on slope/aspect 
          color[0] = color[1] = color[2] = 0.5;
        }
    }
    //bring it from [0,1] to [0, 255]
    u16 rgb_color[3] = {
    (u16) (color[0] * max_rgb_value),
    (u16) (color[1] * max_rgb_value),
    (u16) (color[2] * max_rgb_value)}; 
    write_pixel_to_buffer(viewPB, x, y, rgb_color[0], rgb_color[1], rgb_color[2]);
    }
  }
}


////////////////////////////////////////////////////////////
int main(int argc, char** argv) {
    if (argc != 6) {
        printf("Usage: %s [raster.asc] [output.asc] [x] [y] [height]\n", argv[0]);
        exit(1);
    }

    printf("Reading raster %s\n", argv[1]);
    FILE *file = fopen(argv[1], "r");
    if (!file) {
        printf("Cannot open file %s\n", argv[1]);
        exit(1);
    }

    // Open the output file
    FILE *fileFD = fopen(argv[2], "w");
    if (!fileFD) {
        printf("Cannot open file %s\n", argv[2]);
        exit(1);
    }

    // Process x, y, and height
    int x, y, height;

    if (sscanf(argv[3], "%d", &x) != 1) {
        printf("Invalid value for x: %s\n", argv[3]);
        exit(1);
    }

    if (sscanf(argv[4], "%d", &y) != 1) {
        printf("Invalid value for y: %s\n", argv[4]);
        exit(1);
    }

    if (sscanf(argv[5], "%d", &height) != 1) {
        printf("Invalid value for height: %s\n", argv[5]);
        exit(1);
    }

    char* elev_name = argv[1];
    char* vis_name = argv[2];

    printf("\nRunning with arguments:\n");
    printf("\telev_name: %s\n", elev_name);
    printf("\tvis_name: %s\n", vis_name);
    printf("\tvp=(%d, %d, %d)\n", x, y, height);




    // initializing of necessary grid
    Grid* elev_grid = grid_read(file); 
    Grid* hillshadeGrid = grid_init_from(elev_grid);
    Grid* view_grid = grid_init_from(elev_grid);

    createGrids(elev_grid, hillshadeGrid);



    // initializing fo necessary pixel buffers 
    const PixelBuffer hillshadePB = init_pixel_buffer(elev_grid->ncols, elev_grid->nrows);
    const PixelBuffer viewPB = init_pixel_buffer(elev_grid->ncols, elev_grid->nrows);
    const PixelBuffer outPB = init_pixel_buffer(elev_grid->ncols, elev_grid->nrows);

    // call computer buffer methods
    computeHillshadeBuffer(&hillshadePB, elev_grid, hillshadeGrid);

    free(hillshadeGrid);

    // Start Timer for computing viewshed
    clock_t before = clock(); 

    computeViewshed(elev_grid, view_grid, x, y, height);

    // Calculate time
    clock_t difference = clock() - before;
    int sec = difference / CLOCKS_PER_SEC;
    printf("\nTime to compute viewshed (seconds): %d", sec);
    printf("\nTime to compute viewshed (hours): %.2f\n", (double)sec / 3600.0);
    printf("\nViewshed Size: %d\n\n", VIEWSHED_SIZE);

    computeViewshedBuffer(&viewPB, elev_grid, view_grid);
    pixelbuffer_overlay(&outPB, &viewPB, &hillshadePB,   0.5);
    save_pixel_buffer_to_file(&outPB, "map.view-over-hillshade.bmp");
    save_pixel_buffer_to_file(&viewPB, "map.view.bmp");

    grid_free(view_grid);
    grid_free(elev_grid);
}
