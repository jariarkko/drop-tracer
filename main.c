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

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <ctype.h>
#include "util.h"
#include "phymodel.h"
#include "rock.h"
#include "simul.h"
#include "image.h"

enum drop_tracer_operation {
  drop_tracer_operation_createrock,
  drop_tracer_operation_simulate,
  drop_tracer_operation_copy,
  drop_tracer_operation_image,
  drop_tracer_operation_model
};

static enum drop_tracer_operation operation = drop_tracer_operation_simulate;
static const char* inputfile = 0;
static const char* outputfile = 0;
static unsigned int unit = 1000 * 10; /* 0.1 mm */
static unsigned int xSize = 1024;      /* 12.8 mm */
static unsigned int ySize = 1024;      /* 12.8 mm */
static unsigned int zSize = 1024;      /* 12.8 mm */
static enum rockinitialization creationStyle = rockinitialization_simplecrack;
static unsigned int creationStyleUniform = 0;
static unsigned int creationStyleCrackWidth = 10;
static unsigned int creationStyleCrackGrowthSteps = 10;
static double creationStyleFractalShrink = 0.5;
static unsigned int creationStyleFractalLevels = 2;
static unsigned int creationStyleFractalCardinality = 3;
static enum crackdirection creationStyleDirection = crackdirection_y;
static int creationStyleCave = 0;
static unsigned int imageZ = 10;
static unsigned int imageX = 0;
static unsigned int imageY = 0;
static unsigned int simulRounds = 1000;
static unsigned int simulDropFrequency = 100;
static unsigned int simulDropSize = 30; /* in atoms */
static int simulTextualSnapshot = 0;
static const unsigned int maxTextualSnapshotDimension = 150;
static const char* progressImages = 0;

static struct option long_options[] = {
  
  /*
   * These options set a fixed flag or mode, and do not need extra parsing or parameters.
   */
  
  {"debug", no_argument,               &debug, 1},
  {"no-debug", no_argument,            &debug, 0},
  {"deepdebug", no_argument,           &deepdebug, 1},
  {"no-debug", no_argument,            &deepdebug, 0},
  {"deepdeepdebug", no_argument,       &deepdeepdebug, 1},
  {"no-deepdeepdebug", no_argument,    &deepdeepdebug, 0},
  {"create-rock", no_argument,         (int*)&operation, drop_tracer_operation_createrock},
  {"simple-crack", no_argument,        (int*)&creationStyle, rockinitialization_simplecrack},
  {"fractal-crack", no_argument,       (int*)&creationStyle, rockinitialization_fractalcrack},
  {"uniform", no_argument,             (int*)&creationStyleUniform, 1},
  {"non-uniform", no_argument,         (int*)&creationStyleUniform, 0},
  {"vertical-crack", no_argument,      (int*)&creationStyleDirection, (int)crackdirection_y},
  {"cave", no_argument,                (int*)&creationStyleCave, 1},
  {"no-cave", no_argument,             (int*)&creationStyleCave, 0},
  {"horizontal-crack", no_argument,    (int*)&creationStyleDirection, (int)crackdirection_x},
  {"simulate", no_argument,            (int*)&operation, drop_tracer_operation_simulate},
  {"copy", no_argument,                (int*)&operation, drop_tracer_operation_copy},
  {"image", no_argument,               (int*)&operation, drop_tracer_operation_image},
  {"model", no_argument,               (int*)&operation, drop_tracer_operation_model},
  {"textual-snapshot", no_argument,    (int*)&simulTextualSnapshot, 1},
  {"no-textual-snapshot", no_argument, (int*)&simulTextualSnapshot, 0},
  
  /*
   * These options need an argument
   */
  
  {"unit",                         required_argument, 0, 'u'},
  {"crack-width",                  required_argument, 0, 'r'},
  {"crack-growth-steps",           required_argument, 0, 't'},
  {"fractal-shrink",               required_argument, 0, 'f'},
  {"fractal-levels",               required_argument, 0, 'L'},
  {"fractal-cardinality",          required_argument, 0, 'F'},
  {"drop-frequency",               required_argument, 0, 'D'},
  {"drop-size",                    required_argument, 0, 'P'},
  {"xsize",                        required_argument, 0, 'x'},
  {"ysize",                        required_argument, 0, 'y'},
  {"zsize",                        required_argument, 0, 'z'},
  {"imagez",                       required_argument, 0, 'Z'},
  {"imagex",                       required_argument, 0, 'X'},
  {"imagey",                       required_argument, 0, 'Y'},
  {"rounds",                       required_argument, 0, 'R'},
  {"input",                        required_argument, 0, 'i'},
  {"output",                       required_argument, 0, 'o'},
  {"seed",                         required_argument, 0, 'S'},
  {"progress-images",              required_argument, 0, 'M'},
  
  /*
   * End of the options table
   */
  
  {0, 0, 0, 0}
};

int
main(int argc,
     char** argv) {
  
  struct phymodel* model = 0;
  time_t t;
  unsigned long seed = (unsigned long)time(&t);
  int ival;
  int c;

  /*
   * Parse arguments
   */
  
  while (1) {

      /* getopt_long stores the option index here. */
      int option_index = 0;

      c = getopt_long (argc, argv, "dncsgmo:u:x:y:z:r:i:Z:X:Y:R:",
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
	creationStyleCrackWidth = atoi(optarg);
	if (creationStyleCrackWidth <= 0) {
	  fatals("creation style crack width must be a positive integer, got",optarg);
	}
	break;
	
      case 't':
	creationStyleCrackGrowthSteps = atoi(optarg);
	if (creationStyleCrackGrowthSteps <= 0) {
	  fatals("creation style crack growth steps must be a positive integer, got",optarg);
	}
	break;
	
      case 'f':
	creationStyleFractalShrink = atof(optarg);
	if (creationStyleFractalShrink <= 0.0) {
	  fatals("creation style fractal shrink must be a positive floating number, got",optarg);
	}
	break;
	
      case 'L':
	creationStyleFractalLevels = atoi(optarg);
	if (creationStyleFractalLevels <= 0) {
	  fatals("creation style fractal levels must be a positive integer, got",optarg);
	}
	break;
	
      case 'F':
	creationStyleFractalCardinality = atoi(optarg);
	if (creationStyleFractalCardinality <= 0) {
	  fatals("creation style fractal cardinality must be a positive integer, got",optarg);
	}
	break;
	
      case 'D':
	simulDropFrequency = atoi(optarg);
	if (simulDropFrequency <= 0) {
	  fatals("simulator drop frequency must be a positive integer, got",optarg);
	}
	break;
	
      case 'P':
	simulDropSize = atoi(optarg);
	if (simulDropSize == 0) {
	  fatals("simulator drop size must be a positive integer, got",optarg);
	}
	break;
        
      case 'S':
        seed = (long)atoi(optarg);
	if (seed == 0) {
	  fatals("random seed must be a positive integer, got",optarg);
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
	imageX = 0;
	imageY = 0;
	break;
	
      case 'X':
	imageX = ival = atoi(optarg);
	if (ival < 0) {
	  fatals("image coordinate must be a non-negative integer",optarg);
	}
	imageZ = 0;
	imageY = 0;
	break;
	
      case 'Y':
	imageY = ival = atoi(optarg);
	if (ival < 0) {
	  fatals("image coordinate must be a non-negative integer",optarg);
	}
	imageZ = 0;
	imageX = 0;
	break;
	
      case 'i':
	inputfile = optarg;
	break;
	
      case 'o':
	outputfile = optarg;
	break;

      case 'M':
	progressImages = optarg;
        if (index(progressImages,'%') == 0) {
	  fatals("progress image must have percent sign, got only",optarg);
        }
        break;
        
      case 'R':
	simulRounds = atoi(optarg);
	if (simulRounds > 0 && strlen(optarg) > 0 && isalpha(optarg[strlen(optarg)-1])) {
	  switch (toupper(optarg[strlen(optarg)-1])) {
	  case 'K':
	    simulRounds *= 1000;
	    break;
	  case 'M':
	    simulRounds *= 1000 * 1000;
	    break;
	  case 'B':
	    simulRounds *= 1000 * 1000 * 1000;
	    break;
	  default:
	    fatals("unrecognised unit in --rounds arguments, expected K, M or B, got", optarg);
	    break;
	  }
	}
	if (simulRounds <= 0) {
	  fatals("rounds should be a positive number",optarg);
	}
	break;
	
      case '?':
	/* getopt_long already printed an error message. */
	fatal("invalid option");
	break;
	
      default:
	fatal("drop-tracer: unrecognised option");
      }
  }

  /*
   * Check extra arguments
   */
  
  if (optind < argc) {
    while (optind < argc) {
      fatals("non-option argument not allowed", argv[optind]);
      optind++;
    }
  }
  
  /*
   * Initialize system
   */
  
  srand(seed);
  if (deepdeepdebug) deepdebug = 1;
  if (deepdebug) debug = 1;
  
  /*
   * Perform requested action
   */
  
  switch (operation) {

  case drop_tracer_operation_createrock:

    /*
     * Create a rock model, a roof on top and (optionally) a cave
     * tunnel underneath
     */
    
    if (inputfile != 0) {
      fatal("input file should not be specified for --create-rock");
    }
    if (outputfile == 0) {
      fatal("output file should be specified for --create-rock");
    }
    unsigned int freeSpaceAboveRock = zSize > 30 ? 10 : 1;
    unsigned int rockThickness = zSize > 30 ? 10 : 1;
    model = phymodel_initialize_rock(freeSpaceAboveRock,
				     rockThickness,
				     creationStyle,
				     creationStyleUniform,
				     creationStyleCrackWidth,
				     creationStyleCrackGrowthSteps,
				     creationStyleFractalShrink,
				     creationStyleFractalLevels,
				     creationStyleFractalCardinality,
				     creationStyleDirection,
				     creationStyleCave,
				     unit,
				     xSize,
				     ySize,
				     zSize);
    debugf("now doing the main mod write!");
    phymodel_write(model,outputfile);
    phymodel_destroy(model);
    break;

  case drop_tracer_operation_simulate:

    /*
     * Simulate the model by dropping water droplets
     */
    
    if (inputfile == 0) {
      fatal("input file should be specified for --simulate");
    }
    if (outputfile == 0) {
      fatal("output file should be specified for --simulate");
    }
    model = phymodel_read(inputfile);
    if (model == 0) {
      fatals("failed to read input model",inputfile);
    }
    if (model->xSize > maxTextualSnapshotDimension ||
	model->ySize > maxTextualSnapshotDimension ||
	model->zSize > maxTextualSnapshotDimension) {
      fatalu("cannot use --textual-snapshot for models larger than a limit in any dimension",
	     maxTextualSnapshotDimension);
    }
    simulator_simulate(model,
		       simulRounds,
		       simulDropFrequency,
		       simulDropSize,
		       progressImages);
    phymodel_write(model,outputfile);
    phymodel_destroy(model);
    break;
    
  case drop_tracer_operation_copy:

    /*
     * Copy the model
     */
    
    if (inputfile == 0) {
      fatal("input file should be specified for --copy");
    }
    if (outputfile == 0) {
      fatal("output file should be specified for --copy");
    }
    model = phymodel_read(inputfile);
    if (model == 0) {
      fatals("failed to read input model",inputfile);
    }
    phymodel_write(model,outputfile);
    phymodel_destroy(model);
    break;
    
  case drop_tracer_operation_image:

    /*
     * Convert the (current) model to a 2D image along a chosen axis
     */
    
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
    if (imageX > 0) {
      image_modelx2image(model,
			 imageX,
			 outputfile);
    } else if (imageY > 0) {
      image_modely2image(model,
			 imageY,
			 outputfile);
    } else {
      image_modelz2image(model,
			 imageZ,
			 outputfile);
    }
    phymodel_destroy(model);
    break;
    
  case drop_tracer_operation_model:

    /*
     * Convert the (current) model to a 3D model that can be, for
     * instance, printed with a 3D-printer
     */
    
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
    image_model2image3d(model,
			outputfile);
    phymodel_destroy(model);
    break;
    
  default:

    /*
     * Failure cases
     */
    
    fatal("drop-tracer: operation is invalid");
  }
  
  exit(0);
}
  
