/*
Andre Koka - Created 9/28/2023
             Last Updated: 10/2/2023

The basic API file for the MANET Testbed - to implement:
- GetInterfaceIP - retrieve ipv4 of an interface given its index
- SetInterface   - sets (?)

Adapted from: https://github.com/d0u9/examples/blob/master/C/netlink/ip_show.c
*/

/* #include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/socket.h>         // linux socket API
#include <linux/netlink.h>      // netlink allows kernel<->userspace communications
#include <linux/rtnetlink.h>    // rtnetlink allows for modification of routing table
#include <arpa/inet.h>          // for converting ip addresses to binary
#include <net/if.h>             // for converting network interface names to binary */

#include "api.h"
//uint32_t * addr; // stores address to return
uint32_t err = 0; // indicates error has occurred
char  *interface_name = "wlan0";
pthread_mutex_t lock;

struct rt_request{ // buffer to hold formed rtnetlink request
  struct nlmsghdr nl;
  struct rtmsg    rt;
  char            buf[BUFLEN];
};


static inline void check(int val) // check for error returned
{
	if (val < 0) {
		printf("check error: %s\n", strerror(errno));
		exit(1);
	}
}

static inline char *ntop(int domain, void *buf) // convert ip to string
{
	static char ip[INET6_ADDRSTRLEN];
	inet_ntop(domain, buf, ip, INET6_ADDRSTRLEN);
	return ip;
}


static int add_request(struct sockaddr_nl *sa, int domain, uint32_t dest, uint32_t nexthop) // send netlink message to get ip
{
	// intialize request structure
	struct rt_request req;
	memset(&req, 0, sizeof(req));
	int rt_len = sizeof(struct rtmsg); // rolling calculation of rt attribute sizes
;	int interface = 3;

	// setup netlink header
	//req.nl.nlmsg_len = NLMSG_LENGTH(sizeof(struct rtmsg));
	req.nl.nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK | NLM_F_REPLACE | NLM_F_CREATE | NLM_F_ROOT;
	req.nl.nlmsg_type = RTM_NEWROUTE;

	// set up rtmsg header
	req.rt.rtm_family = domain;
	req.rt.rtm_table = RT_TABLE_MAIN; // liable to change
	req.rt.rtm_protocol = RTPROT_STATIC;
	req.rt.rtm_scope = RT_SCOPE_LINK;
	req.rt.rtm_type = RTN_UNICAST;
	req.rt.rtm_dst_len = 32; // network prefix size is 32 for ipv4

	// set up first attribute (destination address)
	struct rtattr* rta = (struct rtattr*) req.buf;
	rta->rta_type = RTA_DST;
	rta->rta_len = RTA_LENGTH(sizeof(uint32_t));
	//req.nl.nlmsg_len = NLMSG_ALIGN(req.nl.nlmsg_len) + RTA_LENGTH(sizeof(dest));
	memcpy(RTA_DATA(rta), &dest, sizeof(dest));
	rt_len += rta->rta_len;

	// set up second attribute (interface)
	rta = (struct rtattr *) (((char *)rta) + rta->rta_len);
  	rta->rta_type = RTA_OIF; // indicates interface attribute
  	rta->rta_len = RTA_LENGTH(sizeof(int));
  	memcpy(RTA_DATA(rta), &interface, sizeof(int));
 	rt_len += rta->rta_len; // now rt_len is size of rt_msg_hdr + all attributes
	req.nl.nlmsg_len = NLMSG_LENGTH(rt_len); // final update of nlmsg length

	// set up third attribute (gateway)
	rta = (struct rtattr*) (((char *)rta) + rta->rta_len);
  	rta->rta_type = RTA_GATEWAY; // indicates gateway attribute
  	rta->rta_len = RTA_LENGTH(sizeof(uint32_t));
  	memcpy(RTA_DATA(rta), &nexthop, sizeof(uint32_t));
 	rt_len += rta->rta_len; // now rt_len is size of rt_msg_hdr + all attributes
	req.nl.nlmsg_len = NLMSG_LENGTH(rt_len-4); // final update of nlmsg length 
	printf("A: %d\t%d\n", rt_len, req.nl.nlmsg_len); 

	// prepare struct msghdr for sending.
	struct iovec iov = { &req, req.nl.nlmsg_len};
	struct msghdr msg = { sa, sizeof(*sa), &iov, 1, NULL, 0, 0 };

	// send netlink message to kernel.
	int r = sendmsg(fd, &msg, 0);
	return (r < 0) ? -1 : 0;
}

static int get_msg(struct sockaddr_nl *sa, void *buf, size_t len) // receive message over socket
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
}

static uint32_t parse_nl_msg(void *buf, size_t len)
{
	struct nlmsghdr *nl = NULL;
	nl = (struct nlmsghdr*)buf;
	
	if (!NLMSG_OK(nl, len)) 
		return 0;
	return nl->nlmsg_type;
}

int AddUnicastRoutingEntry(uint32_t dest_address, uint32_t next_hop)
{
	pthread_mutex_lock(&lock);
	
	int len = 0;
	if(!fd) {  // create socket if it hasn't been made
		fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
		check(fd); }

	struct sockaddr_nl sa;
	memset(&sa, 0, sizeof(sa));
	sa.nl_family = AF_NETLINK;

	len = add_request(&sa, AF_INET, dest_address, next_hop); // To get ipv6, use AF_INET6 instead
	check(len);

	// after sending, we need to check the result
	char buf[BUFLEN];
	uint32_t nl_msg_type;
	len = get_msg(&sa, buf, BUFLEN);
	check(len);

	nl_msg_type = parse_nl_msg(buf, len);
	if (nl_msg_type == NLMSG_ERROR) {
		struct nlmsgerr *err = (struct nlmsgerr*)NLMSG_DATA(buf);
		switch (err->error) {
		case 0:
			printf("Success\n");
			break;
		case -EADDRNOTAVAIL:
			printf("Failed\n");
			break;
		case -EPERM:
			printf("Permission denied\n");
			break;
		default:
			printf("NL message error: %s\n", strerror(err->error));
		}
	}

	pthread_mutex_unlock(&lock);

	return nl_msg_type;
}

//int set Interface(uint8 *interface)

int main(void)
{
	char *dest = "192.168.1.11";
	char *nexthop = "192.168.1.15";
	//inet_pton(AF_INET, dest, &buf);

	int a = AddUnicastRoutingEntry(inet_addr(dest), inet_addr(nexthop));
	printf("%X\n", a);

	if(err)
		return -1;
}