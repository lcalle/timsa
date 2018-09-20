#======================================================================#
#  M  a  k  e  f  i  l  e                                              #
#                                                                      #
#    makefile for C version of TiMSA                                   #
#    written: leonardo calle                                           #
#    contact: leonardo.meiji@gmail.com                                 #
#======================================================================#

include ../Makefile.inc

OBJS	= timsa.$O

INC     = ../include

HDRS    = $(INC)/config.h $(INC)/errmsg.h $(INC)/fnraster.h $(INC)/fncsv.h $(INC)/timsa.h $(INC)/types.h

LIB     = ../lib

TIMSALIBS = $(LIB)/libfunctions.$A 

EXE	= ../timsa$E

bin:
	(cd functions && $(MAKE))
	$(MAKE) timsa

clean:
	(cd functions && $(MAKE) clean) 
	$(RM) $(RMFLAGS) $(OBJS) ../timsa$E 

$(OBJS): $(HDRS)

.c.$O: 
	$(CC) $(CFLAGS) -I$(INC) -c $*.c

timsa: $(TIMSALIBS) timsa.$O
	$(LINK) $(LNOPTS)$(EXE) timsa.$O $(TIMSALIBS)  $(LIBS)

all: $(EXE)

