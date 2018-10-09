
#ifndef PHYMODEL_H
#define PHYMODEL_H

#define PHYMODEL_MAGIC		0xCA5EF058

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

enum material {
  material_air,
  material_rock,
  material_water
};

struct rgb {
  unsigned char r;
  unsigned char g;
  unsigned char b;
};

struct phyatom {
  enum material mat;
  struct rgb color;
};

struct phymodel {
  unsigned int magic;
  unsigned int unit;  /* in fractions of a meter, e.g., 1000 = 1mm, 100 000 = 0.01mm */
  unsigned int xSize; /* in number of units */
  unsigned int ySize; /* in number of units */
  unsigned int zSize; /* in number of units */
  struct phyatom atoms[1];
};

#define phymodel_sizeinbytes(x,y,z)	(sizeof(struct phymodel) + ((x)*(y)*(z)-1) * sizeof(struct phyatom))
#define phymodel_atomindex(m,x,y,z)	(((z) * (m)->xSize * (m)->ySize) + \
					 ((y) * (m)->xSize) +		   \
					 (x))

typedef void (*phyatom_fn)(unsigned int x,
			   unsigned int y,
			   unsigned int z,
			   struct phymodel* model,
			   struct phyatom* atom,
			   void* data);

extern struct phymodel*
phymodel_create(unsigned int unit,
		unsigned int xSize,
		unsigned int ySize,
		unsigned int zSize);
extern struct phymodel*
phymodel_initialize_rock(enum rockinitialization style,
			 int uniform,
			 unsigned int crackWidth,
			 unsigned int crackGrowthSteps,
			 double fractalShrink,
			 unsigned int fractalLevels,
			 unsigned int fractalCardinality,
			 enum crackdirection direction,
			 unsigned int unit,
			 unsigned int xSize,
			 unsigned int ySize,
			 unsigned int zSize);
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
rgb_set(struct rgb* rgb,
	unsigned char r,
	unsigned char g,
	unsigned char b);
extern void
rgb_set_black(struct rgb* rgb);
extern void
rgb_set_white(struct rgb* rgb);
extern void
phymodel_destroy(struct phymodel* model);
extern struct phymodel*
phymodel_read(const char* filename);
extern void
phymodel_write(struct phymodel* model,
	       const char* filename);

#endif /* PHYMODEL_H */
