
#include <math.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "util.h"
#include "phymodel.h"

unsigned char
phyatom_longrgbtoshort(unsigned char rgb) {
  if (rgb >= 0x00 && rgb <= 0x3F) {
    return(0x00);
  } else if (rgb >= 0x40 && rgb <= 0x7F) {
    return(0x01);
  } else if (rgb >= 0x80 && rgb <= 0xBF) {
    return(0x02);
  } else if (rgb >= 0xC0 && rgb <= 0xFF) {
    return(0x03);
  } else {
    fatal("invalid long RGB value");
    return(0x00);
  }
}

unsigned char
phyatom_shortrgbtolong(unsigned char val) {
  switch (val) {
  case 0:
    return(0x00);
  case 1:
    return(0x50);
  case 2:
    return(0xB0);
  case 3:
    return(0xFF);
  default:
    fatal("invalid short RGB value");
    return(0x00);
  }
}

void
rgb_set(struct rgb* rgb,
	unsigned char r,
	unsigned char g,
	unsigned char b) {
  rgb->r = r;
  rgb->g = g;
  rgb->b = b;
}

void
rgb_set_black(struct rgb* rgb) {
  rgb->r = rgb->g = rgb->b = 0x00;
}

void
rgb_set_blue(struct rgb* rgb) {
  rgb->r = 0x00;
  rgb->g = 0x00;
  rgb->b = 0xFF;
}

void
rgb_set_green(struct rgb* rgb) {
  rgb->r = 0x00;
  rgb->g = 0xFF;
  rgb->b = 0x00;
}

void
rgb_set_red(struct rgb* rgb) {
  rgb->r = 0xFF;
  rgb->g = 0x00;
  rgb->b = 0x00;
}

void
rgb_set_white(struct rgb* rgb) {
  rgb->r = rgb->g = rgb->b = 0xFF;
}
