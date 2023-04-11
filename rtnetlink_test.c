#include <linux/rtnetlink.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <asm/types.h>

int main(int argc, char* argv[]){


    int nlskt = socket(AF_NETLINK,SOCK_RAW,NETLINK_ROUTE);
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
        char attr[1024];
   }packet;
    // Going to attempt to request IP Address of Wlan0
    // Will require both receiving and requesting
    // Should probably check the necessity of the msg structure

    return 0;
}

