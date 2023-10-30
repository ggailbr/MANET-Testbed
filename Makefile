CC=gcc
CFLAGS = -Wall -c -fpic
SRC := src
OBJ := obj
HEAD := head
SOURCES = $(wildcard $(SRC)/*.c)
OBJECTS = $(patsubst $(SRC)/%.c, $(OBJ)/%.o, $(SOURCES))
LIBPATH = -L/home/pi/Documents/MANET-Testbed

all: testbed

testbed: $(OBJECTS)
	$(CC) -shared -Wall $(OBJ)/*.o -o libtestbed.so
	make test
	make clean

$(OBJ)/%.o: $(SRC)/%.c
	$(CC) -I$(HEAD) -c $< -o $@

if: api_if.c
	$(CC) $(CFLAGS) api_if.c -o api_if.o

route: api_route.c
	$(CC) $(CFLAGS) api_route.c -o api_route.o

send: api_send.c
	$(CC) $(CFLAGS) api_send.c -o api_send.o

api: api.c
	$(CC) $(CFLAGS) api.c -o api.o

test: test.c
	$(CC) -Wall test.c -o test.out -ltestbed $(LIBPATH)

.rtnetlink_test.out: rtnetlink_test.c
	gcc -Wall rtnetlink_test.c -o rtnetlink_test.out

.blog_code.out: blog_code.c
	gcc -Wall blog_code.c -o blog_code.out

.random_git_code.out: random_git_code.c
	gcc -Wall random_git_code.c -o random_git_code.out

queue: libnetfilter_queue_example.c
	gcc -Wall libnetfilter_queue_example.c -lnfnetlink -lnetfilter_queue -o a.out

clean:
	rm -f $(OBJ)/*.o
