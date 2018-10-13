
#include <math.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "util.h"
#include "phymodel.h"
#include "rock.h"

static void
phymodel_create_initatom(unsigned int x,
			 unsigned int y,
			 unsigned int z,
			 struct phymodel* model,
			 phyatom* atom,
			 void* data);

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

  debugf("created an image object of %uM bytes (%ux%ux%u), atom memory size = %u",
	 size / 1000000,
	 xSize, ySize, zSize,
	 sizeof(model->atoms[0]));
  debugf("unit is %f mm, model size %fm x %fm x %fm (%f m3)",
	 (1.0 * 1000.0) / unit,
	 (xSize * 1.0) / (unit * 1.0),
	 (ySize * 1.0) / (unit * 1.0),
	 (zSize * 1.0) / (unit * 1.0),
	 (xSize * 1.0) / (unit * 1.0) * (ySize * 1.0) / (unit * 1.0) * (zSize * 1.0) / (unit * 1.0));
  
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
			 phyatom* atom,
			 void* data) {
  struct rgb black;
  rgb_set_black(&black);

  phyatom_reset(atom);
  phyatom_set_mat(atom,material_air);
  phyatom_set_color(atom,&black);
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
	phyatom* atom = &model->atoms[atomIndex];
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
  assert(z < model->zSize);
  for (y = 0; y < model->ySize; y++) {
    for (x = 0; x < model->xSize; x++) {
      unsigned int atomIndex = phymodel_atomindex(model,x,y,z);
      phyatom* atom = &model->atoms[atomIndex];
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
  assert(x < model->xSize);
  for (y = 0; y < model->ySize; y++) {
    for (z = 0; z < model->zSize; z++) {
      unsigned int atomIndex = phymodel_atomindex(model,x,y,z);
      phyatom* atom = &model->atoms[atomIndex];
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
  assert(y < model->ySize);
  for (x = 0; x < model->xSize; x++) {
    for (z = 0; z < model->zSize; z++) {
      unsigned int atomIndex = phymodel_atomindex(model,x,y,z);
      phyatom* atom = &model->atoms[atomIndex];
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
