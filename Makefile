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

.rtnetlink_test.out: rtnetlink_test.c
	gcc -Wall rtnetlink_test.c -o rtnetlink_test.out

.blog_code.out: blog_code.c
	gcc -Wall blog_code.c -o blog_code.out

.random_git_code.out: random_git_code.c
	gcc -Wall random_git_code.c -o random_git_code.out

queue: Examples/libnetfilter_queue_example.c
	gcc -Wall Examples/libnetfilter_queue_example.c -lnfnetlink -lnetfilter_queue -o a.out

clean:
	rm -f $(OBJ)/*.o
	rm -f libtestbed.so
	rm -f test.out
