#ifndef API_H
#define API_H
#include <stdint.h>

/**
 * \brief Adds a unicast route from `src_address` to `dest_address` from the current
 * table selected
 * 
 * \param src_address The source address of the route
 * \param dest_address The destination address of the route
 * 
 * \return The index of the routing entry added (-1 for failure)
 * 
*/
int AddUnicastRoutingEntry(uint8_t *src_address, uint8_t *dest_address);
/**
 * \brief Deletes a route in the current table. If src_address and dest_address are supplied, 
 * will delete the route based on those options. Otherwise, an index can be provided
 * instead to delete the route
 * 
 * \param src_address The source address of the route
 * \param dest_address The destination address of the route
 * \param index The index of the route to delete
 * 
 * \return If the process was a success (-1 for failure)
 * 
*/
int DeleteEntry(uint8_t *src_address, uint8_t *dest_address, int index);

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
int SendUnicast(uint8_t *dest_address, uint8_t *message_buffer, uint8_t *header);

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

#endif