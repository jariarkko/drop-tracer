
#include <math.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "util.h"
#include "phymodel.h"
#include "simul.h"

void
simulator_simulate(struct phymodel* model,
		   unsigned int simulRounds,
		   unsigned int simulDropFrequency,
		   double simulDropSize) {
  debugf("simulating %u rounds...", simulRounds);
}
