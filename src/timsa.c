/***************************************************************************/
/**                                                                       **/
/**   t  i  m  s  a  .  c                                                 **/
/**                                                                       **/
/**     C implementation of TiMSA,                                        **/
/**     Tidal Model of Shallow Water Availability                         **/
/**                                                                       **/
/**     written by Leonardo Calle                                         **/
/**                                                                       **/
/***************************************************************************/

#include <time.h>
#include "timsa.h"
#include "config.h"
#include "fnraster.h"
#include "fncsv.h"
#include "types.h"

int main(int argc,char **argv)
{
  //FILE **output;
  char *progname,*simtype[20];
  time_t tstart,tend;

  /* Create array of functions, uses the typedef of (*Fscanpftparfcn) in pft.h */
  //Fscanpftparfcn scanfcn[NTYPES]={fscanpft_grass,fscanpft_tree,fscanpft_crop};
  Config config;

  //timsa version
  printf("**** %s C Version Beta (" __DATE__ ") ****\n\n",argv[0]);

  //===================================
  // Scan configuration files
  //===================================
  //Read conf file from command line call to timsa
  progname=argv[0];
  if(fscanconfig(&config,&argc,&argv)==FALSE)
    fail("Error reading configuration file\n");

  //===================================
  //  simulation begins
  //===================================
  printf("Simulation begins...\n");
  time(&tstart);  //start timer

  //.................................
  // simulation type
  //.................................
  if(config.simtype==1){
    *simtype="survey";
    iteratesurvey(config);
  }else if(config.simtype==2){
    *simtype="tide";
  }else if(config.simtype==3){
    *simtype="days";
    iterateday(config);
  }else if(config.simtype==4){
    *simtype="fullyear";
  }else{
    fail("Error in simulation type, must specify in config file 1=survey,2=tide,3=days,4=full year.\n");
  }
  time(&tend); //end timer

  //----------------
  // print msgs
  //----------------
  printf("Simulation ended.\n");
  printf("%s %s successfully",progname,*simtype);
  printf(" terminated, %d %s processed.\n"
         "Wall clock time: %d sec, %.2g sec/%s\n",
         config.n,*simtype,(int)(tend-tstart),
         (double)(tend-tstart)/config.n,*simtype);
 
  return EXIT_SUCCESS;
} /* of 'main' */
