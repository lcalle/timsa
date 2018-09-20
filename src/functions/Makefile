#======================================================================#
#  M  a  k  e  f  i  l  e                                              #
#                                                                      #
#    makefile for C version of TiMSA                                   #
#    written: leonardo calle                                           #
#    contact: leonardo.meiji@gmail.com                                 #
#======================================================================#

include ../../Makefile.inc

OBJ	= check_nodata.$O fail.$O fnraster.$O fncsv.$O fscanconfig.$O iterateday.$O iteratesurvey.$O tide_wdchange.$O\
	  writeindices.$O

INC	= ../../include
LIB	= ../../lib/libfunctions.$A

HDR = $(INC)/config.h $(INC)/errmsg.h $(INC)/freadascii.h $(INC)/freadcsv.h $(INC)/timsa.h $(INC)/types.h

$(LIB): $(OBJ)
	$(AR) $(ARFLAGS)$(LIB) $(OBJ)

$(OBJ): $(HDRS)

.c.$O: 
	$(CC) $(CFLAGS) -I$(INC) -c $*.c

clean: 
	$(RM) $(RMFLAGS) $(OBJ) $(LIB)
