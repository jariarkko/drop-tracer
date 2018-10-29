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
#include "util.h"
#include "phymodel.h"
#include "image.h"
#include "rock.h"

void
phymodel_initialize_rock_cavetunnel(struct phymodel* model,
				    enum crackdirection direction,
				    unsigned int startZ) {

  /*
   * Calculate basic characteristics, wall thickness etc.
   */
  
  unsigned int wallthicknessmin = 10;
  unsigned int wallthicknessfraction = 100;
  unsigned int wallthicknessasfraction = model->xSize / wallthicknessfraction;
  unsigned int wallthickness = (wallthicknessasfraction < wallthicknessmin) ? wallthicknessmin : wallthicknessasfraction;
  
  /*
   * Draw the cave as an ellipse, calculate the middle two points of
   * the ellipse
   */
  
  unsigned int verticalcenter = startZ + (model->zSize - startZ - wallthickness) / 2;
  unsigned int horizontalleftcenter = wallthickness + (model->xSize - 2*wallthickness) / 3;
  unsigned int horizontalrightcenter =  wallthickness + (2 * (model->xSize - 2*wallthickness)) / 3;

  /*
   * Calculate the ellipse size parameter, the sum of the two
   * distances from the two middle points into the ellipse line.
   *
   * In our case we will adjust this a bit, because we want to fit the
   * ellipse top and bottom to the top and bottom of the space we have
   * in the model (and to the cave roof we have already created). And
   * to the sides of the space we have in the model, while leaving
   * some wall on all sides.
   */
  
  unsigned int leftroofstart = wallthickness + (model->xSize - 2*wallthickness) / 4;
  unsigned int leftfloorstart = wallthickness + (model->xSize - 2*wallthickness) / 3;
  double ellipsedistance =
    phymodel_distance2d(horizontalleftcenter,
			verticalcenter,
			wallthickness,
			verticalcenter) +
    phymodel_distance2d(horizontalrightcenter,
			verticalcenter,
			wallthickness,
			verticalcenter);
  double ellipsedistancetoroof = 
    phymodel_distance2d(horizontalleftcenter,
			verticalcenter,
			leftroofstart,
			startZ) +
    phymodel_distance2d(horizontalrightcenter,
			verticalcenter,
			leftroofstart,
			startZ);
  double ellipsedistancetofloor = 
    phymodel_distance2d(horizontalleftcenter,
			verticalcenter,
			leftfloorstart,
			model->zSize - wallthickness) +
    phymodel_distance2d(horizontalrightcenter,
			verticalcenter,
			leftfloorstart,
			model->zSize - wallthickness);
  double ellipsedistanceroofdifference =
    ellipsedistancetoroof - ellipsedistance;
  double ellipsedistancefloordifference =
    ellipsedistancetofloor - ellipsedistance;
  double* ellipseddistancesperz = (double*)malloc(model->zSize * sizeof(double));
  unsigned int y;
  unsigned int z;
  
  /*
   * Go through every height in the cave tunnel (z coordinate) and determine
   * the specific ellipse distance seperately for them, as we adjust the
   * distance towards the top and the bottom.
   */

  if (ellipseddistancesperz == 0) {
    fatalu("cannot allocate space for ellipse distance table of entries", model->zSize);
    return;
  }
  
  for (z = startZ; z < model->zSize; z++) {
    double ellipsedistancehere = ellipsedistance;
    if (z < verticalcenter) {
      
      if (ellipsedistancetoroof > ellipsedistancehere) {
	ellipsedistancehere += (((double)(verticalcenter - z)) / ((double)(verticalcenter - startZ))) * ellipsedistanceroofdifference;
      }
      
    } else {
	
      if (ellipsedistancetofloor > ellipsedistancehere) {
	ellipsedistancehere += (((double)(z - verticalcenter)) / ((double)((model->zSize - wallthickness) - verticalcenter))) * ellipsedistancefloordifference;
      }
      
    }
    
    ellipseddistancesperz[z] = ellipsedistancehere;
  }
  
  /*
   * Draw the actual ellipse
   */
  
  debugf("cave tunnel midpoints (%u,%u) and (%u,%u), startz = %u, wallthickness = %u",
	 horizontalleftcenter,verticalcenter,
	 horizontalrightcenter,verticalcenter,
	 startZ,
	 wallthickness);
  debugf("ellipse distance %f (out of %ux%u)", ellipsedistance, model->xSize, model->ySize);
  
  for (y = 0; y < model->ySize; y++) {

    for (z = startZ; z < model->zSize; z++) {

      double ellipsedistancehere = ellipseddistancesperz[z];
      unsigned int x;
      
      for (x = 0; x < model->xSize; x++) {
	
	if (x < wallthickness ||
	    x >= model->xSize - wallthickness ||
	    z >= model->zSize - wallthickness) {
	  
	  phymodel_set_rock_material(model,x,y,z);
	  
	} else {
	  
	  double distancetoleft = phymodel_distance2d(x,z,horizontalleftcenter,verticalcenter);
	  double distancetorigth = phymodel_distance2d(x,z,horizontalrightcenter,verticalcenter);
	  double totaldistance = distancetoleft + distancetorigth;
	  if (totaldistance > ellipsedistancehere) {
	    phymodel_set_rock_material(model,x,y,z);
	  }
	  
	}
      }
    }
  }

  /*
   * Cleanup
   */
  
  free(ellipseddistancesperz);
}
