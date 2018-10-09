
#ifndef SIMUL_H
#define SIMUL_H

extern void
simulator_simulate(struct phymodel* model,
		   unsigned int simulRounds,
		   unsigned int simulDropFrequency,
		   double simulDropSize);

#endif /* SIMUL_H */
