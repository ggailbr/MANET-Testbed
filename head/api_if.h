#ifndef API_IF_H
#define API_IF_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/socket.h>         // linux socket API
#include <linux/netlink.h>      // netlink allows kernel<->userspace communications
#include <pthread.h>			// API should be thread-safe

/**
 * \brief Helper function that initializes netlink socket for the testbed. Also sets local_ip 
 * and broadcast_ip for current node
 * 
 * \return 0 for success, -1 for failure
*/
int InitializeIF();

/**
 * \brief Helper function that formats and sends a netlink message to 
 * acquire a node's local or broadcast ip, using struct msghdr and struct iovec
 * 
 * \param sa Pointer to a struct sockaddr_nl to be included in the iovec
 * \param domain Indicates use of ipv4 or ipv6 (only ipv4, known as AF_INET, currently supported)
 * 
 * \return 0 for success, -1 for failure
*/
static int get_ip(struct sockaddr_nl *sa, int domain);

/**
 * \brief Helper function that processes one IFA message received from netlink
 * 
 * \param ifa Pointer to a struct ifaddrmsg that was received from netlink, pointing to current
 * ifa message to parse
 * \param buf Buffer to hold ifa msg contents
 * \param len Length of buf
 * \param type Indicates which address to get (0 for local, 1 for broadcast)
 * 
 * \return number of bytes read, or -1 on error
*/
static int parse_ifa_msg(struct ifaddrmsg *ifa, void *buf, size_t len, uint8_t type);

/**
 * \brief Helper function that parses one received netlink message regarding interfaces
 * 
 * \param buf Buffer to hold netlink msg contents
 * \param len Length of buf
 * \param type Indicates which address to get (0 for local, 1 for broadcast)
 * 
 * \return number of bytes read, or -1 on error
*/
static uint32_t parse_nl_if_msg(void *buf, size_t len, uint8_t type);

#endif