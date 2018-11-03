/*===============================================================*/
/*                                                               */
/* writesumy.c                                                   */
/*   create average raster from input folder                     */
/*                                                               */
/*===============================================================*/

/* Standard C header files */
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <errno.h>

/* TIMSA C header files */
#include "timsa.h"
#include "fnraster.h"

void writesumy(Config config){ //userinputs
  Raster *rget,*ravg;
  char fname[120];
  int i,j;

  for( i=0; i < config.n; i++){
    //get data for day
    if(config.constrain_daylight == TRUE){
      sprintf(fname, "%s/day_%03d_daylightonly_swa.asc",config.shallowwateravail_outdir,i);
    }else{
      sprintf(fname, "%s/day_%03d_daynight_swa.asc",config.shallowwateravail_outdir,i);
    }
    rget = rasterget(fname);    

    //make empty raster for averages
    if(i==0){
      ravg = rastercopy(rget);
      for(j = 0; j < ravg->count; j++){
        if(ravg->data[j] != ravg->nodata){ ravg->data[j] = 0.0;}
      }
    }

    //..............
    // sum data
    //..............
    for(j=0; j < ravg->count; j++){
      if(ravg->data[j] != ravg->nodata){ ravg->data[j] += rget->data[j];}
    }    
  }//..end get data all N

  //get average for all N by dividing by N
  for(j=0; j < ravg->count; j++){
    if(ravg->data[j] != ravg->nodata){ ravg->data[j] /= config.n;}
  }

  //save raster to file
  if(config.constrain_daylight == TRUE){
    sprintf(fname, "%s/dayavg_daylightonly_swa.asc",config.shallowwateravail_outdir);
  }else{
    sprintf(fname, "%s/dayavg_daynight_swa.asc",config.shallowwateravail_outdir);
  }
  rasterwrite(ravg,fname);
  
  //done
}
