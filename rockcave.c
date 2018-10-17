
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

  unsigned int wallthickness = 10;
  unsigned int verticalcenter = startZ + (model->zSize - startZ - wallthickness) / 2;
  unsigned int horizontalleftcenter = wallthickness + (model->xSize - 2*wallthickness) / 3;
  unsigned int horizontalmiddlecenter = wallthickness + (model->xSize - 2*wallthickness) / 2;
  unsigned int horizontalrightcenter =  wallthickness + (2 * (model->xSize - 2*wallthickness)) / 3;
  double ellipsedistance = 3.0 * phymodel_distance2d(horizontalleftcenter,
						     verticalcenter,
						     horizontalmiddlecenter,
						     verticalcenter);
  
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
    debugf("  setting rock at (%u,%u,%u)", horizontalleftcenter,y,verticalcenter);
    phymodel_set_rock_material(model,horizontalleftcenter,y,verticalcenter);
    phymodel_set_rock_material(model,horizontalrightcenter,y,verticalcenter);
    if (y == 0 && debug) {
      image_modelx2image(model,0,"debug.x.txt");
      image_modely2image(model,y,"debug.y.txt");
      image_modelz2image(model,startZ,"debug.z.txt");
      image_modely2image(model,0,"debug.jpg");
    }
    for (z = startZ; z < model->zSize; z++) {
      /* if (y == 32) debugf("z plane %u", z); */
      for (x = 0; x < model->xSize; x++) {
	double distancetoleft = phymodel_distance2d(x,z,horizontalleftcenter,verticalcenter);
	double distancetorigth = phymodel_distance2d(x,z,horizontalrightcenter,verticalcenter);
	double totaldistance = distancetoleft + distancetorigth;
	if (totaldistance > ellipsedistance) {
	  /* if (y == 32) debugf("  tunnel rock (%u,%u,%u)", x,y,z); */
	  phymodel_set_rock_material(model,x,y,z);
	}
      }
    }
  }
  
  if (debug) {
    image_modelx2image(model,0,"debug.final0.x.txt");
    image_modely2image(model,32,"debug.final0.y.txt");
    image_modelz2image(model,startZ,"debug.final0.z.txt");
    image_modely2image(model,0,"debug.final0.jpg");
  }
}
