#ifndef MANET_TESTBED
#define MANET_TESTBED

/*
Andre Koka - Created 10/2/2023
             Last Updated: 11/9/2023

Header file for MANET Testbed API. Includes definitions for all global variables 
and function that are visibile to the user (routing protocol tester)
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
#include <arpa/inet.h>          // definitions of internet operations
#include <net/if.h>             // for managing network interfaces
#include <pthread.h>

#define PACKET_ACCEPT 1
#define PACKET_DROP 0

typedef uint8_t (*CallbackFunction) (uint8_t *raw_pack, uint32_t src, uint32_t dest, uint8_t *payload, uint32_t payload_length); 

/**
 * \brief Initializes structures for the MANET Testbed. Required to be called first
 * before using any functions provided by the API.
 * 
 * \return 0 for success, -1 for failure
 * 
*/
int InitializeAPI();

/**
 * \brief Adds a unicast route to the current routing table. The route follows:
 * ip route add <dest_address> via <next_hop>
 * 
 * \param dest_address The ultimate destination address of the route
 * \param next_hop The next hop (next address to send to) of the route
 * 
 * \return 0 for success, -1 for failure
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
 * \return 0 for success, -1 for failure
 * 
*/
int DeleteEntry(uint32_t dest_address, uint32_t next_hop);

/** 
 * \brief Switch the current routing table to the table provided (currently unused)
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
 * \return 0 for success, -1 for failure
 * 
 */
int SendUnicast(uint32_t dest_address, uint8_t *msg_buf, uint32_t size, uint8_t *header);

/**
 * \brief Sends a broadcast message to the interface broadcast IP address
 *
 * \param[in] message_buffer The buffer to send in the packet (CRC calculated internally)
 * \param[in] header Optionally overwrite the header of the packet
 * 
 * \return 0 for success, -1 for failure
 */
int SendBroadcast(uint8_t *msg_buf, uint32_t size, uint8_t *header);

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
 * \brief Sets the interface for the protocol to that interface (currently unused)
 * 
 * \param interface The name of the interface to send packets on 
 */
int SetInterface(uint8_t *interface);

/**
 * Planned to-do
 * \brief Search the routing table for a specific entry
 * 
 * \param entry the entry to search for
 */
int SearchTable(uint8_t *entry);

/**
 * \brief Registers the provided function as callback function for handling queued incoming packets, and
 *        begins queueing incoming packets
 * 
 * \param control_cb Pointer to the desired callback for control plane messages, which should have the form:
 *   *       - uint8_t (*CallbackFunction) (uint8_t *raw_pack, uint32_t src, uint32_t dest, uint8_t *payload, uint32_t payload_length);
 *           - function should return PACKET_ACCEPT or PACKET_DROP to indicate verdict
  * \param data_cb Pointer to the desired callback for data plane messages, which should have the form:
 *   *       - uint8_t (*CallbackFunction) (uint8_t *raw_pack, uint32_t src, uint32_t dest, uint8_t *payload, uint32_t payload_length);
 *           - function should return PACKET_ACCEPT or PACKET_DROP to indicate verdict
 * \return 0 for success, -1 for failure
 */
uint32_t RegisterIncomingCallback(CallbackFunction control_cb, CallbackFunction data_cb);

/**
 * \brief Registers the provided function as callback function for handling queued outgoing packets, and
 *        begins queueing outgoing packets
 * 
 * \param cb Pointer to the desired callback function, which should have the form:
 *   *       - uint8_t (*CallbackFunction) (uint8_t *raw_pack, uint32_t src, uint32_t dest, uint8_t *payload, uint32_t payload_length);
 *           - function should return PACKET_ACCEPT or PACKET_DROP to indicate verdict
 * 
 * \return 0 for success, -1 for failure
 */
uint32_t RegisterOutgoingCallback(CallbackFunction cb);

/**
 * \brief Registers the provided function as callback function for handling queued forwarded packets, and
 *        begins queueing forwarded packets
 * 
 * \param cb Pointer to the desired callback function, which should have the form:
 *   *       - uint8_t (*CallbackFunction) (uint8_t *raw_pack, uint32_t src, uint32_t dest, uint8_t *payload, uint32_t payload_length);
 *           - function should return PACKET_ACCEPT or PACKET_DROP to indicate verdict
 * 
 * \return 0 for success, -1 for failure
 */
uint32_t RegisterForwardCallback(CallbackFunction cb);

#endif

