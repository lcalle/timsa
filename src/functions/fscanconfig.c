/*==================================================================*/
/*                                                                  */
/* fscanconfig.c                                                    */
/*   program options and file paths read from configuration file    */
/*                                                                  */
/*==================================================================*/

/* TIMSA C header files */
#include <string.h>
#include "timsa.h"
#include "config.h"
#include "types.h"
#include "errmsg.h"

Bool fscanconfig(Config *config,       /* timsa configuration         */
                 int *argc,            /* number of command line args */
                 char ***argv          /* list of commanfd line args  */
                )                      /* returns error code          */
{
  //char *filename;
  //char **options;
  //char name[255+1];
  //FILE *argfile;

  int i;
  char *buffer = NULL;
  size_t len=0;
  ssize_t read;
  char *ptr = NULL;

  FILE *fp;
  fp = fopen((*argv)[1], "r");      //open file , read only
  if(!fp){
      fprintf (stderr, "failed to open file for reading\n");
      return 1;
  }
  //skip blank lines
  for(i=1; i <14; i++) getline(&buffer, &len, fp);

  //-----------------------------------
  // options in config file
  //-----------------------------------
  if((read = getline(&buffer, &len, fp)) != -1) config->simtype            = (int)strtol(buffer, &ptr, 10);
  if((read = getline(&buffer, &len, fp)) != -1) config->n                  = (int)strtol(buffer, &ptr, 10);
  if((read = getline(&buffer, &len, fp)) != -1) config->simtimestep        = (int)strtol(buffer, &ptr, 10);
  if((read = getline(&buffer, &len, fp)) != -1) config->constrain_daylight = (int)strtol(buffer, &ptr, 10);
  if((read = getline(&buffer, &len, fp)) != -1) config->lowerbound_waterwindow = (double)strtod(buffer, &ptr);
  if((read = getline(&buffer, &len, fp)) != -1) config->upperbound_waterwindow = (double)strtod(buffer, &ptr);
  if((read = getline(&buffer, &len, fp)) != -1) config->usemask                  = (int)strtol(buffer, &ptr, 10);
  if((read = getline(&buffer, &len, fp)) != -1) config->mask_filename            = strdup(strtok(buffer, " "));

  //skip blank lines
  for(i=1; i <5; i++) getline(&buffer, &len, fp);

  //------------------
  // filepath outputs
  //------------------                                    //duplication and create memory(split string by space)
  if((read = getline(&buffer, &len, fp)) != -1) config->shallowwateravail_outdir = strdup(strtok(buffer, " "));
  if((read = getline(&buffer, &len, fp)) != -1) config->waterdepths_outdir       = strdup(strtok(buffer, " "));


  //skip blank lines
  for(i=1; i <5; i++) getline(&buffer, &len, fp);

  //------------------
  // filepaths inputs
  //------------------
  if((read = getline(&buffer, &len, fp)) != -1) config->water_filename           = strdup(strtok(buffer, " "));
  if((read = getline(&buffer, &len, fp)) != -1) config->gauge_filename           = strdup(strtok(buffer, " "));
  if((read = getline(&buffer, &len, fp)) != -1) config->sun_filename             = strdup(strtok(buffer, " "));
  if((read = getline(&buffer, &len, fp)) != -1) config->surveytimes_filename     = strdup(strtok(buffer, " "));
  
  if((read = getline(&buffer, &len, fp)) != -1) config->lowtide_times_1_filename = strdup(strtok(buffer, " "));
  if((read = getline(&buffer, &len, fp)) != -1) config->lowtide_times_2_filename = strdup(strtok(buffer, " "));
  if((read = getline(&buffer, &len, fp)) != -1) config->height_1_filename        = strdup(strtok(buffer, " "));
  if((read = getline(&buffer, &len, fp)) != -1) config->height_2_filename        = strdup(strtok(buffer, " "));
  
  if((read = getline(&buffer, &len, fp)) != -1) config->period_1_ebb_filename    = strdup(strtok(buffer, " "));
  if((read = getline(&buffer, &len, fp)) != -1) config->period_2_ebb_filename    = strdup(strtok(buffer, " "));
  if((read = getline(&buffer, &len, fp)) != -1) config->period_1_flood_filename  = strdup(strtok(buffer, " "));
  if((read = getline(&buffer, &len, fp)) != -1) config->period_2_flood_filename  = strdup(strtok(buffer, " "));

  if((read = getline(&buffer, &len, fp)) != -1) config->range_1_ebb_filename     = strdup(strtok(buffer, " "));
  if((read = getline(&buffer, &len, fp)) != -1) config->range_2_ebb_filename     = strdup(strtok(buffer, " "));
  if((read = getline(&buffer, &len, fp)) != -1) config->range_1_flood_filename   = strdup(strtok(buffer, " "));
  if((read = getline(&buffer, &len, fp)) != -1) config->range_2_flood_filename   = strdup(strtok(buffer, " "));

  //close file connection
  fclose(fp);

  //free memory
  free(buffer);

  return TRUE;
} /* of 'fscanconfig' */
