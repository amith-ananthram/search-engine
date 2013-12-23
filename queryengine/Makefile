# Query make file

CC=gcc
CFLAGS1=-Wall -g
CFLAGS=-g -Wall -pedantic -std=c99 -ggdb
SOURCES=./query.c ./query.h ./queryfuncs.c ./queryfuncs.h
CFILES=./query.c ./queryfuncs.c
TFILES=./queryengine_test.c ./queryfuncs.c

UTILDIR=../util/
UTILFLAG=-ltseutil
UTILLIB=$(UTILDIR)libtseutil.a
UTILC=$(UTILDIR)hash.c $(UTILDIR)html.c $(UTILDIR)file.c $(UTILDIR)dictionary.c
UTILH=$(UTILC:.c=.h)

query:		$(SOURCES) $(UTILDIR)header.h $(UTILLIB)
		$(CC) $(CFLAGS) -o query $(CFILES) -L$(UTILDIR) $(UTILFLAG)

query_test: 	$(SOURCES) ./queryengine_test.c $(UTILDIR)header.h $(UTILLIB)
		$(CC) $(CFLAGS) -o query_test $(TFILES) -L$(UTILDIR) $(UTILFLAG)

$(UTILLIB): $(UTILC) $(UTILH)
			cd $(UTILDIR); make;

clean:
			rm -f query		
			rm -f *~
			rm -f *.o
			rm -f core*
			rm -f ../util/*~
			rm -f query_test
			rm -f valout
			rm -f ../util/libtseutil.a
