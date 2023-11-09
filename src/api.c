/*
Andre Koka - Created 10/8/2023
             Last Updated: 11/7/2023

The basic API file for the MANET Testbed - to implement:
- all common functions between other API files
*/

#include "../manet_testbed.h"
#include "api.h"
#include "api_if.h"
#include "api_send.h"
#include "api_route.h"
#include "api_queue.h"

pthread_mutex_t lock;
int fd = 0;
int f_err = 0;
uint32_t local_ip = 0; 
uint32_t broadcast_ip = 0; 

int InitializeAPI() // required to be called first
{
	check(InitializeIF());
	check(InitializeRoute());
	check(InitializeSend());
	check(InitializeQueue());
	if(f_err != 0)
		return -1;
	return 0;
}

// receive message (through the msghdr format) over socket fd
int get_msg(struct sockaddr_nl *sa, void *buf, size_t len)
{
	struct iovec iov;
	struct msghdr msg;
	iov.iov_base = buf;
	iov.iov_len = len;

	memset(&msg, 0, sizeof(msg));
	msg.msg_name = sa;
	msg.msg_namelen = sizeof(*sa);
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;

	return recvmsg(fd, &msg, 0); // block to receive message from kernel
}

void check(int val) // check for error returned
{
	if (val < 0) {
		f_err = 1;
	}
} 

char *ntop(int domain, void *buf) // convert ip to string
{
	static char ip[INET6_ADDRSTRLEN];
	inet_ntop(domain, buf, ip, INET6_ADDRSTRLEN);
	return ip;
}


