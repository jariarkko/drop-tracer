
#include <math.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "util.h"
#include "phymodel.h"

static void
phymodel_initialize_rock_material(struct phymodel* model,
				  unsigned int x,
				  unsigned int y,
				  unsigned int z);
static void
phymodel_create_initatom(unsigned int x,
			 unsigned int y,
			 unsigned int z,
			 struct phymodel* model,
			 struct phyatom* atom,
			 void* data);
static void
phymodel_initialize_rock_simplecrack(struct phymodel* model,
				     unsigned int startZ,
				     unsigned int zThickness,
				     int uniform,
				     unsigned int styleParam1,
				     unsigned int styleParam2);
static void
phymodel_initialize_rock_fractalcrack(struct phymodel* model,
				      unsigned int startZ,
				      unsigned int zThickness,
				      int uniform,
				      unsigned int styleParam1,
				      unsigned int styleParam2);

struct phymodel*
phymodel_create(unsigned int unit,
		unsigned int xSize,
		unsigned int ySize,
		unsigned int zSize) {
  
  unsigned int size = phymodel_sizeinbytes(xSize,ySize,zSize);
  struct phymodel* model = (struct phymodel*)malloc(size);
  
  if (model == 0) {
    fatalu("cannot allocate model for bytes",size);
  }

  model->magic = PHYMODEL_MAGIC;
  model->unit = unit;
  model->xSize = xSize;
  model->ySize = ySize;
  model->zSize = zSize;
  
  phymodel_mapatoms(model,phymodel_create_initatom,0);
  
  return(model);
}

static void
phymodel_create_initatom(unsigned int x,
			 unsigned int y,
			 unsigned int z,
			 struct phymodel* model,
			 struct phyatom* atom,
			 void* data) {
  atom->mat = material_air;
  rgb_set_black(&atom->color);
}

struct phymodel*
phymodel_initialize_rock(enum rockinitialization style,
			 int uniform,
			 unsigned int styleParam1,
			 unsigned int styleParam2,
			 unsigned int unit,
			 unsigned int xSize,
			 unsigned int ySize,
			 unsigned int zSize) {
  
  unsigned int freeSpaceAboveRock = zSize > 30 ? 10 : 1;
  unsigned int rockThickness = zSize > 30 ? 10 : 1;
  
  struct phymodel* model =
    phymodel_create(unit,
		    xSize,
		    ySize,
		    zSize);
  if (model == 0) return(0);

  debugf("initializing rock...");
  switch (style) {
  case rockinitialization_simplecrack:
    phymodel_initialize_rock_simplecrack(model,freeSpaceAboveRock,rockThickness,uniform,styleParam1,styleParam2);
    break;
  case rockinitialization_fractalcrack:
    phymodel_initialize_rock_fractalcrack(model,freeSpaceAboveRock,rockThickness,uniform,styleParam1,styleParam2);
    break;
  default:
    fatal("unrecognised rock creation style");
  }
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
    debugf("    %uth step is %u units further from center and step size is %u units",
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
					     unsigned int styleParam1,
					     unsigned int styleParam2) {
  unsigned int crackwidth = (styleParam1 < width) ? styleParam1 : 1;
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

    nonUniformCrackWidthSteps = styleParam2;
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
	  halfcrackwidthleft -
	  phymodel_initialize_rock_calculatecrackwidth_stepwidth(nonUniformCrackWidthSteps,length/2 - i,stepsleftdown);
	unsigned int crackright =
	  halfcrackwidthright -
	  phymodel_initialize_rock_calculatecrackwidth_stepwidth(nonUniformCrackWidthSteps,length/2 - i,stepsrightdown);
	table[i].crackwidth = crackleft + crackright;
	table[i].leftsidewidth = width/2 - crackleft;
	
      } else {
	
	unsigned int crackleft = 
	  halfcrackwidthleft -
	  phymodel_initialize_rock_calculatecrackwidth_stepwidth(nonUniformCrackWidthSteps,i - length/2,stepsleftup);
	unsigned int crackright = 
	  halfcrackwidthright -
	  phymodel_initialize_rock_calculatecrackwidth_stepwidth(nonUniformCrackWidthSteps,i - length/2,stepsrightup);
	table[i].crackwidth = crackleft + crackright;
	table[i].leftsidewidth = width/2 - crackleft;
	
      }
    }
    
  }
  
  return(table);
}

static void
phymodel_initialize_rock_simplecrack(struct phymodel* model,
				     unsigned int startZ,
				     unsigned int zThickness,
				     int uniform,
				     unsigned int styleParam1,
				     unsigned int styleParam2) {

  struct crackwidthentry* widthtable =
    phymodel_initialize_rock_calculatecrackwidth(model->ySize,
						 model->xSize,
						 uniform,
						 styleParam1,
						 styleParam2);
  unsigned int x;
  unsigned int y;
  unsigned int z;

  if (widthtable == 0) return;
  for (z = startZ; z < startZ + zThickness; z++) {
    assert(z < model->zSize);
    for (y = 0; y < model->ySize; y++) {
      for (x = 0; x < widthtable[y].leftsidewidth; x++) {
	assert(x < model->xSize);
	phymodel_initialize_rock_material(model,x,y,z);
      }
      for (x = widthtable[y].leftsidewidth + widthtable[y].crackwidth; x < model->xSize; x++) {
	phymodel_initialize_rock_material(model,x,y,z);
      }
    }
  }
  
}

static void
phymodel_initialize_rock_fractalcrack(struct phymodel* model,
				      unsigned int startZ,
				      unsigned int zThickness,
				      int uniform,
				      unsigned int styleParam1,
				      unsigned int styleParam2) {

  unsigned int crackwidth = (styleParam1 < model->xSize) ? styleParam1 : 1;
  unsigned int crackleftsidewidth = (model->xSize - crackwidth) / 2;
  unsigned int x;
  unsigned int y;
  unsigned int z;
  
  for (z = startZ; z < startZ + zThickness; z++) {
    assert(z < model->zSize);
    for (y = 0; y < model->ySize; y++) {
      for (x = 0; x < crackleftsidewidth; x++) {
	assert(x < model->xSize);
	phymodel_initialize_rock_material(model,x,y,z);
      }
      for (x = crackleftsidewidth + crackwidth; x < model->xSize; x++) {
	phymodel_initialize_rock_material(model,x,y,z);
      }
    }
  }
  
}

static void
phymodel_initialize_rock_material(struct phymodel* model,
				  unsigned int x,
				  unsigned int y,
				  unsigned int z) {
  unsigned int atomindex = phymodel_atomindex(model,x,y,z);
  struct phyatom* atom = &model->atoms[atomindex];
  atom->mat = material_rock;
  rgb_set_white(&atom->color);
}

void
phymodel_mapatoms(struct phymodel* model,
		  phyatom_fn fn,
		  void* data) {
  
  unsigned int x;
  unsigned int y;
  unsigned int z;
  
  assert(fn != 0);
  for (z = 0; z < model->zSize; z++) {
    for (y = 0; y < model->ySize; y++) {
      for (x = 0; x < model->xSize; x++) {
	unsigned int atomIndex = phymodel_atomindex(model,x,y,z);
	struct phyatom* atom = &model->atoms[atomIndex];
	(*fn)(x,
	      y,
	      z,
	      model,
	      atom,
	      data);
      }
    }
  }
}
  
void
phymodel_mapatoms_atz(struct phymodel* model,
		      unsigned int z,
		      phyatom_fn fn,
		      void* data) {
  
  unsigned int x;
  unsigned int y;
  
  assert(fn != 0);
  for (y = 0; y < model->ySize; y++) {
    for (x = 0; x < model->xSize; x++) {
      unsigned int atomIndex = phymodel_atomindex(model,x,y,z);
      struct phyatom* atom = &model->atoms[atomIndex];
      (*fn)(x,
	    y,
	    z,
	    model,
	    atom,
	    data);
    }
  }
}

void
phymodel_mapatoms_atx(struct phymodel* model,
		      unsigned int x,
		      phyatom_fn fn,
		      void* data) {
  
  unsigned int z;
  unsigned int y;
  
  assert(fn != 0);
  for (z = 0; z < model->zSize; z++) {
    for (y = 0; y < model->ySize; y++) {
      unsigned int atomIndex = phymodel_atomindex(model,x,y,z);
      struct phyatom* atom = &model->atoms[atomIndex];
      (*fn)(x,
	    y,
	    z,
	    model,
	    atom,
	    data);
    }
  }
}

void
phymodel_mapatoms_aty(struct phymodel* model,
		      unsigned int y,
		      phyatom_fn fn,
		      void* data) {
  
  unsigned int z;
  unsigned int x;
  
  assert(fn != 0);
  for (z = 0; z < model->zSize; z++) {
    for (x = 0; x < model->xSize; x++) {
      unsigned int atomIndex = phymodel_atomindex(model,x,y,z);
      struct phyatom* atom = &model->atoms[atomIndex];
      (*fn)(x,
	    y,
	    z,
	    model,
	    atom,
	    data);
    }
  }
}

void
phymodel_destroy(struct phymodel* model) {
  assert(model->magic == PHYMODEL_MAGIC);
  model->magic = 0;
  free(model);
}

struct phymodel*
phymodel_read(const char* filename) {

  struct phymodel* model = 0;
  unsigned int sz = 0;
  size_t ret = 0;
  
  /*
   * Open file
   */
  
  FILE* f = fopen(filename,"r");
  if (f == 0) {
    fatals("failed to open file", filename);
    return(0);
  }

  /*
   * Determine size
   */
  
  fseek(f, 0L, SEEK_END);
  sz = ftell(f);
  fseek(f, 0L, SEEK_SET);

  /*
   * Check size
   */

  if (sz < sizeof(struct phymodel)) {
    fclose(f);
    fatalsu("file is too short to be a model", filename, sz);
    return(0);
  }
  
  /*
   * Allocate model
   */
  
  model = (struct phymodel*)malloc(sz);
  if (model == 0) {
    fclose(f);
    fatalsu("cannot allocate memory for file contents", filename, sz);
    return(0);
  }
  
  /*
   * Read model
   */
  
  ret = fread(model,sz,1,f);
  
  if (ret != 1) {
    fclose(f);
    fatalsu("unable to read file contents and bytes", filename, sz);
    return(0);
  }
  
  if (model->magic != PHYMODEL_MAGIC) {
    fclose(f);
    fatalxx("file does not contain right magic number for a model",model->magic,PHYMODEL_MAGIC);
    return(0);
  }
  
  /*
   * Cleanup and return
   */
  
  fclose(f);
  return(model);
}

void
phymodel_write(struct phymodel* model,
	       const char* filename) {
  FILE* f = fopen(filename,"w");
  unsigned int size;
  size_t ret;
  
  if (f == 0) {
    fatals("failed to open file", filename);
    return;
  }
  
  size = phymodel_sizeinbytes(model->xSize,
			      model->ySize,
			      model->zSize);
  ret = fwrite(model,size,1,f);
  if (ret != 1) {
    fatalsu("failed to write all model bytes to file",
	    filename,
	    size);
    fclose(f);
    return;
  }
  fclose(f);
}

void
rgb_set(struct rgb* rgb,
	unsigned char r,
	unsigned char g,
	unsigned char b) {
  rgb->r = r;
  rgb->g = g;
  rgb->b = b;
}

void
rgb_set_black(struct rgb* rgb) {
  rgb->r = rgb->g = rgb->b = 0x00;
}

void
rgb_set_white(struct rgb* rgb) {
  rgb->r = rgb->g = rgb->b = 0xFF;
}
