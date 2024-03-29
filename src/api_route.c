/*
Andre Koka - Created 9/28/2023
             Last Updated: 11/7/2023

The basic API file for the MANET Testbed - to implement:
- AddUnicastRoutingEntry - modify current routing table with new address (src, dest, gateway, interface)
- DeleteEntry - remove a route from the routing table given dest, gateway, interface
- SwitchRoutingTable - change the current routing table to be changed by the API (may not use)

Adapted from: https://github.com/d0u9/examples/blob/master/C/netlink/gateway_add.c
*/

#include "api.h"
#include "api_route.h"

// forms and sends netlink message to add route (dest ip, gateway ip, interface)
static int form_request(struct sockaddr_nl *sa, int domain, uint32_t dest, uint32_t nexthop, uint8_t action)
{
	// intialize request structure
	struct rt_request req;
	memset(&req, 0, sizeof(req));
	int rt_len = sizeof(struct rtmsg); // rolling calculation of rt attribute sizes
;	int interface = 3; // indicates wlan0

	// setup netlink header
	req.nl.nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK | NLM_F_REPLACE | NLM_F_CREATE | NLM_F_ROOT;
	req.nl.nlmsg_type = action;

	// set up rtmsg header
	req.rt.rtm_family = domain;
	req.rt.rtm_table = RT_TABLE_MAIN; // liable to change
	req.rt.rtm_protocol = RTPROT_STATIC;
	req.rt.rtm_scope = RT_SCOPE_UNIVERSE;
	req.rt.rtm_type = RTN_UNICAST;
	req.rt.rtm_dst_len = 32; // network prefix size is 32 for ipv4

	// set up first attribute (destination address)
	struct rtattr* rta = (struct rtattr*)(req.buf);
	rta->rta_type = RTA_DST;
	rta->rta_len = RTA_LENGTH(sizeof(uint32_t));
	memcpy(RTA_DATA(rta), &dest, sizeof(dest));
	rt_len += rta->rta_len;
	req.nl.nlmsg_len = NLMSG_LENGTH(rt_len);

	// set up second attribute (interface)
	rta = (struct rtattr *) (((char *)rta) + rta->rta_len);
  	rta->rta_type = RTA_OIF;
  	rta->rta_len = RTA_LENGTH(sizeof(int));
  	memcpy(RTA_DATA(rta), &interface, sizeof(int));
 	rt_len += rta->rta_len; // now rt_len is size of rt_msg_hdr + all attributes
	req.nl.nlmsg_len = NLMSG_LENGTH(rt_len);

	// set up third attribute (gateway)
	rta = (struct rtattr*) (((char *)rta) + rta->rta_len);
  	rta->rta_type = RTA_GATEWAY;
  	rta->rta_len = RTA_LENGTH(sizeof(uint32_t));
  	memcpy(RTA_DATA(rta), &nexthop, sizeof(uint32_t));
 	rt_len += rta->rta_len; // now rt_len is size of rt_msg_hdr + all attributes
	req.nl.nlmsg_len = NLMSG_LENGTH(rt_len); // final update of nlmsg length

	// prepare struct msghdr for sending
	struct iovec iov = { &req, req.nl.nlmsg_len};
	struct msghdr msg = { sa, sizeof(*sa), &iov, 1, NULL, 0, 0 };

	// send netlink message to kernel
	int r = sendmsg(fd, &msg, 0);
	return (r < 0) ? -1 : 0;
}

// parse reply from kernel over netlink socket
uint32_t parse_nl_route_msg(void *buf, size_t len)
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

	struct sockaddr_nl sa;
	memset(&sa, 0, sizeof(sa));
	sa.nl_family = AF_NETLINK; // for now, only ipv4 support

	len = form_request(&sa, AF_INET, dest_address, next_hop, RTM_NEWROUTE);
	check(len);

	// after sending, we need to check the result
	char buf[BUFLEN];
	uint32_t nl_msg_type;
	len = get_msg(&sa, buf, BUFLEN);
	check(len);

	nl_msg_type = parse_nl_route_msg(buf, len);
	if (nl_msg_type == NLMSG_ERROR) {
		struct nlmsgerr *err = (struct nlmsgerr*)NLMSG_DATA(buf);
		switch (err->error) {
		case 0:
			break;
		default: // any error in nlmsg goes here
			f_err = 1;
		}
	}

	pthread_mutex_unlock(&lock);
	return (f_err == 0) ? 0 : -1;
}

int DeleteEntry(uint32_t dest_address, uint32_t next_hop)
{
	pthread_mutex_lock(&lock);

	int len = 0;
	struct sockaddr_nl sa;
	memset(&sa, 0, sizeof(sa));
	sa.nl_family = AF_NETLINK; // only supports ipv4

	len = form_request(&sa, AF_INET, dest_address, next_hop, RTM_DELROUTE); // To get ipv6, use AF_INET6 instead
	check(len);

	// after sending, we need to check the result
	char buf[BUFLEN];
	uint32_t nl_msg_type;
	len = get_msg(&sa, buf, BUFLEN);
	check(len);

	nl_msg_type = parse_nl_route_msg(buf, len);
	if (nl_msg_type == NLMSG_ERROR) {
		struct nlmsgerr *err = (struct nlmsgerr*)NLMSG_DATA(buf);
		switch (err->error) {
		case 0: // indicates no error
			break;
		default: // any error in nlmsg goes here
			f_err = 1;
		}
	}

	pthread_mutex_unlock(&lock);
	return (f_err == 0) ? 0 : -1;
}

int InitializeRoute() // currently unused
{
	//printf("route initialized\n");
	return 0;
}
