
#ifndef DROPTABLE_H
#define DROPTABLE_H

#include "drop.h"

#define simulatordroptable_maxdrops		1024

struct simulatordroptable {
  unsigned int ndrops;
  struct simulatordrop drops[simulatordroptable_maxdrops];
};

void
simulator_droptable_initialize(struct simulatordroptable* state);
void
simulator_droptable_deinitialize(struct simulatordroptable* state);
struct simulatordrop*
simulator_droptable_getdrop(struct simulatordroptable* state);
void
simulator_droptable_deletedrop(struct simulatordroptable* state,
			       struct simulatordrop* drop);

#endif /* DROPTABLE_H */
