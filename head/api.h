#ifndef API_H
#define API_H

/*
Andre Koka - Created 10/27/2023
             Last Updated: 11/9/2023

Internal header file for MANET Testbed. Includes:
- global variables that are used between api source files
- definitions for custom netlink functions
- function headers for shared functions between api files
*/

// Primary Issues
// - counting packets in the queue
// - incoming/outgoing/forwarding logic - is it all correct?
//          - look into queueing based on destination

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
#include <linux/netfilter.h>
#include <libnetfilter_queue/libnetfilter_queue.h>

#define BUFLEN		4096

#define for_each_nlmsg(n, buf, len)					\
	for (n = (struct nlmsghdr*)buf;					\
	     NLMSG_OK(n, (uint32_t)len) && n->nlmsg_type != NLMSG_DONE;	\
	     n = NLMSG_NEXT(n, len))

#define for_each_rattr(n, buf, len)					\
	for (n = (struct rtattr*)buf; RTA_OK(n, len); n = RTA_NEXT(n, len))

// global variables
extern int fd; // netlink socket
extern int f_err;
extern uint32_t local_ip; // node's ipv4 addr on wlan0
extern uint32_t broadcast_ip; // node's broadcast addr for current network
pthread_mutex_t lock; // providing thread safety

void check(int val); // check for error
char *ntop(int domain, void *buf); // convert ip to string

/**
 * \brief Helper function that receives a message over the current netlink socket, using
 * struct msghdr and struct iovec
 * 
 * \param sa Pointer to a struct sockaddr_nl that was used in get_ip()
 * \param buf Buffer to hold msg contents
 * \param len Length of buf
 * 
 * \return number of bytes read, or -1 on error
*/
int get_msg(struct sockaddr_nl *sa, void *buf, size_t len);

/**
 * \brief Very rudimentary error checking function, which sets the 
 * api-specific f_err flag to 1
 * 
 * \param val The return value (of a function) to check for errors
 * 
*/
void check(int val);

/**
 * \brief Simple function to convert an ipv4 address in uint32_t form to 
 * a null-terminated string form. Used in api_send.c and for debugging
 * 
 * \return A character pointer to the string version of the ipv4 address
 * 
*/
char *ntop(int domain, void *buf);

#endif

