.rtnetlink_test.out: rtnetlink_test.c
	gcc -Wall rtnetlink_test.c -o rtnetlink_test.out

.blog_code.out: blog_code.c
	gcc -Wall blog_code.c -o blog_code.out

.random_git_code.out: random_git_code.c
	gcc -Wall random_git_code.c -o random_git_code.out