#ifndef API_ROUTE_H
#define API_ROUTE_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/socket.h>         // linux socket API
#include <linux/netlink.h>      // netlink allows kernel<->userspace communications
#include <linux/rtnetlink.h>    // rtnetlink allows for modification of routing table
#include <pthread.h>			// API should be thread-safe

struct rt_request{ // buffer to hold formed rtnetlink request
  struct nlmsghdr nl;
  struct rtmsg    rt;
  char            buf[BUFLEN];
};

/**
 * \brief Initializes functions related to modifying routes (UNUSED)
 * 
 * \return 0 for success, -1 for failure
*/
int InitializeRoute();

/**
 * \brief Helper function that forms and sends a netlink message to modify a unicast route in the main routing table
 * 
 * \param sa The struct sockaddr_nl pointer to indicate what family (ipv4) the netlink message is for
 * \param domain Indicates which family the netlink message should be formed for (only ipv4 supported)
 * \param dest The destination address (ipv4)
 * \param nexthop The address for where the route should use as its gateway (the next hop needed by the routing protocol)
 * \param action The netlink action required for the netlink message (should always be RTM_NEWROUTE or RTM_DELROUTE)
 * 
 * \return 0 for success, -1 for failure
*/
static int form_request(struct sockaddr_nl *sa, int domain, uint32_t dest, uint32_t nexthop, uint8_t action);

/**
 * \brief Helper function that parses a received netlink message regarding routing table modifications
 * 
 * \param buf Buffer holding the received reply from the kernel
 * \param len The size of buf
 * 
 * \return The netlink message type retrieved from buf
*/
uint32_t parse_nl_route_msg(void *buf, size_t len);


#endif

