/***************************************************************************/
/*  types.h                                                                */
/*    define functions, macros, and shortcuts for numeric procedures       */
/***************************************************************************/


#ifndef TYPES_H /* Already included? */
#define TYPES_H

/* Definition of constants */
#define epsilon 1.0E-6 /* a minimal value -- check if neglegible */
#ifndef TRUE    /* Check whether TRUE or FALSE are already defined */
#define TRUE 0
#endif
#ifndef FALSE
#define FALSE 1
#endif

/* Definition of datatypes */

//typedef double Real; /* Default floating point type in LPJ */

typedef int Bool;

/* Declaration of functions */

extern void fail(const char *,...);

//extern Bool fscanstring(FILE *,char *);

/* Definitions of macros */

#define newvec(type,size) (type *)malloc(sizeof(type)*(size))
#define check(ptr) if((ptr)==NULL) fail("Error allocating memory")

#define new(type) (type *)malloc(sizeof(type))
#ifndef min
#define min(a,b) (((a)<(b)) ?  (a) : (b))
#endif
#ifndef max
#define max(a,b) (((a)>(b)) ?  (a) : (b))
#endif

#endif /* of TYPES_H */
