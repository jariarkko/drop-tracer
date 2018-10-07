
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
				     unsigned int styleParam);
static void
phymodel_initialize_rock_fractalcrack(struct phymodel* model,
				      unsigned int startZ,
				      unsigned int zThickness,
				      int uniform,
				      unsigned int styleParam);

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
			 unsigned int styleParam,
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
  switch (style) {
  case rockinitialization_simplecrack:
    phymodel_initialize_rock_simplecrack(model,freeSpaceAboveRock,rockThickness,uniform,styleParam);
    break;
  case rockinitialization_fractalcrack:
    phymodel_initialize_rock_fractalcrack(model,freeSpaceAboveRock,rockThickness,uniform,styleParam);
    break;
  default:
    fatal("unrecognised rock creation style");
  }
  return(model);
}
 
static void
phymodel_initialize_rock_simplecrack(struct phymodel* model,
				     unsigned int startZ,
				     unsigned int zThickness,
				     int uniform,
				     unsigned int styleParam) {

  unsigned int crackwidth = (styleParam < model->xSize) ? styleParam : 1;
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
phymodel_initialize_rock_fractalcrack(struct phymodel* model,
				      unsigned int startZ,
				      unsigned int zThickness,
				      int uniform,
				      unsigned int styleParam) {

  unsigned int crackwidth = (styleParam < model->xSize) ? styleParam : 1;
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
