
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
#include "image.h"

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
static void
simulator_snapshot(struct phymodel* model,
                   unsigned int roundno,
                   const char* progressImage);

void
simulator_simulate(struct phymodel* model,
		   unsigned int simulRounds,
		   unsigned int simulDropFrequency,
		   unsigned int simulDropSize,
		   const char* progressImage) {

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
  
  if (progressImage) {
    simulator_snapshot(model,0,progressImage);
  }
  
  for (round = 0; round < simulRounds; round++) {
    int drop = ((round % simulDropFrequency) == 0);
    debugf("simulation round %u (drop %u)", round, drop);
    simulator_simulate_round(&state,model,simulDropSize,startingLevel,drop);
    if (progressImage) {
      simulator_snapshot(model,round+1,progressImage);
    }
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

  /*
   * First, move all current drops if they can be moved.
   */
  
  unsigned int i;
  
  for (i = 0; i < state->drops.ndrops; i++) {
    struct simulatordrop* drop = &state->drops.drops[i];
    if (drop->active) {
      simulator_drop_movedrop(model,state,drop);
      if (!drop->active) {
	state->dropFellOffModels++;
      } else {
	state->dropMovements++;
	state->atomMovements += drop->natoms;
      }
    }
  }

  /*
   * Then, create a new drop if we're told to
   */
  
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
    debugf("placing a new drop from level %u", startingLevel);
    simulator_find_randomdropplaceanddirection(state,model,&dropplace,&direction,startingLevel);
    deepdebugf("found initial drop location (%u,%u,%u)", dropplace.x, dropplace.y, dropplace.z);
    if (!simulator_move_dropuntilholeandchangedirection(state,model,&dropplace,direction)) {
      state->failedDropHoleFinding++;
      simulator_droptable_deletedrop(&state->drops,drop);
      debugf("failed to find a hole");
      return;
    }
    deepdebugf("found final drop location (%u,%u,%u)", dropplace.x, dropplace.y, dropplace.z);
    if (!simulator_move_dropintohole(state,model,&dropplace,drop)) {
      state->failedDropHoleFree++;
      simulator_droptable_deletedrop(&state->drops,drop);
      debugf("failed to drop into a hole");
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
	 anotherdirection = (anotherdirection + 1) % direction_howmany) {

      if (anotherdirection == direction_z_towards0) continue;
      if (anotherdirection == direction_z_towardsn) continue;
      deepdeepdebugf("trying another direction from %u to %u...", direction, anotherdirection);
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
      deepdeepdebugf("hole search x_towards0 (%u,%u,%u)", place->x, place->y, place->z);
      if (phymodel_atomisfree(model,place->x,place->y,place->z)) {
	deepdeepdebugf("while scanning for hole found one at (%u,%u,%u)", place->x, place->y, place->z);
	return(1);
      } else if (place->x == 0) return(0);
      else place->x--;
    }

  case direction_x_towardsn:
    while (1) {
      deepdeepdebugf("hole search x_towardsn (%u,%u,%u)", place->x, place->y, place->z);
      if (phymodel_atomisfree(model,place->x,place->y,place->z)) {
	deepdeepdebugf("while scanning for hole found one at (%u,%u,%u)", place->x, place->y, place->z);
	return(1);
      } else if (place->x == model->xSize - 1) return(0);
      else place->x++;
    }

  case direction_y_towards0:
    while (1) {
      deepdeepdebugf("hole search y_towards0 (%u,%u,%u)", place->x, place->y, place->z);
      if (phymodel_atomisfree(model,place->x,place->y,place->z)) {
	deepdeepdebugf("while scanning for hole found one at (%u,%u,%u)", place->x, place->y, place->z);
	return(1);
      } else if (place->y == 0) return(0);
      else place->y--;
    }

  case direction_y_towardsn:
    while (1) {
      deepdeepdebugf("hole search y_towardsn (%u,%u,%u)", place->x, place->y, place->z);
      if (phymodel_atomisfree(model,place->x,place->y,place->z)) {
	deepdeepdebugf("while scanning for hole found one at (%u,%u,%u)", place->x, place->y, place->z);
	return(1);
      } else if (place->y == model->ySize - 1) return(0);
      else place->y++;
    }

  case direction_z_towards0:
    while (1) {
      deepdeepdebugf("hole search z_towards0 (%u,%u,%u)", place->x, place->y, place->z);
      if (phymodel_atomisfree(model,place->x,place->y,place->z)) {
	deepdeepdebugf("while scanning for hole found one at (%u,%u,%u)", place->x, place->y, place->z);
	return(1);
      } else if (place->z == 0) return(0);
      else place->z--;
    }

  case direction_z_towardsn:
    while (1) {
      deepdeepdebugf("hole search z_towardsn (%u,%u,%u)", place->x, place->y, place->z);
      if (phymodel_atomisfree(model,place->x,place->y,place->z)) {
	deepdeepdebugf("while scanning for hole found one at (%u,%u,%u)", place->x, place->y, place->z);
        return(1);
      } else if (place->y == model->ySize - 1) return(0);
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

  deepdebugf("trying to place a drop into hole at (%u,%u,%u)", place->x, place->y, place->z);
  
  if (!simulator_drop_enoughspaceforwater(model,place,drop->size)) {

    deepdebugf("not enough space");
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
  debugf("    spin-off drop not free:      %8llu", state->failedSpinoffDropSpaceFinding);
  debugf("    rounds:                      %8llu", state->rounds);
  debugf("    drop movements:              %8llu", state->dropMovements);
  debugf("    atom creations:              %8llu", state->atomCreations);
  debugf("    atom movements:              %8llu", state->atomMovements);
  debugf("    spin-off drops created:      %8llu", state->spinOffDrops);
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

static void
simulator_snapshot(struct phymodel* model,
                   unsigned int roundno,
                   const char* progressImage) {

  assert(progressImage != 0);
  assert(index(progressImage,'%') != 0);
  const size_t numlen = 20;
  size_t len = strlen(progressImage) + numlen;
  char* tempfile = (char*)malloc(len);
  if (tempfile == 0) {
    fatals("cannot allocate memory for file name",progressImage);
    return;
  }
  memset(tempfile,0,len);
  const char* percentLocation = index(progressImage,'%');
  assert(percentLocation != 0);
  unsigned int beforePercent = percentLocation - progressImage;
  memcpy(tempfile,progressImage,beforePercent);
  snprintf(tempfile+beforePercent,len-beforePercent-1,"%u%s",roundno,percentLocation+1);
  image_modely2image(model,
		     model->ySize / 2,
		     tempfile);
  if (strstr(tempfile,".txt")) {
    FILE* f = fopen(tempfile,"r");
    if (f == 0) fatals("cannot open snapshot file",tempfile);
    int c;
    while ((c = fgetc(f)) != EOF) {
      printf("%c",c);
    }
    fclose(f);
  }
  free(tempfile);
}
