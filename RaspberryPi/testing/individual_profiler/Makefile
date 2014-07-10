 FLAGS= -I../../src/
 LIBS= -lm
 SRC = individual_profiler.c  $(wildcard ../../src/*.c)
 OBJECTS=$(SRC:.c=.o)
 EXECUTABLE=individual_profiler.bin
 CC=gcc

all: $(SRC) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(FLAGS) $(LIBS)  $(OBJECTS)  -o $@ -Wall

.c.o:
	$(CC) $(FLAGS) $(LIBS) -c  $<  -o  $@ 

clean:
	rm -f *.bin
	rm -f $(OBJECTS)

