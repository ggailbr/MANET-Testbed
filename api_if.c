/*
Andre Koka - Created 9/28/2023
             Last Updated: 10/27/2023

The basic API file for the MANET Testbed - to implement:
- GetInterfaceIP - retrieve ipv4 of an interface given its index
- initialization involves setting global ip_address variables of the current node

Adapted from: https://github.com/d0u9/examples/blob/master/C/netlink/ip_show.c
*/

#include "api.h"
#include "api_if.h"

uint32_t * addr; // stores address to return
char  *interface_name = "wlan0"; // wlan0 by default

static int get_ip(struct sockaddr_nl *sa, int domain) // send netlink message to get ip
{
	char buf[BUFLEN];
	memset(buf, 0, BUFLEN);

	// assemble the message according to the netlink protocol
	struct nlmsghdr *nl;
	nl = (struct nlmsghdr*)buf;
	nl->nlmsg_len = NLMSG_LENGTH(sizeof(struct ifaddrmsg));
	nl->nlmsg_type = RTM_GETADDR;
	nl->nlmsg_flags = NLM_F_REQUEST | NLM_F_ROOT; // need root flag

	struct ifaddrmsg *ifa;
	ifa = (struct ifaddrmsg*)NLMSG_DATA(nl);
	ifa->ifa_family = domain; // we only get ipv4 address here

	// prepare struct msghdr for sending.
	struct iovec iov = { nl, nl->nlmsg_len };
	struct msghdr msg = { sa, sizeof(*sa), &iov, 1, NULL, 0, 0 };

	// send netlink message to kernel.
	int r = sendmsg(fd, &msg, 0);
	return (r < 0) ? -1 : 0;
}

// receive reply from kernel over netlink socket
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

// interpret the ifa message(s) received from the kernel
static int parse_ifa_msg(struct ifaddrmsg *ifa, void *buf, size_t len, uint8_t type)
{
	if(ifa->ifa_index == if_nametoindex(interface_name)) {
		struct rtattr *rta = NULL;
		for_each_rattr(rta, buf, len) {
			if (rta->rta_type == IFA_ADDRESS && !type) {
				addr = (uint32_t  *)RTA_DATA(rta);
			}
			else if (rta->rta_type == IFA_LOCAL) {}
			else if (rta->rta_type == IFA_BROADCAST && type) {
				addr = (uint32_t *)RTA_DATA(rta);
			}
		}
	}
	else if (if_nametoindex(interface_name) == 0)
		f_err = -1;

	return f_err;
}

static uint32_t parse_nl_msg(void *buf, size_t len, uint8_t type)
{
	struct nlmsghdr *nl = NULL;
	for_each_nlmsg(nl, buf, len) {
		if (nl->nlmsg_type == NLMSG_ERROR) {
			f_err = -1;
			return -1;
		}

		if (nl->nlmsg_type == RTM_NEWADDR) {
			struct ifaddrmsg *ifa;
			ifa = (struct ifaddrmsg*)NLMSG_DATA(nl);
			parse_ifa_msg(ifa, IFA_RTA(ifa), IFA_PAYLOAD(nl), type);
			continue;
		}
	}
	return nl->nlmsg_type;
}

uint32_t GetInterfaceIP(uint8_t *interface, uint8_t type)
{
	pthread_mutex_lock(&lock);
	int len = 0;
	if(interface != NULL) // if bad input, use last inteface (wlan0 default)
		interface_name = (char *)interface; 

	struct sockaddr_nl sa;
	memset(&sa, 0, sizeof(sa));
	sa.nl_family = AF_NETLINK;

	len = get_ip(&sa, AF_INET);
	check(len);

	char buf[BUFLEN];
	uint32_t nl_msg_type;
	do {
		len = get_msg(&sa, buf, BUFLEN);
		check(len);

		nl_msg_type = parse_nl_msg(buf, len, type);
	} while (nl_msg_type != NLMSG_DONE && nl_msg_type != NLMSG_ERROR);
	pthread_mutex_unlock(&lock);

	return (f_err != 0) ? -1 : *addr;
}

int InitializeIF()
{
	if(!fd) {  // create socket if it hasn't been made
		fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
		check(fd); }
	local_ip = GetInterfaceIP(NULL, 0);
	broadcast_ip = GetInterfaceIP(NULL, 1);
	
	if(local_ip == 0 || broadcast_ip == 0 || f_err != 0)
		return -1;
	else
		return 0;

}

