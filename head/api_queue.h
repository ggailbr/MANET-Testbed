#ifndef API_QUEUE_H
#define API_QUEUE_H

#include "../manet_testbed.h"

// store registered callback functions
CallbackFunction incoming;
CallbackFunction outgoing;
CallbackFunction forwarded;

pthread_t in_thread; // to pull from incoming queue
pthread_t out_thread; // to pull from outgoing queue
pthread_t forward_thread; // to pull from forward queue

#define IP_UDP_HDR_OFFSET 16

/**
 * \brief Initializes functions related to NFQUEUE.
 * 
 * \return 0 for success, -1 for failure
*/
int InitializeQueue();


/**
 * \brief Helper function to handle queued incoming packets
 * 
 * \return 0 for success, -1 for failure
*/
int handle_incoming(struct nfq_q_handle *qh, struct nfgenmsg *nfmsg, struct nfq_data *nfa, void *data);

/**
 * \brief Helper function to handle queued outgoing packets
 * 
 * \return 0 for success, -1 for failure
*/
int handle_outgoing(struct nfq_q_handle *qh, struct nfgenmsg *nfmsg, struct nfq_data *nfa, void *data);

/**
 * \brief Helper function to handle queued forwarded packets
 * 
 * \return 0 for success, -1 for failure
*/
int handle_forwarded(struct nfq_q_handle *qh, struct nfgenmsg *nfmsg, struct nfq_data *nfa, void *data);

#endif

