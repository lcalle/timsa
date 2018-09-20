########################################################################
##                                                                    ##
##               M  a  k  e  f  i  l  e  .  i n c                     ##
##                                                                    ##
##   Make include file for compiler and os specific settings          ##
##   Compile and link options for GNU C compiler gcc                  ##
##                                                                    ##
##                                                                    ##
########################################################################

CC      = gcc
CFLAGS  = -g -O3 -Wall
O	= o
A	= a
E	=
AR	= ar
ARFLAGS	= r 
RM	= rm 
RMFLAGS	= -f
LIBS	= -lm -lnetcdf
LINK	= cc
MKDIR	= mkdir -p
LNOPTS	= $(CFLAGS) -o 
LM	= -lm
