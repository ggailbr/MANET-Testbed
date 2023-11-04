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

/**
 * \brief Initializes functions related to sending UDP messages. Also runs bash
 * script to set up iptables rules.
 * 
 * \return 0 for success, -1 for failure
*/
int InitializeSend();
#endif

