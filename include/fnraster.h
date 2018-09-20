/***************************************************************************/
/*  fnraster.h                                                             */
/*    define raster structure and functions                                */
/***************************************************************************/

#ifndef FNRASTER_H /* Already included? */
#define FNRASTER_H

/* Definition of constants */

/* Definition of datatypes */
typedef struct
{
  int    nrows;
  int    ncols;
  double xllcorner;
  double yllcorner;
  double cellsize;
  double nodata;
  int    count;
  double *data;
} Raster;

//typedef double Real; /* Default floating point type in LPJ */

/* Declaration of functions */

extern Raster *rasterget(char *);
extern int rasterwrite(Raster *, const char *);
extern Raster *rastercopy(Raster *);
extern void rasterinfo(Raster *);

/* Definitions of macros */

#define newvec(type,size) (type *)malloc(sizeof(type)*(size))
#define check(ptr) if((ptr)==NULL) fail("Error allocating memory")
#endif /* of FNRASTER_H */
