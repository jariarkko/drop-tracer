
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include "util.h"
#include "phymodel.h"
#include "image.h"

enum drop_tracer_operation {
  drop_tracer_operation_createrock,
  drop_tracer_operation_simulate,
  drop_tracer_operation_image,
  drop_tracer_operation_model
};

static enum drop_tracer_operation operation = drop_tracer_operation_simulate;
static const char* inputfile = 0;
static const char* outputfile = 0;
static unsigned int unit = 1000 * 10; /* 0.1 mm */
static unsigned int xSize = 128;      /* 12.8 mm */
static unsigned int ySize = 128;      /* 12.8 mm */
static unsigned int zSize = 128;      /* 12.8 mm */
static enum rockinitialization creationStyle = rockinitialization_simplecrack;
static unsigned int creationStyleParam = 1;
static unsigned int imageZ = 0;
static unsigned int imageX = 0;
static unsigned int imageY = 0;

static struct option long_options[] = {
  
  /* These options set a flag. */
  {"debug", no_argument,       &debug, 1},
  {"no-debug", no_argument,    &debug, 0},
  {"create-rock", no_argument, (int*)&operation, drop_tracer_operation_createrock},
  {"simulate", no_argument,    (int*)&operation, drop_tracer_operation_simulate},
  {"image", no_argument,       (int*)&operation, drop_tracer_operation_image},
  {"model", no_argument,       (int*)&operation, drop_tracer_operation_model},
  
  /* These options don’t set a flag. */
  
  {"unit",                  required_argument, 0, 'u'},
  {"creation-parameter",    required_argument, 0, 'r'},
  {"xsize",                 required_argument, 0, 'x'},
  {"ysize",                 required_argument, 0, 'y'},
  {"zsize",                 required_argument, 0, 'z'},
  {"imagez",                required_argument, 0, 'Z'},
  {"imagex",                required_argument, 0, 'X'},
  {"imagey",                required_argument, 0, 'Y'},
  {"input",                 required_argument, 0, 'i'},
  {"output",                required_argument, 0, 'o'},
  
  /* End of options table */
  {0, 0, 0, 0}
};

int
main(int argc,
     char** argv) {
  
  struct phymodel* model = 0;
  int ival;
  int c;

  /*
   * Parse arguments
   */
  
  while (1) {

      /* getopt_long stores the option index here. */
      int option_index = 0;

      c = getopt_long (argc, argv, "dncsgmo:u:x:y:z:r:i:Z:X:Y:",
                       long_options, &option_index);

      /* Detect the end of the options. */
      if (c == -1)
        break;
      
      switch (c) {
	
      case 0:
	break;

      case 'u':
	unit = atoi(optarg);
	if (unit <= 0) {
	  fatals("unit must be a positive integer, expressed as units in one meter, got",optarg);
	}
	break;
	
      case 'r':
	creationStyleParam = atoi(optarg);
	if (creationStyleParam <= 0) {
	  fatals("creation style parameter must be a positive integer, got",optarg);
	}
	break;
	
      case 'x':
	xSize = atoi(optarg);
	if (xSize <= 0) {
	  fatals("size must be a positive integer, got",optarg);
	}
	break;
	
      case 'y':
	ySize = atoi(optarg);
	if (ySize <= 0) {
	  fatals("size must be a positive integer, got",optarg);
	}
	break;
	
      case 'z':
	zSize = atoi(optarg);
	if (zSize <= 0) {
	  fatals("size must be a positive integer, got",optarg);
	}
	break;
	
      case 'Z':
	imageZ = ival = atoi(optarg);
	if (ival < 0) {
	  fatals("image coordinate must be a non-negative integer",optarg);
	}
	break;
	
      case 'X':
	imageX = ival = atoi(optarg);
	if (ival < 0) {
	  fatals("image coordinate must be a non-negative integer",optarg);
	}
	break;
	
      case 'Y':
	imageY = ival = atoi(optarg);
	if (ival < 0) {
	  fatals("image coordinate must be a non-negative integer",optarg);
	}
	break;
	
      case 'i':
	inputfile = optarg;
	break;
	
      case 'o':
	outputfile = optarg;
	break;
	
      case '?':
	/* getopt_long already printed an error message. */
	break;

      default:
	fatal("drop-tracer: unrecognised option");
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
    if (inputfile != 0) {
      fatal("input file should not be specified for --create-rock");
    }
    if (outputfile == 0) {
      fatal("output file should be specified for --create-rock");
    }
    model = phymodel_initialize_rock(creationStyle,
				     creationStyleParam,
				     unit,
				     xSize,
				     ySize,
				     zSize);
    phymodel_write(model,outputfile);
    phymodel_destroy(model);
    break;
    
  case drop_tracer_operation_simulate:
    if (inputfile == 0) {
      fatal("input file should be specified for --simulate");
    }
    if (outputfile == 0) {
      fatal("output file should be specified for --simulate");
    }
    /* ... */
    fatal("simulation not implemented");
    break;
    
  case drop_tracer_operation_image:
    if (inputfile == 0) {
      fatal("input file should be specified for --image");
    }
    if (outputfile == 0) {
      fatal("output file should be specified for --image");
    }
    model = phymodel_read(inputfile);
    if (model == 0) {
      fatals("failed to read input model",inputfile);
    }
    image_modelz2image(model,
		       imageZ,
		       outputfile);
    phymodel_destroy(model);
    break;
    
  case drop_tracer_operation_model:
    if (inputfile == 0) {
      fatal("input file should be specified for --model");
    }
    if (outputfile == 0) {
      fatal("output file should be specified for --model");
    }
    model = phymodel_read(inputfile);
    if (model == 0) {
      fatals("failed to read input model",inputfile);
    }
    image_modelz2image3d(model,
			 outputfile);
    phymodel_destroy(model);
    break;
    
  default:
    fatal("drop-tracer: operation is invalid");
  }
  
  exit(0);
}
  
