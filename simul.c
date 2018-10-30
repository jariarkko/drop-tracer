
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
simulator_move_dropuntilholeandchangedirection(struct simulatorstate* state,
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
    if (!simulator_move_dropuntilholeandchangedirection(state,model,&dropplace,direction)) {
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
simulator_move_dropuntilholeandchangedirection(struct simulatorstate* state,
					       struct phymodel* model,
					       struct atomcoordinates* place,
					       enum direction direction) {
  
  if (simulator_move_dropuntilhole(state,model,place,direction)) {
    
    return(1);
    
  } else {
    
    enum direction anotherdirection;
    
    for (anotherdirection = (direction + 1) % direction_howmany;
	 anotherdirection != direction;
	 anotherdirection = (anotherdirection + 1) & direction_howmany) {

      debugf("trying another direction...");
      if (simulator_move_dropuntilhole(state,model,place,anotherdirection)) return(1);
      
    }
    
    return(0);
    
  }
}

static int
simulator_move_dropuntilhole(struct simulatorstate* state,
			     struct phymodel* model,
			     struct atomcoordinates* place,
			     enum direction direction) {

  switch (direction) {

  case direction_x_towards0:
    while (1) {
      debugf("hole search x_towards0 (%u,%u,%u)", place->x, place->y, place->z);
      if (phymodel_atomisfree(model,place->x,place->y,place->z)) return(1);
      else if (place->x == 0) return(0);
      else place->x--;
    }

  case direction_x_towardsn:
    while (1) {
      debugf("hole search x_towardsn (%u,%u,%u)", place->x, place->y, place->z);
      if (phymodel_atomisfree(model,place->x,place->y,place->z)) return(1);
      else if (place->x == model->xSize - 1) return(0);
      else place->x++;
    }

  case direction_y_towards0:
    while (1) {
      debugf("hole search y_towards0 (%u,%u,%u)", place->x, place->y, place->z);
      if (phymodel_atomisfree(model,place->x,place->y,place->z)) return(1);
      else if (place->y == 0) return(0);
      else place->y--;
    }

  case direction_y_towardsn:
    while (1) {
      debugf("hole search y_towardsn (%u,%u,%u)", place->x, place->y, place->z);
      if (phymodel_atomisfree(model,place->x,place->y,place->z)) return(1);
      else if (place->y == model->ySize - 1) return(0);
      else place->y++;
    }

  case direction_z_towards0:
    while (1) {
      debugf("hole search z_towards0 (%u,%u,%u)", place->x, place->y, place->z);
      if (phymodel_atomisfree(model,place->x,place->y,place->z)) return(1);
      else if (place->z == 0) return(0);
      else place->z--;
    }

  case direction_z_towardsn:
    while (1) {
      debugf("hole search z_towardsn (%u,%u,%u)", place->x, place->y, place->z);
      if (phymodel_atomisfree(model,place->x,place->y,place->z)) return(1);
      else if (place->y == model->ySize - 1) return(0);
      else place->y++;
    }

  default:
    fatal("unrecognised direction");
  }

  return(0);
}

static int
simulator_move_dropintohole(struct simulatorstate* state,
			    struct phymodel* model,
			    struct atomcoordinates* place,
			    struct simulatordrop* drop) {

  debugf("trying to place a drop into hole at (%u,%u,%u)", place->x, place->y, place->z);
  
  if (!simulator_drop_enoughspaceforwater(model,place,drop->size)) {

    debugf("not enough space");
    return(0);
    
  } else {
    
    if (simulator_drop_putdrop(model,place,drop)) {
      
      state->atomCreations += drop->natoms;
      return(1);
      
    } else {
      
      return(0);
      
    }
    
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
  debugf("    atom creations:              %8llu", state->atomCreations);
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
