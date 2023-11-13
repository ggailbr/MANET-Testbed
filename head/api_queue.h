#ifndef API_QUEUE_H
#define API_QUEUE_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/socket.h>         // linux socket API
#include <linux/netlink.h>      // netlink allows kernel<->userspace communications
#include <pthread.h>			// API should be thread-safe
#include <linux/netfilter.h>
#include <libnetfilter_queue/libnetfilter_queue.h>
#include <linux/ip.h> // for IP header

#define QUEUE_LEN 100000
// define queue numbers

typedef uint8_t (*CallbackFunction) (uint8_t *raw_pack, uint32_t src, uint32_t dest, uint8_t *payload, uint32_t payload_length); 

// store registered callback functions from user
CallbackFunction incoming_control;
CallbackFunction incoming_data;
CallbackFunction outgoing;
CallbackFunction forwarded;

pthread_t in_thread_control; // to pull from incoming control plane queue
pthread_t in_thread_data; // to pull from incoming data plane queue
pthread_t out_thread; // to pull from outgoing queue
pthread_t forward_thread; // to pull from forward queue

// size of ipv4 pseudoheader + udp header
#define IP_UDP_HDR_OFFSET 28 

/**
 * \brief Initializes functions related to NFQUEUE. Also sets up iptables rules to enable
 * ipv4 forwarding and to disable ipv6
 * 
 * \return 0 for success, -1 for failure
*/
int InitializeQueue();

/**
 * \brief Helper function to handle queued incoming control plane packets. Called by nfq_handle_packet and used
 * to call the user's incoming packet function with packet information. Function structure determined by
 * libnetfilter_queue standards for nfq_handle_packet(). Once registered as a callback function using 
 * nfq_create_queue, this function is never directly called in the API.
 * 
 * \param qh Queue handle for the queue that calls this function (passed automatically by nfq_handle_packet)
 * \param nfmsg Pointer to a netfilter msg header (passed automatically by nfq_handle_packet)
 * \param nfa Pointer to the data of the netfilter message, which is the packet (passed automatically by nfq_handle_packet)
 * \param data Custom data passed by nfq_handle_packet (UNUSED)
 * 
 * \return 0 for success, -1 for failure
*/
int handle_incoming_control(struct nfq_q_handle *qh, struct nfgenmsg *nfmsg, struct nfq_data *nfa, void *data);

/**
 * \brief Helper function to handle queued incoming data plane packets. Called by nfq_handle_packet and used
 * to call the user's incoming packet function with packet information. Function structure determined by
 * libnetfilter_queue standards for nfq_handle_packet(). Once registered as a callback function using 
 * nfq_create_queue, this function is never directly called in the API.
 * 
 * \param qh Queue handle for the queue that calls this function (passed automatically by nfq_handle_packet)
 * \param nfmsg Pointer to a netfilter msg header (passed automatically by nfq_handle_packet)
 * \param nfa Pointer to the data of the netfilter message, which is the packet (passed automatically by nfq_handle_packet)
 * \param data Custom data passed by nfq_handle_packet (UNUSED)
 * 
 * \return 0 for success, -1 for failure
*/
int handle_incoming_data(struct nfq_q_handle *qh, struct nfgenmsg *nfmsg, struct nfq_data *nfa, void *data);


/**
 * \brief Helper function to handle queued outgoing packets. Called by nfq_handle_packet and used
 * to call the user's incoming packet function with packet information. Function structure determined by
 * libnetfilter_queue standards for nfq_handle_packet(). Once registered as a callback function using 
 * nfq_create_queue, this function is never directly called in the API.
 * 
 * \param qh Queue handle for the queue that calls this function (passed automatically by nfq_handle_packet)
 * \param nfmsg Pointer to a netfilter msg header (passed automatically by nfq_handle_packet)
 * \param nfa Pointer to the data of the netfilter message, which is the packet (passed automatically by nfq_handle_packet)
 * \param data Custom data passed by nfq_handle_packet (UNUSED)
 * 
 * \return 0 for success, -1 for failure
*/
int handle_outgoing(struct nfq_q_handle *qh, struct nfgenmsg *nfmsg, struct nfq_data *nfa, void *data);

/**
 * \brief Helper function to handle queued forwarded packets. Called by nfq_handle_packet and used
 * to call the user's incoming packet function with packet information. Function structure determined by
 * libnetfilter_queue standards for nfq_handle_packet(). Once registered as a callback function using 
 * nfq_create_queue, this function is never directly called in the API.
 * 
 * \param qh Queue handle for the queue that calls this function (passed automatically by nfq_handle_packet)
 * \param nfmsg Pointer to a netfilter msg header (passed automatically by nfq_handle_packet)
 * \param nfa Pointer to the data of the netfilter message, which is the packet (passed automatically by nfq_handle_packet)
 * \param data Custom data passed by nfq_handle_packet (UNUSED)
 * 
 * \return 0 for success, -1 for failure
*/
int handle_forwarded(struct nfq_q_handle *qh, struct nfgenmsg *nfmsg, struct nfq_data *nfa, void *data);

/**
 * \brief Helper function to pull packets from the incoming control plane queue. It is used as the start function for the 
 * pthread_t thread that pulls from the queue of incoming control plane packets
 * 
*/
void *thread_func_in_control();


/**
 * \brief Helper function to pull packets from the incoming data plane queue. It is used as the start function for the 
 * pthread_t thread that pulls from the queue of incoming data plane packets
 * 
*/
void *thread_func_in_data();

/**
 * \brief Helper function to pull packets from the outgoing queue. It is used as the start function for the 
 * pthread_t thread that pulls from the queue of outgoing packets
 * 
*/
void *thread_func_out();

/**
 * \brief Helper function to pull packets from the forward queue. It is used as the start function for the 
 * pthread_t thread that pulls from the queue of forwarded packets
 * 
*/
void *thread_func_for();

#endif