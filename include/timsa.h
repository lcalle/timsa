/***************************************************************************/
/*  timsa.h                                                                */
/*    define functions                                                     */
/***************************************************************************/

#ifndef TIMSA_H /* Already included? */
#define TIMSA_H

#define TIMSA_VERSION  "0.9.0"  /*  now beta, validation in progress */

/* Necessary header files */

/* Standard C header files */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <errno.h>

/*  Defined header files for LPJ */

#include "config.h"
#include "errmsg.h"
#include "fnraster.h"
#include "fncsv.h"
#include "types.h"

/* Declaration of functions */

extern void iteratesurvey(Config);
extern void iterateday(Config);
extern void iterateday_prescribewd(Config);
extern void tide_wdchange(Raster *, double *[], int, int, int, int, int);
extern int  check_nodata(Raster *, int *[], double *[], double *[], int, int, int);
extern void writeindices(Config *, Raster *, Raster *, int, int, int);
extern void writesumy(Config);

/* Definition of macros */

#endif
