
#include <stdio.h>
#include <stdlib.h>
#include "util.h"

int debug = 0;

void
fatal(const char* message) {
  fprintf(stderr,"drop-tracer: error: %s -- exit\n",
	  message);
  exit(1);
}

void
fatals(const char* message,
       const char* string) {
  fprintf(stderr,"drop-tracer: error: %s: %s -- exit\n",
	  message,
	  string);
  exit(1);
}

void
fatalu(const char* message,
       unsigned int x) {
  fprintf(stderr,"drop-tracer: error: %s: %u -- exit\n",
	  message,
	  x);
  exit(1);
}

void
fatalsu(const char* message,
	const char* string,
	unsigned int x) {
  fprintf(stderr,"drop-tracer: error: %s: %s: %u -- exit\n",
	  message,
	  string,
	  x);
  exit(1);
}

void
fatalxx(const char* message,
	unsigned int x1,
	unsigned int x2) {
  fprintf(stderr,"drop-tracer: error: %s: %x: %x -- exit\n",
	  message,
	  x1,
	  x2);
  exit(1);
}

void
fatalsxx(const char* message,
	 const char* string,
	 unsigned int x1,
	 unsigned int x2) {
  fprintf(stderr,"drop-tracer: error: %s: %s: %x: %x -- exit\n",
	  message,
	  string,
	  x1,
	  x2);
  exit(1);
}
