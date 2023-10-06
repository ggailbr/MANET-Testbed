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
uint32_t * addr; // stores address to return
uint32_t err = 0; // indicates error has occurred
char  *interface_name = "wlan0";
pthread_mutex_t lock;


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


static int add_request(struct sockaddr_nl *sa, int domain, char *ip) // send netlink message to get ip
{
	char buf[BUFLEN];
	memset(buf, 0, BUFLEN);
	int ip_len = 0;

	// assemble the message according to the netlink protocol
	struct nlmsghdr *nl;
	nl = (struct nlmsghdr*)buf;
	nl->nlmsg_len = NLMSG_LENGTH(sizeof(struct rtmsg));
	nl->nlmsg_type = RTM_NEWROUTE;
	nl->nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK | NLM_F_REPLACE | NLM_F_CREATE;

	// assemble rt message header according to the rtnetlink protocol
	struct rtmsg *rt;
	rt = (struct rtmsg*)NLMSG_DATA(nl);
	rt->rtm_family = domain;
	rt->rtm_table = RT_TABLE_MAIN; // liable to change
	rt->rtm_protocol = RTPROT_STATIC;
	rt->rtm_scope = RT_SCOPE_UNIVERSE;
	rt->rtm_type = RTN_UNICAST;

	// assemble rt attribute (src address)
	struct rtattr *rta = (struct rtattr*)RTM_RTA(rt);
	rta->rta_type = RTA_GATEWAY;
	ip_len = pton(domain, RTA_DATA(rta), ip);
	rta->rta_len = RTA_LENGTH(ip_len);
	nl->nlmsg_len = NLMSG_ALIGN(nl->nlmsg_len) + rta->rta_len;

	int l = BUFLEN - nl->nlmsg_len;
	rta = (struct rtattr*)RTA_NEXT(rta, l);
	rta->rta_type = RTA_OIF;
	rta->rta_len = RTA_LENGTH(sizeof(int));
	*((int*)RTA_DATA(rta)) = 2;
	nl->nlmsg_len += rta->rta_len;

	// assemble rt attribute (dest address)

	// prepare struct msghdr for sending.
	struct iovec iov = { nl, nl->nlmsg_len };
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

static int parse_ifa_msg(struct ifaddrmsg *ifa, void *buf, size_t len)
{
	char ifname[IF_NAMESIZE];
	if(ifa->ifa_index == if_nametoindex(interface_name)) {

		printf("==================================\n");
		printf("family:\t\t%d\n", (ifa->ifa_family == AF_INET) ? 4 :6);
		printf("dev:\t\t%s\n", if_indextoname(ifa->ifa_index, ifname));
		printf("prefix length:\t%d\n", ifa->ifa_prefixlen);
		printf("index:\t%d\n", ifa->ifa_index);
		printf("\n");

		struct rtattr *rta = NULL;
		int fa = ifa->ifa_family;
		for_each_rattr(rta, buf, len) {
			if (rta->rta_type == IFA_ADDRESS) {
				printf("if address:\t%s\n", ntop(fa, RTA_DATA(rta)));
				addr = (uint32_t  *)RTA_DATA(rta);
			}

			if (rta->rta_type == IFA_LOCAL) {
				printf("local address:\t%s\n", ntop(fa, RTA_DATA(rta)));
			}

			if (rta->rta_type == IFA_BROADCAST) {
				printf("broadcast:\t%s\n", ntop(fa, RTA_DATA(rta)));
			}
		}
	}

	return 0;
}

static uint32_t parse_nl_msg(void *buf, size_t len)
{
	struct nlmsghdr *nl = NULL;
	for_each_nlmsg(nl, buf, len) {
		if (nl->nlmsg_type == NLMSG_ERROR) {
			printf("error");
			return -1;
		}

		if (nl->nlmsg_type == RTM_NEWADDR) {
			struct ifaddrmsg *ifa;
			ifa = (struct ifaddrmsg*)NLMSG_DATA(nl);
			parse_ifa_msg(ifa, IFA_RTA(ifa), IFA_PAYLOAD(nl));
			continue;
		}
	}
	return nl->nlmsg_type;
}

int AddUnicastRoutingEntry(uint32_t dest_address, uint32_t next_hop)
{
	pthread_mutex_lock(&lock);
	
	// send request
	// receive acknowledgement


	pthread_mutex_unlock(&lock);

	return *addr;
}

//int set Interface(uint8 *interface)

int main(void)
{
	char *name = "eth0";

	int a = GetInterfaceIP((uint8_t *)name);
	printf("%X\n", a);

	if(err)
		return -1;
	else
		return *addr;
}