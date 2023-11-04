/*
Andre Koka - Created 11/4/2023
             Last Updated: 11/4/2023

The basic API file for the MANET Testbed - to implement:
- all NFQUEUE related functions
*/

#include "api.h"
#include "api_queue.h"
#include <linux/ip.h> // for IP header
#include <linux/udp.h> // for UDP header
#include <net/ethernet.h> // for ethernet header
#include "../manet_testbed.h"

int InitializeQueue()
{
	incoming = outgoing = forwarded = NULL;
	int r = system("sudo ./OtherAPI/api_shell.sh");
	return (r < 0) ? -1 : 0;
}

// raw packet, src, dest, payload, length of payload
int handle_incoming(struct nfq_q_handle *qh, struct nfgenmsg *nfmsg, struct nfq_data *nfa, void *data)
{
    printf("entering callback: incoming\n");   

	uint32_t id = -1; // id of packet in the queue
    int p_length = 0; // length of entire packet including headers
    uint8_t *p_data; // payload of packet, including headers

    uint32_t src = 0; // src ip addr
    uint32_t dest = 0; // dest ip addr   

    // get packet sender, destination, payload, and payload_length
    struct nfqnl_msg_packet_hdr *p_header = nfq_get_msg_packet_hdr(nfa);
    if(p_header)
        id = ntohl(p_header->packet_id);
    p_length = nfq_get_payload(nfa, &p_data);

    unsigned short iphdrlen;
	struct iphdr *iph = ((struct iphdr *) data);
	iphdrlen = iph->ihl * 4;
    src = iph->saddr; // get packet sender
    dest = iph->daddr; // get packet destination

    printf("the protocol is %d\n", iph->protocol); // protocol check
	printf("p_data:%p\tsrc:%X\tdest:%X\tp_data+16:%p\tpayload len:%d\n", 
		p_data, src, dest, p_data+16, p_length);

	// call user function
 	uint32_t ret = (*incoming)(p_data, src, dest, p_data+16, p_length);

	// set verdict
	if (ret == 0)
		return nfq_set_verdict(qh, id, NF_DROP, 0, NULL);
	else
		return nfq_set_verdict(qh, id, NF_ACCEPT, 0, NULL);
}

int handle_outgoing(struct nfq_q_handle *qh, struct nfgenmsg *nfmsg, struct nfq_data *nfa, void *data)
{
    printf("entering callback: outgoing\n");   

	uint32_t id = -1; // id of packet in the queue
    int p_length = 0; // length of entire packet including headers
    uint8_t *p_data; // payload of packet, including headers

    uint32_t src = 0; // src ip addr
    uint32_t dest = 0; // dest ip addr   

    // get packet sender, destination, payload, and payload_length
    struct nfqnl_msg_packet_hdr *p_header = nfq_get_msg_packet_hdr(nfa);
    if(p_header)
        id = ntohl(p_header->packet_id);
    p_length = nfq_get_payload(nfa, &p_data);

    unsigned short iphdrlen;
	struct iphdr *iph = ((struct iphdr *) data);
	iphdrlen = iph->ihl * 4;
    src = iph->saddr; // get packet sender
    dest = iph->daddr; // get packet destination

    printf("the protocol is %d\n", iph->protocol); // protocol check
	printf("p_data:%p\tsrc:%X\tdest:%X\tp_data+16:%p\tpayload len:%d\n", 
		p_data, src, dest, p_data+16, p_length);

	// call user function
 	uint32_t ret = (*outgoing)(p_data, src, dest, p_data+16, p_length);

	// set verdict
	if (ret == 0)
		return nfq_set_verdict(qh, id, NF_DROP, 0, NULL);
	else
		return nfq_set_verdict(qh, id, NF_ACCEPT, 0, NULL);
}

int handle_forwarded(struct nfq_q_handle *qh, struct nfgenmsg *nfmsg, struct nfq_data *nfa, void *data)
{
    printf("entering callback: forwarded\n");   

	uint32_t id = -1; // id of packet in the queue
    int p_length = 0; // length of entire packet including headers
    uint8_t *p_data; // payload of packet, including headers
	uint8_t *p_payload; // payload of packet, no headers

    uint32_t src = 0; // src ip addr
    uint32_t dest = 0; // dest ip addr   

    // get packet sender, destination, payload, and payload_length
    struct nfqnl_msg_packet_hdr *p_header = nfq_get_msg_packet_hdr(nfa);
    if(p_header)
        id = ntohl(p_header->packet_id);
    p_length = nfq_get_payload(nfa, &p_data);
	p_payload = p_data + IP_UDP_HDR_OFFSET;

    unsigned short iphdrlen;
	struct iphdr *iph = ((struct iphdr *) data);
	iphdrlen = iph->ihl * 4;
    src = iph->saddr; // get packet sender
    dest = iph->daddr; // get packet destination

    printf("the protocol is %d\n", iph->protocol); // protocol check
	printf("p_data:%p\tsrc:%X\tdest:%X\tp_data+16:%p\tpayload len:%d\n", 
		p_data, src, dest, p_payload, p_length);

	// call user function
 	uint32_t ret = (*forwarded)(p_data, src, dest, p_payload, p_length);

	// set verdict
	if (ret == 0)
		return nfq_set_verdict(qh, id, NF_DROP, 0, NULL);
	else
		return nfq_set_verdict(qh, id, NF_ACCEPT, 0, NULL);
}

void *thread_func(uint8_t type) // function for thread to poll for incoming packets
{
	// setup queue
	struct nfq_handle *h;
	struct nfq_q_handle *qh;
	char buf[128000] __attribute__ ((aligned));
	int num_recv = 0;
	int thread_fd = 0;
	void* cb;
	
	switch(type) {
		case 0:
			cb = &handle_incoming;
		case 1:
			cb = &handle_outgoing;
		case 2:
			cb = &handle_forwarded;
		default:
			cb = NULL;
	}

	// open queue
	printf("open handle to the netfilter_queue - > queue %d \n", type);
	h = nfq_open();
	if (!h) {
		fprintf(stderr, "cannot open nfq_open()\n");
		return NULL;
	}

	//connect the thread for specific socket
	printf("binding this socket to queue %d\n", type);
	qh = nfq_create_queue(h, (int) type, cb, NULL);
	if (!qh) {
		fprintf(stderr, "error during nfq_create_queue()\n");
		return NULL;
	}

	//set queue length before start dropping packages
	uint32_t ql = nfq_set_queue_maxlen(qh, 100000);

	//set the queue for copy mode
	if (nfq_set_mode(qh, NFQNL_COPY_PACKET, 0xffff) < 0) {
		fprintf(stderr, "can't set packet_copy mode\n");
		return NULL;
	}

	//getting the file descriptor
	thread_fd = nfq_fd(h);

	while ((num_recv = recv(thread_fd, buf, sizeof(buf), 0)) && num_recv >= 0) {
		printf("packet received: %d \n", type);
		nfq_handle_packet(h, buf, num_recv);
	}

	printf("unbinding from queue %d  \n", type);
	nfq_destroy_queue(qh);

	printf("closing library handle\n");
	nfq_close(h);

	return NULL;
}

uint32_t RegisterIncomingCallback(CallbackFunction cb)
{
	// setup iptables rule
	system("sudo /sbin/iptables -A INPUT -p UDP --dport 269 -j NFQUEUE --queue-num 0"); // queue incoming udp

	int num = 0;
	void *type = &num;
	if(cb != NULL)
		incoming = cb;
	else
		return -1;
	int check = pthread_create(&in_thread, NULL, (void *)thread_func, type); // thread to poll queue 0
	if(check)
	{
		printf("error creating thread");
		return -1;
	}
	return 0;
}

uint32_t RegisterOutgoingCallback(CallbackFunction cb)
{
	// setup iptables rule
	system("sudo /sbin/iptables -A OUTPUT -p UDP --dport 269 -j NFQUEUE --queue-num 1"); // queue outgoing udp

	int num = 1;
	void *type = &num;
	if(cb != NULL)
		incoming = cb;
	else
		return -1;
	int check = pthread_create(&out_thread, NULL, (void *)thread_func, type); // thread to poll queue 1
	if(check)
	{
		printf("error creating thread");
		return -1;
	}
	return 0;
}

uint32_t RegisterForwardCallback(CallbackFunction cb)
{
	// setup iptables rule
	system("sudo /sbin/iptables -A FORWARD -p UDP --dport 269 -j NFQUEUE --queue-num 2"); // queue forwarded udp

	int num = 2;
	void *type = &num;
	if(cb != NULL)
		incoming = cb;
	else
		return -1;
	int check = pthread_create(&out_thread, NULL, (void *)thread_func, type); // thread to poll queue 2
	if(check)
	{
		printf("error creating thread");
		return -1;
	}
	return 0;
}