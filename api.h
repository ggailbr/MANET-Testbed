#ifndef API_H
#define API_H

/*
Andre Koka - Created 10/2/2023
             Last Updated: 10/2/2023

Header file for MANET Testbed API. Includes:
- global variables for socket, interface id, current routing table
- definitions for custom netlink functions
- function headers
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
int fd = 0; // netlink socket
uint32_t local_ip = 0; // node's ipv4 addr on wlan0
uint32_t broadcast_ip = 0; // node's broadcast addr for current network

// INITIALIZATION
/**
 * \brief Initializes functions related to interfaces. All sets local_ip 
 * and broadcast_ip for current node
 * 
 * \return 1 for success, -1 for failure
*/
int InitializeIF();

/**
 * \brief Initializes functions related to modifying routes. (unneeded?)
 * 
 * \return 1 for success, -1 for failure
*/
int InitializeRoute();

/**
 * \brief Initializes functions related to sending UDP messages. Also runs bash
 * script to set up iptables rules.
 * 
 * \return 1 for success, -1 for failure
*/
int InitializeSend();

/**
 * \brief Adds a unicast route to the current routing table. The route follows:
 * ip route add <dest_address> via <next_hop>
 * 
 * \param dest_address The ultimate destination address of the route
 * \param next_hop The next hop (next address to send to) of the route
 * 
 * \return 1 for success, -1 for failure
 * 
*/
int AddUnicastRoutingEntry(uint32_t dest_address, uint32_t next_hop);

/**
 * \brief Deletes a route in the current table. Only routes added by 
 * AddUnicastRoutingEntry should be deleted using this function.
 * 
 * \param dest_address The destination address of the route to delete
 * \param next_hop The next hop (gateway) of the route to delete 
 * 
 * \return 1 for success, -1 for failure
 * 
*/
int DeleteEntry(uint32_t dest_address, uint32_t next_hop);

/** 
 * \brief Switch the current routing table to the table provided
 * 
 * \param label The label of the table to switch to or create
 * 
 * \return If the process was a success
*/
int SwitchRoutingTable(uint8_t *table);

/**
 * \brief Sends a unicast message to the `dest_address`
 * 
 * \param[in] dest_address Character string with the destination address
 * \param[in] message_buffer The buffer to send in the packet (CRC calculated internally)
 * \param[in] header Optionally overwrite the header of the packet
 * 
 * \return If operation of sending had no errors
 * 
 */
int SendUnicast(uint32_t dest_address, uint8_t *msg_buf, uint8_t *header);

/**
 * \brief Sends a broadcast message to the interface broadcast IP address
 *
 * \param[in] message_buffer The buffer to send in the packet (CRC calculated internally)
 * \param[in] header Optionally overwrite the header of the packet
 * 
 * \return If operation of sending had no errors
 */
int SendBroadcast(uint8_t *msg_buf, uint8_t *header);

/**
 * \brief Gets the desired IP address associated with the given interface
 * 
 * \param interface The name of the interface to get the IP address
 * \param type The type of ip address to get (0 - default, 1 - broadcast)
 * 
 * \return The IP address of the interface or -1 for failure
 */
uint32_t GetInterfaceIP(uint8_t *interface, uint8_t type);

/**
 * \brief Sets the interface for the protocol to that interface (likely unneeded)
 * 
 * \param interface The name of the interface to send packets on 
 */
int SetInterface(uint8_t *interface);

/**
 * \brief Sets the interface for the protocol to that interface
 * 
 * \param interface The name of the interface to send packets on 
 */
int SetInterface(uint8_t *interface);

#endif

