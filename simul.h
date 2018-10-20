
#ifndef SIMUL_H
#define SIMUL_H

#define simulatorstate_maxdrops		1024
#define simulatorstate_maxatomsperdrop	 128

struct atomcoordinates {
  unsigned int x;
  unsigned int y;
  unsigned int z;
};

enum direction {
  direction_x_towards0 = 0,
  direction_x_towardsn = 1,
  direction_y_towards0 = 2,
  direction_y_towardsn = 3,
  direction_z_towards0 = 4,
  direction_z_towardsn = 5,
  direction_howmany    = 6
};

struct simulatordrop {
  int active;                 /* 1 when used */
  unsigned int index;         /* index in the drops table of struct simulatorstate */
  double calcite;             /* 0 .. 1.0 */
  struct rgb calcitecolor;    /* color of calcite contained in the water */
  unsigned int natoms;        /* in how many phyatoms there are water */
  struct atomcoordinates atoms[simulatorstate_maxatomsperdrop];
};

struct simulatorstate {
  unsigned int successfullyCreatedDrops;
  unsigned int failedDropAllocations;
  unsigned int failedDropHoleFinding;
  unsigned int failedDropHoleFree;
  unsigned int ndrops;
  struct simulatordrop drops[simulatorstate_maxdrops];
};

extern void
simulator_simulate(struct phymodel* model,
		   unsigned int simulRounds,
		   unsigned int simulDropFrequency,
		   unsigned int simulDropSize);

#endif /* SIMUL_H */
