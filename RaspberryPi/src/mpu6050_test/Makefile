 FLAGS=
 LIBS=-lwiringPi -lm
 SRC = mputest.c 
 OBJECTS=$(SRC:.c=.o)
 EXECUTABLE=mputest.bin
 CC=gcc

all: $(SRC) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(FLAGS) $(LIBS)  $(OBJECTS)  -o $@ -DGLPIb -Wall

.c.o:
	$(CC) $(FLAGS) $(LIBS) -DGLPI -c  $<  -o  $@ 

clean:
	rm -f *.bin
	rm -f $(OBJECTS)

