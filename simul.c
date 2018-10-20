
#include <math.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "util.h"
#include "phymodel.h"
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
static struct simulatordrop*
simulator_state_getdrop(struct simulatorstate* state);
static void
simulator_state_deletedrop(struct simulatorstate* state,
			   struct simulatordrop* drop);
static struct simulatordrop*
simulator_state_findunuseddrop(struct simulatorstate* state);
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
			    unsigned int dropSize,
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
	 simulatorstate_maxdrops,
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
  
}

static void
simulator_simulate_drop(struct simulatorstate* state,
			struct phymodel* model,
			unsigned int dropSize,
			unsigned int startingLevel) {
  struct simulatordrop* drop = simulator_state_getdrop(state);
  if (drop == 0) {
    state->failedDropAllocations++;
  } else {
    struct atomcoordinates dropplace;
    enum direction direction;
    assert(drop->active);
    drop->calcite = 1.0;
    rgb_set_white(&drop->calcitecolor);
    drop->natoms = dropSize;
    simulator_find_randomdropplaceanddirection(state,model,&dropplace,&direction,startingLevel);
    if (!simulator_move_dropuntilhole(state,model,&dropplace,direction)) {
      state->failedDropHoleFinding++;
      simulator_state_deletedrop(state,drop);
      return;
    }
    if (!simulator_move_dropintohole(state,model,&dropplace,dropSize,drop)) {
      state->failedDropHoleFree++;
      simulator_state_deletedrop(state,drop);
      return;
    }
    state->successfullyCreatedDrops++;
  }
}

static void
simulator_state_initialize(struct simulatorstate* state,
			   struct phymodel* model) {
  memset(state,0,sizeof(*state));
  state->ndrops = 0;
}

static void
simulator_state_deinitialize(struct simulatorstate* state,
			     struct phymodel* model) {
  memset(state,0xFF,sizeof(*state));
}

static struct simulatordrop*
simulator_state_findunuseddrop(struct simulatorstate* state) {
  
  unsigned int i;
  
  assert(state->ndrops <= simulatorstate_maxdrops);
  for (i = 0; i < state->ndrops; i++) {
    struct simulatordrop* drop = &state->drops[i];
    if (!drop->active) {
      drop->active = 1;
      drop->index = i;
      return(drop);
    }
  }

  return(0);
}

static struct simulatordrop*
simulator_state_getdrop(struct simulatorstate* state) {
  struct simulatordrop* drop = simulator_state_findunuseddrop(state);
  if (drop != 0) {
    return(drop);
  } else if (state->ndrops < simulatorstate_maxdrops) {
    drop = &state->drops[state->ndrops++];
    drop->active = 1;
    drop->index = state->ndrops - 1;
    assert(state->ndrops <= simulatorstate_maxdrops);
    return(drop);
  } else {
    return(0);
  }
}

static void
simulator_state_deletedrop(struct simulatorstate* state,
			   struct simulatordrop* drop) {
  drop->active = 0;
  while (!drop->active && drop->index == state->ndrops - 1) {
    state->ndrops--;
    drop--;
  }
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

  phyatom* atom;
  
  switch (direction) {
    
  case direction_x_towards0:
    while (1) {
      atom = phymodel_getatom(model,place->x,place->y,place->z);
      if (phyatom_mat(atom) != material_rock) return(1);
      else if (place->x == 0) return(0);
      else place->x--;
    }
    
  case direction_x_towardsn:
    while (1) {
      atom = phymodel_getatom(model,place->x,place->y,place->z);
      if (phyatom_mat(atom) != material_rock) return(1);
      else if (place->x == model->xSize - 1) return(0);
      else place->x++;
    }
    
  case direction_y_towards0:
    while (1) {
      atom = phymodel_getatom(model,place->x,place->y,place->z);
      if (phyatom_mat(atom) != material_rock) return(1);
      else if (place->y == 0) return(0);
      else place->y--;
    }
    
  case direction_y_towardsn:
    while (1) {
      atom = phymodel_getatom(model,place->x,place->y,place->z);
      if (phyatom_mat(atom) != material_rock) return(1);
      else if (place->y == model->ySize - 1) return(0);
      else place->y++;
    }

  case direction_z_towards0:
    while (1) {
      atom = phymodel_getatom(model,place->x,place->y,place->z);
      if (phyatom_mat(atom) != material_rock) return(1);
      else if (place->z == 0) return(0);
      else place->z--;
    }
    
  case direction_z_towardsn:
    while (1) {
      atom = phymodel_getatom(model,place->x,place->y,place->z);
      if (phyatom_mat(atom) != material_rock) return(1);
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
			    unsigned int dropSize,
			    struct simulatordrop* drop) {
  /* ... */
  return(0);
}

static void
simulator_stats(struct simulatorstate* state,
		struct phymodel* model) {
  
  debugf("  successfully created drops:    %8u", state->successfullyCreatedDrops);
  debugf("  failed drops:                  %8u",
	 state->failedDropAllocations + state->failedDropHoleFinding + state->failedDropHoleFree);
  debugf("    unable to allocate:          %8u", state->failedDropAllocations);
  debugf("    unable to find a hole:       %8u", state->failedDropHoleFinding);
  debugf("    found hole not free:         %8u", state->failedDropHoleFree);
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
