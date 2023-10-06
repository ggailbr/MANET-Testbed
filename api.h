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

#define BUFLEN		4096

#define for_each_nlmsg(n, buf, len)					\
	for (n = (struct nlmsghdr*)buf;					\
	     NLMSG_OK(n, (uint32_t)len) && n->nlmsg_type != NLMSG_DONE;	\
	     n = NLMSG_NEXT(n, len))

#define for_each_rattr(n, buf, len)					\
	for (n = (struct rtattr*)buf; RTA_OK(n, len); n = RTA_NEXT(n, len))

// global socket
int fd = 0;

/**
 * \brief Adds a unicast route from `src_address` to `dest_address` from the current
 * table selected
 * 
 * \param dest_address The destination address of the route
 * \param next_hop The next hop of the 
 * 
 * \return The index of the routing entry added (-1 for failure)
 * 
*/
int AddUnicastRoutingEntry(uint32_t dest_address, uint32_t next_hop);
/**
 * \brief Deletes a route in the current table. If src_address and dest_address are supplied, 
 * will delete the route based on those options. Otherwise, an index can be provided
 * instead to delete the route
 * 
 * \param dest_address The destination address of the route
 * \param next_hop The next hop of the 
 * \param index The index of the route to delete
 * 
 * \return If the process was a success (-1 for failure)
 * 
*/
int DeleteEntry(uint32_t dest_address, uint32_t next_hop, int index);

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
int SendUnicast(uint32_t dest_address, uint8_t *message_buffer, uint8_t *header);

/**
 * \brief Sends a broadcast message to the interface broadcast IP address
 *
 * \param[in] message_buffer The buffer to send in the packet (CRC calculated internally)
 * \param[in] header Optionally overwrite the header of the packet
 * 
 * \return If operation of sending had no errors
 */
int SendBroadcast(uint8_t *message_buffer, uint8_t *header);

/**
 * \brief Gets the IP address associated with this interface
 * 
 * \param interface The name of the interface to get the IP address
 * 
 * \return The IP address of the interface or -1 for failure
 */
uint32_t GetInterfaceIP(uint8_t *interace);

/**
 * \brief Sets the interface for the protocol to that interface
 * 
 * \param interface The name of the interface to send packets on 
 */
int SetInterface(uint8_t *interface);

void InitializeApi();

#endif