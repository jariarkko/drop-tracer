
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

#include <math.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "util.h"
#include "coords.h"

int
simulator_coords_equal(struct atomcoordinates* coord1,
		       struct atomcoordinates* coord2) {
  return(coord1->x == coord2->x &&
	 coord1->y == coord2->y &&
	 coord1->z == coord2->z);
}

static int
simulator_coords_onecoordisadjacent(unsigned int a,
				    unsigned int b) {
  if (a == b) return(1);
  if (b > 0 && a == b - 1) return(1);
  if (a == b + 1) return(1);
  return(0);
}

int
simulator_coords_adjacent(struct atomcoordinates* coord1,
			  struct atomcoordinates* coord2) {
  return(simulator_coords_onecoordisadjacent(coord1->x,coord2->x) &&
	 simulator_coords_onecoordisadjacent(coord1->y,coord2->y) &&
	 simulator_coords_onecoordisadjacent(coord1->z,coord2->z));
}
