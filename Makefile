########################################################################
##                                                                    ##
##               M  a  k  e  f  i  l  e                               ##
##                                                                    ##
##   Makefile for C implementation of..                               ##
##   Tidal Model of Shallow Water Availability  (TiMSA)               ##
##   written by Leonardo Calle                                        ##
########################################################################

include Makefile.inc

INC     = include

HDRS    = $(INC)/config.h $(INC)/freadascii.h $(INC)/freadcsv.h $(INC)/timsa.h $(INC)/types.h

CONF	= timsa.conf

FILES	= Makefile Makefile.gcc\
          $(CONF) $(HDRS)

bin:
	$(MKDIR) lib
	(cd src && $(MAKE))

all: bin

test: bin
	./timsa

clean:
	rm -r lib
	(cd src && $(MAKE) clean)

archive: 
	tar -zcvf timsa.tar.gz $(FILES) ${INC}/*.h src/Makefile src/*.c

