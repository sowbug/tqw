GAMES = fire.o life.o sprites.o bowl.o chord.o krypton.o \
    wave.o testpat.o zipper.o ringer.o
OBJECTS = tqw.o random.o memory.o $(GAMES)

wgig : ${OBJECTS}
	gcc -o $@ -g $^ `gtk-config --libs` -lpthread

memory.o : memory.c
	gcc -o $@ -c -g -Wall -DMODEL=0 -DSIMULATING $< `gtk-config --cflags`

.c.o :
	gcc -o $@ -c -g -Wall -DSIMULATING $< `gtk-config --cflags`

*.o :  tqw.h rand.h memory.h