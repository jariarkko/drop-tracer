
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

#ifndef COORDS_H
#define COORDS_H

struct atomcoordinates {
  unsigned int x;
  unsigned int y;
  unsigned int z;
};

struct atomboundingbox {
  struct atomcoordinates lowercorner;
  struct atomcoordinates uppercorner;
};

int
simulator_coords_equal(struct atomcoordinates* coord1,
		       struct atomcoordinates* coord2);
int
simulator_coords_adjacent(struct atomcoordinates* coord1,
			  struct atomcoordinates* coord2);

#endif /* COORDS_H */
