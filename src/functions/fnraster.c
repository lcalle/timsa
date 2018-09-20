/*===============================================================*/
/*                                                               */
/* fnraster.c                                                    */
/*   ascii formatted files into multi-dimensional array          */
/*   read, write, copy functions                                 */
/*                                                               */
/*===============================================================*/

/* Standard C header files */
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <errno.h>
#include <regex.h>
#include <ctype.h>

/* TIMSA C header files */
#include "fnraster.h"
#include "types.h"

/* define constants */
#define separatorpattern   "[ \t\r\n]*" //10 chars
#define valuepattern       "(?<value>(-?\d+(\.\d+)?[eE]-?\d+|-?\d+(\.\d+)?|-?\.\d+))" //56 chars
#define MAX_ERROR_MSG 500

//external fns
//Raster *rasterget(char *filename)
//Bool rasterwrite(Raster *r, const char *filename)
//Raster *rastercopy(Raster *r)
//void rasterinfo(Raster *r){

//internal fns
//static int compile_regex(regex_t * r, const char * regex_text)
//static int match_regex(regex_t *r, const char *to_match, int max_matches)

/* Compile the regular expression described by "regex_text" into "r". */
static int compile_regex(regex_t * r, const char * regex_text)
{
    int status = regcomp (r, regex_text, REG_EXTENDED|REG_NEWLINE);
    if(status != 0){
	  char error_message[MAX_ERROR_MSG];
	  regerror(status, r, error_message, MAX_ERROR_MSG);
      printf("Regex error compiling '%s': %s\n",regex_text, error_message);
      return 1;
    }
    return 0;
}

/* Match the string in "to_match" against the compiled regular expression in "r"  */
static int match_regex(regex_t *r, const char *to_match, int max_matches)
{
    /* "P" is a pointer into the string which points to the end of the previous match. */
    const char * p = to_match;            //p is a pointer into the string which points to the end of the previous match
    const int n_matches = max_matches;    //N_matches is the maximum number of matches allowed
    //int i, start, finish;
    int nomatch;

    /* "M" contains the matches found. */
    regmatch_t m[n_matches];

    nomatch = regexec(r, p, n_matches, m, 0);
//printf("\n match phrase is %s", p);
//printf("\n match status (nomatch) is %d", nomatch);

//printf("\n matchpos 0 is %lld", m[0].rm_so);
//printf("\n matchpos 1 is %lld", m[1].rm_so);

    if(nomatch == 0){ //return status for regexec is zero for match, non-zero otherwise
      //yes match
      return TRUE;
    }else{
      //no match
      return FALSE; 
    }
    
   // while(1){
   //     i = 0;
   //     nomatch = regexec(r, p, n_matches, m, 0);
   //     if(nomatch){
   //         printf ("No more matches.\n");
   //         return ;
   //     }
   //     for (i = 0; i < n_matches; i++) {
   //         if (m[i].rm_so == -1) break;

   //         start = m[i].rm_so + (p - to_match);
   //         finish = m[i].rm_eo + (p - to_match);
   //         
   //         if(i == 0){
   //           printf("$& is ");
   //         }else{
   //           printf("$%d is ", i);
   //         }
   //         if(i != 0) printf("%d", i);
   //     }
   //     p += m[0].rm_eo;
   // }
   // return 0;
}
 
//function
Raster *rastercopy(Raster *r)
{
    int i;
    Raster *tmp;
    tmp = new(Raster); //allocate memory for the raster structure

    //copy header
    tmp->nrows     = r->nrows;
    tmp->ncols     = r->ncols;
    tmp->cellsize  = r->cellsize;
    tmp->xllcorner = r->xllcorner;
    tmp->yllcorner = r->yllcorner;
    tmp->nodata    = r->nodata;
    tmp->count     = r->count;

    //allocate memory for data vector 
    tmp->data = newvec(double,r->nrows*r->ncols);
    //tmp->data = (double*)realloc(tmp->data,sizeof(double)*(r->nrows*r->ncols));

    //copy data values
    for(i=0; i < r->count; i++) tmp->data[i] = r->data[i];

    return tmp;
}//..end copy raster

Raster *rasterget(char *filename)
{
    char    *fileline=0,*fileline2=0;
    char    *token,*ptr_remainder;
    char    *cellsize, *xllcorner, *yllcorner, *ncols, *nrows, *nodata;
    int     i,j,count_data,count_headerlines;
    size_t  linesize=0; 
    ssize_t linelen=0; 
    Raster  *raster;
    FILE    *file_raster=NULL;
    regex_t cellsizeregex, xllcornerregex, yllcornerregex, ncolsregex, nrowsregex, nodataregex;

    //allocate memory for the initial raster structure
    raster = new(Raster);

    //--------------------------
    // set regular expressions
    //--------------------------
    //set values
    ncols     = "[nN][cC][oO][lL][sS]";
    nrows     = "[nN][rR][oO][wW][sS]";
    xllcorner = "[xX][lL][lL][cC][oO][rR][nN][eE][rR]";
    yllcorner = "[yY][lL][lL][cC][oO][rR][nN][eE][rR]";
    cellsize  = "[cC][eE][lL][lL][sS][iI][zZ][eE]";
    nodata    = "[nN][oO][dD][aA][tT][aA]_[vV][aA][lL][uU][eE]";
    
    //make the regular expressions
    compile_regex(&nrowsregex, nrows);
    compile_regex(&ncolsregex, ncols);
    compile_regex(&xllcornerregex, xllcorner);
    compile_regex(&yllcornerregex, yllcorner);
    compile_regex(&cellsizeregex, cellsize);
    compile_regex(&nodataregex, nodata);
    //compile_regex(&dataregex, data);
    
    //-----------------------------------
    // read raster from file
    // ..(1) header
    // ..(2) data values
    //-----------------------------------
    
    //open connection to file stream for reading
    file_raster = fopen(filename,"r");
    if(!file_raster){
      printf("ERROR raster not found %s\n", filename);
      fail("failed to open raster for reading");
    }
    count_headerlines=0;

    while(count_headerlines < 6){
      getline(&fileline2, &linesize, file_raster);
      fileline=fileline2; //duplicate pointer, point to different address
      if(match_regex(&ncolsregex,fileline,2) == TRUE){
        token = strsep(&fileline, " ");
        //check order and replace token if not a value
        if(isdigit(token[0])==0) token=fileline;
        raster->ncols = strtol(token, &ptr_remainder,10);
      }else if(match_regex(&nrowsregex,fileline,2) == TRUE){
        token = strsep(&fileline, " ");
        if(isdigit(token[0])==0) token=fileline;
        raster->nrows = strtol(token, &ptr_remainder,10);
      }else if(match_regex(&xllcornerregex,fileline,2) == TRUE){
        token = strsep(&fileline, " ");
        if(isdigit(token[0])==0) token=fileline;
        raster->xllcorner = strtod(token, &ptr_remainder);
      }else if(match_regex(&yllcornerregex,fileline,2) == TRUE){
        token = strsep(&fileline, " ");
        if(isdigit(token[0])==0) token=fileline;
        raster->yllcorner = strtod(token, &ptr_remainder);
      }else if(match_regex(&cellsizeregex,fileline,2) == TRUE){
        token = strsep(&fileline, " ");
        if(isdigit(token[0])==0)token=fileline;
        raster->cellsize = strtod(token, &ptr_remainder);
      }else if(match_regex(&nodataregex,fileline,2) == TRUE){
        token = strsep(&fileline, " ");
        if(isdigit(token[0])==0) token=fileline;
        raster->nodata = strtod(token, &ptr_remainder);
      }
      //update count 
      count_headerlines +=1;
    }//..end header

    //add count information; N data values
    raster->count = raster->nrows*raster->ncols;

    printf("filename %s, raster->count %d, raster->nrows %d, raster->ncols %d\n",filename, raster->count, raster->nrows,raster->ncols);
    //allocate memory for data vector 
    raster->data = newvec(double,raster->nrows*raster->ncols);      
    count_data=0;

    //initialize everything to nodata value for safety
    for(i=0; i < raster->count; i++){
      raster->data[i] = raster->nodata;
    }

    linesize  =0;
    fileline  =0;
    linelen   =0;
    count_data=0;
    for(i=0; i < raster->nrows; i++){
      //get row of data
      if(!(linelen=getline(&fileline, &linesize, file_raster)>0)) break;
      fileline2=fileline;
      for(j=0; j < raster->ncols; j++){
        //put data into array
        if((token = strsep(&fileline2, " ")) != NULL){
          //convert to double
          raster->data[count_data] = strtod(token, &ptr_remainder);
          count_data+=1;
        }//..catch empty safety
      }
    }
 
    //free memory allocated to the pattern buffer by regcomp
    regfree(&cellsizeregex);
    regfree(&xllcornerregex);
    regfree(&yllcornerregex);
    regfree(&ncolsregex);
    regfree(&nrowsregex);
    regfree(&nodataregex);
    //regfree(&dataregex);

    //close file connection
    fclose(file_raster);
 
    //return the pointer to the raster structure
    return raster; 
}//..end getraster

Bool rasterwrite(Raster *r, const char *filename)
{
    int i,j,k;

    if(1==10) //(*filename=="exists")
    {
        //(safety) change name of file to have a suffix so doesn't get overwritten
    }
    else
    {
      //create text file
      FILE *f = fopen(filename, "w");

      if(!f){
        printf("\nERROR output location does not exist.. %s\n", filename);
        fail("failed to write raser to file.");
      }
      //write header
      fprintf(f,"ncols %d\n",r->ncols);
      fprintf(f,"nrows %d\n",r->nrows);
      fprintf(f,"xllcorner %f\n",r->xllcorner);
      fprintf(f,"yllcorner %f\n",r->yllcorner);
      fprintf(f,"cellsize %f\n",r->cellsize);
      fprintf(f,"nodata_value %f\n",r->nodata);

      //write data
      i=0; //count data; j is rows, k is cols
      for(j=0; j < r->nrows; j++)
      {
        for(k=0; k < r->ncols-1; k++){
           if(isnan(r->data[i])){          //clean up cases if nodata is not defined but nans exist
             fprintf(f,"%f ",r->nodata);
           }else if(r->data[i] == 0){      //reduce file size
             fprintf(f,"0 "); 
           }else if( (roundf(r->data[i] * 10000.0)/10000.0) == 0){      //simplify and reduce file size
             fprintf(f,"0 ");
           }else{
             fprintf(f,"%f ",r->data[i]);
           }
           //update counter
           i+=1;
        }//..end of cols-1 in row

        //add last element in row and add line termination 
        if(isnan(r->data[i])){          
          fprintf(f,"%f\n",r->nodata);   
        }else if(r->data[i] == 0){   
          fprintf(f,"0\n");                          
        }else if( (roundf(r->data[i] * 10000.0)/10000.0) == 0){
          fprintf(f,"0\n");
        }else{
          fprintf(f,"%f\n",r->data[i]);
        }
        //update counter
        i+=1;

      }//..end of row loop

      //close file connection
      fclose(f);
    }//..end if file exists
    return 0;
}

void rasterinfo(Raster *r){
  //message to user..
  printf("nrows %d\t ncols %d\t cellsize %f nodata %f \t data[0] %f data[1] %f ...\n",r->nrows,r->ncols,r->cellsize,r->nodata,r->data[0],r->data[1]);
}


