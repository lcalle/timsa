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
  Raster *rget,*ravg,*rcnt;
  char fname[120];
  char *isprescribed;
  int i,j,n_dat;

  //get number of days
  n_dat = (config.n * config.simtimestep)/1440; //1440 min/day

  if(config.simtype==3 || config.simtype==4){
    isprescribed="prescribewdepth_";
  }else{
    isprescribed="";
  }
  //make an empty matrix to keep count of days with data
  //get data for single day to create the empty raster
  if(config.constrain_daylight == TRUE){
    sprintf(fname, "%s/day_%03d_daylightonly_%sswa.asc",config.shallowwateravail_outdir,0,isprescribed);
  }else{
    sprintf(fname, "%s/day_%03d_daynight_%sswa.asc",config.shallowwateravail_outdir,0,isprescribed);
  }
  rcnt = rasterget(fname);
  for(j = 0; j < rcnt->count; j++){
    if(rcnt->data[j] != rcnt->nodata){ rcnt->data[j] = 0.0;}
  }

  //loop over all days to sum data and determine N
  for( i=0; i < n_dat; i++){
    //get data for day
    if(config.constrain_daylight == TRUE){
      sprintf(fname, "%s/day_%03d_daylightonly_%sswa.asc",config.shallowwateravail_outdir,i,isprescribed);
    }else{
      sprintf(fname, "%s/day_%03d_daynight_%sswa.asc",config.shallowwateravail_outdir,i,isprescribed);
    }
    rget = rasterget(fname);    

    //make empty raster for averages
    //..rcnt has not been updated yet so on i==0, rnt is an empty raster
    if(i==0){ravg = rastercopy(rcnt);}

    //..............
    // sum data
    //..............
    for(j=0; j < ravg->count; j++){
      if(ravg->data[j] != ravg->nodata){ravg->data[j] += rget->data[j]; rcnt->data[j] += 1.0;}
    }    
  }//..end get data all N

  //get average for all N by dividing by N
  for(j=0; j < ravg->count; j++){
    if(ravg->data[j] != ravg->nodata){ ravg->data[j] /= rcnt->data[j];}
  }

  //save raster to file
  if(config.constrain_daylight == TRUE){
    sprintf(fname, "%s/dayavg_daylightonly_%sswa.asc",config.shallowwateravail_outdir,isprescribed);
  }else{
    sprintf(fname, "%s/dayavg_daynight_%sswa.asc",config.shallowwateravail_outdir,isprescribed);
  }
  rasterwrite(ravg,fname);
  
  //done
}
