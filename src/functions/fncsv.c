/*=============================================*/
/* fncsv.c                                     */
/*   stream reader for comma separated values  */
/*=============================================*/

#include <stdio.h>
#include <stdlib.h>
#include "fncsv.h"

//array pointer pass out
int CSV2array2d_int(char *filePath, int *array[], int nrows, int ncols)
{
    //int *array2[ncols];
    int i,j;
    char *buffer = NULL;
    size_t len;
    ssize_t read;
    char *ptr = NULL;

    FILE *fp;
    fp = fopen(filePath, "r");      //open file , read only
    if(!fp){
        fprintf (stderr, "failed to open file for reading\n");
        return 1;
    }

    len=0; //length of line in characters
    i=0; //rows
    j=0;   //cols

    //skip header
    getline (&buffer, &len, fp);

    for(i=0; i < nrows; i++){
      if(((read = getline (&buffer, &len, fp)) == -1)) break;
      for(j=0, ptr = buffer; j < ncols; j++,ptr++){
        array[i][j] = (int)strtol(ptr, &ptr,10);
      }
    }
/*
    while ((read = getline (&buffer, &len, fp)) != -1 && idx < nrows) {
        //allocate memory for each row in the array
        //array[idx] = (int*)malloc(sizeof(ncols));
        for(j = 0, ptr = buffer; j < ncols; j++, ptr++){
          array[idx][j] = (int)strtol(ptr, &ptr,10);
        }
        idx++;
    }
*/
    fclose(fp);
    free(buffer);
    return (0);
}//..end of csv array2d

int CSV2array2d_double(char *filePath, double *array[],int nrows, int ncols)
{   
    //double *array[ncols];
    int i,j;
    char *buffer = NULL;
    size_t len;
    ssize_t read;
    char *ptr = NULL;
    
    FILE *fp;
    fp = fopen(filePath, "r");      //open file , read only
    if(!fp){
        fprintf (stderr, "failed to open file for reading\n");
        return 1;
    }
    
    len=0; //length of line in characters
    i=0; //rows
    j=0;   //cols
    
    //skip header
    getline (&buffer, &len, fp);

    for(i=0; i < nrows; i++){
      if(((read = getline (&buffer, &len, fp)) == -1)) break;
      for(j=0, ptr = buffer; j < ncols; j++,ptr++){
        array[i][j] = (double)strtod(ptr, &ptr);
      }
    } 
/*
    while ((read = getline (&buffer, &len, fp)) != -1  && idx < nrows) {
        //allocate memory for each row in the array
        //array[idx] = (double*)malloc(sizeof(ncols)); 
        for(j = 0, ptr = buffer; j < ncols; j++, ptr++){
          array[idx][j] = (double)strtod(ptr, &ptr);
        }
        idx++;
    }
*/
    fclose(fp);
    free(buffer);
    return(0);
}//..end of csv array2d
