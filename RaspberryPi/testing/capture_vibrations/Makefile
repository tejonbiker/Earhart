 FLAGS= -I../../src/
 LIBS= -lm -lpthread
 SRC = capture_vibrations.c $(wildcard ../../src/*.c)
 OBJECTS=$(SRC:.c=.o)
 EXECUTABLE=capture_vibrations.bin
 CC=gcc

all: $(SRC) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(FLAGS) $(LIBS)  $(OBJECTS)  -o $@ -Wall

.c.o:
	$(CC) $(FLAGS) $(LIBS) -c  $<  -o  $@ 

clean:
	rm -f *.bin
	rm -f $(OBJECTS)

