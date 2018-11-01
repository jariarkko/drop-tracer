
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

#include <math.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "util.h"
#include "phymodel.h"
#include "drop.h"
#include "droptable.h"
#include "simul.h"

static void
simulator_drop_seekfreey(struct phymodel* model,
			 unsigned int x,
			 unsigned int y,
			 unsigned int z,
			 unsigned int* pDropSize);

static void
simulator_drop_seekfreey(struct phymodel* model,
			 unsigned int x,
			 unsigned int y,
			 unsigned int z,
			 unsigned int* pDropSize) {

  unsigned int othery = y;

  while ((*pDropSize) > 0 &&
	 othery > 0 &&
	 phymodel_atomisfree(model,x,othery-1,z)) {
    (*pDropSize)--;
    othery--;
  }

  othery = y;
  while ((*pDropSize) > 0 &&
	 othery < model->ySize - 1 &&
	 phymodel_atomisfree(model,x,othery+1,z)) {
    (*pDropSize)--;
    othery++;
  }
}

int
simulator_drop_enoughspaceforwater(struct phymodel* model,
				   struct atomcoordinates* place,
				   unsigned int dropSize) {

  assert(phymodel_isvalid(model));
  
  unsigned int z = place->z;
  unsigned int y = place->y;

  while (dropSize > 0) {

    if (z >= model->zSize) {

      /*
       * If we have run to the bottom of the model, assume that water
       * can trivially flow out
       */

      return(1);

    } else if (phymodel_atommat(model,place->x,y,z) != material_air) {

      /*
       * No space, something else (rock or water) occupying the
       * place.
       */

      return(0);

    } else {

      /*
       * There is space, at least one atom. Find out how much of the
       * drop we can place in the same z level.
       */

      unsigned int x = place->x;

      /*
       * Figure out if there's space at this height (z) for enough
       * atoms to contain a dropSize drop.
       *
       * We will do this by first using all the space that is
       * available in y direction, e.g., lets imagine that we're
       * inserting a drop at the coordinates (10,5). There's 0 spaces
       * to the smaller y values available next to it because of the
       * rock (R) atoms below, and 6 spaces to the higher y
       * values. The x-y picture of the start of the placement of the
       * drop would look like this:
       *
       *
       *       (y)
       *
       *       	.
       *       /|\
       *       	|
       *      11|RRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRR
       *      10|         W   RRRRRRRRRRRRRRRRRRRRRRRRRRRRR
       *       9|         W
       *       8|         W
       *       7|         W
       *       6|         W
       *       5|         W
       *       4|      RRRRRRR
       *       3|RRRRRRRRRRRRRRRRRRRR
       *       2|RRRRRRRRRRRRRRRRRRRRRRRRRRRRRR
       *       1|RRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRR
       *        +-------------------------------------------------> (x)
       *       0 123456789111111111122222222223333333333444444444 
       *                  012345678901234567890123456789012345678
       *
       *
       * Then lets imagine the drop size is 10 atoms, so we need to
       * find 4 more slots for the water atoms. The algorithm looks
       * at the adjacent x values next, by going to lower x values
       * first. After attempting to fill the next column (x=9)
       * in this picture we find that we can fit in 4 more atoms,
       * which is enough to contain the water droplet.
       *
       *       (y)
       *
       *       	.
       *       /|\
       *       	|
       *      11|RRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRR
       *      10|         W   RRRRRRRRRRRRRRRRRRRRRRRRRRRRR
       *       9|         W
       *       8|        WW
       *       7|        WW
       *       6|        WW
       *       5|        WW
       *       4|      RRRRRRR
       *       3|RRRRRRRRRRRRRRRRRRRR
       *       2|RRRRRRRRRRRRRRRRRRRRRRRRRRRRRR
       *       1|RRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRR
       *        +-------------------------------------------------> (x)
       *       0 123456789111111111122222222223333333333444444444 
       *                  012345678901234567890123456789012345678
       */

      while (dropSize > 0) {

	dropSize--;
	
	simulator_drop_seekfreey(model,x,y,z,&dropSize);
	
	while (dropSize > 0 &&
	       x > 0 &&
	       phymodel_atomisfree(model,x-1,y,z)) {
	  
	  dropSize--;
	  x--;
	  simulator_drop_seekfreey(model,x,y,z,&dropSize);
	  
	}
	
	x = place->x;
	
	while (dropSize > 0 &&
	       x < model->xSize -1 &&
	       phymodel_atomisfree(model,x+1,y,z)) {
	  
	  dropSize--;
	  x++;
	  simulator_drop_seekfreey(model,x,y,z,&dropSize);
	  
	}

      }

    }
      
    /*
     * If still not done, descend lower in the model
     */

    if (dropSize > 0) {
      
      z++;
      
    }
    
  }

  /*
   * We've found enough space for all atoms in the drop
   */

  return(1);
}

static int
simulator_drop_nextatomsareinthisdrop(struct phymodel* model,
				      struct simulatordrop* drop,
				      struct atomcoordinates* place) {
  unsigned int i;

  assert(phymodel_isvalid(model));
  
  for (i = 0; i < drop->natoms; i++) {
    struct atomcoordinates* coords = &drop->atoms[i];
    if (simulator_coords_equal(coords,place)) return(1);
    else if (simulator_coords_adjacent(coords,place)) return(1);
  }
  return(0);
}

static int
simulator_drop_addatom(struct phymodel* model,
		       struct simulatordrop* drop,
		       struct atomcoordinates* place) {
  struct atomcoordinates* coords;
  assert(phymodel_isvalid(model));
  assert(drop->natoms < simulatorstate_maxatomsperdrop);
  coords = &drop->atoms[drop->natoms++];
  coords->x = place->x;
  coords->y = place->y;
  coords->z = place->z;
  return(1);
}

static void
simulator_remove_dropatoms(struct phymodel* model,
			   struct simulatordrop* drop) {

  assert(phymodel_isvalid(model));
  
  while (drop->natoms > 0) {
    struct atomcoordinates* coords = &drop->atoms[drop->natoms - 1];
    phyatom* atom = phymodel_getatom(model,coords->x,coords->y,coords->z);
    phyatom_set_mat(atom,material_air);
    drop->natoms--;
  }
}

struct circledrawingcontext {
  struct simulatordrop* drop;
};

static void
simulator_putdrop_circledistance_onecircle(unsigned int x,
					   unsigned int y,
					   unsigned int z,
					   struct phymodel* model,
					   phyatom* atom,
					   void* data) {
  struct circledrawingcontext* context = (struct circledrawingcontext*)data;

  /*
   * Some sanity checks
   */

  assert(model != 0);
  assert(phymodel_isvalid(model));
  assert(atom != 0);
  assert(context != 0);
  struct simulatordrop* drop = context->drop;
  assert(drop != 0);
  assert(drop->size > 0);
  
  /*
   * Check if we still need to add water atoms
   */
  
  if (drop->natoms >= drop->size) return;

  /*
   * We do. Check if there's just air there, or something else.
   */

  if (phyatom_mat(atom) != material_air) return;
  
  /*
   * Nothing there. Add a water atom to the indicated (x,y,z) location.
   */
  
  /* debugf("adding water atom at (%u,%u,%u) to drop (%u/%u)",
		x, y, z,
		drop->natoms,
		drop->size); */
  
  phyatom_set_mat(atom,material_water);
  struct atomcoordinates coords;
  coords.x = x;
  coords.y = y;
  coords.z = z;
  simulator_drop_addatom(model,drop,&coords);
}

static int
simulator_putdrop_circledistance(struct phymodel* model,
				 struct atomcoordinates* place,
				 struct simulatordrop* drop,
				 unsigned int distance) {

  assert(phymodel_isvalid(model));

  debugf("simulator_putdrop_circledistance (%u,%u,%u) distance %u",
	 place->x, place->y, place->z,
	 distance);
  
  if (distance == 0) {
    
    if (!phymodel_atomisfree(model,place->x,place->y,place->z)) return(0);
    phyatom* atom = phymodel_getatom(model,place->x,place->y,place->z);
    phyatom_set_mat(atom,material_water);
    simulator_drop_addatom(model,drop,place);
    return(1);
    
  } else {
    
    /*
     * Recurse for a smaller circle
     */
    
    simulator_putdrop_circledistance(model,place,drop,distance-1);
    
    /*
     * And then draw a circle with distance, putting a water atom in every
     * atom slot in the model that is that distance away from the drop place
     * (in 3D; if in free space, the water drop will form a sphere).
     */

    struct circledrawingcontext context;
    context.drop = drop;
    phymodel_mapatoms_atdistance3d(model,
				   place->x,place->y,place->z,
				   distance,
				   simulator_putdrop_circledistance_onecircle,
				   &context);
    
    /*
     * Done
     */
    
    return(1);
    
  }
  
}

int
simulator_drop_putdrop(struct phymodel* model,
		       struct atomcoordinates* place,
		       struct simulatordrop* drop) {
  unsigned int distance = 0;
  assert(phymodel_isvalid(model));
  assert(drop->size <= simulatorstate_maxatomsperdrop);
  
  debugf("putting a drop of size %u at (%u,%u,%u)", drop->size, place->x, place->y, place->z);
  /* debugf("initial natoms = %u", drop->natoms); */
  
  while (drop->natoms < drop->size) {
    debugf("drop circle round %u", distance);
    if (!simulator_putdrop_circledistance(model,place,drop,distance)) {
      if (drop->natoms < drop->size) return(0);
    }
    distance++;
  }
  
  return(1);
}
