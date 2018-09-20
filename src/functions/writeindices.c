/*===============================================================*/
/*                                                               */
/* writeindices.c                                                */
/*   calculate sum over field                                    */
/*   ..return                                                    */
/*         sum (minutes)                                         */
/*         count of cells with swa > 0                           */
/*         cellsize                                              */
/*                                                               */
/*===============================================================*/

#include <math.h>
#include "timsa.h"
#include "fnraster.h"
#include "types.h"

void writeindices(Config *config, Raster *r, Raster *mask, int row, int USE_MASK, int FILE_EXIST){
  
  double sum = 0;
  int i,count = 0;
  char outname[120];
  char *ismask[10];

  if(USE_MASK==TRUE){
    *ismask="_withmask";
    for(i=0; i < r->count; i++){
      if(mask->data[i]==0) continue;
      if(r->data[i] > 0)
      {
        sum   += r->data[i];
        count += 1;
      }
    }//..end loop
  }else{
    *ismask="";
    for(i=0; i < r->count; i++){
      if(r->data[i] > 0)
      {
        sum   += r->data[i];
        count += 1;
      }
    }//..end loop
  }

  sprintf(outname, "%s/fieldsummary%s.txt",config->shallowwateravail_outdir,*ismask);
  if(FILE_EXIST==FALSE){
    FILE *f = fopen(outname, "w");
    //write the header, then write the data
    fprintf(f, "index,sum,count,cellsize\n");
    fprintf(f, "%d,%f,%d,%f\n",row,sum,count,r->cellsize);
    fclose(f);
  }else{
    //append text file
    FILE *f = fopen(outname, "a");
    fprintf(f, "%d,%f,%d,%f\n",row,sum,count,r->cellsize);
    fclose(f);
  }
}//..end write indices
