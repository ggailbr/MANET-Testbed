/*
Andre Koka - Created 5/31/2023
             Last Updated: 9/24/2023

The basic API for the MANET Testbed - to include/implement all basic API functions
- file structure of entire API subject to change (ie split into multiple files)
- this will likely be a library file in final form, for now contains main() for testing purposes

Currently:
- disregards gateways
- assumes one interface (wlan0 for now)
- assumes each node in Ad-hoc networks has a unique ID (aka static unique IP)
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

#define BUFSIZE 4096

#define for_each_nlmsg(n, buf, len)					\
	for (n = (struct nlmsghdr*)buf;					\
	     NLMSG_OK(n, (uint32_t)len) && n->nlmsg_type != NLMSG_DONE;	\
	     n = NLMSG_NEXT(n, len))

#define for_each_rattr(n, buf, len)					\
	for (n = (struct rtattr*)buf; RTA_OK(n, len); n = RTA_NEXT(n, len))

// ------------- Globals + Structures ------------------- //
struct { // buffer to hold formed rtnetlink request
  struct nlmsghdr nl;
  struct rtmsg    rt;
  char            buf[8192];
} rt_request;

struct { // buffer to hold formed ifaddrmsg request (for getting ip address of interface) 
  struct nlmsghdr   nl;
  struct ifaddrmsg  ip;
  char              buf[8192];
} ip_request;

// for socket communications
int fd; // global to hold socket
//struct sockaddr_nl sa; // local address to bind fd

// ----------------Variables for use with API (possibly edited by API user)-----------------
// interface - what network interface to use API with
// RTNETLINK message ptrs and lengths for processing messages
struct nlmsghdr *nl_ptr;
int nl_len;
struct rtmsg *rt_ptr;
int rt_len;
struct rtattr *rt_attr_ptr;
struct ipaddrmsg *ip_ptr;
int ip_len;


// void new routing table/ switch routing tables (seems to require opening and editing of /etc/iproute2/rt_tables)



// --------------- API Functions ------------------- //
static inline int check(int val)
{
	if (val < 0) {
		printf("check error: %s\n", strerror(errno));
		exit(1);
	}
  else 
    return val;
}
static inline char *ntop(int domain, void *buf)
{
	/*
	 * this function is not thread safe
	 */
	static char ip[INET6_ADDRSTRLEN];
	inet_ntop(domain, buf, ip, INET6_ADDRSTRLEN);
	return ip;
}

int open_nl_socket(struct sockaddr_nl *sa) //opens and binds netlink socket to current process
{
    fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
    if (fd < 0) {
        perror("Failed to open netlink socket");
        return -1;
    }

    bind(fd, (struct sockaddr*) sa, sizeof(*sa));
    return 0;
}


// int send_broadcast_message(message * msg)
// {
        // a dgram socket, needs broadcast permissions, or 
        // use local network broadcast address 255.255.255.255
// }

// standard port - 269 (according to RFC3561 (aodv) its port 654 - allow user to determine)
int send_unicast_message()
{
    // open dgram socket for udp
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
      if(sock < 0) { printf("error opening socket"); exit(0); }
    
    // bind socket
    struct sockaddr_in sock_addr;
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_port = htons(2000);
    sock_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    int b = bind(sock, (struct sockaddr *) &sock_addr, sizeof(sock_addr));
    if(b < 0) { printf("error binding socket"); exit(0); }

    // send with sendto
    char message_to_send[] = "test message";
    struct sockaddr_in client;
    int client_length = sizeof(client);
    client.sin_family = AF_INET;
    client.sin_port = htons(269);
    client.sin_addr.s_addr = inet_addr("192.168.1.8");
    int a = sendto(sock, message_to_send, strlen(message_to_send), 0, (struct sockaddr*) &client, (socklen_t) client_length);
      if(a < 0) { printf("error sending"); exit(0); }

    close(sock);
    return 0;
}

// standard dest addr - 224.0.0.109
// int send_multicast_message(message * msg, __u32 src_addr) 
// {
      // a dgram socket, needs to subscribe to multicast groups
// }

// //int register_function_callback(int)


void form_add_request(char * src, char * dest, __u16 flags) // form rtnetlink msg to add a route entry
{
  // attributes of the route entry (interface index (wlan0) and network prefix size)
  int interface = 3, pn = 32;

  // initialize RTNETLINK request buffer
  bzero(&rt_request, sizeof(rt_request));

  // compute the initial length of the RTNETLINK request
  rt_len = sizeof(struct rtmsg);

  // add first attrib:
  // set destination IP addr and increment the RTNETLINK buffer size
  rt_attr_ptr = (struct rtattr *) rt_request.buf;
  rt_attr_ptr->rta_type = RTA_DST; // rtnetlink destination
  rt_attr_ptr->rta_len = sizeof(struct rtattr) + 4; // increment buffer (addr is 4 bytes)
  inet_pton(AF_INET, dest,
     ((char *)rt_attr_ptr) + sizeof(struct rtattr)); // place dest_addr in buffer
  rt_len += rt_attr_ptr->rta_len;

  // add new attrib: source address?

  // add second attrib:
  // set interface index and increment the size
  rt_attr_ptr = (struct rtattr *) (((char *)rt_attr_ptr)
            + rt_attr_ptr->rta_len);
  rt_attr_ptr->rta_type = RTA_OIF; // indicates interface attribute
  rt_attr_ptr->rta_len = sizeof(struct rtattr) + 4;
  memcpy(((char *)rt_attr_ptr) + sizeof(struct rtattr),
           &interface, 4);
  rt_len += rt_attr_ptr->rta_len; // now rt_len is size of rt_msg_hdr + all attributes

  // setup the NETLINK header
  rt_request.nl.nlmsg_len = NLMSG_LENGTH(rt_len);
  //rt_request.nl.nlmsg_flags = NLM_F_REQUEST | NLM_F_CREATE | NLM_F_ACK;
  rt_request.nl.nlmsg_flags = flags; // function parameter
  rt_request.nl.nlmsg_type = RTM_NEWROUTE;

  // setup the service header (struct rtmsg)
  rt_request.rt.rtm_family = AF_INET; // IP protocol
  rt_request.rt.rtm_table = RT_TABLE_MAIN; // editing main routing table
  rt_request.rt.rtm_protocol = RTPROT_STATIC; // we can implement custom protocol definitions if required
  rt_request.rt.rtm_scope = RT_SCOPE_LINK; // ?
  rt_request.rt.rtm_type = RTN_UNICAST; // new route should be unicast
  // set the network prefix size
  rt_request.rt.rtm_dst_len = pn; // ?
}


int get_ip(struct sockaddr_nl *sa, int domain) //only ipv4 for now, using globals fd and sa
{
  char buf[BUFSIZE];
	memset(buf, 0, BUFSIZE);

	// assemble message (nlmsghdr + ifaddrmsg)
	struct nlmsghdr *nl;
	nl = (struct nlmsghdr*)buf;
	nl->nlmsg_len = NLMSG_LENGTH(sizeof(struct ifaddrmsg));
	nl->nlmsg_type = RTM_GETADDR;
	nl->nlmsg_flags = NLM_F_REQUEST;

	struct ifaddrmsg *ifa;
	ifa = (struct ifaddrmsg*)NLMSG_DATA(nl);
	ifa->ifa_family = domain; // we only get ipv4 address here

	// prepare struct msghdr for sending.
	struct iovec iov = { nl, nl->nlmsg_len };
	struct msghdr msg = { sa, sizeof(*sa), &iov, 1, NULL, 0, 0 };

	// send netlink message to kernel.
	int r = sendmsg(fd, &msg, 0);
	return (r < 0) ? -1 : 0;
}

static
int get_msg(struct sockaddr_nl *sa, void *buf, size_t len)
{
  struct iovec iov;
	struct msghdr msg;
	iov.iov_base = buf;
	iov.iov_len = len;

	memset(&msg, 0, sizeof(msg));
	msg.msg_name = sa;
	msg.msg_namelen = sizeof(*sa);
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;

	return recvmsg(fd, &msg, 0);
}

int parse_ifa_msg(struct ifaddrmsg *ifa, void *buf, size_t len)
{
	char ifname[IF_NAMESIZE];
	printf("==================================\n");
	printf("family:\t\t%d\n", (ifa->ifa_family == AF_INET) ? 4 :6);
	printf("dev:\t\t%s\n", if_indextoname(ifa->ifa_index, ifname));
	printf("prefix length:\t%d\n", ifa->ifa_prefixlen);
	printf("\n");

	struct rtattr *rta = NULL;
	int fa = ifa->ifa_family;
	for_each_rattr(rta, buf, len) {
		if (rta->rta_type == IFA_ADDRESS) {
			printf("if address:\t%s\n", ntop(fa, RTA_DATA(rta)));
		}

		if (rta->rta_type == IFA_LOCAL) {
			printf("local address:\t%s\n", ntop(fa, RTA_DATA(rta)));
		}

		if (rta->rta_type == IFA_BROADCAST) {
			printf("broadcast:\t%s\n", ntop(fa, RTA_DATA(rta)));
		}
	}

	return 0;
}

uint32_t parse_nl_msg(void *buf, size_t len)
{
	struct nlmsghdr *nl = NULL;
	for_each_nlmsg(nl, buf, len) {
		if (nl->nlmsg_type == NLMSG_ERROR) {
			printf("error");
			return -1;
		}

		if (nl->nlmsg_type == RTM_NEWADDR) {
			struct ifaddrmsg *ifa;
			ifa = (struct ifaddrmsg*)NLMSG_DATA(nl);
			parse_ifa_msg(ifa, IFA_RTA(ifa), IFA_PAYLOAD(nl));
			continue;
		}


	}
	return nl->nlmsg_type;
}


// -------------------- Main ----------------//
int main (int argc, char *argv[])
{
    char dest[24] = "192.168.1.11"; // dummy ip
    char src[24] = "192.168.1.8"; // needs to be local machine's ipv4 addr
    //open_nl_socket();

    int len = 0;
    struct sockaddr_nl sa;
	  memset(&sa, 0, sizeof(sa));
	  sa.nl_family = AF_NETLINK;
    open_nl_socket(&sa);
	  check(fd);

      printf("asdf");

    len = get_ip(&sa, AF_INET); // To get ipv6, use AF_INET6 instead
	  check(len);


    char buf[BUFSIZE];
	  uint32_t nl_msg_type;
	  do {
		    len = get_msg(&sa, buf, BUFSIZE);
		    check(len);
		    nl_msg_type = parse_nl_msg(buf, len);
	  } while (nl_msg_type != NLMSG_DONE && nl_msg_type != NLMSG_ERROR);

    //add_route(0, src, dest, 0, NLM_F_REQUEST | NLM_F_CREATE | NLM_F_ACK);
    //check(parse_ip());


    //del_route(0, dest);
    //send_unicast_message();

    close(fd);
    // memory leaks?
}