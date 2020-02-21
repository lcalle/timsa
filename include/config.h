/***************************************************************************/
/*  freadascii.h                                                           */
/*    define raster structure and functions                                */
/***************************************************************************/

#ifndef CONFIG_H /* Already included? */
#define CONFIG_H

/* Definition of datatypes */

typedef struct
{
  //-----------------------------------
  // additional options in config file
  //-----------------------------------
  int simtype;                   //simulation type; 1=surveys,2=days,3=days wdepth prescribed
  int n;                         //number of rows in driver data; represents surveys, tides, or days
  int simtimestep;               //timestep_interval    (in minutes)
  int constrain_daylight;        //constrain_daylight   (Bool is int; TRUE 1=yes, FALSE 0=no)
  int save_waterdepth;           //save water depths? (Bool is int; TRUE 1=yes, FALSE 0=no)
  double lowerbound_waterwindow; //lower bound in water window to estimate availability (negative is water)
  double upperbound_waterwindow; //upper bound in water window to estimate availability (positive is dry)
  int usemask;                   //use ascii mask for field summaries (Bool is int; TRUE 1=yes, FALSE 0=no)
  char *mask_filename;           //ascii mask 0 exclude 1 include or use dummy filename if usemask if false

  //------------------
  // filepath outputs
  //------------------
  char *shallowwateravail_outdir;
  char *waterdepths_outdir;

  //------------------
  // filepaths inputs
  //------------------
  char *water_filename;
  char *gauge_filename;
  char *sun_filename;
  char *surveytimes_filename;

  char *lowtide_times_1_filename;
  char *lowtide_times_2_filename;
  char *height_1_filename;
  char *height_2_filename;

  char *period_1_ebb_filename;
  char *period_2_ebb_filename;
  char *period_1_flood_filename;
  char *period_2_flood_filename;

  char *range_1_ebb_filename;
  char *range_2_ebb_filename;
  char *range_1_flood_filename;
  char *range_2_flood_filename;

} Config;

/* Declaration of functions */

extern int fscanconfig(Config *,int *,char***);

/* Declare macros */
 
#endif
