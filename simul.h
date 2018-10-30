
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

#ifndef SIMUL_H
#define SIMUL_H

#include "drop.h"
#include "droptable.h"

struct simulatorstate {
  unsigned long long successfullyCreatedDrops;
  unsigned long long failedDropAllocations;
  unsigned long long failedDropHoleFinding;
  unsigned long long failedDropHoleFree;
  unsigned long long rounds;
  unsigned long long dropMovements;
  unsigned long long atomCreations;
  unsigned long long atomMovements;
  struct simulatordroptable drops;
};

extern void
simulator_simulate(struct phymodel* model,
		   unsigned int simulRounds,
		   unsigned int simulDropFrequency,
		   unsigned int simulDropSize);

#endif /* SIMUL_H */
