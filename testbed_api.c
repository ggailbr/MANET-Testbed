/*
Andre Koka - Created 5/31/2023
             Last Updated: 7/7/2023

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
struct { // buffer to hold rtnetlink request
  struct nlmsghdr nl;
  struct rtmsg    rt;
  char            buf[8192];
} request;

// for socket communications
int fd;
struct sockaddr_nl local_addr;
struct sockaddr_nl pa;
struct msghdr msg;
struct iovec iov;
int rtn;

// buffer to hold the RTNETLINK reply(s)
char buf[8192];

// RTNETLINK message ptrs and lengthss for processing messages
struct nlmsghdr *nl_ptr;
int nl_len;
struct rtmsg *rt_ptr;
int rt_len;
struct rtattr *rt_attr_ptr;

// Function Headers
int add_route(int table, char * src_addr, char * dest_addr, int duration, int  flags);
int del_route();
int open_nl_socket();
void form_add_request(char * src, char * dest, __u16 flags);
void form_get_request();
void send_request();
void recv_reply();
void read_reply();

// -------------------- Main ----------------//
int main (int argc, char *argv[])
{
    char dest[24] = "192.168.1.11"; // dummy ip
    char src[24] = "192.168.1.8"; // needs to be local machine's ipv4 addr
    open_nl_socket();

    add_route(0, src, dest, 0, NLM_F_REQUEST | NLM_F_CREATE | NLM_F_ACK);

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
    // create rtnetlink message, send over socket, receive reply & process message
    form_add_request(src_addr, dest_addr, flags);
    send_request();
    //recv_reply(); // currently only needed when getting routing table
    //read_reply(); // currently only needed when getting routing table
    return 0;
}

int del_route(int table, char * dest_addr)
{
    // utilize same form_add_request?
    return 0;
}

// int send_broadcast_message(message * msg)
// {

// }

// int send_unicast_message(message * msg, __u32 src_addr)
// {

// }

// int send_multicast_message(message * msg, __u32 src_addr) 
// {

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
  bzero(&request, sizeof(request));

  // compute the initial length of the RTNETLINK request
  rt_len = sizeof(struct rtmsg);

  // add first attrib:
  // set destination IP addr and increment the RTNETLINK buffer size
  rt_attr_ptr = (struct rtattr *) request.buf;
  rt_attr_ptr->rta_type = RTA_DST; // rtnetlink destination
  rt_attr_ptr->rta_len = sizeof(struct rtattr) + 4; // increment buffer
  inet_pton(AF_INET, dest,
     ((char *)rt_attr_ptr) + sizeof(struct rtattr));
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
  request.nl.nlmsg_len = NLMSG_LENGTH(rt_len);
  //request.nl.nlmsg_flags = NLM_F_REQUEST | NLM_F_CREATE | NLM_F_ACK;
  request.nl.nlmsg_flags = flags;
  request.nl.nlmsg_type = RTM_NEWROUTE;

  // setup the service header (struct rtmsg)
  request.rt.rtm_family = AF_INET; // IP protocol
  request.rt.rtm_table = RT_TABLE_MAIN; // editing main routing table
  request.rt.rtm_protocol = RTPROT_STATIC; // ?
  request.rt.rtm_scope = RT_SCOPE_LINK; // ?
  request.rt.rtm_type = RTN_UNICAST; // new route should be unicast
  // set the network prefix size
  request.rt.rtm_dst_len = pn; // ?
}

void send_request() // sends the current request through socket fd (global)
{
  // create the remote address to communicate
  bzero(&pa, sizeof(pa));
  pa.nl_family = AF_NETLINK;

  // initialize & create the struct msghdr for the sendmsg() function
  bzero(&msg, sizeof(msg));
  msg.msg_name = (void *) &pa;
  msg.msg_namelen = sizeof(pa);

  // place the pointer & size of the RTNETLINK
  // message in the struct msghdr
  iov.iov_base = (void *) &request.nl;
  iov.iov_len = request.nl.nlmsg_len;
  msg.msg_iov = &iov;
  msg.msg_iovlen = 1;

  // send the RTNETLINK message to kernel (using socket API)
  rtn = sendmsg(fd, &msg, 0);
}

// -------------- UNUSED FUNCTIONS --------------//
void form_get_request() // forms netlink message to request routing table
{
  // initialize the request buffer (which is a global)
  bzero(&request, sizeof(request));

  // set the NETLINK header
  request.nl.nlmsg_len = NLMSG_LENGTH(sizeof(struct rtmsg));
  request.nl.nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP;
  request.nl.nlmsg_type = RTM_GETROUTE;

  // set the routing message header
  request.rt.rtm_family = AF_INET;
  request.rt.rtm_table = RT_TABLE_MAIN;
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
        {printf("NL socket returned an error\n"); exit(0); }

    // increment the buffer pointer to place next message
    p += rtn;

    // increment the total size by the size of
    // the last received message
    nl_len += rtn;

    if((local_addr.nl_groups & RTMGRP_IPV4_ROUTE) == RTMGRP_IPV4_ROUTE) // ?
      break;
  }
}
void read_reply() // reads routing table reply (which may be in multiple messages)
{
  // string to hold content of the route
  // table (i.e. one entry)
  char dsts[24], gws[24], ifs[16], ms[24];

  // outer loop: loops thru all the NETLINK
  // headers that also include the route entry header
  nl_ptr = (struct nlmsghdr *) buf;
  for( ; NLMSG_OK(nl_ptr, nl_len); nl_ptr=NLMSG_NEXT(nl_ptr, nl_len))
  {
    // get route entry header
    rt_ptr = (struct rtmsg *) NLMSG_DATA(nl_ptr);

    // for now only concerned about the main route table
    if(rt_ptr->rtm_table != RT_TABLE_MAIN)
      continue;

    // init all the strings
    bzero(dsts, sizeof(dsts));
    bzero(gws, sizeof(gws));
    bzero(ifs, sizeof(ifs));
    bzero(ms, sizeof(ms));

    // inner loop: loop thru all the attributes of
    // one route entry
    rt_attr_ptr = (struct rtattr *) RTM_RTA(rt_ptr);
    rt_len = RTM_PAYLOAD(nl_ptr);
    for( ; RTA_OK(rt_attr_ptr, rt_len); rt_attr_ptr=RTA_NEXT(rt_attr_ptr, rt_len))
    {
      switch(rt_attr_ptr->rta_type)
      {
        // destination IPv4 address
        case RTA_DST:
          inet_ntop(AF_INET, RTA_DATA(rt_attr_ptr),
                                     dsts, 24);
          break;

        // next hop IPv4 address
        case RTA_GATEWAY:
          inet_ntop(AF_INET, RTA_DATA(rt_attr_ptr),
                                     gws, 24);
          break;

        // unique ID associated with the network interface
        case RTA_OIF:
          sprintf(ifs, "%d",
                   *((int *) RTA_DATA(rt_attr_ptr)));
        default:
          break;
      }
    }
    sprintf(ms, "%d", rt_ptr->rtm_dst_len);

    printf("dst %s/%s gw %s if %s\n",
                          dsts, ms, gws, ifs);
  }
}
int rtattr_add(struct nlmsghdr *n, int maxlen, int type, const void *data, int alen) //Add a new rt attribute to the nlmsghdr n
{
    /* int len = RTA_LENGTH(alen);
    struct rtattr *rta;

    if (NLMSG_ALIGN(n->nlmsg_len) + RTA_ALIGN(len) > maxlen) {
        fprintf(stderr, "rtattr_add error: message exceeded bound of %d\n", maxlen);
        return -1;
    }

    rta = NLMSG_TAIL(n);
    rta->rta_type = type;
    rta->rta_len = len; 

    if (alen) {
        memcpy(RTA_DATA(rta), data, alen);
    }

    n->nlmsg_len = NLMSG_ALIGN(n->nlmsg_len) + RTA_ALIGN(len);
    */
    return 0;
}