
#include <stdio.h>
#include <stdlib.h>
#include "util.h"

int debug = 0;

void fatal(const char* message,
	   const char* string) {
  fprintf(stderr,"drop-tracer: error: %s",
	  message);
  if (string != 0) {
    fprintf(stderr,"%s",string);
  }
  fprintf(stderr," -- exit\n");
  exit(1);
}

