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
int fd;
struct sockaddr_nl local_addr;
struct sockaddr_nl pa;
struct msghdr msg;
struct iovec iov;
int rtn;

// ----------------Variables for use with API (possibly edited by API user)-----------------
// interface - what network interface to use API with

// buffer to hold the RTNETLINK reply(s)
char buf[8192];

// RTNETLINK message ptrs and lengths for processing messages
struct nlmsghdr *nl_ptr;
int nl_len;
struct rtmsg *rt_ptr;
int rt_len;
struct rtattr *rt_attr_ptr;
struct ipaddrmsg *ip_ptr;
int ip_len;



// Function Headers
int add_route(int table, char * src_addr, char * dest_addr, int duration, int  flags);
int del_route(int table, char * dest_addr);
int del_route();
int open_nl_socket();
void form_add_request(char * src, char * dest, __u16 flags);
void form_del_request(char * dest);
void form_get_request();
void send_request(int a);
void recv_reply();
void read_ip_reply();
void get_my_ipv4();
// void new routing table/ switch routing tables (seems to require opening and editing of /etc/iproute2/rt_tables)

// -------------------- Main ----------------//
int main (int argc, char *argv[])
{
    char dest[24] = "192.168.1.11"; // dummy ip
    char src[24] = "192.168.1.8"; // needs to be local machine's ipv4 addr
    open_nl_socket();

    
    //add_route(0, src, dest, 0, NLM_F_REQUEST | NLM_F_CREATE | NLM_F_ACK);
    get_my_ipv4();

    //del_route(0, dest);
    //send_unicast_message();

    close(fd);
}

// --------------- API Functions ------------------- //
/*  Creates an entry in the routing table 
    table - the routing table to edit
    src_addr - the source address for this entry (string)
    dest_addr  - the destination address for this entry (string)
    duration - defines a number of hops until the route is timed out (implementation with a stack?)
    flags - defines route parameters (to be implemented)
*/
int add_route(int table, char * src_addr, char * dest_addr, int duration, int  flags)
{
    // create rtnetlink message, send over socket
    form_add_request(src_addr, dest_addr, flags);
    send_request(1);
    //recv_reply(); // currently only needed when getting routing table
    //read_reply(); // currently only needed when getting routing table
    return 0;
}
int del_route(int table, char * dest_addr)
{
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

//--------------------- HELPER FUNCTIONS ----------------------//
int open_nl_socket() //opens and binds netlink socket to current process
{
    fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
    if (fd < 0) {
        perror("Failed to open netlink socket");
        return -1;
    }

    bzero(&local_addr, sizeof(local_addr));
    local_addr.nl_family = AF_NETLINK;
    local_addr.nl_pid = getpid();
    bind(fd, (struct sockaddr*) &local_addr, sizeof(local_addr));
    return 0;
}

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

void form_del_request(char * dest) // form rtnetlink msg to delete a route entry
{
}

void send_request(int a) // sends the current request through socket fd (global)
{
  // create the remote address to communicate
  bzero(&pa, sizeof(pa));
  pa.nl_family = AF_NETLINK;

  // initialize & create the struct msghdr for the sendmsg() function
  bzero(&msg, sizeof(msg));
  msg.msg_name = (void *) &pa;
  msg.msg_namelen = sizeof(pa);

  // place the pointer & size of the message in msghdr
  if(a)
  { // rt message
    iov.iov_base = (void *) &rt_request.nl;
    iov.iov_len = rt_request.nl.nlmsg_len;
  }
  else
  { // ip message
    iov.iov_base = (void *) &ip_request.nl;
    iov.iov_len = ip_request.nl.nlmsg_len;
  }
  msg.msg_iov = &iov;
  msg.msg_iovlen = 1;

  // send the RTNETLINK message to kernel (using socket API)
  rtn = sendmsg(fd, &msg, 0);
}

void get_my_ipv4()
{
  // attributes of the route entry (interface index (wlan0) and network prefix size)
  int interface = 3, pn = 16;

  // initialize RTNETLINK ip request buffer
  bzero(&ip_request, sizeof(ip_request));
  // compute the initial length of the RTNETLINK ip request
  ip_len = sizeof(struct ifaddrmsg);

    // setup the NETLINK header
  ip_request.nl.nlmsg_len = NLMSG_LENGTH(ip_len);
  //request.nl.nlmsg_flags = NLM_F_REQUEST | NLM_F_CREATE | NLM_F_ACK;
  ip_request.nl.nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK; // function parameter
  ip_request.nl.nlmsg_type = RTM_GETADDR;

  // setup the service header (struct ifaddrmsg)
  ip_request.ip.ifa_family = AF_INET; // IPv4 protocol
  ip_request.ip.ifa_prefixlen = pn; // 
  ip_request.ip.ifa_flags = 0;
  ip_request.ip.ifa_scope = 0; // ?
  ip_request.ip.ifa_index = interface; // 

  printf("sending");
  send_request(0);
  recv_reply();
  read_ip_reply();

}

// -------------- UNUSED FUNCTIONS --------------//
void form_get_request() // forms netlink message to request routing table
{
  // initialize the request buffer (which is a global)
  bzero(&rt_request, sizeof(rt_request));

  // set the NETLINK header
  rt_request.nl.nlmsg_len = NLMSG_LENGTH(sizeof(struct rtmsg));
  rt_request.nl.nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP;
  rt_request.nl.nlmsg_type = RTM_GETROUTE;

  // set the routing message header
  rt_request.rt.rtm_family = AF_INET;
  rt_request.rt.rtm_table = RT_TABLE_MAIN;
}
void recv_reply() // receives netlink replies from kernel across socket fd
{
  char *p;

  // initialize the socket read buffer
  bzero(buf, sizeof(buf));

  p = buf;
  nl_len = 0;

  // read from the socket until the NLMSG_DONE is
  // returned in the type of the RTNETLINK message
  // or if it was a monitoring socket
  while(1) {
    rtn = recv(fd, p, sizeof(buf) - nl_len, 0);

    nl_ptr = (struct nlmsghdr *) p;

    if(nl_ptr->nlmsg_type == NLMSG_DONE)
        break;
    if(nl_ptr->nlmsg_type == NLMSG_ERROR)
    {
      struct nlmsgerr *err = (struct nlmsgerr*)NLMSG_DATA(buf);
        printf("%s\n", strerror(err->error)); 
        exit(0); 
    }

    // increment the buffer pointer to place next message
    p += rtn;

    // increment the total size by the size of
    // the last received message
    nl_len += rtn;

    if((local_addr.nl_groups & RTMGRP_IPV4_ROUTE) == RTMGRP_IPV4_ROUTE) // ?
      break;
  }
}
void read_ip_reply() // uhh
{
  // outer loop: loops thru all the NETLINK
  // headers that also include the route entry header
  nl_ptr = (struct nlmsghdr *) buf;
  printf("num: %d\n", NLMSG_OK(nl_ptr, nl_len));
}
int rtattr_add_addr(struct rtattr * rt_attr_ptr, int * rt_len, int type, int offset, char *addr) //Add a new rt attribute to the nlmsghdr n
{
  rt_attr_ptr->rta_type = type;
  rt_attr_ptr->rta_len = sizeof(struct rtattr) + offset;
  inet_pton(AF_INET, addr, ((char *)rt_attr_ptr) + sizeof(struct rtattr)); // place dest addr in buffer
  rt_len += rt_attr_ptr->rta_len;

  return 0;
}
