
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

static struct simulatordrop*
simulator_droptable_findunuseddrop(struct simulatordroptable* state) {

  unsigned int i;

  assert(state->ndrops <= simulatordroptable_maxdrops);
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

struct simulatordrop*
simulator_droptable_getdrop(struct simulatordroptable* state) {
  struct simulatordrop* drop = simulator_droptable_findunuseddrop(state);
  if (drop != 0) {
    return(drop);
  } else if (state->ndrops < simulatordroptable_maxdrops) {
    drop = &state->drops[state->ndrops++];
    drop->active = 1;
    drop->index = state->ndrops - 1;
    assert(state->ndrops <= simulatordroptable_maxdrops);
    return(drop);
  } else {
    return(0);
  }
}

void
simulator_droptable_deletedrop(struct simulatordroptable* state,
			       struct simulatordrop* drop) {
  drop->active = 0;
  while (!drop->active && drop->index == state->ndrops - 1) {
    state->ndrops--;
    drop--;
  }
}

void
simulator_droptable_initialize(struct simulatordroptable* state){
  state->ndrops = 0;
}

void
simulator_droptable_deinitialize(struct simulatordroptable* state){
  memset(state,0xFF,sizeof(*state));
}
