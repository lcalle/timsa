/*===============================================================*/
/*                                                               */
/* check_nodata.c                                                */
/*   checks for missing data in any of the input files           */
/*                                                               */
/*===============================================================*/

#include <math.h>
#include "timsa.h"
#include "fnraster.h"
#include "types.h"

int check_nodata(Raster *wd, int *period[], double *trange[], double *height[], int cell, int row, int gaugenumber){

   if (wd->data[cell] == wd->nodata || period[row][gaugenumber - 1] == 9999 || trange[row][gaugenumber - 1] == 9999 || height[row][gaugenumber - 1] == 9999){
     return(TRUE);
   }
   //no missing data
   return(FALSE);
}
