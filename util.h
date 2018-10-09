
#ifndef UTIL_H
#define UTIL_H

extern int debug;
extern void fatal(const char* message);
extern void fatals(const char* message,
		   const char* string);
extern void fatalu(const char* message,
		   unsigned int x);
extern void fatalsu(const char* message,
		    const char* string,
		    unsigned int x);
extern void fataluu(const char* message,
		    unsigned int x1,
		    unsigned int x2);
extern void fatalxx(const char* message,
		    unsigned int x1,
		    unsigned int x2);
extern void fatalsxx(const char* message,
		     const char* string,
		     unsigned int x1,
		     unsigned int x2);
extern void debugf(const char* format,
		   ...);
extern unsigned int subsorzero(unsigned int a,
			       unsigned int b);

#endif /* UTIL_H */
