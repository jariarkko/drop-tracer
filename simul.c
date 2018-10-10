
#include <math.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "util.h"
#include "phymodel.h"
#include "simul.h"

static void
simulator_simulate_round(struct phymodel* model,
			 double simulDropSize,
			 int drop);
static void
simulator_simulate_drop(struct phymodel* model,
			double simulDropSize);

void
simulator_simulate(struct phymodel* model,
		   unsigned int simulRounds,
		   unsigned int simulDropFrequency,
		   double simulDropSize) {
  unsigned int round;
  
  debugf("simulating %u rounds...", simulRounds);
  
  for (round = 0; round < simulRounds; round++) {
    int drop = ((round % simulDropFrequency) == 0);
    simulator_simulate_round(model,simulDropSize,drop);
  }
  
  debugf("simulation complete");
}

static void
simulator_simulate_round(struct phymodel* model,
			 double simulDropSize,
			 int drop) {
  if (drop) {
    simulator_simulate_drop(model,simulDropSize);
  }
  
  
}

static void
simulator_simulate_drop(struct phymodel* model,
			double simulDropSize) {
}
