
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
simulator_simulate_round(struct simulatorstate* state,
			 struct phymodel* model,
			 unsigned int simulDropSize,
			 unsigned int startingLevel,
			 int drop);
static void
simulator_simulate_drop(struct simulatorstate* state,
			struct phymodel* model,
			unsigned int simulDropSize,
			unsigned int startingLevel);
static void
simulator_state_initialize(struct simulatorstate* state,
			   struct phymodel* model);
static void
simulator_state_deinitialize(struct simulatorstate* state,
			     struct phymodel* model);
static void
simulator_find_randomdropplaceanddirection(struct simulatorstate* state,
					   struct phymodel* model,
					   struct atomcoordinates* dropplace,
					   enum direction* direction,
					   unsigned int startingLevel);
static int
simulator_move_dropuntilhole(struct simulatorstate* state,
			     struct phymodel* model,
			     struct atomcoordinates* place,
			     enum direction direction);
static int
simulator_move_dropintohole(struct simulatorstate* state,
			    struct phymodel* model,
			    struct atomcoordinates* place,
			    struct simulatordrop* dop);
static void
simulator_stats(struct simulatorstate* state,
		struct phymodel* model);
static unsigned int
simulator_find_startinglevel(struct phymodel* model);
static void
simulator_seekfreey(struct simulatorstate* state,
		    struct phymodel* model,
		    unsigned int x,
		    unsigned int y,
		    unsigned int z,
		    unsigned int* pDropSize);

void
simulator_simulate(struct phymodel* model,
		   unsigned int simulRounds,
		   unsigned int simulDropFrequency,
		   unsigned int simulDropSize) {

  struct simulatorstate state;
  unsigned int round;

  simulator_state_initialize(&state,model);
  debugf("simulating %u rounds...", simulRounds);
  debugf("simulator state size %u, one drop size %u, max %u drops, max %u atoms per drop...",
	 sizeof(state),
	 sizeof(struct simulatordrop),
	 simulatordroptable_maxdrops,
	 simulatorstate_maxatomsperdrop);

  unsigned int startingLevel = simulator_find_startinglevel(model);

  for (round = 0; round < simulRounds; round++) {
    int drop = ((round % simulDropFrequency) == 0);
    simulator_simulate_round(&state,model,simulDropSize,startingLevel,drop);
  }

  debugf("simulation complete");
  simulator_stats(&state,model);
  simulator_state_deinitialize(&state,model);
}

static void
simulator_simulate_round(struct simulatorstate* state,
			 struct phymodel* model,
			 unsigned int simulDropSize,
			 unsigned int startingLevel,
			 int drop) {
  if (drop) {
    simulator_simulate_drop(state,
			    model,
			    simulDropSize,
			    startingLevel);
  }
  
  state->rounds++;
  
}

static void
simulator_simulate_drop(struct simulatorstate* state,
			struct phymodel* model,
			unsigned int dropSize,
			unsigned int startingLevel) {
  struct simulatordrop* drop = simulator_droptable_getdrop(&state->drops);
  if (drop == 0) {
    state->failedDropAllocations++;
  } else {
    struct atomcoordinates dropplace;
    enum direction direction;
    assert(drop->active);
    drop->size = dropSize;
    drop->calcite = 1.0;
    rgb_set_white(&drop->calcitecolor);
    drop->natoms = dropSize;
    simulator_find_randomdropplaceanddirection(state,model,&dropplace,&direction,startingLevel);
    if (!simulator_move_dropuntilhole(state,model,&dropplace,direction)) {
      state->failedDropHoleFinding++;
      simulator_droptable_deletedrop(&state->drops,drop);
      return;
    }
    if (!simulator_move_dropintohole(state,model,&dropplace,drop)) {
      state->failedDropHoleFree++;
      simulator_droptable_deletedrop(&state->drops,drop);
      return;
    }
    state->successfullyCreatedDrops++;
  }
}

static void
simulator_state_initialize(struct simulatorstate* state,
			   struct phymodel* model) {
  memset(state,0,sizeof(*state));
  simulator_droptable_initialize(&state->drops);
}

static void
simulator_state_deinitialize(struct simulatorstate* state,
			     struct phymodel* model) {
  simulator_droptable_initialize(&state->drops);
  memset(state,0xFF,sizeof(*state));
}

static void
simulator_find_randomdropplaceanddirection(struct simulatorstate* state,
					   struct phymodel* model,
					   struct atomcoordinates* dropplace,
					   enum direction* direction,
					   unsigned int startingLevel) {
  *direction = rand() % (int)(direction_howmany);
  dropplace->x = rand() % model->xSize;
  dropplace->y = rand() % model->ySize;
  dropplace->z = startingLevel;
}

static int
simulator_move_dropuntilhole(struct simulatorstate* state,
			     struct phymodel* model,
			     struct atomcoordinates* place,
			     enum direction direction) {

  switch (direction) {

  case direction_x_towards0:
    while (1) {
      if (phymodel_atomisfree(model,place->x,place->y,place->z)) return(1);
      else if (place->x == 0) return(0);
      else place->x--;
    }

  case direction_x_towardsn:
    while (1) {
      if (phymodel_atomisfree(model,place->x,place->y,place->z)) return(1);
      else if (place->x == model->xSize - 1) return(0);
      else place->x++;
    }

  case direction_y_towards0:
    while (1) {
      if (phymodel_atomisfree(model,place->x,place->y,place->z)) return(1);
      else if (place->y == 0) return(0);
      else place->y--;
    }

  case direction_y_towardsn:
    while (1) {
      if (phymodel_atomisfree(model,place->x,place->y,place->z)) return(1);
      else if (place->y == model->ySize - 1) return(0);
      else place->y++;
    }

  case direction_z_towards0:
    while (1) {
      if (phymodel_atomisfree(model,place->x,place->y,place->z)) return(1);
      else if (place->z == 0) return(0);
      else place->z--;
    }

  case direction_z_towardsn:
    while (1) {
      if (phymodel_atomisfree(model,place->x,place->y,place->z)) return(1);
      else if (place->y == model->ySize - 1) return(0);
      else place->y++;
    }

  default:
    fatal("unrecognised direction");
  }

  return(0);
}

static void
simulator_seekfreey(struct simulatorstate* state,
		    struct phymodel* model,
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

static int
simulator_spaceforwater(struct simulatorstate* state,
			struct phymodel* model,
			struct atomcoordinates* place,
			unsigned int dropSize) {

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
	
	simulator_seekfreey(state,model,x,y,z,&dropSize);
	
	while (dropSize > 0 &&
	       x > 0 &&
	       phymodel_atomisfree(model,x-1,y,z)) {
	  
	  dropSize--;
	  x--;
	  simulator_seekfreey(state,model,x,y,z,&dropSize);
	  
	}
	
	x = place->x;
	
	while (dropSize > 0 &&
	       x < model->xSize -1 &&
	       phymodel_atomisfree(model,x+1,y,z)) {
	  
	  dropSize--;
	  x++;
	  simulator_seekfreey(state,model,x,y,z,&dropSize);
	  
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
simulator_coordsequal(struct atomcoordinates* coord1,
		      struct atomcoordinates* coord2) {
  return(coord1->x == coord2->x &&
	 coord1->y == coord2->y &&
	 coord1->z == coord2->z);
}

static int
simulator_onecoordisadjacent(unsigned int a,
			     unsigned int b) {
  if (a == b) return(1);
  if (b > 0 && a == b - 1) return(1);
  if (a == b + 1) return(1);
  return(0);
}

static int
simulator_coordsadjacent(struct atomcoordinates* coord1,
			 struct atomcoordinates* coord2) {
  return(simulator_onecoordisadjacent(coord1->x,coord2->x) &&
	 simulator_onecoordisadjacent(coord1->y,coord2->y) &&
	 simulator_onecoordisadjacent(coord1->z,coord2->z));
}

static int
simulator_nextatomsareinthisdrop(struct simulatorstate* state,
				 struct phymodel* model,
				 struct simulatordrop* drop,
				 struct atomcoordinates* place) {
  unsigned int i;

  for (i = 0; i < drop->natoms; i++) {
    struct atomcoordinates* coords = &drop->atoms[i];
    if (simulator_coordsequal(coords,place)) return(1);
    else if (simulator_coordsadjacent(coords,place)) return(1);
  }
  return(0);
}

static int
simulator_drop_addatom(struct simulatorstate* state,
		  struct phymodel* model,
		  struct simulatordrop* drop,
		  struct atomcoordinates* place) {
  struct atomcoordinates* coords;
  assert(drop->natoms < simulatorstate_maxatomsperdrop);
  coords = &drop->atoms[drop->natoms++];
  coords->x = place->x;
  coords->y = place->y;
  coords->z = place->z;
  return(1);
}

static void
simulator_remove_dropatoms(struct simulatorstate* state,
			   struct phymodel* model,
			   struct simulatordrop* drop) {

  while (drop->natoms > 0) {
    struct atomcoordinates* coords = &drop->atoms[drop->natoms - 1];
    phyatom* atom = phymodel_getatom(model,coords->x,coords->y,coords->z);
    phyatom_set_mat(atom,material_air);
    drop->natoms--;
  }
}

static int
simulator_putdrop_circledistance(struct simulatorstate* state,
				 struct phymodel* model,
				 struct atomcoordinates* place,
				 struct simulatordrop* drop,
				 unsigned int distance) {
  if (distance == 0) {
    if (!phymodel_atomisfree(model,place->x,place->y,place->z)) return(0);
    phyatom* atom = phymodel_getatom(model,place->x,place->y,place->z);
    phyatom_set_mat(atom,material_water);
    simulator_drop_addatom(state,model,drop,place);
    return(1);
  } else {
    /* ... */
    return(1);
  }
}

static int
simulator_putdrop(struct simulatorstate* state,
		  struct phymodel* model,
		  struct atomcoordinates* place,
		  struct simulatordrop* drop) {
  unsigned int distance = 0;
  assert(drop->size <= simulatorstate_maxatomsperdrop);
  while (drop->natoms < drop->size) {
    if (!simulator_putdrop_circledistance(state,model,place,drop,distance)) {
      if (drop->natoms < drop->size) return(0);
    }
    distance++;
  }
  
  return(1);
}
  
static int
simulator_move_dropintohole(struct simulatorstate* state,
			    struct phymodel* model,
			    struct atomcoordinates* place,
			    struct simulatordrop* drop) {
  
  if (!simulator_spaceforwater(state,model,place,drop->size)) {
    
    return(0);
    
  } else {
    
    assert(drop->natoms == 0);
    return(simulator_putdrop(state,model,place,drop));
    
  }
  
}

static void
simulator_stats(struct simulatorstate* state,
		struct phymodel* model) {

  debugf("  successfully created drops:    %8llu", state->successfullyCreatedDrops);
  debugf("  failed drops:                  %8llu",
	 state->failedDropAllocations + state->failedDropHoleFinding + state->failedDropHoleFree);
  debugf("    unable to allocate:          %8llu", state->failedDropAllocations);
  debugf("    unable to find a hole:       %8llu", state->failedDropHoleFinding);
  debugf("    found hole not free:         %8llu", state->failedDropHoleFree);
  debugf("    rounds:                      %8llu", state->rounds);
  debugf("    drop movements:              %8llu", state->dropMovements);
  debugf("    atom movements:              %8llu", state->atomMovements);
}

static unsigned int
simulator_find_startinglevel(struct phymodel* model) {
  const unsigned int xplace = 0;
  const unsigned int yplace = 0;
  unsigned int z = 0;
  phyatom* atom = phymodel_getatom(model,xplace,yplace,z);
  while (z < model->zSize) {
    if (phyatom_mat(atom) == material_rock) {
      debugf("simulator_find_startinglevel: %u", z);
      return(z);
    } else {
      z++;
      atom = phymodel_getatom(model,xplace,yplace,z);
    }
  }
  fatal("cannot find rock starting level from the top");
  return(0);
}
