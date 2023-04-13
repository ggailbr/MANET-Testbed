.rtnetlink_test.out: rtnetlink_test.c
	gcc -Wall rtnetlink_test.c -o rtnetlink_test.out

default: .rtnetlink_test.out
	./rtnetlink_test.out