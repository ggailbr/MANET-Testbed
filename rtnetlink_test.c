#include <linux/rtnetlink.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <asm/types.h>
#include <unistd.h>
#include <net/if.h> /* For index to dev name and back*/
#include <errno.h>
#include <sys/uio.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

void print_msg_values(struct msghdr *);
void printft(char *string, int tabs);
void print_msghdr_flags(int flags, int tabs);
void print_addrfamily(unsigned short family, int tabs);

int main(int argc, char* argv[]){

    /*------GENERAL PACKET CREATION--------------------
    struct{
        nlmsgheader
        <packet_specific header>
        buffer for data/attributes
    }
    NLMSG_LENGTH(sizeof(packet_specific_header)) - This calculates the length of nlmsgheader+ type header
        -This is used for an offset for placing the attributes
    NLMSG_ALIGN(len) - THis will give you the number of bytes in the message currently
        - Add to the nlmsghdr to get where the attribute should go
    RTA_DATA(*attrheader) - This calculates the offset from the attribute header to start the data
    RTA_LENGTH(len) - Calculates the total length including the header bytes. 
        - Typically used to update the nlmsghdr length after adding attributes
    */

   /*------Segment out Packet Types to Functions-------------*/
   /* For example, make route_table message                  */
   struct {
        struct nlmsghdr nlh;
        struct ifaddrmsg addr;
   }packet;
   struct {
        struct nlmsghdr nlh;
        struct ifaddrmsg addr;
   } *received;
   /*---------------------------------------------------------*/


    /* This is the socket address to send and receive messages*/
    // Going to attempt to request IP Address of Wlan0
    // Will require both receiving and requesting
    struct sockaddr_nl nladdr = {
        .nl_family = AF_NETLINK,
        .nl_pad = 0,
        .nl_pid = getpid(),
        .nl_groups = 0};



    /* Segment out MSGHDR control to a function(Look at AODV-UU)*/
    struct iovec iov;
    struct msghdr msg= {
        .msg_name = &nladdr,
        .msg_namelen = sizeof(nladdr),
        .msg_iov = &iov,
        .msg_iovlen = 1,
        .msg_control = NULL,
        .msg_controllen = 0,
        .msg_flags = 0};
    /*----------------------------------------------------*/


    /*Will need a function to parse the attribute buffer after (again, AODV-UU)*/


    int fd, success, sequence_number = 1,len;
    unsigned char* buff;
    memset(&packet,0,sizeof(packet));
    if((fd = socket(AF_NETLINK,SOCK_RAW,NETLINK_ROUTE)) < 0){
        perror("Error Opening Socket");
        return errno;
    }
    if((success =  bind(fd, (struct sockaddr *) &nladdr, sizeof(nladdr))) < 0){
        perror("Error Binding to Socket");
        return errno;
    }

    // I shouldn't have overwritten and just should have made another address
    nladdr.nl_pid = 0;
    
    packet.nlh.nlmsg_type = RTM_GETADDR;
    packet.nlh.nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP;
    packet.nlh.nlmsg_len = NLMSG_LENGTH(sizeof(struct ifaddrmsg));
    packet.nlh.nlmsg_pid = 0;
    packet.nlh.nlmsg_seq = sequence_number++;
    packet.addr.ifa_family = AF_INET;
    iov.iov_base = (void *)&packet;
    iov.iov_len = packet.nlh.nlmsg_len;
    if(sendmsg(fd,&msg,0) < 0){
        perror("sendmsg(): ");
    }
    printf("Sent Message\n");

    /* When we implement a function, add error handling*/
    /* First need to peek at message, MSG_TRUNC does something special for
        netlink message where it will return the length of the full message*/
    len = recvmsg(fd,&msg,MSG_PEEK | MSG_TRUNC);
    //print_msg_values(&msg);
    buff = (unsigned char *) malloc(len);
    msg.msg_iov->iov_base = buff;
    msg.msg_iov->iov_len = len;
    recvmsg(fd,&msg,0);
    print_msg_values(&msg);
    received = msg.msg_iov->iov_base;
    printf("{nlmsghdr: \
        \n\tnlmsg_len: %d\
        \n\tnlmsg_type: %d\
        \n\tnlmsg_flags: %d\
        \n\tnlmsg_seq: %d\
        \n\tnlmsg_pid: %d\n}\n",received->nlh.nlmsg_len,received->nlh.nlmsg_type,received->nlh.nlmsg_flags,received->nlh.nlmsg_seq,received->nlh.nlmsg_pid);

    return 0;
}
void print_msg_values(struct msghdr *msg){
    struct sockaddr_nl sender;
    printf("{ struct msghdr:\n");
    printf("\tmsg_namelen: %d\n",msg->msg_namelen);
    printf("\tmsg_name: %p\n",msg->msg_name);
        if(msg->msg_namelen == sizeof(struct sockaddr_nl)){
            sender = *((struct sockaddr_nl*)msg->msg_name);
            printf("\t{sockaddr_nl:\n");
            printf("\t\tnl_family:\n");
            print_addrfamily(sender.nl_family,3);
            printf("\t\tnl_pad:%d\n",sender.nl_pad);
            printf("\t\tnl_pid:%d\n",sender.nl_pid);
            printf("\t\tnl_groups:0x%x\n",sender.nl_groups);
            printf("\t}\n");
        }
        else{
            printf("\t\tUnknown Address Format\n");
        }
    printf("\tmsg_iovlen: %d\n",msg->msg_iovlen);
    printf("\tmsg_iov: %p\n",msg->msg_iov);
        printf("\t{iovec:\n\t\tiov_base: %p\n\t\tiov_len: %d\n\t}\n",msg->msg_iov->iov_base,msg->msg_iov->iov_len);
    printf("\tmsg_control: %p\n",msg->msg_control);
    printf("\tmsg_controllen: %d\n",msg->msg_controllen);
    printf("\tmsg_flags: \n");
    print_msghdr_flags(msg->msg_flags, 2);
    printf("}\n");
}

// Prints the flags in a format for json
void print_msghdr_flags(int flags, int tabs){
    
    if((flags & MSG_OOB) != 0){
        printft("MSG_OOB (Process out-of-band data)\n",tabs);
    }
    if((flags & MSG_PEEK) != 0){
        printft("MSG_PEEK (Peek at incoming messages)\n",tabs);
    }
    if((flags & MSG_DONTROUTE) != 0){
        printft("MSG_DONTROUTE (Don't use local routing)\n",tabs);
    }
    if((flags & MSG_CTRUNC) != 0){
        printft("MSG_CTRUNC (Control data lost before delivery)\n",tabs);
    }
    if((flags & MSG_PROXY) != 0){
        printft("MSG_PROXY (Supply or ask second address)\n",tabs);
    }
    if((flags & MSG_TRUNC) != 0){
        printft("MSG_TRUNC (Message was truncated)\n",tabs);
    }
    if((flags & MSG_DONTWAIT) != 0){
        printft("MSG_DONTWAIT (Nonblocking IO)\n",tabs);
    }
    if((flags & MSG_EOR) != 0){
        printft("MSG_EOR (End of record)\n",tabs);
    }
    if((flags & MSG_WAITALL) != 0){
        printft("MSG_WAITALL (Wait for a full request)\n",tabs);
    }
    if((flags & MSG_FIN) != 0){
        printft("MSG_FIN (Unknown)\n",tabs);
    }
    if((flags & MSG_SYN) != 0){
        printft("MSG_SYN (Unknown)\n",tabs);
    }
    if((flags & MSG_CONFIRM) != 0){
        printft("MSG_CONFIRM (Confirm path validity)\n",tabs);
    }
    if((flags & MSG_RST) != 0){
        printft("MSG_RST (Unknwon)\n",tabs);
    }
    if((flags & MSG_ERRQUEUE) != 0){
        printft("MSG_ERRQUEUE (Fetch message from error queue)\n",tabs);
    }
    if((flags & MSG_NOSIGNAL) != 0){
        printft("MSG_NOSIGNAL (Do not generate SIGPIPE)\n",tabs);
    }
    if((flags & MSG_MORE) != 0){
        printft("MSG_MORE (Sender will send more)\n",tabs);
    }
    if((flags & MSG_WAITFORONE) != 0){
        printft("MSG_WAITFORONE (Wait for at least one packet to return)\n",tabs);
    }
    if((flags & MSG_BATCH) != 0){
        printft("MSG_BATCH (sendmmsg: more messages coming)\n",tabs);
    }
    if((flags & MSG_ZEROCOPY) != 0){
        printft("MSG_ZEROCOPY (Use user data in kernel path)\n",tabs);
    }
    if((flags & MSG_FASTOPEN) != 0){
        printft("MSG_FASTOPEN (Send data in TCP SYN)\n",tabs);
    }
    if((flags & MSG_CMSG_CLOEXEC) != 0){
        printft("MSG_CMSG_CLOEXEC (Set close_on_exit for file descriptor received through SCM_RIGHTS)\n",tabs);
    }
}
void printft(char *string, int tabs){
    for(int i = 0; i < tabs; i++){
        printf("\t");
    }
    printf(string);
}
void print_addrfamily(unsigned short family, int tabs){
    if(family == AF_UNSPEC){
        printft("AF_UNSPEC(Unspecified)\n",tabs);
    }
    if(family == AF_LOCAL){
        printft("AF_LOCAL(Local to host (pipes and file-domain))\n",tabs);
    }
    if(family == AF_UNIX){
        printft("AF_UNIX(POSIX name for PF_LOCAL)\n",tabs);
    }
    if(family == AF_FILE){
        printft("AF_FILE(Another non-standard name for PF_LOCAL)\n",tabs);
    }
    if(family == AF_INET){
        printft("AF_INET(IP protocol family)\n",tabs);
    }
    if(family == AF_AX25){
        printft("AF_AX25(Amateur Radio AX.25)\n",tabs);
    }
    if(family == AF_IPX){
        printft("AF_IPX(Novell Internet Protocol)\n",tabs);
    }
    if(family == AF_APPLETALK){
        printft("AF_APPLETALK(Appletalk DDP)\n",tabs);
    }
    if(family == AF_NETROM){
        printft("AF_NETROM(Amateur radio NetROM)\n",tabs);
    }
    if(family == AF_BRIDGE){
        printft("AF_BRIDGE(Multiprotocol bridge)\n",tabs);
    }
    if(family == AF_ATMPVC){
        printft("AF_ATMPVC(ATM PVCs)\n",tabs);
    }
    if(family == AF_X25){
        printft("AF_X25(Reserved for X.25 project)\n",tabs);
    }
    if(family == AF_INET6){
        printft("AF_INET6(IP version 6)\n",tabs);
    }
    if(family == AF_ROSE){
        printft("AF_ROSE(Amateur Radio X.25 PLP)\n",tabs);
    }
    if(family == AF_DECnet){
        printft("AF_DECnet(Reserved for DECnet project)\n",tabs);
    }
    if(family == AF_NETBEUI){
        printft("AF_NETBEUI(Reserved for 802.2LLC project)\n",tabs);
    }
    if(family == AF_SECURITY){
        printft("AF_SECURITY(Security callback pseudo AF)\n",tabs);
    }
    if(family == AF_KEY){
        printft("AF_KEY(PF_KEY key management API)\n",tabs);
    }
    if(family == AF_NETLINK){
        printft("AF_NETLINK(Netlink Interface)\n",tabs);
    }
    if(family == AF_ROUTE){
        printft("AF_ROUTE(Alias to emulate 4.4BSD)\n",tabs);
    }
    if(family == AF_PACKET){
        printft("AF_PACKET(Packet family)\n",tabs);
    }
    if(family == AF_ASH){
        printft("AF_ASH(Ash)\n",tabs);
    }
    if(family == AF_ECONET){
        printft("AF_ECONET(Acorn Econet)\n",tabs);
    }
    if(family == AF_ATMSVC){
        printft("AF_ATMSVC(ATM SVCs)\n",tabs);
    }
    if(family == AF_RDS){
        printft("AF_RDS(RDS sockets)\n",tabs);
    }
    if(family == AF_SNA){
        printft("AF_SNA(Linux SNA Project)\n",tabs);
    }
    if(family == AF_IRDA){
        printft("AF_IRDA(IRDA sockets)\n",tabs);
    }
    if(family == AF_PPPOX){
        printft("AF_PPPOX(PPPoX sockets)\n",tabs);
    }
    if(family == AF_WANPIPE){
        printft("AF_WANPIPE(Wanpipe API sockets)\n",tabs);
    }
    if(family == AF_LLC){
        printft("AF_LLC(Linux LLC)\n",tabs);
    }
    if(family == AF_IB){
        printft("AF_IB(Native InfiniBand address)\n",tabs);
    }
    if(family == AF_MPLS){
        printft("AF_MPLS(MPLS)\n",tabs);
    }
    if(family == AF_CAN){
        printft("AF_CAN(Controller Area Network)\n",tabs);
    }
    if(family == AF_TIPC){
        printft("AF_TIPC(TIPC sockets)\n",tabs);
    }
    if(family == AF_BLUETOOTH){
        printft("AF_BLUETOOTH(Bluetooth sockets)\n",tabs);
    }
    if(family == AF_IUCV){
        printft("AF_IUCV(IUCV sockets)\n",tabs);
    }
    if(family == AF_RXRPC){
        printft("AF_RXRPC(RxRPC sockets)\n",tabs);
    }
    if(family == AF_ISDN){
        printft("AF_ISDN(mISDN sockets)\n",tabs);
    }
    if(family == AF_PHONET){
        printft("AF_PHONET(Phonet sockets)\n",tabs);
    }
    if(family == AF_IEEE802154){
        printft("AF_IEEE802154(IEEE 802.15.4 sockets)\n",tabs);
    }
    if(family == AF_CAIF){
        printft("AF_CAIF(CAIF sockets)\n",tabs);
    }
    if(family == AF_ALG){
        printft("AF_ALG(Algorithm sockets)\n",tabs);
    }
    if(family == AF_NFC){
        printft("AF_NFC(NFC sockets)\n",tabs);
    }
    if(family == AF_VSOCK){
        printft("AF_VSOCK(vSockets)\n",tabs);
    }
    if(family == AF_KCM){
        printft("AF_KCM(Kernel Connection Multiplexor)\n",tabs);
    }
    if(family == AF_QIPCRTR){
        printft("AF_QIPCRTR(Qualcomm IPC Router)\n",tabs);
    }
    if(family == AF_SMC){
        printft("AF_SMC(SMC sockets)\n",tabs);
    }
    if(family == AF_MAX){
        printft("AF_MAX(Max number)\n",tabs);
    }
}