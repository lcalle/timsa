/*===============================================================*/
/*                                                               */
/* iteratesurvey.c                                               */
/*   simulates tidal inudation for specific survey times         */
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

void iteratesurvey(Config config){ //userinputs
  //declare single variables
  char outname[120];
  int i,j,k,g,x,bb;
  int tidegauges=0, gaugenumber=0;
  int startsimulation      = 0;
  int maxsurveyduration    = 0;
  int maxtideminutes_ebb   = 0;
  int maxtideminutes_flood = 0;
  int simulation_starttime,simulation_timeactual,TFLrise,TFLset;
  //int waterdepth_output_time = 30;
  int tides;
  
  double lowerbound, upperbound;
  double wdchange,Yt1,Yt2,phaseYt1,phaseYt2;

  //declare array variables
  //..memory allocated below
  //int *sun,*surveytime,*gauge1stLT,*PeriodEbb1,*PeriodFlood1;
  //double *TRangeEbb1,*TRangeFlood1,*HGT1st;
  //double wdchange,Yt2,Yt1,phaseYt2,phaseYt1;
                  
  Raster *gaugeREF,*depthsX,*defaultRaster,*tideFHA,*depths1E,*depths1F,*waterdepth_output;  //structure for Raster memory
  Raster *mask;

  //...........................
  //get config data
  //...........................
  tides     =config.n;
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
  int *sun[tides];
  int *surveytime[tides];
  int *gauge1stLT[tides];
  int *PeriodEbb1[tides];
  int *PeriodFlood1[tides];
  double *TRangeEbb1[tides];
  double *TRangeFlood1[tides];
  double *HGT1st[tides];

  for(i=0; i < tides; i++){
    sun[i]          = malloc(sizeof(int)*tidegauges);
    surveytime[i]   = (int*)malloc(sizeof(int)*tidegauges);
    gauge1stLT[i]   = (int*)malloc(sizeof(int)*tidegauges);
    PeriodEbb1[i]   = (int*)malloc(sizeof(int)*tidegauges);
    PeriodFlood1[i] = (int*)malloc(sizeof(int)*tidegauges);
    TRangeEbb1[i]   = (double*)malloc(sizeof(double)*tidegauges);
    TRangeFlood1[i] = (double*)malloc(sizeof(double)*tidegauges);
    HGT1st[i]       = (double*)malloc(sizeof(double)*tidegauges);
  }
  
  //---------------------------//
  //        get data           //
  //---------------------------//
  CSV2array2d_int(config.sun_filename, sun, tides,2);
  CSV2array2d_int(config.surveytimes_filename, surveytime, tides, 2);
  CSV2array2d_int(config.lowtide_times_1_filename, gauge1stLT, tides, tidegauges);
  CSV2array2d_double(config.height_1_filename, HGT1st, tides, tidegauges);

  CSV2array2d_int(config.period_1_ebb_filename, PeriodEbb1, tides, tidegauges);
  CSV2array2d_int(config.period_1_flood_filename, PeriodFlood1, tides, tidegauges);
  CSV2array2d_double(config.range_1_ebb_filename, TRangeEbb1, tides, tidegauges);
  CSV2array2d_double(config.range_1_flood_filename, TRangeFlood1 ,tides, tidegauges);

  //---------------------------//
  //     main tidal loop       //
  // ..adjust water maps       //
  // .. .. for each day        //
  // .. .. for each survey     //
  // ..then tidal simulation   //
  //---------------------------//
  for(i = 0; i < tides; i++)  //for each survey; ebb or flood rasters = 0.5 of tide [e.g. Total time available (TTA) = ebb + flood]
  {
      if(i % 10 == 0) printf("\t%.0f%% of surveys completed\n",round((double)i / (double)tides*100.0));

      tideFHA = rastercopy(defaultRaster);//sums FHA for day
      if(config.save_waterdepth == TRUE){waterdepth_output = rastercopy(defaultRaster);}

      //......................................................//
      //    height adjustments for the day of the survey      //
      //......................................................//
      depths1E = rastercopy(depthsX);
      for(x = 0; x < depthsX->count; x++)
      {
          if(depthsX->data[x] == depthsX->nodata || gaugeREF->data[x] == gaugeREF->nodata) continue;
          depths1E->data[x] += HGT1st[i][(int)gaugeREF->data[x] - 1]; //height adjustment for the day
      }
      depths1F = rastercopy(depths1E);//ebb and flood both start at low tide for a tide simulation, i.e. the same low tide height     
  
      //......................................................//
      //    height adjustments for the time of the survey     //
      //......................................................//     
      for(bb = 0; bb < depthsX->count; bb++)
      { 
         if(depthsX->data[bb] == depthsX->nodata  || gaugeREF->data[bb] == gaugeREF->nodata) continue;
          
          gaugenumber     = (int)gaugeREF->data[bb];
          startsimulation = surveytime[i][0] - gauge1stLT[i][gaugenumber - 1];//sets the water levels for the simulation to be near the start of the survey

          if(startsimulation < 0)//survey start time is on the ebb tide, adjust the ebb-tide water depth raster
          {
              maxtideminutes_ebb = PeriodEbb1[i][gaugenumber - 1];
              //IMPORTANT: forloop on ebb must decrement to the most negative number, startsimulation.
              for(j = 0; j > startsimulation; j--) //survey start time is somewhere on the ebb tide, adjust the ebb-tide water depth raster by flooding the ebb raster
              {
                  //Yt2 - Yt1 = change in water level per minute
                  phaseYt2 = (M_PI/maxtideminutes_ebb) * (j-1);
                  phaseYt1 = (M_PI/maxtideminutes_ebb) * j;//IMPORTANT: here -> j+1 is the previous time step (check counter in loop for proof)
                  Yt2 = (0.5 * TRangeEbb1[i][gaugenumber - 1]) * (sin(phaseYt2 - (M_PI/2)));
                  Yt1 = (0.5 * TRangeEbb1[i][gaugenumber - 1]) * (sin(phaseYt1 - (M_PI/2)));
                  wdchange = fabs(Yt2 - Yt1);
                  depths1E->data[bb] -= wdchange;  //water is added to the reference water depths 
              }
          }
          else if (startsimulation > 0)//survey start time is on the flood tide, adjust the flood-tide water depth raster
          {
              maxtideminutes_flood = PeriodFlood1[i][(gaugenumber - 1)];
              for(j = 0; j < startsimulation; j++)
              {
                  //Yt2 - Yt1 = change in water level per minute
                  phaseYt2 = (M_PI/maxtideminutes_flood) * (j+1);
                  phaseYt1 = (M_PI/maxtideminutes_flood) * j;//IMPORTANT: Key difference from above: here -> j-1 is the previous time step
                  Yt2 = (0.5 * TRangeFlood1[i][gaugenumber - 1]) * (sin(phaseYt2 - (M_PI/2)));
                  Yt1 = (0.5 * TRangeFlood1[i][gaugenumber - 1]) * (sin(phaseYt1 - (M_PI/2)));
                  wdchange = fabs(Yt2 - Yt1);
                  depths1F->data[bb] -= wdchange;  //water is added to the reference water depths 
                  //printf("\t.. depths1F->data[bb] %f, wdchange %f\n",depths1F->data[bb],wdchange);
              }
          }
          else
          {
              //do nothing         
              printf("no adjustments required...\n");
          }
      }   

      //....................................//
      //        tidal simulation            //
      //....................................//
      maxsurveyduration = surveytime[i][1] - surveytime[i][0]; 
      for(k = 0; k < (maxsurveyduration+1); k+=config.simtimestep) //runs tidal simulation for duration = elapsed time during survey
      {
          for(bb = 0; bb < depthsX->count; bb++)
          {
              if (depthsX->data[bb] == depthsX->nodata || gaugeREF->data[bb] == gaugeREF->nodata) continue;
              
              //get gauge number from map
              gaugenumber = (int)gaugeREF->data[bb];

              //get gridcell survey starttime and time actual
              simulation_starttime  = surveytime[i][0] - gauge1stLT[i][gaugenumber - 1]; //time relative to lowtide, simulation is forward
              simulation_timeactual = k + simulation_starttime;
              
              //get time from low tide for sunrise and sunset
              TFLrise = sun[i][0] - gauge1stLT[i][gaugenumber - 1];  //time from low, sunrise, NO buffer
              TFLset  = sun[i][1] - gauge1stLT[i][gaugenumber - 1];  //time from low, sunrise, NO buffer

              if(simulation_timeactual < 0) //use ebb raster until [simulation time is > 0] i.e. until flood tide. While k < 0, tide is ebbing
              {
                  //update if saving waterdepths
                  if(config.save_waterdepth == TRUE && k % 10 == 0){waterdepth_output->data[bb] = depths1E->data[bb];}

                  //get maximum tide duration
                  maxtideminutes_ebb = PeriodEbb1[i][gaugenumber - 1];
                  
                  //water is outgoing on the ebb tide. When water depths are too low to forage, break (continue is safety); no more foraging occurs on ebb tide
                  if(depths1E->data[bb] > upperbound || abs(simulation_timeactual) > maxtideminutes_ebb || TFLrise > 0){continue;}
                  
                  // diurnal availability
                  //.............................
                  if(depths1E->data[bb] >= lowerbound && depths1E->data[bb] <= upperbound)
                  {
                      if(config.constrain_daylight == 1){
                        if(TFLrise >= 0 || (TFLrise < 0 && simulation_timeactual < TFLrise) || (TFLset <= 0 && simulation_timeactual > TFLset))
                        {
                            //do nothing
                        }
                        else
                        {
                            tideFHA->data[bb] += config.simtimestep;
                        }
                        // if sunrise is after low tide, only the flood (limited) component required
                        // if sunrise is before lowtide, both ebb (limited) and flood required  
                        // if sunset is after low tide, ebb is not limited by sunset         
                        // if sunset is before lowtide, only ebb (limited) required
                      }else{
                          tideFHA->data[bb] += config.simtimestep;
                      }
                   
                  }
              }
              else if(simulation_timeactual >= 0)
              {
                  //update if saving waterdepths
                  if(config.save_waterdepth == TRUE && k % 10 == 0){waterdepth_output->data[bb] = depths1F->data[bb];}

                  //get maximum tide duration
                  maxtideminutes_flood = PeriodFlood1[i][gaugenumber - 1];
                  
                  //water is added to the flood raster, break (continue) loop if water depths are already too deep
                  if(depths1F->data[bb] < lowerbound || simulation_timeactual > maxtideminutes_flood || TFLset < 0) continue;

                  // diurnal availability
                  //.............................
                  if(depths1F->data[bb] >= lowerbound && depths1F->data[bb] <= upperbound)
                  {
                      if(config.constrain_daylight == 1){
                        //if sunrise is after low tide, only the flood (limited) component is required
                        if ((TFLrise >= 0 && simulation_timeactual < TFLrise) || TFLset <= 0 || (TFLset >= 0 && simulation_timeactual > TFLset))
                        {
                            //do nothing
                        }
                        else
                        {
                            tideFHA->data[bb] += config.simtimestep;
                        }
                        // if sunrise is after low tide, only the flood (limited) is required
                        // if sunrise is before lowtide, flood is not limited by sunrise        
                        // if sunset is before low tide, only ebb (limited) is required         
                        // if sunset is after lowtide, both ebb and flood (limited) required    
                      }else{
                          tideFHA->data[bb] += config.simtimestep;
                      }
                  }
              }
          }//..end gridcell loop

          //.....................//
          //  save water depth?  //
          //.....................//
          if(config.save_waterdepth == TRUE){
            sprintf(outname, "%s/survey_%03d_waterdepth_surveyminute_%03d.asc",config.shallowwateravail_outdir,i,k+surveytime[i][0]);
            rasterwrite(waterdepth_output,outname);
          }
 
          //...........................//
          //      tide adjustment      //
          //...........................//
          for(g = 0; g < depthsX->count; g++)
          {
              if(depthsX->data[g] == depthsX->nodata  || gaugeREF->data[g] == gaugeREF->nodata) continue;
              
              gaugenumber = (int)gaugeREF->data[g];
              simulation_starttime  = surveytime[i][0] - gauge1stLT[i][gaugenumber - 1]; //time relative to lowtide, simulation is forward
              simulation_timeactual = k + simulation_starttime;
              if(simulation_timeactual < 0)
              {
                  maxtideminutes_ebb = PeriodEbb1[i][gaugenumber - 1];
                  //Yt2 - Yt1 = change in water level per minute
                  phaseYt2 = (M_PI/maxtideminutes_ebb) * (simulation_timeactual + 1);
                  phaseYt1 = (M_PI/maxtideminutes_ebb) * simulation_timeactual;
                  Yt2 = (0.5 * TRangeEbb1[i][gaugenumber - 1]) * (sin(phaseYt2 - (M_PI/2)));
                  Yt1 = (0.5 * TRangeEbb1[i][gaugenumber - 1]) * (sin(phaseYt1 - (M_PI/2)));
                  wdchange = fabs(Yt2 - Yt1);
                  depths1E->data[g] += wdchange;
              }
              else if (simulation_timeactual >= 0)
              {
                  maxtideminutes_flood = PeriodFlood1[i][gaugenumber - 1];
                  //Yt2 - Yt1 = change in water level per minute
                  phaseYt2 = (M_PI/maxtideminutes_flood) * (simulation_timeactual + 1);
                  phaseYt1 = (M_PI/maxtideminutes_flood) * simulation_timeactual;
                  Yt2 = (0.5 * TRangeFlood1[i][gaugenumber - 1]) * (sin(phaseYt2 - (M_PI/2)));
                  Yt1 = (0.5 * TRangeFlood1[i][gaugenumber - 1]) * (sin(phaseYt1 - (M_PI/2)));
                  wdchange = fabs(Yt2 - Yt1);
                  depths1F->data[g] -= wdchange; 
              }
          }//..end tide adjustment
      }//..end tidal simulation for survey

      //.............//
      //  save maps  //
      //.............//
      if(config.constrain_daylight == TRUE){
        sprintf(outname, "%s/survey_%03d_daylightonly_swa.asc",config.shallowwateravail_outdir,i);
      }else{
        sprintf(outname, "%s/survey_%03d_daynight_swa.asc",config.shallowwateravail_outdir,i);
      }
      rasterwrite(tideFHA,outname);
      
      //.................//
      // field summaries //
      //.................//
      if(i==0){
        writeindices(&config, tideFHA, mask, i, config.usemask, FALSE);
      }else{
        writeindices(&config, tideFHA, mask, i, config.usemask, TRUE);
      }

  }//..end of survey loop

  //----------------
  //free memory
  //----------------
  //free structure
  free(gaugeREF);
  free(depthsX);
  free(defaultRaster);
  free(tideFHA);
  free(depths1E);
  free(depths1F); 

  for(i=0; i < tides; i++){
    free(sun[i]);
    free(surveytime[i]);
    free(gauge1stLT[i]);
    free(PeriodEbb1[i]);
    free(PeriodFlood1[i]);
    free(TRangeEbb1[i]);
    free(TRangeFlood1[i]);
    free(HGT1st[i]);
  }
} //..end iteratesurvey



