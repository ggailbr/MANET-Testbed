#ifndef API_IF_H
#define API_IF_H

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

/**
 * \brief Initializes functions related to interfaces. Also sets local_ip 
 * and broadcast_ip for current node
 * 
 * \return 0 for success, -1 for failure
*/
int InitializeIF();
#endif

