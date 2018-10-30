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

#ifndef PHYMODEL_H
#define PHYMODEL_H

#define PHYMODEL_MAGIC		0xCA5EF058

enum material {
  material_air   = 0,
  material_rock  = 1,
  material_water = 2
};

struct rgb {
  unsigned char r;
  unsigned char g;
  unsigned char b;
};

typedef unsigned char phyatom;

#define phyatom_reset(a)		(*(a) = 0x00)
#define phyatom_mat(a)                  ((enum material)((*(a))&0x03))
#define phyatom_set_mat(a,m)            (*(a) = ((unsigned char)((*(a))&0xFC) + (m)))
#define phyatom_color_rgb(a)            (((*(a)) >> 2) & 0x3F)
#define phyatom_color_rgb_r(a)          ((phyatom_color_rgb(a) >> 4) & 0x03)
#define phyatom_color_rgb_g(a)          ((phyatom_color_rgb(a) >> 2) & 0x03)
#define phyatom_color_rgb_b(a)          ((phyatom_color_rgb(a) >> 0) & 0x03)
#define phyatom_color(rgb,a)            { ((rgb)->r = phyatom_shortrgbtolong(phyatom_color_rgb_r(a)));   \
                                          ((rgb)->g = phyatom_shortrgbtolong(phyatom_color_rgb_g(a)));   \
                                          ((rgb)->b = phyatom_shortrgbtolong(phyatom_color_rgb_b(a))); }
#define phyatom_set_color(a,rgb)        (*(a) = (((*(a))&0x03) +	                                 \
			   		        ((phyatom_longrgbtoshort((rgb)->r) & 0x03) << 6) +       \
			   		        ((phyatom_longrgbtoshort((rgb)->g) & 0x03) << 4) +       \
						((phyatom_longrgbtoshort((rgb)->b) & 0x03) << 2)))

/*
 * The coordinate systems is (x,y,z) and the coordinates represent a
 * cube as follows:
 *
 *                        .
 *                       /|\  z -- vertical
 *                        |
 *                        |
 *                  _     |
 *                 /\     |
 *          y --     \    |
 *        horizontal  \   |
 *                     \  |
 *                      \ |
 *                       \|
 *                        o--------------->   x -- horizontal
 *
 *  
 */

struct phymodel {
  unsigned int magic;
  unsigned int unit;  /* in fractions of a meter, e.g., 1000 = 1mm, 100 000 = 0.01mm */
  unsigned int xSize; /* in number of units */
  unsigned int ySize; /* in number of units */
  unsigned int zSize; /* in number of units */
  phyatom atoms[1];
};

#define phymodel_sizeinbytes(x,y,z)	(sizeof(struct phymodel) + ((x)*(y)*(z)-1) * sizeof(phyatom))
#define phymodel_isvalid(m)		((m) != 0 && (m)->magic == PHYMODEL_MAGIC)
#define phymodel_atomindex(m,x,y,z)	(((z) * (m)->xSize * (m)->ySize) + \
					 ((y) * (m)->xSize) +		   \
					 (x))
#define phymodel_atommat(m,x,y,z)       phyatom_mat(phymodel_getatom((m),(x),(y),(z)))
#define phymodel_atomisfree(m,x,y,z)    (phyatom_mat(phymodel_getatom((m),(x),(y),(z))) == material_air)

typedef void (*phyatom_fn)(unsigned int x,
			   unsigned int y,
			   unsigned int z,
			   struct phymodel* model,
			   phyatom* atom,
			   void* data);

extern struct phymodel*
phymodel_create(unsigned int unit,
		unsigned int xSize,
		unsigned int ySize,
		unsigned int zSize);
extern phyatom*
phymodel_getatom(struct phymodel* model,
		 unsigned int x,
		 unsigned int y,
		 unsigned int z);
extern void
phymodel_mapatoms(struct phymodel* model,
		  phyatom_fn fn,
		  void* data);
extern void
phymodel_mapatoms_atz(struct phymodel* model,
		      unsigned int z,
		      phyatom_fn fn,
		      void* data);
extern void
phymodel_mapatoms_atx(struct phymodel* model,
		      unsigned int x,
		      phyatom_fn fn,
		      void* data);
extern void
phymodel_mapatoms_aty(struct phymodel* model,
		      unsigned int y,
		      phyatom_fn fn,
		      void* data);
extern void
phymodel_mapatoms_atdistance2dx(struct phymodel* model,
				unsigned int x,
				unsigned int origoy,
				unsigned int origoz,
				unsigned int distance,
				phyatom_fn fn,
				void* data);
extern void
phymodel_mapatoms_atdistance2dy(struct phymodel* model,
				unsigned int origox,
				unsigned int y,
				unsigned int origoz,
				unsigned int distance,
				phyatom_fn fn,
				void* data);
extern void
phymodel_mapatoms_atdistance2dz(struct phymodel* model,
				unsigned int origox,
				unsigned int origoy,
				unsigned int z,
				unsigned int distance,
				phyatom_fn fn,
				void* data);
extern void
phymodel_mapatoms_atdistance3d(struct phymodel* model,
			       unsigned int origox,
			       unsigned int origoy,
			       unsigned int origoz,
			       unsigned int distance,
			       phyatom_fn fn,
			       void* data);
extern void
rgb_set(struct rgb* rgb,
	unsigned char r,
	unsigned char g,
	unsigned char b);
extern void
rgb_set_black(struct rgb* rgb);
extern void
rgb_set_white(struct rgb* rgb);
extern void
rgb_set_blue(struct rgb* rgb);
extern void
rgb_set_red(struct rgb* rgb);
extern void
rgb_set_green(struct rgb* rgb);
extern void
phymodel_destroy(struct phymodel* model);
extern struct phymodel*
phymodel_read(const char* filename);
extern void
phymodel_write(struct phymodel* model,
	       const char* filename);
extern unsigned char
phyatom_longrgbtoshort(unsigned char rgb);
extern unsigned char
phyatom_shortrgbtolong(unsigned char val);
extern double
phymodel_distance2d(unsigned int x1,
		    unsigned int y1,
		    unsigned int x2,
		    unsigned int y2);
extern double
phymodel_distance3d(unsigned int x1,
		    unsigned int y1,
		    unsigned int z1,
		    unsigned int x2,
		    unsigned int y2,
		    unsigned int z2);
#endif /* PHYMODEL_H */
