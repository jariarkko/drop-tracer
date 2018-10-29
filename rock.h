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

#ifndef ROCK_H
#define ROCK_H

enum rockinitialization {
  rockinitialization_simplecrack,
  rockinitialization_fractalcrack
};

enum crackdirection {
  crackdirection_y,
  crackdirection_x
};

#define crackdirection_is_y(d)    ((d) == crackdirection_y)
#define crackdirection_is_x(d)    ((d) == crackdirection_x)

extern struct phymodel*
phymodel_initialize_rock(unsigned int freeSpaceAboveRock,
			 unsigned int rockThickness,
			 enum rockinitialization style,
			 int uniform,
			 unsigned int crackWidth,
			 unsigned int crackGrowthSteps,
			 double fractalShrink,
			 unsigned int fractalLevels,
			 unsigned int fractalCardinality,
			 enum crackdirection direction,
			 int cave,
			 unsigned int unit,
			 unsigned int xSize,
			 unsigned int ySize,
			 unsigned int zSize);
extern void
phymodel_initialize_rock_cavetunnel(struct phymodel* model,
				    enum crackdirection direction,
				    unsigned int startZ);
extern void
phymodel_set_rock_material(struct phymodel* model,
			   unsigned int x,
			   unsigned int y,
			   unsigned int z);
extern void
phymodel_set_rock_crackmaterial(struct phymodel* model,
				unsigned int x,
				unsigned int y,
				unsigned int z);
extern void
phymodel_set_rock_crackmaterial_thickness(struct phymodel* model,
					  unsigned int x,
					  unsigned int y,
					  unsigned int startZ,
					  unsigned int zThickness);

#endif /* ROCK_H */
