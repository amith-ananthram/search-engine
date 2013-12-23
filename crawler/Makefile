# Crawler make file

CC=gcc
CFLAGS1=-Wall -g
CFLAGS=-g -Wall -pedantic -std=c99 -ggdb
SOURCES=./crawler.c ./crawler.h
CFILES=./crawler.c

UTILDIR=../util/
UTILFLAG=-ltseutil
UTILLIB=$(UTILDIR)libtseutil.a
UTILC=$(UTILDIR)hash.c $(UTILDIR)html.c $(UTILDIR)file.c $(UTILDIR)dictionary.c
UTILH=$(UTILC:.c=.h)

crawler:	$(SOURCES) $(UTILDIR)header.h $(UTILLIB)
			$(CC) $(CFLAGS) -o crawler $(CFILES) -L$(UTILDIR) $(UTILFLAG)

$(UTILLIB): $(UTILC) $(UTILH)
			cd $(UTILDIR); make;

clean:
			rm -f crawler		
			rm -f *~
			rm -f data/*
			rm -f *.o
			rm -f core*
			rm -f ../util/*~
			rm -f valout
			rm -f ../util/libtseutil.a
