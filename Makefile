CC=gcc
CFLAGS = -Wall -c -fpic
DEPS = api.h
LIBPATH = -L/home/pi/Documents/MANET-Testbed/

testbed: if route send api api_if.o api_route.o api_send.o api.o
	$(CC) -shared -Wall api_if.o api_route.o api_send.o api.o -o libtestbed.so
	make test
	make clean

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

clean:
	rm -f api_if.o
	rm -f api_route.o
	rm -f api_send.o
	rm -f api.o
