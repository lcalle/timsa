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

void iterateday(Config config){ //userinputs
  //declare single variables
  char outname[120];
  int i,k,x,bb;
  int tidegauges=0, gaugenumber = 0;
  int maxtideminutes = 0;
  int TFLrise,TFLset;
  //int waterdepth_output_time = 30;
  int days;
  int trackday=0;
  int dayminute=0;
  double lowerbound, upperbound;
  int writeinterval=1;

  Raster *gaugeREF,*depthsX,*defaultRaster,*dayFHA; //structure for Raster memory
  Raster *depths1E,*depths2E,*depths1F,*depths2F;
  Raster *mask; 
  
  //...........................
  //get config data
  //...........................
  days      =config.n;
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
  int *sun[days];
  int *gauge1stLT[days];
  int *gauge2ndLT[days];

  int *PeriodEbb1[days];
  int *PeriodEbb2[days];
  int *PeriodFlood1[days];
  int *PeriodFlood2[days];

  double *TRangeEbb1[days];
  double *TRangeEbb2[days];
  double *TRangeFlood1[days];
  double *TRangeFlood2[days];

  double *HGT1st[days];
  double *HGT2nd[days];

  for(i=0; i < days; i++){
    sun[i]          = malloc(sizeof(int)*tidegauges);
    gauge1stLT[i]   = (int*)malloc(sizeof(int)*tidegauges);
    gauge2ndLT[i]   = (int*)malloc(sizeof(int)*tidegauges);

    PeriodEbb1[i]   = (int*)malloc(sizeof(int)*tidegauges);
    PeriodEbb2[i]   = (int*)malloc(sizeof(int)*tidegauges);
    PeriodFlood1[i] = (int*)malloc(sizeof(int)*tidegauges);
    PeriodFlood2[i] = (int*)malloc(sizeof(int)*tidegauges);

    TRangeEbb1[i]   = (double*)malloc(sizeof(double)*tidegauges);
    TRangeEbb2[i]   = (double*)malloc(sizeof(double)*tidegauges);
    TRangeFlood1[i] = (double*)malloc(sizeof(double)*tidegauges);
    TRangeFlood2[i] = (double*)malloc(sizeof(double)*tidegauges);

    HGT1st[i]       = (double*)malloc(sizeof(double)*tidegauges);
    HGT2nd[i]       = (double*)malloc(sizeof(double)*tidegauges);
  }
  
  //---------------------------//
  //        get data           //
  //---------------------------//
  printf("getting sun data..\n");
  CSV2array2d_int(config.sun_filename, sun, days,2);
  printf("getting low tide times data..\n");
  CSV2array2d_int(config.lowtide_times_1_filename, gauge1stLT, days, tidegauges);
  CSV2array2d_int(config.lowtide_times_2_filename, gauge2ndLT, days, tidegauges);

  printf("getting low tide heights data..\n");
  CSV2array2d_double(config.height_1_filename, HGT1st, days, tidegauges);
  CSV2array2d_double(config.height_2_filename, HGT2nd, days, tidegauges);

  printf("getting low tide periods data..\n");
  CSV2array2d_int(config.period_1_ebb_filename, PeriodEbb1, days, tidegauges);
  CSV2array2d_int(config.period_2_ebb_filename, PeriodEbb2, days, tidegauges);
  CSV2array2d_int(config.period_1_flood_filename, PeriodFlood1, days, tidegauges);
  CSV2array2d_int(config.period_2_flood_filename, PeriodFlood2, days, tidegauges);

  printf("getting low tide tidal range data..\n");
  CSV2array2d_double(config.range_1_ebb_filename, TRangeEbb1, days, tidegauges);
  CSV2array2d_double(config.range_2_ebb_filename, TRangeEbb2, days, tidegauges);
  CSV2array2d_double(config.range_1_flood_filename, TRangeFlood1 ,days, tidegauges);
  CSV2array2d_double(config.range_2_flood_filename, TRangeFlood2 ,days, tidegauges);

  //........................
  // make csv for wdoutputs
  //........................
  if(config.save_waterdepth == TRUE){
    FILE *f = fopen("waterdepths_prescribed_day.txt", "w");
    //write the header, then write the data
    fprintf(f, "gridcell,gauge_ref,day,minute,waterdepth_m\n");
    fclose(f);
  }

  //........................
  // make csv for SWA
  //........................
  if(config.save_swa_perpixel == TRUE){
    FILE *f2 = fopen("swa_prescribedwd.txt", "w");
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
  for(i = 0; i < days; i++)  //for each day; ebb or flood rasters = 0.5 of tide [e.g. Total time available (TTA) = ebb + flood]
  {
      if(i % 10 == 0) printf("\t%.0f%% of days completed\n",round((double)i / (double)days*100.0));

      dayFHA = rastercopy(defaultRaster); //sums FHA for day
      //......................................................//
      //    height adjustments for day                        //
      //......................................................//
      depths1E = rastercopy(depthsX);
      depths2E = rastercopy(depthsX);

      for(x = 0; x < depthsX->count; x++)
      {
          //check no data
          if(depthsX->data[x] == depthsX->nodata || gaugeREF->data[x] == gaugeREF->nodata)continue;

          depths1E->data[x] += HGT1st[i][(int)gaugeREF->data[x] - 1]; //height adjustment for the day
          depths2E->data[x] += HGT2nd[i][(int)gaugeREF->data[x] - 1];
      }
      depths1F = rastercopy(depths1E); //ebb and flood both start at low tide for a tide simulation, i.e. the same low tide height     
      depths2F = rastercopy(depths2E);

      //...............................................//
      //        tidal simulation                       //
      // ..for each of ebb,flood on 1st and 2nd tide,  //
      // .. ..loop over grid                           //
      // .. ..future output by ebb/flood, 1st/2nd tide //
      //                                               //
      // TODO (19 Sept 2018)                           //
      // ..probably a more efficient compact approach  //
      // ..consider for future developments            //
      // ..will be helpful for parallel runs           //
      //...............................................//

      for(bb = 0; bb < depthsX->count; bb++)
      {
          if(depthsX->data[bb] == depthsX->nodata || gaugeREF->data[bb] == gaugeREF->nodata) continue;
          
          gaugenumber    = (int)gaugeREF->data[bb];
          maxtideminutes = PeriodEbb1[i][gaugenumber - 1];

          if(check_nodata(depthsX,PeriodEbb1,TRangeEbb1,HGT1st,bb,i,gaugenumber)==TRUE)continue; 
          
          //initialize for looping
          trackday=i;
          dayminute = 0; //initialize
          for (k = 1; k < 700; k+=config.simtimestep)//ebb1 tide simulation
          {
              dayminute = config.simtimestep*(gauge1stLT[i][gaugenumber - 1]-k+1); //1440 min in day
              if(dayminute < 0){trackday=i-1; dayminute+=1440;}

              if(depths1E->data[bb] < lowerbound || k > maxtideminutes) break;
              TFLrise = sun[i][0] - gauge1stLT[i][gaugenumber - 1]; //time from low, sunrise, NO buffer
              TFLset  = sun[i][1] - gauge1stLT[i][gaugenumber - 1]; //time from low, sunset, NO buffer
              
              if(depths1E->data[bb] >= lowerbound && depths1E->data[bb] <= upperbound)
              {
                  //-----------------------------
                  //   diurnal availability
                  //-----------------------------
                  if(config.constrain_daylight==TRUE){
                    if ( TFLrise >= 0 || (TFLrise < 0 && k > abs(TFLrise)) || (TFLset <= 0 && k < abs(TFLset)))
                    {
                        //do nothing
                    }
                    else
                    {
                        dayFHA->data[bb] += config.simtimestep;
                    }
                    ////  || if sunrise is after low tide, only the flood (limited) component required   ||
                    ////  || if sunrise is before lowtide, both ebb (limited) and flood required         ||
                    ////  || if sunset is after low tide, ebb is not limited by sunset                   ||
                    ////  || if sunset is before lowtide, only ebb (limited) required                    ||
                  }else{
                      dayFHA->data[bb] += config.simtimestep;
                  }
              }

              //update water levels
              tide_wdchange(depths1E,TRangeEbb1,bb,i,maxtideminutes,gaugenumber,k);

              //save water depth??
              if(config.save_waterdepth == TRUE && dayminute % writeinterval == 0){ //print water depths every 1 min
                //day, minute, .. then wd
                FILE *f = fopen("waterdepths_prescribed_day.txt", "a");
                //order: gridcell,gauge_ref,day,minute,waterdepth_mm
                fprintf(f, "%d,%d,%d,%d,%f\n",bb,(int)gaugeREF->data[bb],trackday,dayminute, depths1E->data[bb]); //add k for correct minute 
                fclose(f);
              }
          }
      }//..end of ebb1

      for(bb = 0; bb < depthsX->count; bb++)
      {
          if(depthsX->data[bb] == depthsX->nodata || gaugeREF->data[bb] == gaugeREF->nodata) continue;
          gaugenumber    = (int)gaugeREF->data[bb];
          maxtideminutes = PeriodFlood1[i][gaugenumber - 1];

          if(check_nodata(depthsX,PeriodFlood1,TRangeFlood1,HGT1st,bb,i,gaugenumber)==TRUE)continue;

          trackday=i; 
          dayminute = 0; //initialize
          for(k = 1; k < 700; k+=config.simtimestep) //flood1 tide simulation
          {
              dayminute = config.simtimestep*(gauge1stLT[i][gaugenumber - 1]+k+1); //1440 min in day
              if(dayminute < 0){trackday=i-1; dayminute+=1440;}

              if(depths1F->data[bb] < lowerbound || k > maxtideminutes) break;
              TFLrise = sun[i][0] - gauge1stLT[i][gaugenumber - 1];  //time from low, sunrise, NO buffer
              TFLset  = sun[i][1] - gauge1stLT[i][gaugenumber - 1];  //time from low, sunset, NO buffer              

              if(depths1F->data[bb] >= lowerbound && depths1F->data[bb] <= upperbound)
              {
                  //-----------------------------
                  //   diurnal availability
                  //-----------------------------
                  if(config.constrain_daylight==TRUE){
                    if((TFLrise >= 0 && k < abs(TFLrise)) || TFLset <= 0 || (TFLset >= 0 && k > abs(TFLset)))
                    {
                        //if sunrise is after low tide, only the flood (limited) component is required
                        //do nothing
                    }
                    else
                    {
                        dayFHA->data[bb] += 1;
                    }
                    ////  || if sunrise is after low tide, only the flood (limited) is required   ||
                    ////  || if sunrise is before lowtide, flood is not limited by sunrise        ||
                    ////  || if sunset is before low tide, only ebb (limited) is required         ||
                    ////  || if sunset is after lowtide, both ebb and flood (limited) required    ||
                  }else{
                    dayFHA->data[bb] += config.simtimestep;
                  }
              }
              //update water levels
              tide_wdchange(depths1F,TRangeFlood1,bb,i,maxtideminutes,gaugenumber,k);
              //save water depth??
              if(config.save_waterdepth == TRUE && dayminute % writeinterval == 0){ //print water depths every 1 min
                //day, minute, .. then wd
                FILE *f = fopen("waterdepths_prescribed_day.txt", "a");
                //order: gridcell,gauge_ref,day,minute,waterdepth_mm
                fprintf(f, "%d,%d,%d,%d,%f\n",bb,(int)gaugeREF->data[bb],trackday,dayminute, depths1F->data[bb]); //add k for correct minute 
                fclose(f);
              }
          }
      }//..end of flood1
 
      for(bb = 0; bb < depthsX->count; bb++)
      {
          if(depthsX->data[bb] == depthsX->nodata || gaugeREF->data[bb] == gaugeREF->nodata) continue;
          
          gaugenumber    = (int)gaugeREF->data[bb];
          maxtideminutes = PeriodEbb2[i][gaugenumber - 1];

          if(check_nodata(depthsX,PeriodEbb2,TRangeEbb2,HGT2nd,bb,i,gaugenumber)==TRUE)continue;

          trackday=i;
          dayminute = 0; //initialize
          for(k = 1; k < 700; k+=config.simtimestep) //ebb2 tide simulation
          {
              dayminute = config.simtimestep*(gauge2ndLT[i][gaugenumber - 1]-k+1); //1440 min in day
              if(dayminute < 0){trackday=i-1; dayminute+=1440;}

              if (depths2E->data[bb] < lowerbound || k > maxtideminutes) continue;
              
              TFLrise = sun[i][0] - gauge2ndLT[i][gaugenumber - 1]; //time from low, sunrise, NO buffer
              TFLset  = sun[i][1] - gauge2ndLT[i][gaugenumber - 1]; //time from low, sunset, NO buffer
              
              if(depths2E->data[bb] >= lowerbound && depths2E->data[bb] <= upperbound)
              {
                  //-----------------------------
                  //   diurnal availability
                  //-----------------------------
                  if(config.constrain_daylight==TRUE){  
                    if(TFLrise >= 0 || (TFLrise < 0 && k > abs(TFLrise)) || (TFLset <= 0 && k < abs(TFLset)))
                    {
                        //do nothing
                    }
                    else
                    {
                        dayFHA->data[bb] += config.simtimestep;
                    }
                    ////  || if sunrise is after low tide, only the flood (limited) component required   ||
                    ////  || if sunrise is before lowtide, both ebb (limited) and flood required         ||
                    ////  || if sunset is after low tide, ebb is not limited by sunset                   ||
                    ////  || if sunset is before lowtide, only ebb (limited) required                    ||
                  }else{
                      dayFHA->data[bb] += config.simtimestep;
                  }
              }
              //update water levels
              tide_wdchange(depths2E,TRangeEbb2,bb,i,maxtideminutes,gaugenumber,k);
              //save water depth??
              if(config.save_waterdepth == TRUE && dayminute % writeinterval == 0){ //print water depths every 1 min
                //day, minute, .. then wd
                FILE *f = fopen("waterdepths_prescribed_day.txt", "a");
                //order: gridcell,gauge_ref,day,minute,waterdepth_mm
                fprintf(f, "%d,%d,%d,%d,%f\n",bb,(int)gaugeREF->data[bb],trackday,dayminute, depths2E->data[bb]); //add k for correct minute 
                fclose(f);
              }
          }
      }//..end ebb2

      for(bb = 0; bb < depthsX->count; bb++)
      {
          if(depthsX->data[bb] == depthsX->nodata || gaugeREF->data[bb] == gaugeREF->nodata) continue;
          
          gaugenumber    = (int)gaugeREF->data[bb];
          maxtideminutes = PeriodFlood2[i][gaugenumber - 1];
         
          if(check_nodata(depthsX,PeriodFlood2,TRangeFlood2,HGT2nd,bb,i,gaugenumber)==TRUE)continue;

          trackday=i; 
          dayminute = 0; //initialize
          for (k = 1; k < 700; k+=config.simtimestep) //flood2 tide simulation
          {
              dayminute = config.simtimestep*(gauge2ndLT[i][gaugenumber - 1]+k+1); //1440 min in day
              if(dayminute < 0){trackday=i-1; dayminute+=1440;}

              if(depths2F->data[bb] < lowerbound || k > maxtideminutes) break;
              TFLrise = sun[i][0] - gauge2ndLT[i][gaugenumber - 1]; //time from low, sunrise, NO buffer
              TFLset  = sun[i][1] - gauge2ndLT[i][gaugenumber - 1]; //time from low, sunset, NO buffer

              if(depths2F->data[bb] >= lowerbound && depths2F->data[bb] <= upperbound)
              {
                  //-----------------------------
                  //   diurnal availability
                  //-----------------------------
                  if(config.constrain_daylight==TRUE){                  
                    if((TFLrise >= 0 && k < abs(TFLrise)) || TFLset <= 0 || (TFLset >= 0 && k > abs(TFLset)))
                    {
                        //if sunrise is after low tide, only the flood (limited) component is required
                        //do nothing
                    }
                    else
                    {
                        dayFHA->data[bb] += config.simtimestep;
                    }
                    ////  || if sunrise is after low tide, only the flood (limited) is required   ||
                    ////  || if sunrise is before lowtide, flood is not limited by sunrise        ||
                    ////  || if sunset is before low tide, only ebb (limited) is required         ||
                    ////  || if sunset is after lowtide, both ebb and flood (limited) required    ||
                  }else{
                      dayFHA->data[bb] += config.simtimestep;
                  }
              }
              //update water levels
              tide_wdchange(depths2F,TRangeFlood2,bb,i,maxtideminutes,gaugenumber,k);
              //save water depth??
              if(config.save_waterdepth == TRUE && dayminute % writeinterval == 0){ //print water depths every 1 min
                //day, minute, .. then wd
                FILE *f = fopen("waterdepths_prescribed_day.txt", "a");
                //order: gridcell,gauge_ref,day,minute,waterdepth_mm
                fprintf(f, "%d,%d,%d,%d,%f\n",bb,(int)gaugeREF->data[bb],trackday,dayminute, depths2F->data[bb]); //add k for correct minute 
                fclose(f);
              }
          }
      }//..end flood2
              
      //.............//
      //  save maps  //
      //.............//
      if(config.save_waterdepth == FALSE){
        if(config.constrain_daylight == TRUE){
          sprintf(outname, "%s/day_%03d_daylightonly_swa.asc",config.shallowwateravail_outdir,i);
        }else{
          sprintf(outname, "%s/day_%03d_daynight_swa.asc",config.shallowwateravail_outdir,i);
        }
        rasterwrite(dayFHA,outname);

        //.................//
        // field summaries //
        //.................//
        if(i==0){
          writeindices(&config, dayFHA, mask, i, config.usemask, FALSE);
        }else{
          writeindices(&config, dayFHA, mask, i, config.usemask, TRUE);
        }
      }

      //....................
      // save swa perpixel
      //....................
      if(config.save_swa_perpixel == TRUE){
        FILE *f2 = fopen("swa_prescribedwd.txt", "a");
        for(bb = 0; bb < depthsX->count; bb++){
           fprintf(f2, "%d,%d,%d,%d,%f\n",bb,(int)gaugeREF->data[bb],trackday,dayminute, dayFHA->data[bb]);
        }
        fclose(f2);
      }
  }//..end of day loop

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
  free(depths1E);
  free(depths2E); 
  free(depths1F);
  free(depths2F);

  for(i=0; i < days; i++){
    free(sun[i]);
    free(gauge1stLT[i]);
    free(gauge2ndLT[i]);
    free(PeriodEbb1[i]);
    free(PeriodEbb2[i]);
    free(PeriodFlood1[i]);
    free(PeriodFlood2[i]);
    free(TRangeEbb1[i]);
    free(TRangeEbb2[i]);
    free(TRangeFlood1[i]);
    free(TRangeFlood2[i]);
    free(HGT1st[i]);
    free(HGT2nd[i]);
  }

} //..end iteratesurvey



