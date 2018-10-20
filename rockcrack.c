
#include <math.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "util.h"
#include "phymodel.h"
#include "image.h"
#include "rock.h"

static void
phymodel_initialize_rock_simplecrack(struct phymodel* model,
				     enum crackdirection direction,
				     unsigned int startZ,
				     unsigned int zThickness,
				     int uniform,
				     unsigned int crackWidth,
				     unsigned int crackGrowthSteps);
static void
phymodel_initialize_rock_fractalcrack(struct phymodel* model,
				      enum crackdirection direction,
				      unsigned int startZ,
				      unsigned int zThickness,
				      unsigned int startX,
				      unsigned int xSize,
				      unsigned int startY,
				      unsigned int ySize,
				      int uniform,
				      unsigned int crackWidth,
				      unsigned int crackGrowthSteps,
				      double fractalShrink,
				      unsigned int fractalLevels,
				      unsigned int fractalCardinality);

struct phymodel*
phymodel_initialize_rock(unsigned int freeSpaceAboveRock,
			 unsigned int rockThickness,
			 enum rockinitialization style,
			 int uniform,
			 unsigned int crackWidth,
			 unsigned int crackGrowthSteps,
			 double fractalShrink,
			 unsigned int fractalLevels,
			 unsigned int fractalCardinality,
			 enum crackdirection direction,
			 int cave,
			 unsigned int unit,
			 unsigned int xSize,
			 unsigned int ySize,
			 unsigned int zSize) {
  
  unsigned int x;
  unsigned int y;
  unsigned int z;
  
  struct phymodel* model =
    phymodel_create(unit,
		    xSize,
		    ySize,
		    zSize);
  if (model == 0) return(0);

  debugf("initializing rock...");
  debugf("phymodel_initialize_rock param1 %u param2 %u param3 %f param4 %u",
	 crackWidth, crackGrowthSteps, fractalShrink, fractalCardinality);
  debugf("cave = %u", cave);
  
  /*
   * Fill the entire model with rock to the designated thickness
   */
  
  for (z = freeSpaceAboveRock; z < freeSpaceAboveRock + rockThickness && z < model->zSize; z++) {
    for (y = 0; y < model->ySize; y++) {
      for (x = 0; x < model->xSize; x++) {
	phymodel_set_rock_material(model,x,y,z);
      }
    }
  }
  
  /*
   * Clean out the crack
   */
  
  switch (style) {
  case rockinitialization_simplecrack:
    phymodel_initialize_rock_simplecrack(model,direction,
					 freeSpaceAboveRock,
					 rockThickness,
					 uniform,
					 crackWidth,
					 crackGrowthSteps);
    break;
  case rockinitialization_fractalcrack:
    phymodel_initialize_rock_fractalcrack(model,
					  direction,
					  freeSpaceAboveRock,
					  rockThickness,
					  0,
					  model->xSize,
					  0,
					  model->ySize,
					  uniform,
					  crackWidth,
					  crackGrowthSteps,
					  fractalShrink,
					  fractalLevels,
					  fractalCardinality);
    break;
  default:
    fatal("unrecognised rock creation style");
  }

  /*
   * Draw a cave tunnel underneath?
   */
  
  if (cave) {
    
    phymodel_initialize_rock_cavetunnel(model,
					direction,
					freeSpaceAboveRock + rockThickness);
    if (debug) {
      image_modelx2image(model,0,"debug.final1.x.txt");
      image_modely2image(model,32,"debug.final1.y.txt");
      image_modelz2image(model,freeSpaceAboveRock + rockThickness,"debug.final1.z.txt");
      image_modely2image(model,0,"debug.final1.jpg");
    }
    
  }
  
  /*
   * Done
   */
  
  return(model);
}

struct crackwidthentry {
  unsigned int leftsidewidth;
  unsigned int crackwidth;
};

struct crackstep {
  unsigned int length;
  unsigned int step;
};

#define maxNonUniformCrackWidthSteps 500
static unsigned int nonUniformCrackWidthSteps = 5;

static void
phymodel_initialize_rock_calculatecrackwidth_step(unsigned nSteps,
						  unsigned int space,
						  unsigned int maxwidth,
						  struct crackstep* steps) {
  assert(steps != 0);
  assert(nSteps > 0);
  unsigned int i;
  unsigned int usespace = space > 1 ? space/2 : space;
  unsigned int usemaxwidth = maxwidth > 1 ? maxwidth/2 : maxwidth;
  
  debugf("  calculating crack width with %u steps in a space of %u units, maxwidth %u", nSteps, space, maxwidth);
  
  for (i = 0; i < nSteps; i++) {
    unsigned int thisSpace = space > 0 ? 1 + (rand() % usespace) : 0;
    unsigned int thisStep = maxwidth > 0 ? 1 + (rand() % usemaxwidth) : 0;
    if (0) debugf("    %uth step is %u units further from center and step size is %u units",
		  i, thisSpace, thisStep);
    steps[i].length = thisSpace;
    steps[i].step = thisStep;
    space -= thisSpace;
    maxwidth -= thisStep;
  }
}

static unsigned int
phymodel_initialize_rock_calculatecrackwidth_stepwidth(unsigned int nSteps,
						       unsigned int distancefromcenter,
						       struct crackstep* steps) {
  if (nSteps == 0) {
    return(0);
  } else if (distancefromcenter < steps->length) {
    return(0);
  } else {
    return(steps->step +
	   phymodel_initialize_rock_calculatecrackwidth_stepwidth(nSteps - 1,
								  distancefromcenter - steps->length,
								  steps + 1));
  }
}

static struct crackwidthentry*
phymodel_initialize_rock_calculatecrackwidth(unsigned int length,
					     unsigned int width,
					     int uniform,
					     unsigned int crackWidth,
					     unsigned int crackGrowthSteps) {
  unsigned int crackwidth = (crackWidth < width) ? crackWidth : 1;
  unsigned int halfcrackwidthleft = crackwidth / 2;
  unsigned int halfcrackwidthright = crackwidth % 2 == 0 ? halfcrackwidthleft : halfcrackwidthleft + 1;
  unsigned int leftsidewidth = (width - crackwidth) / 2;
  struct crackwidthentry* table = (struct crackwidthentry*)malloc(length * sizeof(struct crackwidthentry));
  if (table == 0) {
    fatalu("cannot allocate a crack width table for entries", length);
    return(0);
  }

  if (uniform) {
    
    unsigned int i;
    debugf("creating a uniform crack of length %u", length);
    for (i = 0; i < length; i++) {
      table[i].leftsidewidth = leftsidewidth;
      table[i].crackwidth = crackwidth;
    }
    
  } else {
    
    struct crackstep stepsleftup[maxNonUniformCrackWidthSteps];
    struct crackstep stepsleftdown[maxNonUniformCrackWidthSteps];
    struct crackstep stepsrightup[maxNonUniformCrackWidthSteps];
    struct crackstep stepsrightdown[maxNonUniformCrackWidthSteps];
    unsigned int i;

    nonUniformCrackWidthSteps = crackGrowthSteps;
    if (nonUniformCrackWidthSteps > maxNonUniformCrackWidthSteps) {
      fataluu("cannot calculate non-uniform crack steps, max vs given number of steps",
	      maxNonUniformCrackWidthSteps,
	      nonUniformCrackWidthSteps);
      return(0);
    }
    
    debugf("creating a non-uniform crack of length %u with %u steps",
	   length,
	   nonUniformCrackWidthSteps);
    
    phymodel_initialize_rock_calculatecrackwidth_step(nonUniformCrackWidthSteps,length/2,halfcrackwidthleft,stepsleftup);
    phymodel_initialize_rock_calculatecrackwidth_step(nonUniformCrackWidthSteps,length/2,halfcrackwidthleft,stepsleftdown);
    phymodel_initialize_rock_calculatecrackwidth_step(nonUniformCrackWidthSteps,length/2,halfcrackwidthright,stepsrightup);
    phymodel_initialize_rock_calculatecrackwidth_step(nonUniformCrackWidthSteps,length/2,halfcrackwidthright,stepsrightdown);
    
    for (i = 0; i < length; i++) {
      if (i < length/2) {
	
	unsigned int crackleft =
	  subsorzero(halfcrackwidthleft,
		     phymodel_initialize_rock_calculatecrackwidth_stepwidth(nonUniformCrackWidthSteps,length/2 - i,stepsleftdown));
	unsigned int crackright =
	  subsorzero(halfcrackwidthright,
		     phymodel_initialize_rock_calculatecrackwidth_stepwidth(nonUniformCrackWidthSteps,length/2 - i,stepsrightdown));
	table[i].crackwidth = crackleft + crackright;
	table[i].leftsidewidth = width/2 - crackleft;
	
      } else {
	
	unsigned int crackleft = 
	  subsorzero(halfcrackwidthleft,
		     phymodel_initialize_rock_calculatecrackwidth_stepwidth(nonUniformCrackWidthSteps,i - length/2,stepsleftup));
	unsigned int crackright = 
	  subsorzero(halfcrackwidthright,
		     phymodel_initialize_rock_calculatecrackwidth_stepwidth(nonUniformCrackWidthSteps,i - length/2,stepsrightup));
	table[i].crackwidth = crackleft + crackright;
	table[i].leftsidewidth = width/2 - crackleft;
	
      }

      if (0) debugf("  crack at %u: crackwidth %u leftsidewidth %u",
		    i, table[i].crackwidth, table[i].leftsidewidth);
      
    }
    
  }
  
  return(table);
}

static void
phymodel_initialize_rock_simplecrack(struct phymodel* model,
				     enum crackdirection direction,
				     unsigned int startZ,
				     unsigned int zThickness,
				     int uniform,
				     unsigned int crackWidth,
				     unsigned int crackGrowthSteps) {

  /*
   * Allocate a table that shows the widths of the crack for each unit
   */
  
  unsigned int length = crackdirection_is_y(direction) ? model->ySize : model->xSize;
  unsigned int width = crackdirection_is_y(direction) ? model->xSize : model->ySize;
  struct crackwidthentry* widthtable =
    phymodel_initialize_rock_calculatecrackwidth(length,
						 width,
						 uniform,
						 crackWidth,
						 crackGrowthSteps);
  unsigned int x;
  unsigned int y;
  
  if (widthtable == 0) return;
  
  /*
   * Carve out the crack
   */
  
  switch (direction) {

  case crackdirection_y:

    /*
     * Draw the base crack (in y direction)
     */
    
    for (y = 0; y < model->ySize; y++) {
      for (x = widthtable[y].leftsidewidth;
	   x < widthtable[y].leftsidewidth + widthtable[y].crackwidth;
	   x++) {
	assert(x < model->xSize);
	phymodel_set_rock_crackmaterial_thickness(model,x,y,startZ,zThickness);
      }
    }

    break;
  
  case crackdirection_x:

    /*
     * Draw the base crack (in x direction)
     */
    
    for (x = 0; x < model->xSize; x++) {
      for (y = widthtable[x].leftsidewidth;
	   y < widthtable[x].leftsidewidth + widthtable[x].crackwidth;
	   y++) {
	assert(y < model->ySize);
	phymodel_set_rock_crackmaterial_thickness(model,x,y,startZ,zThickness);
      }
    }
    
    break;
    
  default:
    fatal("unrecognised crack direction");
    break;
  }
  
  /*
   * Free the table
   */
  
  free(widthtable);
}

static void
phymodel_initialize_rock_fractalcrack(struct phymodel* model,
				      enum crackdirection direction,
				      unsigned int startZ,
				      unsigned int zThickness,
				      unsigned int startX,
				      unsigned int xSize,
				      unsigned int startY,
				      unsigned int ySize,
				      int uniform,
				      unsigned int crackWidth,
				      unsigned int crackGrowthSteps,
				      double fractalShrink,
				      unsigned int fractalLevels,
				      unsigned int fractalCardinality) {

  
  /*
   * Allocate a table that shows the widths of the crack for each unit
   */

  unsigned int length = crackdirection_is_y(direction) ? ySize : xSize;
  unsigned int width = crackdirection_is_y(direction) ? xSize : ySize;
  debugf("phymodel_initialize_rock_fractalcrack param1 %u param2 %u param3 %f param4 %u",
	 crackWidth, crackGrowthSteps, fractalShrink, fractalCardinality);
  
  struct crackwidthentry* widthtable =
    phymodel_initialize_rock_calculatecrackwidth(length,
						 width,
						 uniform,
						 crackWidth,
						 crackGrowthSteps);
  unsigned int x;
  unsigned int y;
  
  if (widthtable == 0) return;
  
  /*
   * Carve out the crack
   */

  switch (direction) {

  case crackdirection_y:

    /*
     * Draw the base crack (in y direction)
     */
    
    debugf("crackdirection_y size %u x %u (start %u, %u)",
	   xSize, ySize, startX, startY);
    for (y = startY; y < startY + ySize && y < model->ySize; y++) {
      for (x = startX + widthtable[y-startY].leftsidewidth;
	   x < startX + widthtable[y-startY].leftsidewidth + widthtable[y-startY].crackwidth && x < model->xSize;
	   x++) {
	assert(x < startX + xSize);
	assert(x < model->xSize);
	phymodel_set_rock_crackmaterial_thickness(model,x,y,startZ,zThickness);
      }
    }

    /*
     * Recurse to the next level fractals
     */
    
    if (crackWidth > 0 && xSize > 1 && ySize > 1 && fractalLevels > 1) {
      unsigned int i;
      debugf("  generate %u fractal side cracks", fractalCardinality);
      for (i = 0; i < fractalCardinality; i++) {
	unsigned int atLength = rand() % length;
	if (widthtable[atLength].crackwidth == 0) {
	  i--;
	  continue;
	} else {
	  unsigned int center = length/2;
	  int difffromcenter = center - atLength;
	  unsigned int awayfromcenter = difffromcenter < 0 ? -difffromcenter : difffromcenter;
	  const double awayfactor = 1.3;
	  double awayfromcenterRedux = ((double)center - (awayfromcenter/awayfactor))/((double)center);
	  unsigned int xSizeRedux = (unsigned int)(((double)xSize) * fractalShrink * awayfromcenterRedux);
	  unsigned int ySizeRedux = (unsigned int)(((double)ySize) * fractalShrink * awayfromcenterRedux);
	  unsigned int xSizeSideDiff = (xSize - xSizeRedux) / 2;
	  unsigned int ySizeSideDiff = (ySize - ySizeRedux) / 2;
	  unsigned int crackWidthRedux = (unsigned int)(((double)crackWidth) * fractalShrink * awayfromcenterRedux);
	  debugf("    %uth fractal side crack is at length %u, diff to center %d, away from center %u (redux %f), crackwidth %u->%u",
		 i, atLength, difffromcenter, awayfromcenter, awayfromcenterRedux, crackWidth, crackWidthRedux);
	  phymodel_initialize_rock_fractalcrack(model,
						crackdirection_x,
						startZ,
						zThickness,
						startX + xSizeSideDiff,
						xSizeRedux,
						startY + ySizeSideDiff + difffromcenter,
						ySizeRedux,
						uniform,
						crackWidthRedux,
						crackGrowthSteps,
						fractalShrink,
						fractalLevels - 1,
						fractalCardinality);
	}
      }
    }
    break;

  case crackdirection_x:

    /*
     * Draw the base crack (in x direction)
     */
    
    debugf("crackdirection_x size %u x %u (start %u, %u)", xSize, ySize, startX, startY);
    debugf("x from %u to %u+%u = %u (full size %u)",
	   startX, startX, xSize, startX + xSize, model->xSize);
    for (x = startX; x < startX + xSize && x < model->xSize; x++) {
      if (0) debugf("at x %u, sidewidth is %u and crackwidth %u (startY=%u)",
		    x, widthtable[x-startX].leftsidewidth, widthtable[x-startX].crackwidth, startY);
      for (y = startY + widthtable[x-startX].leftsidewidth;
	   y < startY + widthtable[x-startX].leftsidewidth + widthtable[x-startX].crackwidth && y < model->ySize;
	   y++) {
	assert(y < startY + ySize);
	assert(y < model->ySize);
	phymodel_set_rock_crackmaterial_thickness(model,x,y,startZ,zThickness);
      }
    }

    /*
     * Recurse to the next level fractals
     */
    
    if (crackWidth > 0 && xSize > 1 && ySize > 1 && fractalLevels > 1) {
      unsigned int i;
      debugf("  generate %u fractal side cracks", fractalCardinality);
      for (i = 0; i < fractalCardinality; i++) {
	unsigned int atLength = rand() % length;
	if (widthtable[atLength].crackwidth == 0) {
	  i--;
	  continue;
	} else {
	  unsigned int center = length/2;
	  int difffromcenter = center - atLength;
	  unsigned int awayfromcenter = difffromcenter < 0 ? -difffromcenter : difffromcenter;
	  const double awayfactor = 1.3;
	  double awayfromcenterRedux = ((double)center - (awayfromcenter/awayfactor))/((double)center);
	  unsigned int xSizeRedux = (unsigned int)(((double)xSize) * fractalShrink * awayfromcenterRedux);
	  unsigned int ySizeRedux = (unsigned int)(((double)ySize) * fractalShrink * awayfromcenterRedux);
	  unsigned int xSizeSideDiff = (xSize - xSizeRedux) / 2;
	  unsigned int ySizeSideDiff = (ySize - ySizeRedux) / 2;
	  unsigned int crackWidthRedux = (unsigned int)(((double)crackWidth) * fractalShrink * awayfromcenterRedux);
	  debugf("    %uth fractal side crack is at length %u, diff to center %d, away from center %u (redux %f), crackwidth %u->%u",
		 i, atLength, difffromcenter, awayfromcenter, awayfromcenterRedux, crackWidth, crackWidthRedux);
	  phymodel_initialize_rock_fractalcrack(model,
						crackdirection_y,
						startZ,
						zThickness,
						startX + xSizeSideDiff + difffromcenter,
						xSizeRedux,
						startY + ySizeSideDiff,
						ySizeRedux,
						uniform,
						crackWidthRedux,
						crackGrowthSteps,
						fractalShrink,
						fractalLevels - 1,
						fractalCardinality);
	}
      }
    }
    break;
    
  default:
    fatal("unrecognised crack direction");
    break;
  }
  
  /*
   * Free the table
   */
  
  free(widthtable);
}


