/*
Andre Koka - Created 5/31/2023
             Last Updated: 6/2/2023

             another blog code, for rtnetlink manipulation
rtnetlink2
*/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/socket.h>         // linux socket API
#include <linux/netlink.h>      // netlink specific socket definitions
#include <linux/rtnetlink.h>    // rtnetlink allows for modification of routing table
#include <arpa/inet.h>          // for converting ip addresses to binary
#include <net/if.h>             // for converting network interface names to binary
#include <bits/sockaddr.h>
#include <asm/types.h>

// ------------- Globals ------------------- //
// buffer to hold the RTNETLINK request
struct {
  struct nlmsghdr nl;
  struct rtmsg    rt;
  char            buf[8192];
} req;

// variables used for socket communications
int fd;
struct sockaddr_nl la;
struct sockaddr_nl pa;
struct msghdr msg;
struct iovec iov;
int rtn;

// buffer to hold the RTNETLINK reply(s)
char buf[8192];

// RTNETLINK message pointers & lengths, used when processing messages
struct nlmsghdr *nlp;
int nll;
struct rtmsg *rtp;
int rtl;
struct rtattr *rtap;

void send_request();
void form_get_request();
void form_add_request();
void recv_reply();
void read_reply();

int main(int argc, char *argv[])
{
    // open socket
    fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);

    // setup local address & bind using this address
    bzero(&la, sizeof(la));
    la.nl_family = AF_NETLINK;
    la.nl_pid = getpid();
    bind(fd, (struct sockaddr*) &la, sizeof(la));

    // sub functions to create RTNETLINK message, send over socket, receive reply & process message
    form_add_request();
    //printf("request formed\n");
    send_request();
    //printf("request sent\n");
    recv_reply();
    //printf("reply received\n"); 
    read_reply();
    //printf("reply read\n");

    // close socket
    close(fd);
}

void send_request()
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
  iov.iov_base = (void *) &req.nl;
  iov.iov_len = req.nl.nlmsg_len;
  msg.msg_iov = &iov;
  msg.msg_iovlen = 1;

  // send the RTNETLINK message to kernel (using socket API)
  rtn = sendmsg(fd, &msg, 0);
}

void recv_reply()
{
  char *p;

  // initialize the socket read buffer
  bzero(buf, sizeof(buf));

  p = buf;
  nll = 0;

  // read from the socket until the NLMSG_DONE is
  // returned in the type of the RTNETLINK message
  // or if it was a monitoring socket
  while(1) {
    rtn = recv(fd, p, sizeof(buf) - nll, 0);

    nlp = (struct nlmsghdr *) p;

    if(nlp->nlmsg_type == NLMSG_DONE)
        break;
    if(nlp->nlmsg_type == NLMSG_ERROR)
        {printf("NL socket returned an error\n"); exit(0); }

    // increment the buffer pointer to place next message
    p += rtn;

    // increment the total size by the size of
    // the last received message
    nll += rtn;

    if((la.nl_groups & RTMGRP_IPV4_ROUTE) == RTMGRP_IPV4_ROUTE)
      break;
  }
}

void form_get_request() // getting ipv4 routing table
{
  // initialize the request buffer
  bzero(&req, sizeof(req));

  // set the NETLINK header
  req.nl.nlmsg_len = NLMSG_LENGTH(sizeof(struct rtmsg));
  req.nl.nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP;
  req.nl.nlmsg_type = RTM_GETROUTE;

  // set the routing message header
  req.rt.rtm_family = AF_INET;
  req.rt.rtm_table = RT_TABLE_MAIN;
}

void form_add_request() // for adding an entry
{
  // attributes of the route entry
  char dsts[24] = "192.168.1.8";
  int ifcn = 2, pn = 32;

  // initialize RTNETLINK request buffer
  bzero(&req, sizeof(req));

  // compute the initial length of the service request
  rtl = sizeof(struct rtmsg);

  // add first attrib:
  // set destination IP addr and increment the RTNETLINK buffer size
  rtap = (struct rtattr *) req.buf;
  rtap->rta_type = RTA_DST;
  rtap->rta_len = sizeof(struct rtattr) + 4;
  inet_pton(AF_INET, dsts,
     ((char *)rtap) + sizeof(struct rtattr));
  rtl += rtap->rta_len;

  // add second attrib:
  // set ifc index and increment the size
  rtap = (struct rtattr *) (((char *)rtap)
            + rtap->rta_len);
  rtap->rta_type = RTA_OIF;
  rtap->rta_len = sizeof(struct rtattr) + 4;
  memcpy(((char *)rtap) + sizeof(struct rtattr),
           &ifcn, 4);
  rtl += rtap->rta_len;

  // setup the NETLINK header
  req.nl.nlmsg_len = NLMSG_LENGTH(rtl);
  req.nl.nlmsg_flags = NLM_F_REQUEST | NLM_F_CREATE | NLM_F_ACK;
  req.nl.nlmsg_type = RTM_NEWROUTE;

  // setup the service header (struct rtmsg)
  req.rt.rtm_family = AF_INET;
  req.rt.rtm_table = RT_TABLE_MAIN;
  req.rt.rtm_protocol = RTPROT_STATIC;
  req.rt.rtm_scope = RT_SCOPE_UNIVERSE;
  req.rt.rtm_type = RTN_UNICAST;
  // set the network prefix size
  req.rt.rtm_dst_len = pn;
}

void read_reply()
{
  // string to hold content of the route
  // table (i.e. one entry)
  char dsts[24], gws[24], ifs[16], ms[24];

  // outer loop: loops thru all the NETLINK
  // headers that also include the route entry
  // header
  nlp = (struct nlmsghdr *) buf;
  for(;NLMSG_OK(nlp, nll);nlp=NLMSG_NEXT(nlp, nll))
  {

    // get route entry header
    rtp = (struct rtmsg *) NLMSG_DATA(nlp);

    // we are only concerned about the
    // main route table
    if(rtp->rtm_table != RT_TABLE_MAIN)
      continue;

    // init all the strings
    bzero(dsts, sizeof(dsts));
    bzero(gws, sizeof(gws));
    bzero(ifs, sizeof(ifs));
    bzero(ms, sizeof(ms));

    // inner loop: loop thru all the attributes of
    // one route entry
    rtap = (struct rtattr *) RTM_RTA(rtp);
    rtl = RTM_PAYLOAD(nlp);
    for(;RTA_OK(rtap, rtl);rtap=RTA_NEXT(rtap,rtl))
    {
      switch(rtap->rta_type)
      {
        // destination IPv4 address
        case RTA_DST:
          inet_ntop(AF_INET, RTA_DATA(rtap),
                                     dsts, 24);
          break;

        // next hop IPv4 address
        case RTA_GATEWAY:
          inet_ntop(AF_INET, RTA_DATA(rtap),
                                     gws, 24);
          break;

        // unique ID associated with the network
        // interface
        case RTA_OIF:
          sprintf(ifs, "%d",
                   *((int *) RTA_DATA(rtap)));
        default:
          break;
      }
    }
    sprintf(ms, "%d", rtp->rtm_dst_len);

    printf("dst %s/%s gw %s if %s\n",
                          dsts, ms, gws, ifs);
  }
}
