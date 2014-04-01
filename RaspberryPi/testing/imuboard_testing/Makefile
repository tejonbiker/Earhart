 FLAGS= -I../../src/
 LIBS= -lm
 SRC = imuboard_testing.c  $(wildcard ../../src/*.c)
 OBJECTS=$(SRC:.c=.o)
 EXECUTABLE=imuboard_testing.bin
 CC=gcc

all: $(SRC) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(FLAGS) $(LIBS)  $(OBJECTS)  -o $@ -Wall

.c.o:
	$(CC) $(FLAGS) $(LIBS) -c  $<  -o  $@ 

clean:
	rm -f *.bin
	rm -f $(OBJECTS)

