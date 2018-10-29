/*
 * **************************************************************************
 * ****************************                     *************************
 * ***************************  D R O P T R A C E R  ************************
 * ****************************                     *************************
 * **************************************************************************
 * *******      *****    *****  **    *************      ***    ****  *******
 * ****	  	 *** 	  ***  	**     	*********      	 ***   	 ** 	 ****
 * *   	       	  *    	   * 	**	  *****	       	 ** 	 **	    *
 *  		   	   *   	 O     	   *** 		 **	  *
 * 		  	   o   	       	   ***		 **	  *
 *     	       	       	    		    * 		  *
 * 		  			    o		  *
 * 		  			     		  o
 * 		  			    o
 *
 *
 *                          Cave Forms Simulation Software
 *                                Jari Arkko, 2018
 *
 *                      https://github.com/jariarkko/drop-tracer
 *                              License: BSD 3-Clause
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdarg.h>
#include <string.h>
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
fataluu(const char* message,
	unsigned int x1,
	unsigned int x2) {
  fprintf(stderr,"drop-tracer: error: %s: %u: %u -- exit\n",
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

void
debugf(const char* format, ...) {
  
  assert(format != 0);
 
  if (debug) {
    
    va_list args;
    printf("debug: ");
    va_start (args, format);
    vprintf(format, args);
    va_end (args);
    printf("\n");
    
  }
}

unsigned int
subsorzero(unsigned int a,
	   unsigned int b) {
  if (b <= a) return(a-b);
  else return(0);
}

int
stringendswith(const char *string,
	       const char *suffix)
{
  int totallen = strlen(string);
  int suffixlen = strlen(suffix);
  
  if (totallen < suffixlen)
    return(0);
  else if (strcmp(string + (totallen-suffixlen), suffix) != 0)
    return(0);
  else
    return(1);
}
