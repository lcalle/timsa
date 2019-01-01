/*===============================================================*/
/*                                                               */
/* iterateday.c                                                  */
/*   simulates tidal inudation for entire day                    */
/*   ..w/ and w/o daylight constraint                            */
/*   ..NOT for subdaily time restrictions (use iteratesurvey.c)  */      
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
#include "fncsv.h"
#include "types.h"

void iterateday_prescribewd(Config config){ //userinputs
  //declare single variables
  char outname[120];
  int i,k,x,bb;
  int tidegauges=0, gaugenumber = 0;
  int TFLrise,TFLset;
  //int waterdepth_output_time = 30;
  int nrows;
  int trackday=0; //track days
  int dayminute;  //day in minutes [0-1440]

  double lowerbound, upperbound;
  //double wdchange,Yt1,Yt2,phaseYt1,phaseYt2;

  //declare array variables
  //..memory allocated below
                  
  Raster *gaugeREF,*depthsX,*defaultRaster,*dayFHA; //structure for Raster memory
  Raster *depths_sim;
  Raster *mask; 
  
  //...........................
  //get config data
  //...........................
  nrows      =config.n;
  lowerbound=config.lowerbound_waterwindow;
  upperbound=config.upperbound_waterwindow;  

  printf("loading raster data...\n");
  //get raster data
  gaugeREF      = rasterget(config.gauge_filename);
  depthsX       = rasterget(config.water_filename);
  defaultRaster = rastercopy(depthsX);     
  
  for(i = 0; i < defaultRaster->count; i++)
  {
      if(defaultRaster->data[i] != defaultRaster->nodata){ defaultRaster->data[i] = 0;}
  }

  //search for the maximum number of tidal gauges in the data by searching the file gauge_reference.asc
  for(i = 0; i < gaugeREF->count; i++)
  {
    tidegauges = max(tidegauges,gaugeREF->data[i]);
  }
  ///////////////
  
  //use mask for indices  
  if(config.usemask == TRUE){ mask = rasterget(config.mask_filename);}

  printf("loading csv data...\n");
  //memory allocated for number of rows
  int *sun[nrows];
  int *gaugewdepths[nrows];  

  for(i=0; i < nrows; i++){
    sun[i]          = malloc(sizeof(int)*tidegauges);
    gaugewdepths[i] = (double*)malloc(sizeof(double)*tidegauges);
  }
  
  //---------------------------//
  //        get data           //
  //---------------------------//
  CSV2array2d_int(config.sun_filename, sun, nrows,2);
  CSV2array2d_double(config.prescribewd_filename, gaugedata,nrows,tidegauges);

  //---------------------------//
  //     main tidal loop       //
  // ..adjust water maps       //
  // .. .. for each day        //
  // .. .. for each survey     //
  // ..then tidal simulation   //
  //---------------------------//
  for(i = 0; i < nrows; i++)  //for each day; ebb or flood rasters = 0.5 of tide [e.g. Total time available (TTA) = ebb + flood]
  {
      if(i %% 240 == 0){
        if(i > 0){trackday += 1;}
        dayFHA   = rastercopy(defaultRaster);  //sums FHA for day
        printf("\t%.0f%% of nrows completed\n",round((double)i / (double)nrows*100.0));
      }

      //......................................................//
      //    height adjustments for first day only             //
      //......................................................//
      if(i==0){
        depths_sim = rastercopy(depthsX);
        for(x = 0; x < depthsX->count; x++)
        {
            //check no data
            if(depthsX->data[x] == depthsX->nodata || gaugeREF->data[x] == gaugeREF->nodata)continue;
            depths_sim->data[x] += gaugewdepths[i][(int)gaugeREF->data[x] - 1]; //height adjustment for the day
        }
      }//..only adjust for first day of year

      //...............................................//
      //        tidal simulation                       //
      // .. ..loop over grid                           //
      // .. ..add water levels by basin (gauge_ref)    //
      // .. ..use 6-min data direct from gauges        //
      // .. ..future output by ebb/flood, 1st/2nd tide //
      //                                               //
      // TODO (31 Dec 2018)                            //
      // ..consider parallel runs                      //
      //...............................................//
      dayminute = config.simtimestep*i-(1440*trackday); //1440 min in day,
      TFLrise   = sun[i][0] - dayminute; //time from sunrise, NO buffer
      TFLset    = sun[i][1] - dayminute; //time from sunset, NO buffer
      
      for(bb = 0; bb < depthsX->count; bb++)
      {
          if(depthsX->data[bb] == depthsX->nodata || gaugeREF->data[bb] == gaugeREF->nodata) continue;
          gaugenumber    = (int)gaugeREF->data[bb];
          
          if(depths_sim->data[bb] < lowerbound) break;
          if(depths_sim->data[bb] >= lowerbound && depths_sim->data[bb] <= upperbound)
          {
              //-----------------------------
              //   diurnal availability
              //-----------------------------
              if(config.constrain_daylight==TRUE){
                if (TFLrise >= 0 || TFLset <= 0)
                {
                    //do nothing
                }
                else
                {
                    dayFHA->data[bb] += config.simtimestep;
                }
                ////  || if sunrise is after current time, do nothing  ||
                ////  || if sunset         ||
                ////  || if sunset is after low tide, ebb is not limited by sunset                   ||
                ////  || if sunset is before lowtide, only ebb (limited) required                    ||
              }else{
                  dayFHA->data[bb] += config.simtimestep;
              }
          }

          //update water levels
          if(i < nrows) depths_sim->data[bb] = gaugewdepths[i+1][gaugenumber - 1] - gaugewdepths[i][gaugenumber - 1]; 
      }//end grid loop
              
      //.............//
      //  save maps  //
      //.............//
      if(i %% 240 == 0){//daily
        if(config.constrain_daylight == TRUE){
          sprintf(outname, "%s/day_%03d_daylightonly_swa.asc",config.shallowwateravail_outdir,(int)i/240);
        }else{
          sprintf(outname, "%s/day_%03d_daynight_swa.asc",config.shallowwateravail_outdir,(int)i/240);
        }
        rasterwrite(dayFHA,outname);

        //.................//
        // field summaries //
        //.................//
        if(i==0){
          writeindices(&config, dayFHA, mask, (int)i/240, config.usemask, FALSE);
        }else{
          writeindices(&config, dayFHA, mask, (int)i/240, config.usemask, TRUE);
        }
      }
  }//..end of loop

  //----------------------------------
  // write day average summary raster
  //----------------------------------
  writesumy(config);

  //----------------
  //free memory
  //----------------
  //free structure
  free(gaugeREF);
  free(depthsX);
  free(defaultRaster);
  free(dayFHA);
  free(depths_sim);

  for(i=0; i < nrows; i++){
    free(sun[i]);
    free(gaugewdepths[i]);
  }

} //..end iterateday_prescribewd



