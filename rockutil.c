
#include <math.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "util.h"
#include "phymodel.h"
#include "rock.h"

void
phymodel_set_rock_material(struct phymodel* model,
			   unsigned int x,
			   unsigned int y,
			   unsigned int z) {
  unsigned int atomindex = phymodel_atomindex(model,x,y,z);
  phyatom* atom = &model->atoms[atomindex];
  struct rgb rgb;
  phyatom_set_mat(atom,material_rock);
  rgb_set_white(&rgb);
  phyatom_set_color(atom,&rgb);
}

void
phymodel_set_rock_crackmaterial(struct phymodel* model,
				unsigned int x,
				unsigned int y,
				unsigned int z) {
  unsigned int atomindex = phymodel_atomindex(model,x,y,z);
  phyatom* atom = &model->atoms[atomindex];
  phyatom_set_mat(atom,material_air);
}

void
phymodel_set_rock_crackmaterial_thickness(struct phymodel* model,
					  unsigned int x,
					  unsigned int y,
					  unsigned int startZ,
					  unsigned int zThickness) {
  unsigned int z;
  for (z = startZ; z < startZ + zThickness; z++) {
    phymodel_set_rock_crackmaterial(model,x,y,z);
  }
}
