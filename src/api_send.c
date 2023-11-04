/*
Andre Koka - Created 10/8/2023
             Last Updated: 10/8/2023

The basic API file for the MANET Testbed - to implement:
- SendUnicast - send a unicast message using UDP socket
- SendBroadcast - broadcast a message using UDP socket

*/

#include "api.h"
#include "api_send.h"

int sock = 0;

int send_sock_msg(uint32_t dest_address, uint8_t *msg_buf, uint8_t *header, int type)
{
    if(!sock) // open dgram socket for udp, if there isn't one
	{
        sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		int broadcastEnable=1;
		int ret=setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable)); // grant broadcast socket permissions
		check(ret);
	}
    
	// initalize socket address and send
    struct sockaddr_in destination;
    memset(&destination, 0, sizeof(struct sockaddr_in));
    destination.sin_family = AF_INET;
	if(type) // sending a broadcast msg
    	inet_pton(AF_INET, "192.168.1.255", &(destination.sin_addr));
	else // sending a unicast msg
		inet_pton(AF_INET, ntop(AF_INET, &dest_address), &(destination.sin_addr));
    destination.sin_port = htons(269); // standard port for MANET comms

    int r = sendto(sock, msg_buf, strlen((char *)msg_buf), 0, (struct sockaddr*) &destination, sizeof(destination));
    if(r < 0 || f_err != 0)
		return -1;
    return r;
}

int SendUnicast(uint32_t dest_address, uint8_t *msg_buf, uint8_t *header)
{
	int r = send_sock_msg(dest_address, msg_buf, header, 0);
	return (r < 0) ? -1 : 0;
}

int SendBroadcast(uint8_t *msg_buf, uint8_t *header)
{
	int r = send_sock_msg(0, msg_buf, header, 1);
	return (r < 0) ? -1 : 0;
}

int InitializeSend()
{
	int r = 0;
	//system("/sbin/iptables -F"); // delete current iptables rules
	
	return (r < 0) ? -1 : 0;
}
