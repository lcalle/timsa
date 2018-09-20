/*===============================================================*/
/*                                                               */
/* tide_wdchange.c                                               */
/*   adds water to a water map based on sinusoidial function     */
/*                                                               */
/*===============================================================*/

#include <math.h>
#include "timsa.h"
#include "fnraster.h"

void tide_wdchange(Raster *raster, double *tiderange[], int cell, int row, int maxminutes, int gaugenumber, int k){

  double phaseYt1,phaseYt2,Yt1,Yt2,wdchange;
  
  //Yt2 - Yt1 = change in water level per minute
  phaseYt2 = (M_PI/maxminutes) * k;
  phaseYt1 = (M_PI/maxminutes) * (k - 1);
  Yt2 = (0.5 * tiderange[row][gaugenumber - 1]) * (sin(phaseYt2 - (M_PI/2)));
  Yt1 = (0.5 * tiderange[row][gaugenumber - 1]) * (sin(phaseYt1 - (M_PI/2)));
  wdchange = fabs(Yt2 - Yt1);
  raster->data[cell] -= wdchange; //minus wdepth equals adding water to the depths 
}
