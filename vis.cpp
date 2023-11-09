// Ben Secor and Liam Jachetta
// oct 2023 

#include "vis.hpp"
#include "grid.h"


#include <queue>
#include <vector>
#include <limits>

#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>

void computeViewshed(Grid* elev_grid, Grid* vis_grid, int row, int col, int elevation){
    int new_elev = grid_get(elev_grid, row,col) + elevation;
    grid_set(elev_grid, row, col, new_elev);
    // Iterate through each cell to compute viewshed
    for (int r = 0; r < elev_grid->nrows; r++) {
        for (int c = 0; c < elev_grid->ncols; c++) {
            // Skip the observer point
            if (r == row && c == col) {
                continue;
            }

            // Check if the cell is visible from the observer
            // if (isVisible(elev_grid, row, col, r, c)) {
            grid_set(vis_grid, r, c, isVisible(elev_grid, row, col, r, c));
            // } else {
            //     grid_set_nodata(vis_grid, r, c);
            // }

        }
    }
}

int isVisible(Grid* grid, int source_r, int source_c, int target_r, int target_c) {
    double slope_r;
    double slope_c;
    int distance_r = target_r - source_r;
    int distance_c = target_c - source_c;

    if (distance_c != 0) {
    //     slope_r = 0.0;
    // } else {
        slope_r = (double)distance_r / double(abs(distance_c));
        // slope_c = (double)distance_c / abs(distance_r);
    }

    if (distance_r != 0) {
    //     slope_c = 
    // } else {
        // slope_r = (double)distance_r / abs(distance_c);
        slope_c = (double)distance_c / double(abs(distance_r));
    }

    double target_elevation = grid_get(grid, target_r, target_c);
    double source_elevation = grid_get(grid, source_r, source_c);

    // Calculate the angle of view between the observer and the target
    //double angle_of_view = 180.0+(180.0/ M_PI) * atan2(sqrt(distance_r * distance_r + distance_c * distance_c), source_elevation - target_elevation);
    double angle_of_view = (target_elevation - source_elevation) / sqrt(distance_r * distance_r + distance_c * distance_c);

   
    double weight;
    double field_of_view;
    double interp_height;
    int col;

    //first quadrant 
    if (distance_r >= 0) {
         double c;
        for (int r = 0; r < abs(distance_r); r++) {
            c = slope_c * r;
            col = (int)c;
            weight = c - (double)col;
            if (!weight) {
                interp_height = grid_get(grid, r + source_r, col + source_c);
            } else {
                // Horizontal interpolation (bilinear)
                double height1 = grid_get(grid, r + source_r, col + source_c);
                double height2 = grid_get(grid, r + source_r, col + source_c + 1);
                interp_height = height1 + (height2 - height1) * weight;
            }

            //field_of_view = 180.0+(180.0/ M_PI) * atan2(sqrt(r * r + c * c), source_elevation -interp_height);
            field_of_view = (interp_height -source_elevation) / sqrt(r * r + c * c);
            // Check if the angle of view is within the specified field of view
            if (angle_of_view < field_of_view) {
                return 0; // not visible within the field of view
            }
        }
    } else {
        double c;
        for (int r = 0; r < abs(distance_r); r++) {
            c = slope_c * r;
            col = (int)c; 
            weight = c - (double)col;

            if (!weight) {
                interp_height = grid_get(grid, -r + source_r, col + source_c);
            } else {
                // Horizontal interpolation (bilinear)
                double height1 = grid_get(grid, -r + source_r, col + source_c);
                double height2 = grid_get(grid, -r + source_r, col + source_c + 1);
                interp_height = height1 + (height2 - height1) * weight;
            }

            //field_of_view = 180.0+(180.0/ M_PI) * atan2(sqrt(r * r + c * c), source_elevation -interp_height);

            field_of_view = (interp_height -source_elevation) / sqrt(r * r + c * c);

            // Check if the angle of view is within the specified field of view
            if (angle_of_view < field_of_view) {
                return 0; // not visible within the field of view
            }
        }
    }
    
    if (distance_c >= 0) {
            double r;
            for (int c = 0; c < abs(distance_c); c++) {
                r = slope_r * c;
                int row = (int)r;
                weight = r - (double)row;

                if (!weight) {
                    interp_height = grid_get(grid, row + source_r, c + source_c);
                } else {
                    // Vertical interpolation (bilinear)
                    double height1 = grid_get(grid, row + source_r, c + source_c);
                    double height2 = grid_get(grid, row + source_r + 1, c + source_c);
                    interp_height = height1 + (height2 - height1) * weight;
                }

                //field_of_view = 180.0+(180.0/ M_PI) * atan2(sqrt(r * r + c * c), source_elevation -interp_height);
               field_of_view = (interp_height -source_elevation) / sqrt(r * r + c * c);
                // Check if the angle of view is within the specified field of view
                if (angle_of_view < field_of_view) {
                    return 0; // not visible within the field of view
                }
            }
        } else {
            double r;
            for (int c = 0; c < abs(distance_c); c++) {
                r = slope_r * c;
                int row = (int)r;
                weight = r - (double)row;

                if (!weight) {
                    interp_height = grid_get(grid, row + source_r, -c + source_c);
                } else {
                    // Vertical interpolation (bilinear)
                    double height1 = grid_get(grid, row + source_r, -c + source_c);
                    double height2 = grid_get(grid, row + source_r + 1, -c + source_c);
                    interp_height = height1 + (height2 - height1) * weight;
                }

                //field_of_view = 180.0+(180.0/ M_PI) * atan2(sqrt(r * r + c * c), source_elevation -interp_height);
                
               field_of_view = (interp_height -source_elevation) / sqrt(r * r + c * c);

                // Check if the angle of view is within the specified field of view
                if (angle_of_view < field_of_view) {
                    return 0; // not visible within the field of view
                }
            }
        }

    return 1; // it is visible
}












