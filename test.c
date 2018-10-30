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
#include "coords.h"

static void atomtests(void);
static void phymodeltests(void);
static void circlemaptests(void);

int
main(int argc,
     char** argv) {
  if (argc > 1) debug = 1;
  atomtests();
  phymodeltests();
  circlemaptests();
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

  /*
   * 2D
   */
  
  dist = phymodel_distance2d(0,0,0,0);
  assert(approxcompare(0.0,0.001,dist));
  dist = phymodel_distance2d(1,0,0,0);
  assert(approxcompare(1.0,0.001,dist));
  dist = phymodel_distance2d(1,1,0,0);
  assert(approxcompare(1.4,0.1,dist));

  /*
   * 3D
   */
  
  dist = phymodel_distance3d(0,0,0,0,0,0);
  assert(approxcompare(0.0,0.001,dist));
  dist = phymodel_distance3d(1,0,0,0,0,0);
  assert(approxcompare(1.0,0.001,dist));
  dist = phymodel_distance3d(1,1,1,0,0,0);
  assert(approxcompare(1.7,0.1,dist));
}

static unsigned int ntab;
static struct atomcoordinates tab[100];

static void
circlemaptestsaux(unsigned int x,
		  unsigned int y,
		  unsigned int z,
		  struct phymodel* model,
		  phyatom* atom,
		  void* data) {
  assert(ntab < sizeof(tab)/sizeof(struct atomcoordinates));
  tab[ntab].x = x;
  tab[ntab].y = y;
  tab[ntab].z = z;
  ntab++;
}

static const char*
circlemapteststabstring() {
  static char buf[5000];
  unsigned int i;
  memset(buf,0,sizeof(buf));
  for (i = 0; i < ntab; i++) {
    char onebuf[100];
    snprintf(onebuf,sizeof(onebuf)-1,"(%u,%u,%u)",
	     tab[i].x,
	     tab[i].y,
	     tab[i].z);
    if (buf[0] != 0) strncat(buf,",",sizeof(buf)-1);
    strncat(buf,onebuf,sizeof(buf)-1);
  }
  return(buf);
}
  
static void
circlemaptests(void) {
  
  struct phymodel* m = phymodel_create(1,3,3,3);
  const char* string;
  assert(m->xSize == 3);
  assert(m->ySize == 3);
  assert(m->zSize == 3);
  
  /*
   * 2D
   */
  
  ntab = 0;
  phymodel_mapatoms_atdistance2dz(m,1,1,1,1,circlemaptestsaux,(void*)0);
  string = circlemapteststabstring();
  debugf("tab = %s", string);
  assert(strcmp(string,"(0,0,1),(0,1,1),(0,2,1),(1,0,1),(1,2,1),(2,0,1),(2,1,1),(2,2,1)") == 0);
  
  ntab = 0;
  phymodel_mapatoms_atdistance2dz(m,2,2,1,1,circlemaptestsaux,(void*)0);
  string = circlemapteststabstring();
  debugf("tab = %s", string);
  assert(strcmp(string,"(1,1,1),(1,2,1),(2,1,1)") == 0);
  
  /*
   * 3D
   */
  
  ntab = 0;
  phymodel_mapatoms_atdistance3d(m,1,1,1,1,circlemaptestsaux,(void*)0);
  string = circlemapteststabstring();
  debugf("tab = %s", string);
  assert(strcmp(string,"(0,0,0),(0,0,1),(0,0,2),(0,1,0),(0,1,1),(0,1,2),(0,2,0),(0,2,1),(0,2,2),(1,0,0),(1,0,1),(1,0,2),(1,1,0),(1,1,2),(1,2,0),(1,2,1),(1,2,2),(2,0,0),(2,0,1),(2,0,2),(2,1,0),(2,1,1),(2,1,2),(2,2,0),(2,2,1),(2,2,2)") == 0);
}
