#include <linux/rtnetlink.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <asm/types.h>
#include <unistd.h>
#include <net/if.h> /* For index to dev name and back*/
#include <errno.h>
#include <sys/uio.h>

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
        - Add to the nlmsghdr to get where teh attribute should go
    RTA_DATA(*attrheader) - This calculates the offset from the attribute header to start the data
    RTA_LENGTH(len) - Calculates the total length including the header bytes. 
        - Typically used to update the nlmsghdr length after adding attributes
    */
   struct {
        struct nlmsghdr nlh;
        struct ifaddrmsg addr;
   }packet;
    // Going to attempt to request IP Address of Wlan0
    // Will require both receiving and requesting
    struct sockaddr_nl nladdr = {
        .nl_family = AF_INET,
        .nl_pad = 0,
        .nl_pid = getpid(),
        .nl_groups = 0};
    struct iovec iov;
    struct msghdr msg= {
        .msg_name = &nladdr,
        .msg_namelen = sizeof(nladdr),
        .msg_iov = &iov,
        .msg_iovlen = 1,
        .msg__control = NULL,
        .msg_controllen = 0,
        .msg_flags = 0};
    int fd, success, sequence_number = 1;

    fd = socket(AF_NETLINK,SOCK_RAW,NETLINK_ROUTE);
    if((success =  bind(fd, (struct sockaddr *) &nladdr, sizeof(nladdr))) < 0){
        printf("Error Binding to Socket\n");
        return errno;
    }
    nladdr.nl_pid = 0;
    
    packet.nlh.nlmsg_type = RTM_GETADDR;
    packet.nlh.nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP;
    packet.nlh.nlmsg_len = NLMSG_LENGTH(sizeof(struct ifaddrmsg));
    packet.nlh.nlmsg_pid = 0;
    packet.nlh.nlmsg_seq = sequence_number++;
    iov.iov_base = (void *)&packet;
    iov.iov_len = packet.nlh.nlmsg_len;
    sendmsg(fd,&msg,0);
    printf("Sent Message");



    return 0;
}

