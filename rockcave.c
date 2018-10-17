
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

  unsigned int wallthicknessmin = 10;
  unsigned int wallthicknessfraction = 100;
  unsigned int wallthicknessasfraction = model->xSize / wallthicknessfraction;
  unsigned int wallthickness = (wallthicknessasfraction < wallthicknessmin) ? wallthicknessmin : wallthicknessasfraction;
  unsigned int verticalcenter = startZ + (model->zSize - startZ - wallthickness) / 2;
  unsigned int horizontalleftcenter = wallthickness + (model->xSize - 2*wallthickness) / 3;
  unsigned int horizontalrightcenter =  wallthickness + (2 * (model->xSize - 2*wallthickness)) / 3;
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
  unsigned int y;
  unsigned int z;
  unsigned int x;
  
  debugf("cave tunnel midpoints (%u,%u) and (%u,%u), startz = %u, wallthickness = %u",
	 horizontalleftcenter,verticalcenter,
	 horizontalrightcenter,verticalcenter,
	 startZ,
	 wallthickness);
  debugf("ellipse distance %f (out of %ux%u)", ellipsedistance, model->xSize, model->ySize);
  for (y = 0; y < model->ySize; y++) {
    
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
}
