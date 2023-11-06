CC=gcc
CFLAGS = -Wall -c -fpic
LIBFLAGS = -ltestbed
SRC := src
OBJ := obj
HEAD := head
SOURCES = $(wildcard $(SRC)/*.c)
OBJECTS = $(patsubst $(SRC)/%.c, $(OBJ)/%.o, $(SOURCES))
LIBPATH = -L/home/pi/Documents/MANET-Testbed

all: testbed

testbed:
	make clean
	make $(OBJECTS)
	$(CC) -shared -Wall $(OBJ)/*.o -o libtestbed.so

$(OBJ)/%.o: $(SRC)/%.c
	$(CC) -I$(HEAD) -c $< -o $@

test: test.c
	$(CC) -Wall test.c -o test.out -ltestbed $(LIBPATH) -pthread -lnetfilter_queue

debug:
	make clean
	make $(OBJECTS)
	$(CC) -shared -DDEBUG -Wall $(OBJ)/*.o -o libtestbed.so

clean:
	rm -f $(OBJ)/*.o
	rm -f libtestbed.so
	rm -f test.out
