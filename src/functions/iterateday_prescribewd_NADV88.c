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

void iterateday_prescribewd_NADV88(Config config){ //userinputs
  //declare single variables
  char outname[120];
  int i,x,bb;
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
                  
  Raster *gaugeREF,*dem,*depthsX,*defaultRaster,*dayFHA; //structure for Raster memory
  Raster *depths_sim;
  Raster *mask; 
  
  //...........................
  //get config data
  //...........................
  nrows     =config.n;
  lowerbound=config.lowerbound_waterwindow;
  upperbound=config.upperbound_waterwindow;  

  //get raster data
  gaugeREF      = rasterget(config.gauge_filename);
  dem           = rasterget(config.water_filename);
  depthsX       = rasterget(config.water_filename);
  defaultRaster = rastercopy(dem);     

  for(i = 0; i < defaultRaster->count; i++)
  {
      if(defaultRaster->data[i] != defaultRaster->nodata){ defaultRaster->data[i] = 0;}
  }

  //search for the maximum number of tidal gauges in the data by searching the file gauge_reference.asc
  for(i = 0; i < gaugeREF->count; i++)
  {
    tidegauges = max(tidegauges,gaugeREF->data[i]);
  }
  //hard code
  tidegauges = 13;
  ///////////////
  
  //use mask for indices  
  if(config.usemask == TRUE){ mask = rasterget(config.mask_filename);}

  printf("loading csv data...\n");
  //memory allocated for number of rows
  int    ndays=(nrows*config.simtimestep)/1440;
  int    *sun[ndays]; //365 days
  double *gaugewdepths[nrows];  

  printf(" sun days ndays %d\n",ndays);
  for(i=0; i < ndays; i++){
    sun[i] = malloc(sizeof(int)*2);
  }
  for(i=0; i < nrows; i++){
    gaugewdepths[i] = (double*)malloc(sizeof(double)*tidegauges);
  }
  
  //---------------------------//
  //        get data           //
  //---------------------------//
  //2 columns sunrise, sunset for 365 days
  CSV2array2d_int(config.sun_filename, sun, ndays,2);
  //for the prescribed_waterdepths, surveytimes_filename is dummy name in config
  CSV2array2d_double(config.surveytimes_filename, gaugewdepths,nrows,tidegauges);

  //........................
  // make csv for wdoutputs
  //........................
  if(config.save_waterdepth == TRUE){
    FILE *f = fopen("waterdepths_negvalues_iswater_posvalues_isdry_prescribed_day.txt", "w");
    //write the header, then write the data
    fprintf(f, "gridcell,gauge_ref,day,minute,waterdepth_m\n");
    fclose(f);
  }

  //........................
  // make csv for SWA
  //........................
  if(config.save_swa_perpixel == TRUE){
    FILE *f2 = fopen("swa_perpixel_prescribedwdNADV88_day.txt", "w");
    //write the header, then write the data
    fprintf(f2, "gridcell,gauge_ref,day,minute,swa_min\n");
    fclose(f2);
  }

  //---------------------------//
  //     main tidal loop       //
  // ..adjust water maps       //
  // .. .. for each day        //
  // .. .. for each survey     //
  // ..then tidal simulation   //
  //---------------------------//
  trackday  = 0;
  dayminute = 0; //initialize
  dayFHA    = rastercopy(defaultRaster);  //sums FHA for day

  for(i = 0; i < nrows; i++)  //for each day; ebb or flood rasters = 0.5 of tide [e.g. Total time available (TTA) = ebb + flood]
  {
      dayminute = config.simtimestep*(i+1)-(1440*trackday); //1440 min in day
      //......................................................//
      //    height adjustments for first day only             //
      //......................................................//
      if(i==0){
        printf("making height adjustment for first day...\n");
        depths_sim = rastercopy(depthsX);
        for(x = 0; x < depthsX->count; x++)
        {
            //check no data
            if(depthsX->data[x] == depthsX->nodata || gaugeREF->data[x] == gaugeREF->nodata)continue;
            //add water to dem
            //..becomes the water surface
            //..but water depths need to be calculated as the dem minus depths_sim so that water is negative relative to water surface
            depths_sim->data[x] += gaugewdepths[i][(int)gaugeREF->data[x] - 1] - dem->data[x]; //height adjustment for the day
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
      TFLrise   = sun[trackday][0] - dayminute; //time from sunrise, NO buffer
      TFLset    = sun[trackday][1] - dayminute; //time from sunset, NO buffer

      //printf("\t tidal simulation..day %d..dayminute %d..\n",trackday,dayminute);
      for(bb = 0; bb < depthsX->count; bb++)
      {
          //save water depth??
          if(config.save_waterdepth == TRUE && dayminute % dayminute == 0){ //print water depths every 1 min
            FILE *f = fopen("waterdepths_negvalues_iswater_posvalues_isdry_prescribed_day.txt", "a");
            fprintf(f, "%d,%d,%d,%d,%f\n",bb,(int)gaugeREF->data[bb],trackday,dayminute, dem->data[bb] - depths_sim->data[bb]); //add k for correct minute 
            fclose(f);
          }

          if(depthsX->data[bb] == depthsX->nodata || gaugeREF->data[bb] == gaugeREF->nodata) continue;
          gaugenumber    = (int)gaugeREF->data[bb];
         
          if( (dem->data[bb] - depths_sim->data[bb]) >= lowerbound && (dem->data[bb] - depths_sim->data[bb]) <= upperbound )
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

          //update water surface levels
          //..gauge provides the rate of change
          //..negative values reduce water levels (makes shallower)
          //..positive values add water
          if(i < nrows){
            if(gaugewdepths[i+1][gaugenumber - 1] == depths_sim->nodata || gaugewdepths[i][gaugenumber - 1] == depths_sim->nodata){
              //no change in water depth if there is nodata
              depths_sim->data[bb] += 0;
            }else{
              depths_sim->data[bb] += gaugewdepths[i+1][gaugenumber - 1] - gaugewdepths[i][gaugenumber - 1];
            }
          } 
      }//end grid loop
             
      //.............//
      //  save maps  //
      //.............//
      if(config.save_waterdepth == FALSE && dayminute == 1440){//1440 in day
        if(config.constrain_daylight == TRUE){
          sprintf(outname, "%s/day_%03d_daylightonly_prescribewdepth_swa.asc",config.shallowwateravail_outdir,trackday);
        }else{
          sprintf(outname, "%s/day_%03d_daynight_prescribewdepth_swa.asc",config.shallowwateravail_outdir,trackday);
        }
        rasterwrite(dayFHA,outname);

        //.................//
        // field summaries //
        //.................//
        if(i==0){
          writeindices(&config, dayFHA, mask, trackday, config.usemask, FALSE);
        }else{
          writeindices(&config, dayFHA, mask, trackday, config.usemask, TRUE);
        }
      }//..save maps if not printing to csv

      //....................
      // save swa perpixel
      //....................
      if(config.save_swa_perpixel == TRUE && dayminute == 1440){//1440 in day
        FILE *f2 = fopen("swa_perpixel_prescribedwdNADV88_day.txt", "a");
        for(bb = 0; bb < depthsX->count; bb++){
           fprintf(f2, "%d,%d,%d,%d,%f\n",bb,(int)gaugeREF->data[bb],trackday,dayminute, dayFHA->data[bb]);
        }
        fclose(f2);
      }

      //....................
      // get new FHA map
      // ..update day
      //....................
      if(dayminute == 1440){
        trackday += 1;
		free(dayFHA); //try for safety, freeing memory
        dayFHA   = rastercopy(defaultRaster);  //sums FHA for day
        printf("\t%d nrows %.0f%% of nrows completed\n",nrows,round((double)i / (double)nrows*100.0));
      }

  }//..end of loop

  //----------------------------------
  // write day average summary raster
  //----------------------------------
  if(config.save_waterdepth == FALSE){writesumy(config);}

  //----------------
  //free memory
  //----------------
  //free structure
  free(gaugeREF);
  free(depthsX);
  free(defaultRaster);
  free(dayFHA);
  free(depths_sim);

  for(i=0; i < ndays; i++){
    free(sun[i]);
  }
  for(i=0; i < nrows; i++){
    free(gaugewdepths[i]);
  }
} //..end iterateday_prescribewd



