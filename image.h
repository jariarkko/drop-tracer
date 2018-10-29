/*
 * **************************************************************************
 * ****************************                     *************************
 * ***************************  D R O P T R A C E R  ************************
 * ****************************                     *************************
 * **************************************************************************
 * *******      *****    *****  **    *************      ***    ****  *******
 * ****          ***      ***   **      *********        ***     **      ****
 * *              *        *    **        *****          **      **         *
 *                         *     O         ***           **       *
 *                         o               ***           **       *
 *                                          *             *
 *                                          o             *
 *                                                        o
 *                                          o
 *
 *
 *                          Cave Forms Simulation Software
 *                                Jari Arkko, 2018
 *
 *                      https://github.com/jariarkko/drop-tracer
 *                              License: BSD 3-Clause
 *
 */

#ifndef IMAGE_H
#define IMAGE_H

#include "phymodel.h"

enum coordinatetype {
  coordinatetype_z,
  coordinatetype_x,
  coordinatetype_y
  
};

void
image_modelz2image(struct phymodel* model,
		   unsigned int z,
		   const char* filename);
void
image_modelx2image(struct phymodel* model,
		   unsigned int x,
		   const char* filename);
void
image_modely2image(struct phymodel* model,
		   unsigned int y,
		   const char* filename);
void
image_model2image3d(struct phymodel* model,
		     const char* filename);

#endif /* IMAGE_H */
