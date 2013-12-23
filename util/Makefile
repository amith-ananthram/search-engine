CFILES= ./hash.c ./html.c ./dictionary.c
HFILES=$(CFILES:.c=.h)

library:	$(CFILES) $(HFILES) ./file.c ./file.h
			gcc -Wall -c ./file.c			
			gcc -Wall -c -std=c99 $(CFILES)
			ar -cvq libtseutil.a *.o

clean:
			rm -f *~
			rm -f *.o
			rm -f *.a
