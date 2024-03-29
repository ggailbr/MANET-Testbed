/*
Andre Koka - Created 11/4/2023
             Last Updated: 11/7/2023

The basic API file for the MANET Testbed - to implement:
- RegisterIncomingCallback - queue incoming packets and handle with the given callback functions (control and data planes separated)
- RegisterOutgoingCallback - queue outgoing packets and handle with the given callback function
- RegisterForwardCallback - queue forwarded packets and handle with the given callback function
- InitializeQueue() - run iptables rules to enable ipv4 forwarding and disable ipv6
*/

#include "api.h"
#include "api_queue.h"

// ---------------------- HELPER FUNCTIONS ------------------

int handle_incoming_control(struct nfq_q_handle *qh, struct nfgenmsg *nfmsg, struct nfq_data *nfa, void *data)
{
    printf("entering callback: incoming (control)\n");   

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

	// remove ipv4+udp header and set length accordingly
    p_length = nfq_get_payload(nfa, &p_data) - IP_UDP_HDR_OFFSET;
	p_payload = p_data + IP_UDP_HDR_OFFSET;

	// process ip header to get src and dest addresses
    unsigned short iphdrlen;
	struct iphdr *iph = ((struct iphdr *) p_data);
	iphdrlen = iph->ihl * 4;
    src = iph->saddr; // get packet sender
    dest = iph->daddr; // get packet destination

	// prevent delivery of own broadcast messages to user-space
	if(dest == broadcast_ip && src == local_ip)
		return nfq_set_verdict(qh, id, NF_DROP, 0, NULL);

    printf("the protocol is %d\n", iph->protocol); // protocol check
	printf("p_data:%p\tsrc:%X\tdest:%X\tp_data+16:%p\tpayload len:%d\n", 
		p_data, src, dest, p_payload, p_length);

	// call user function
 	uint32_t ret = (*incoming_control)(p_data, src, dest, p_payload, p_length);

	// set verdict
	if (ret == 0)
		return nfq_set_verdict(qh, id, NF_DROP, 0, NULL);
	else
		return nfq_set_verdict(qh, id, NF_ACCEPT, 0, NULL);
}

int handle_incoming_data(struct nfq_q_handle *qh, struct nfgenmsg *nfmsg, struct nfq_data *nfa, void *data)
{
    printf("entering callback: incoming (data)\n");   

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

	// remove ipv4+udp header and set length accordingly
    p_length = nfq_get_payload(nfa, &p_data) - IP_UDP_HDR_OFFSET;
	p_payload = p_data + IP_UDP_HDR_OFFSET;

	// process ip header to get src and dest addresses
    unsigned short iphdrlen;
	struct iphdr *iph = ((struct iphdr *) p_data);
	iphdrlen = iph->ihl * 4;
    src = iph->saddr; // get packet sender
    dest = iph->daddr; // get packet destination

	// prevent delivery of own broadcast messages to user-space
	if(dest == broadcast_ip && src == local_ip)
		return nfq_set_verdict(qh, id, NF_DROP, 0, NULL);

    printf("the protocol is %d\n", iph->protocol); // protocol check
	printf("p_data:%p\tsrc:%X\tdest:%X\tp_data+16:%p\tpayload len:%d\n", 
		p_data, src, dest, p_payload, p_length);

	// call user function
 	uint32_t ret = (*incoming_data)(p_data, src, dest, p_payload, p_length);

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
	uint8_t *p_payload; // payload of packet, no headers

    uint32_t src = 0; // src ip addr
    uint32_t dest = 0; // dest ip addr

    // get packet sender, destination, payload, and payload_length
    struct nfqnl_msg_packet_hdr *p_header = nfq_get_msg_packet_hdr(nfa);
    if(p_header)
        id = ntohl(p_header->packet_id);

	// remove ipv4+udp header and set length accordingly
    p_length = nfq_get_payload(nfa, &p_data) - IP_UDP_HDR_OFFSET;
	p_payload = p_data + IP_UDP_HDR_OFFSET;

	// process ip header to get src and dest addresses
    unsigned short iphdrlen;
	struct iphdr *iph = ((struct iphdr *) p_data);
	iphdrlen = iph->ihl * 4;
    src = iph->saddr; // get packet sender
    dest = iph->daddr; // get packet destination

	// prevent delivery of own broadcast messages to user-space
	if(dest == broadcast_ip && src == local_ip)
		return nfq_set_verdict(qh, id, NF_DROP, 0, NULL);

    printf("the protocol is %d\n", iph->protocol); // protocol check
	printf("p_data:%p\tsrc:%X\tdest:%X\tp_data+16:%p\tpayload len:%d\n", 
		p_data, src, dest, p_payload, p_length);

	// call user function
 	uint32_t ret = (*outgoing)(p_data, src, dest, p_payload, p_length);

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
	
	// remove ipv4+udp header and set length accordingly
    p_length = nfq_get_payload(nfa, &p_data) - IP_UDP_HDR_OFFSET;
	p_payload = p_data + IP_UDP_HDR_OFFSET;

	// process ip header to get src and dest addresses
    unsigned short iphdrlen;
	struct iphdr *iph = ((struct iphdr *) p_data);
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

void *thread_func_in_control()
{
	struct nfq_handle *h;
	struct nfq_q_handle *qh;
	char buf[128000] __attribute__ ((aligned));
	int num_recv = 0;
	int thread_fd = 0;

	// open queue
	printf("open handle to the netfilter_queue - > queue 0 (incoming control)\n");
	h = nfq_open();
	if (!h) {
		fprintf(stderr, "cannot open nfq_open()\n");
		return NULL;
	}

	//connect the thread for specific socket
	printf("binding this socket to queue 0 (incoming control)\n");
	qh = nfq_create_queue(h, 0, &handle_incoming_control, NULL);
	if (!qh) {
		fprintf(stderr, "error during nfq_create_queue()\n");
		return NULL;
	}

	uint32_t ql = nfq_set_queue_maxlen(qh, QUEUE_LEN); // set queue length

	//set the queue for copy mode (copy whole packet)
	if (nfq_set_mode(qh, NFQNL_COPY_PACKET, 0xffff) < 0) {
		fprintf(stderr, "can't set packet_copy mode\n");
		return NULL;
	}

	thread_fd = nfq_fd(h); // get file descriptor for this socket
	while ((num_recv = recv(thread_fd, buf, sizeof(buf), 0)) && num_recv >= 0) {
		printf("incoming packet received from queue: queue 0\n");
		nfq_handle_packet(h, buf, num_recv); // callback functions activated here
	}

	printf("unbinding from queue 0\n");
	nfq_destroy_queue(qh);

	printf("closing library handle\n");
	nfq_close(h);

	return NULL;
}

void *thread_func_in_data()
{
	struct nfq_handle *h;
	struct nfq_q_handle *qh;
	char buf[128000] __attribute__ ((aligned));
	int num_recv = 0;
	int thread_fd = 0;

	// open queue
	printf("open handle to the netfilter_queue - > queue 4 (incoming data)\n");
	h = nfq_open();
	if (!h) {
		fprintf(stderr, "cannot open nfq_open()\n");
		return NULL;
	}

	//connect the thread for specific socket
	printf("binding this socket to queue 4 (incoming data)\n");
	qh = nfq_create_queue(h, 4, &handle_incoming_data, NULL);
	if (!qh) {
		fprintf(stderr, "error during nfq_create_queue (data)()\n");
		return NULL;
	}

	uint32_t ql = nfq_set_queue_maxlen(qh, QUEUE_LEN); // set queue length

	//set the queue for copy mode (copy whole packet)
	if (nfq_set_mode(qh, NFQNL_COPY_PACKET, 0xffff) < 0) {
		fprintf(stderr, "can't set packet_copy mode\n");
		return NULL;
	}

	thread_fd = nfq_fd(h); // get file descriptor for this socket
	while ((num_recv = recv(thread_fd, buf, sizeof(buf), 0)) && num_recv >= 0) {
		printf("incoming packet received from queue: queue 4\n");
		nfq_handle_packet(h, buf, num_recv); // callback functions activated here
	}

	printf("unbinding from queue 4\n");
	nfq_destroy_queue(qh);

	printf("closing library handle\n");
	nfq_close(h);

	return NULL;
}

void *thread_func_out()
{
	struct nfq_handle *h;
	struct nfq_q_handle *qh;
	char buf[128000] __attribute__ ((aligned));
	int num_recv = 0;
	int thread_fd = 0;

	// open queue
	printf("open handle to the netfilter_queue - > queue 1 (outgoing)\n");
	h = nfq_open();
	if (!h) {
		fprintf(stderr, "cannot open nfq_open()\n");
		return NULL;
	}

	//connect the thread for specific socket
	printf("binding this socket to queue 1 (outgoing)\n");
	qh = nfq_create_queue(h, 1, &handle_outgoing, NULL);
	if (!qh) {
		fprintf(stderr, "error during nfq_create_queue()\n");
		return NULL;
	}

	uint32_t ql = nfq_set_queue_maxlen(qh, QUEUE_LEN); // set queue length

	//set the queue for copy mode (copy whole packet)
	if (nfq_set_mode(qh, NFQNL_COPY_PACKET, 0xffff) < 0) {
		fprintf(stderr, "can't set packet_copy mode\n");
		return NULL;
	}

	
	thread_fd = nfq_fd(h); //get file descriptor for this socket
	while ((num_recv = recv(thread_fd, buf, sizeof(buf), 0)) && num_recv >= 0) {
		printf("outgoing packet received from queue: queue 1\n");
		nfq_handle_packet(h, buf, num_recv); // callback functions activated here
	}

	printf("unbinding from queue 1\n");
	nfq_destroy_queue(qh);

	printf("closing library handle\n");
	nfq_close(h);

	return NULL;
}

void *thread_func_for()
{
	struct nfq_handle *h;
	struct nfq_q_handle *qh;
	char buf[128000] __attribute__ ((aligned));
	int num_recv = 0;
	int thread_fd = 0;

	// open queue
	printf("open handle to the netfilter_queue - > queue 2 (forward)\n");
	h = nfq_open();
	if (!h) {
		fprintf(stderr, "cannot open nfq_open()\n");
		return NULL;
	}

	//connect the thread for specific socket
	printf("binding this socket to queue 2 (forward)\n");
	qh = nfq_create_queue(h, 2, &handle_forwarded, NULL);
	if (!qh) {
		fprintf(stderr, "error during nfq_create_queue()\n");
		return NULL;
	}

	uint32_t ql = nfq_set_queue_maxlen(qh, QUEUE_LEN);	//set queue length

	//set the queue for copy mode (copy whole packet)
	if (nfq_set_mode(qh, NFQNL_COPY_PACKET, 0xffff) < 0) {
		fprintf(stderr, "can't set packet_copy mode\n");
		return NULL;
	}

	thread_fd = nfq_fd(h); //get file descriptor for this socket
	while ((num_recv = recv(thread_fd, buf, sizeof(buf), 0)) && num_recv >= 0) {
		printf("forwarded packet received from queue: queue 2\n");
		nfq_handle_packet(h, buf, num_recv); // callback functions activated here
	}

	printf("unbinding from queue 2\n");
	nfq_destroy_queue(qh);

	printf("closing library handle\n");
	nfq_close(h);

	return NULL;
}

// ---------------------- API FUNCTIONS ------------------

uint32_t RegisterIncomingCallback(CallbackFunction control_cb, CallbackFunction data_cb)
{
	pthread_mutex_lock(&lock);

	// setup iptables rules (queue incoming control and data plane message separately)
	system("sudo /sbin/iptables -A INPUT -p UDP --dport 269 -j NFQUEUE --queue-num 0");
	system("sudo /sbin/iptables -A INPUT -m iprange --dst-range 192.168.1.1-192.168.1.100 -j NFQUEUE --queue-num 4");

	if(control_cb != NULL)
	{
		incoming_control = control_cb;
		if(pthread_create(&in_thread_control, NULL, (void *)thread_func_in_control, NULL))
		{
			printf("error creating incoming thread (data)\n");
			return -1;
		}
	}

	if(data_cb != NULL)
	{
		incoming_data = data_cb;
		if(pthread_create(&in_thread_data, NULL, (void *)thread_func_in_data, NULL))
		{
			printf("error creating incoming thread (data)\n");
			return -1;
		}
	}

	pthread_mutex_unlock(&lock);
	return 0;
}

uint32_t RegisterOutgoingCallback(CallbackFunction cb)
{
	pthread_mutex_lock(&lock);

	// setup iptables rules (queue outgoing data plane messages)
	system("sudo /sbin/iptables -I OUTPUT -p UDP --dport 269 -j ACCEPT");
	system("sudo /sbin/iptables -A OUTPUT -m iprange --dst-range 192.168.1.1-192.168.1.100 -j NFQUEUE --queue-num 1");

	if(cb != NULL)
		outgoing = cb;
	else
		return -1;
	if(pthread_create(&out_thread, NULL, (void *)thread_func_out, NULL)) // create thread for outgoing queue
	{
		printf("error creating outgoing thread\n");
		return -1;
	}

	pthread_mutex_unlock(&lock);
	return 0;
}

uint32_t RegisterForwardCallback(CallbackFunction cb)
{
	pthread_mutex_lock(&lock);

	// setup iptables rules (queue forwarded data plane messages)
	system("sudo /sbin/iptables -A FORWARD -p UDP --dport 269 -j DROP");
	system("sudo /sbin/iptables -A FORWARD -j NFQUEUE --queue-num 2");

	if(cb != NULL)
		forwarded = cb;
	else
		return -1;
	if(pthread_create(&out_thread, NULL, (void *)thread_func_for, NULL)) // create thread for forward queue
	{
		printf("error creating forward thread");
		return -1;
	}

	pthread_mutex_unlock(&lock);
	return 0;
}

int InitializeQueue()
{
	incoming_control = incoming_data = outgoing = forwarded = NULL;
	int r = system("/sbin/iptables -F"); // flush current iptables rules
	r = system("sh -c 'echo 1 > /proc/sys/net/ipv4/ip_forward'"); // enable ipv4 forwarding
	r = system("sh -c 'echo 1 > /proc/sys/net/ipv6/conf/wlan0/disable_ipv6'"); // disable ipv6 
	return (r < 0) ? -1 : 0;
}