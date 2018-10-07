
#ifndef IMAGE_H
#define IMAGE_H

#include "phymodel.h"

void
image_modelz2image(struct phymodel* model,
		   unsigned int z,
		   const char* filename);
void
image_modelz2image3d(struct phymodel* model,
		     const char* filename);

#endif /* IMAGE_H */
