#ifndef API_H
#define API_H

/*
Andre Koka - Created 10/27/2023
             Last Updated: 10/27/2023

Internal header file for MANET Testbed. Includes:
- global variables for socket, interface id, current routing table
- definitions for custom netlink functions
- function headers for shared functions between api files
*/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/socket.h>         // linux socket API
#include <linux/netlink.h>      // netlink allows kernel<->userspace communications
#include <linux/rtnetlink.h>    // rtnetlink allows for modification of routing table
#include <arpa/inet.h>          // for converting ip addresses to binary
#include <net/if.h>             // for converting network interface names to binary
#include <pthread.h>			// API should be thread-safe

#define BUFLEN		4096

#define for_each_nlmsg(n, buf, len)					\
	for (n = (struct nlmsghdr*)buf;					\
	     NLMSG_OK(n, (uint32_t)len) && n->nlmsg_type != NLMSG_DONE;	\
	     n = NLMSG_NEXT(n, len))

#define for_each_rattr(n, buf, len)					\
	for (n = (struct rtattr*)buf; RTA_OK(n, len); n = RTA_NEXT(n, len))

// global variables
extern int fd; 
extern int f_err; // netlink socket
extern uint32_t local_ip; // node's ipv4 addr on wlan0
extern uint32_t broadcast_ip; // node's broadcast addr for current network
pthread_mutex_t lock; // providing thread safety

void check(int val); // check for error
char *ntop(int domain, void *buf); // convert ip to string

#endif

