
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include "util.h"
#include "phymodel.h"

enum drop_tracer_operation {
  drop_tracer_operation_createrock,
  drop_tracer_operation_simulate
};

static enum drop_tracer_operation operation = drop_tracer_operation_simulate;
static const char* outputfile = 0;

static struct option long_options[] = {
  
  /* These options set a flag. */
  {"debug", no_argument,       &debug, 1},
  {"no-debug", no_argument,    &debug, 0},
  {"create-rock", no_argument, (int*)&operation, drop_tracer_operation_createrock},
  {"simulate", no_argument,    (int*)&operation, drop_tracer_operation_simulate},
  
  /* These options don’t set a flag. */
  
  {"output",  required_argument, 0, 'o'},
  
  /* End of options table */
  {0, 0, 0, 0}
};

int
main(int argc,
     char** argv) {
  
  int c;

  /*
   * Parse arguments
   */
  
  while (1) {

      /* getopt_long stores the option index here. */
      int option_index = 0;

      c = getopt_long (argc, argv, "dncso:",
                       long_options, &option_index);

      /* Detect the end of the options. */
      if (c == -1)
        break;
      
      switch (c) {
	
      case 0:
	break;

      case 'o':
	outputfile = optarg;
	break;
	
      case '?':
	/* getopt_long already printed an error message. */
	break;

      default:
	fatal("drop-tracer: unrecognised option","");
      }
  }

  if (optind < argc) {
    while (optind < argc) {
      printf ("Non-option argument %s\n", argv[optind]);
      optind++;
    }
  }
  
  /*
   * Perform requested action
   */
  
  switch (operation) {
  case drop_tracer_operation_createrock:
    /* ... */
    break;
    
  case drop_tracer_operation_simulate:
    /* ... */
    break;
    
  default:
    fatal("drop-tracer: operation is invalid",0);
  }
  
  exit(0);
}
  
