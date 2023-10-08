CC=gcc
CFLAGS = -Wall
DEPS = api.h

testbed: if route send

if: api_if.c
	$(CC) $(CFLAGS) api_if.c -o api_if.out

route: api_route.c
	$(CC) $(CFLAGS) api_route.c -o api_route.out

send: api_send.c
	$(CC) $(CFLAGS) api_send.c -o api_send.out

.rtnetlink_test.out: rtnetlink_test.c
	gcc -Wall rtnetlink_test.c -o rtnetlink_test.out

.blog_code.out: blog_code.c
	gcc -Wall blog_code.c -o blog_code.out

.random_git_code.out: random_git_code.c
	gcc -Wall random_git_code.c -o random_git_code.out

clean:
	rm -f api_if.out
	rm -f api_route.out
	rm -f api_send.out
