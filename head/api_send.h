#ifndef API_SEND_H
#define API_SEND_H

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/socket.h>         // linux socket API
#include <arpa/inet.h>          // for converting ip addresses to binary
#include <net/if.h>             // for converting network interface names to binary
#include <pthread.h>			

int sock; // UDP socket for communcations between nodes

/**
 * \brief Initializes functions related to sending UDP messages by opening a 
 * UDP Datagram socket for API communications between nodes
 * 
 * \return 0 for success, -1 for failure
*/
int InitializeSend();

/**
 * \brief Formats and sends a message on socket sock, either broadcast or unicast. Only supports
 * sending broadcast within the network (192.168.1.X)
 * 
 * \param dest_address The destination ipv4 address
 * \param msg_buf Buffer containing the contents of the message to send
 * \param header Parameter to provide a custom header to the UDP packet (UNUSED)
 * \param type Indicates if the message is broadcast(1) or unicast(0)
 * \param size Size of the message to be sent
 * 
 * \return number of bytes sent, or -1 for failure
*/
int send_sock_msg(uint32_t dest_address, uint8_t *msg_buf, uint8_t *header, int type, uint32_t size);
#endif

