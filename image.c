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

#include <string.h>
#include <time.h>
#include <magick/MagickCore.h>
#include "util.h"
#include "phymodel.h"
#include "image.h"

static void
image_model2image_zpixel(unsigned int x,
			 unsigned int y,
			 unsigned int z,
			 struct phymodel* model,
			 phyatom* atom,
			 void* data);
static void
image_model2image_xpixel(unsigned int x,
			 unsigned int y,
			 unsigned int z,
			 struct phymodel* model,
			 phyatom* atom,
			 void* data);
static void
image_model2image_ypixel(unsigned int x,
			 unsigned int y,
			 unsigned int z,
			 struct phymodel* model,
			 phyatom* atom,
			 void* data);
static void
image_model2txtimage_zpixel(unsigned int x,
			    unsigned int y,
			    unsigned int z,
			    struct phymodel* model,
			    phyatom* atom,
			    void* data);
static void
image_model2txtimage_xpixel(unsigned int x,
			    unsigned int y,
			    unsigned int z,
			    struct phymodel* model,
			    phyatom* atom,
			    void* data);
static void
image_model2txtimage_ypixel(unsigned int x,
			    unsigned int y,
			    unsigned int z,
			    struct phymodel* model,
			    phyatom* atom,
			    void* data);
static void
image_modelgen2image(struct phymodel* model,
		     enum coordinatetype coord,
		     unsigned int coordval,
		     const char* filename);
static void
image_modelgen2imagetxt(struct phymodel* model,
			enum coordinatetype coord,
			unsigned int coordval,
			const char* filename,
			unsigned int coord1size,
			unsigned int coord2size);
  
void
image_modelz2image(struct phymodel* model,
		   unsigned int z,
		   const char* filename) {
  image_modelgen2image(model,coordinatetype_z,z,filename);
}

void
image_modelx2image(struct phymodel* model,
		   unsigned int x,
		   const char* filename) {
  image_modelgen2image(model,coordinatetype_x,x,filename);
}

void
image_modely2image(struct phymodel* model,
		   unsigned int y,
		   const char* filename) {
  image_modelgen2image(model,coordinatetype_y,y,filename);
}

static void
image_modelgen2image(struct phymodel* model,
		     enum coordinatetype coord,
		     unsigned int coordval,
		     const char* filename) {

  /*
   * Determine what coordinates to use
   */

  unsigned int coord1size;
  unsigned int coord2size;
  switch (coord) {
  case coordinatetype_z:
    coord1size = model->xSize;
    coord2size = model->ySize;
    break;
  case coordinatetype_x:
    coord1size = model->zSize;
    coord2size = model->ySize;
    break;
  case coordinatetype_y:
    coord1size = model->zSize;
    coord2size = model->xSize;
    break;
  default:
    fatal("unrecognised coordinate type");
    return;
  }

  /*
   * Check for exceptional formats
   */

  if (stringendswith(filename,".txt")) {
    image_modelgen2imagetxt(model,
			    coord,
			    coordval,
			    filename,
			    coord1size,
			    coord2size);
    return;
  }
    
  /*
   * Allocations
   */

  {
    
    ExceptionInfo* exception;
    Image* image = 0;
    ImageInfo* image_info;
    unsigned int nPixels = coord1size * coord2size;
    unsigned char* pixels = (unsigned char*)malloc(3 * nPixels);
    
    if (pixels == 0) {
      fatalsu("cannot allocate pixels for file",filename,nPixels);
    }
    
    /*
     * Initialize ImageMagick
     */
    
    MagickCoreGenesis("drop-tracer",MagickTrue);
    exception=AcquireExceptionInfo();
    
    /*
     * Set the properties of the image as desired,
     * i.e., the right filename
     */
    
    image_info = CloneImageInfo((ImageInfo *)0);
    if (image_info == 0) {
      fatals("cannot allocate image info for file",filename);
    }
    strcpy(image_info->filename,filename);
    
    /*
     * Put the table of pixels in an array
     */
    
    switch (coord) {
    case coordinatetype_z:
      phymodel_mapatoms_atz(model,
			    coordval,
			    image_model2image_zpixel,
			    (void*)pixels);
      break;
    case coordinatetype_x:
      phymodel_mapatoms_atx(model,
			    coordval,
			    image_model2image_xpixel,
			    (void*)pixels);
      break;
    case coordinatetype_y:
      phymodel_mapatoms_aty(model,
			    coordval,
			    image_model2image_ypixel,
			    (void*)pixels);
      break;
    default:
      fatal("unrecognised coordinate type");
      return;
    }
    
    /*
     * Create the ImageMagick image object
     */
    
    image = ConstituteImage(coord1size,
			    coord2size,
			    "RGB",
			    CharPixel,
			    pixels,
			    exception);
    
    /*
     * Write the image to file
     */
    
    debugf("writing image to file %s", image_info->filename);
    strcpy(image->filename,image_info->filename);
    MagickBooleanType wres = WriteImage(image_info,image);
    switch (wres) {
    case MagickTrue:
      debugf("write returned true");
      break;
    case MagickFalse:
      debugf("write returned false");
      fatals("cannot write image file",filename);
      break;
    default:
      fatal("unexpected boolean value");
      break;
    }
    
    /*
     * Cleanup
     */
    
    debugf("image write done");
    DestroyImage(image);
    
  }
}

static void
image_modelgen2imagetxt(struct phymodel* model,
			enum coordinatetype coord,
			unsigned int coordval,
			const char* filename,
			unsigned int coord1size,
			unsigned int coord2size) {
  /*
   * Allocations
   */

  unsigned int nPixels = coord1size * coord2size;
  char* pixels = (char*)malloc(nPixels);
    
  if (pixels == 0) {
    fatalsu("cannot allocate pixels for file",filename,nPixels);
  }
  
  /*
   * Put the table of pixels in an array
   */
  
  switch (coord) {
  case coordinatetype_z:
    phymodel_mapatoms_atz(model,
			  coordval,
			  image_model2txtimage_zpixel,
			  (void*)pixels);
    break;
  case coordinatetype_x:
    phymodel_mapatoms_atx(model,
			  coordval,
			  image_model2txtimage_xpixel,
			  (void*)pixels);
    break;
  case coordinatetype_y:
    phymodel_mapatoms_aty(model,
			  coordval,
			  image_model2txtimage_ypixel,
			  (void*)pixels);
    break;
  default:
    fatal("unrecognised coordinate type");
    return;
  }
  
  /*
   * Write the image to file
   */
  
  debugf("writing image to file %s", filename);
  FILE* f = fopen(filename,"w");
  if (f == 0) {
    fatals("cannot open file for writing",filename);
    return;
  }
  
  unsigned int i;
  
  for (i = 0; i < coord2size; i++) {
    fwrite(pixels + i*coord1size,1,coord1size,f);
    fprintf(f,"\n");
  }
  
  /*
   * Cleanup
   */
  
  fclose(f);
}

static void
image_model2image_zpixel(unsigned int x,
			 unsigned int y,
			 unsigned int z,
			 struct phymodel* model,
			 phyatom* atom,
			 void* data) {
  unsigned char* pixels = (unsigned char*)data;
  enum material mat = phyatom_mat(atom);
  struct rgb rgb;
  
  switch (mat) {
  case material_air:
    pixels[3*(y * (model->xSize) + x)+0] = 0;
    pixels[3*(y * (model->xSize) + x)+1] = 0;
    pixels[3*(y * (model->xSize) + x)+2] = 0;
    break;
  case material_rock:
    phyatom_color(&rgb,atom);
    pixels[3*(y * (model->xSize) + x)+0] = rgb.r;
    pixels[3*(y * (model->xSize) + x)+1] = rgb.g;
    pixels[3*(y * (model->xSize) + x)+2] = rgb.b;
    break;
  case material_water:
    pixels[3*(y * (model->xSize) + x)+0] = 0;
    pixels[3*(y * (model->xSize) + x)+1] = 0;
    pixels[3*(y * (model->xSize) + x)+2] = 255;
    break;
  default:
    fatalu("unrecognised atom material type",(int)mat);
  }
}

static void
image_model2image_xpixel(unsigned int x,
			 unsigned int y,
			 unsigned int z,
			 struct phymodel* model,
			 phyatom* atom,
			 void* data) {
  unsigned char* pixels = (unsigned char*)data;
  enum material mat = phyatom_mat(atom);
  struct rgb rgb;
  
  switch (mat) {
  case material_air:
    pixels[3*(z * (model->zSize) + y)+0] = 0;
    pixels[3*(z * (model->zSize) + y)+1] = 0;
    pixels[3*(z * (model->zSize) + y)+2] = 0;
    break;
  case material_rock:
    phyatom_color(&rgb,atom);
    pixels[3*(z * (model->zSize) + y)+0] = rgb.r;
    pixels[3*(z * (model->zSize) + y)+1] = rgb.g;
    pixels[3*(z * (model->zSize) + y)+2] = rgb.b;
    break;
  case material_water:
    pixels[3*(z * (model->zSize) + y)+0] = 0;
    pixels[3*(z * (model->zSize) + y)+1] = 0;
    pixels[3*(z * (model->zSize) + y)+2] = 255;
    break;
  default:
    fatalu("unrecognised atom material type",(int)mat);
  }
}

static void
image_model2image_ypixel(unsigned int x,
			 unsigned int y,
			 unsigned int z,
			 struct phymodel* model,
			 phyatom* atom,
			 void* data) {
  unsigned char* pixels = (unsigned char*)data;
  enum material mat = phyatom_mat(atom);
  struct rgb rgb;
  
  switch (mat) {
  case material_air:
    pixels[3*(z * (model->xSize) + x)+0] = 0;
    pixels[3*(z * (model->xSize) + x)+1] = 0;
    pixels[3*(z * (model->xSize) + x)+2] = 0;
    break;
  case material_rock:
    phyatom_color(&rgb,atom);
    pixels[3*(z * (model->xSize) + x)+0] = rgb.r;
    pixels[3*(z * (model->xSize) + x)+1] = rgb.g;
    pixels[3*(z * (model->xSize) + x)+2] = rgb.b;
    break;
  case material_water:
    pixels[3*(z * (model->xSize) + x)+0] = 0;
    pixels[3*(z * (model->xSize) + x)+1] = 0;
    pixels[3*(z * (model->xSize) + x)+2] = 255;
    break;
  default:
    fatalu("unrecognised atom material type",(int)mat);
  }
}

static void
image_model2txtimage_zpixel(unsigned int x,
			    unsigned int y,
			    unsigned int z,
			    struct phymodel* model,
			    phyatom* atom,
			    void* data) {
  char* pixels = (char*)data;
  enum material mat = phyatom_mat(atom);
  
  switch (mat) {
  case material_air:
    pixels[y * model->xSize + x] = ' ';
    break;
  case material_rock:
    pixels[y * model->xSize + x] = 'R';
    break;
  case material_water:
    pixels[y * model->xSize + x] = 'W';
    break;
  default:
    fatalu("unrecognised atom material type",(int)mat);
  }
}

static void
image_model2txtimage_xpixel(unsigned int x,
			    unsigned int y,
			    unsigned int z,
			    struct phymodel* model,
			    phyatom* atom,
			    void* data) {
  char* pixels = (char*)data;
  enum material mat = phyatom_mat(atom);
  
  switch (mat) {
  case material_air:
    pixels[z * model->zSize + y] = ' ';
    break;
  case material_rock:
    pixels[z * model->zSize + y] = 'R';
    break;
  case material_water:
    pixels[z * model->zSize + y] = 'W';
    break;
  default:
    fatalu("unrecognised atom material type",(int)mat);
  }
}

static void
image_model2txtimage_ypixel(unsigned int x,
			    unsigned int y,
			    unsigned int z,
			    struct phymodel* model,
			    phyatom* atom,
			    void* data) {
  char* pixels = (char*)data;
  enum material mat = phyatom_mat(atom);
  
  switch (mat) {
  case material_air:
    pixels[z * model->xSize + x] = ' ';
    break;
  case material_rock:
    pixels[z * model->xSize + x] = 'R';
    break;
  case material_water:
    pixels[z * model->xSize + x] = 'W';
    break;
  default:
    fatalu("unrecognised atom material type",(int)mat);
  }
}

void
image_model2image3d(struct phymodel* model,
		    const char* filename) {
  /* ... */
  fatal("3D model conversion not implemented");
}
