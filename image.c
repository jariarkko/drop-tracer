

#include <string.h>
#include <time.h>
#include <magick/MagickCore.h>
#include "util.h"
#include "phymodel.h"
#include "image.h"

static void
image_modelz2image_pixel(unsigned int x,
			 unsigned int y,
			 unsigned int z,
			 struct phymodel* model,
			 struct phyatom* atom,
			 void* data);

void
image_modelz2image(struct phymodel* model,
		   unsigned int z,
		   const char* filename) {

  /*
   * Allocations
   */
  
  ExceptionInfo* exception;
  Image* image = 0;
  ImageInfo* image_info;
  unsigned int nPixels = model->xSize * model->ySize * model->zSize;
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
  
  phymodel_mapatoms_atz(model,
			z,
			image_modelz2image_pixel,
			(void*)pixels);
  
  /*
   * Create the ImageMagick image object
   */
  
  image = ConstituteImage(model->xSize,
			  model->ySize,
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

static void
image_modelz2image_pixel(unsigned int x,
			 unsigned int y,
			 unsigned int z,
			 struct phymodel* model,
			 struct phyatom* atom,
			 void* data) {
  unsigned char* pixels = (unsigned char*)data;
  switch (atom->mat) {
  case material_air:
    pixels[3*(y * (model->xSize) + x)+0] = 0;
    pixels[3*(y * (model->xSize) + x)+1] = 0;
    pixels[3*(y * (model->xSize) + x)+2] = 0;
    break;
  case material_rock:
    pixels[3*(y * (model->xSize) + x)+0] = atom->color.r;
    pixels[3*(y * (model->xSize) + x)+1] = atom->color.g;
    pixels[3*(y * (model->xSize) + x)+2] = atom->color.b;
    break;
  case material_water:
    pixels[3*(y * (model->xSize) + x)+0] = 0;
    pixels[3*(y * (model->xSize) + x)+1] = 0;
    pixels[3*(y * (model->xSize) + x)+2] = 255;
    break;
  default:
    fatal("unrecognised atom material type");
  }
}

void
image_modelz2image3d(struct phymodel* model,
		     const char* filename) {
  /* ... */
  fatal("3D model conversion not implemented");
}
