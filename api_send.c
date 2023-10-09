/*
Andre Koka - Created 10/8/2023
             Last Updated: 10/8/2023

The basic API file for the MANET Testbed - to implement:
- SendUnicast - send a unicast message using UDP socket
- SendBroadcast - broadcast a message using UDP socket

*/

#include "api.h"

uint32_t f_err = 0; // indicates error has occurred
int sock = 0;
pthread_mutex_t lock;

static inline void check(int val) // check for error returned
{
	if (val < 0) {
		//printf("check error: %s\n", strerror(errno));
		f_err = 1;
	}
}

/* static inline char *ntop(int domain, void *buf) // convert ip to string
{
	static char ip[INET6_ADDRSTRLEN];
	inet_ntop(domain, buf, ip, INET6_ADDRSTRLEN);
	return ip;
} */

/* static int get_msg(struct sockaddr_nl *sa, void *buf, size_t len) // receive message over socket
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

	return recvmsg(fd, &msg, 0);
} */

static inline char *ntop(int domain, void *buf) // convert ip to string
{
	static char ip[INET6_ADDRSTRLEN];
	inet_ntop(domain, buf, ip, INET6_ADDRSTRLEN);
	return ip;
}

int send_sock_msg(uint32_t dest_address, uint8_t *msg_buf, uint8_t *header, int type)
{
    if(!sock) // open dgram socket for udp, if there isn't one
	{
        sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		int broadcastEnable=1;
		int ret=setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable)); // grant broadcast socket permissions
		check(ret);
	}
    if(sock < 0) { printf("error opening socket"); exit(0); }
    
	// initalize socket address and send
    struct sockaddr_in destination;
    memset(&destination, 0, sizeof(struct sockaddr_in));
    destination.sin_family = AF_INET;
	if(type)
    	inet_pton(AF_INET, "192.168.1.255", &(destination.sin_addr)); // broadcast
	else
		inet_pton(AF_INET, ntop(AF_INET, &dest_address), &(destination.sin_addr)); // unicast
    destination.sin_port = htons(269); // standard port for MANET comms

    int r = sendto(sock, msg_buf, strlen((char *)msg_buf), 0, (struct sockaddr*) &destination, sizeof(destination));
    if(r < 0) { printf("error sending: %d", errno); exit(0); }

    return r;
}

int SendUnicast(uint32_t dest_address, uint8_t *msg_buf, uint8_t *header)
{
	return send_sock_msg(dest_address, msg_buf, header, 0);
}

int SendBroadcast(uint8_t *msg_buf, uint8_t *header)
{
	return send_sock_msg(0, msg_buf, header, 1);
}

int main(void)
{
	char *dest = "192.168.1.8";
	char *nexthop = "192.168.1.15";

    uint8_t* msg = (uint8_t *)"test message";

    //SendUnicast(inet_addr(dest), msg, (uint8_t *) nexthop);
	//SendBroadcast(msg, (uint8_t *) nexthop);
	return 0;
}
