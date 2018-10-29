/*
 * **************************************************************************
 * ****************************                     *************************
 * ***************************  D R O P T R A C E R  ************************
 * ****************************                     *************************
 * **************************************************************************
 * *******      *****    *****  **    *************      ***    ****  *******
 * ****	  	 *** 	  ***  	**     	*********      	 ***   	 ** 	 ****
 * *   	       	  *    	   * 	**	  *****	       	 ** 	 **	    *
 *  		   	   *   	 O     	   *** 		 **	  *
 * 		  	   o   	       	   ***		 **	  *
 *     	       	       	    		    * 		  *
 * 		  			    o		  *
 * 		  			     		  o
 * 		  			    o
 *
 *
 *                          Cave Forms Simulation Software
 *                                Jari Arkko, 2018
 *
 *                      https://github.com/jariarkko/drop-tracer
 *                              License: BSD 3-Clause
 *
 */

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include "util.h"
#include "phymodel.h"
#include "image.h"

static void atomtests(void);
static void phymodeltests(void);

int
main(int argc,
     char** argv) {
  if (argc > 1) debug = 1;
  atomtests();
  phymodeltests();
  exit(0);
}

static void
atomtests(void) {
  struct rgb color1;
  struct rgb color2;
  phyatom atom1;

  assert(sizeof(atom1) == 1);
  phyatom_reset(&atom1);
  phyatom_color(&color1,&atom1);
  assert(color1.r == 0);
  assert(color1.g == 0);
  assert(color1.b == 0);
  rgb_set_white(&color1);
  phyatom_set_color(&atom1,&color1);
  debugf("atom = %02x, color = %02x.%02x.%02x\n", atom1, color1.r, color1.g, color1.b);
  debugf("rgb = %02x\n", phyatom_color_rgb(&atom1));
  debugf("rgb_r = %02x\n", phyatom_color_rgb_r(&atom1));
  debugf("rgb_g = %02x\n", phyatom_color_rgb_g(&atom1));
  debugf("rgb_b = %02x\n", phyatom_color_rgb_b(&atom1));
  assert(phyatom_shortrgbtolong(phyatom_color_rgb_r(&atom1)) == 0xFF);
  assert(phyatom_shortrgbtolong(phyatom_color_rgb_g(&atom1)) == 0xFF);
  assert(phyatom_shortrgbtolong(phyatom_color_rgb_b(&atom1)) == 0xFF);
  rgb_set_blue(&color1);
  phyatom_set_color(&atom1,&color1);
  phyatom_color(&color2,&atom1);
  assert(color2.r == 0);
  assert(color2.g == 0);
  assert(color2.b == 0xFF);
  phyatom_set_mat(&atom1,material_water);
  phyatom_color(&color2,&atom1);
  assert(color2.r == 0);
  assert(color2.g == 0);
  assert(color2.b == 0xFF);
  phyatom_set_mat(&atom1,material_rock);
  phyatom_color(&color2,&atom1);
  assert(color2.r == 0);
  assert(color2.g == 0);
  assert(color2.b == 0xFF);
}

static int
approxcompare(double expect,
	      double maxerror,
	      double value) {
  if (debug) debugf("is value %f in range of %f +-%f?", value, expect, maxerror);
  if (value == expect) return(1);
  if (value < expect - maxerror) return(0);
  if (value > expect + maxerror) return(0);
  return(1);
}

static void
phymodeltests(void) {
  double dist;

  dist = phymodel_distance2d(0,0,0,0);
  assert(approxcompare(0.0,0.001,dist));
  dist = phymodel_distance2d(1,0,0,0);
  assert(approxcompare(1.0,0.001,dist));
  dist = phymodel_distance2d(1,1,0,0);
  assert(approxcompare(1.4,0.1,dist));
}
