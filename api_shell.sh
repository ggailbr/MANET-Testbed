#!/bin/sh

#Andre Koka - Created 10/16/2023
#             Last Updated: 10/16/2023

#Shell file to be run at API initialization for each node
# - sets up all appropriate iptables rules for Netfilter queueing
# a node needs to - queue all packets destined for itself
#                 - accept all other packets as normal
# https://linux.die.net/man/8/iptables

# xt_NFQUEUE
echo "Starting iptable rule setup"
/sbin/iptables –F # flush (delete) all current rules
#/sbin/iptables -I INPUT -s 192.168.1.7 -j DROP # drop all incoming packets from 192.168.1.7
#/sbin/iptables -t mangle -A PREROUTING -m mac --mac-source 48:5d:60:7e:bb:8f -j DROP


/sbin/iptables -A INPUT -p UDP --sport 269 --dport 269 -j NFQUEUE --queue-num 0 # queue all incoming UDP packets that are destined to and sent from port 269
/sbin/iptables -I OUTPUT -p UDP --sport 269 --dport 269 -j ACCEPT # accept all outgoing UDP packets that are destined to and sent from port 269

#/sbin/iptables -I OUTPUT -s 127.0.0.1/8 -j ACCEPT
    # currently still routes packets from ethernet interface as well, which we dont want
#/sbin/iptables -A OUTPUT -j NFQUEUE --queue-num 1 # queues packets leaving user space

/sbin/iptables -L –n # list all rules in numeric form
#/sbin/ip -s -s neigh flush all # flushes all neighbor table entries (DO NOT DO)
#ip route flush table main # flushes all routing table entries (DO NOT DO)

sh -c 'echo 1 > /proc/sys/net/ipv4/ip_forward' # enables IP forwarding
sh -c 'echo 1 > /proc/sys/net/ipv6/conf/wlan0/disable_ipv6' # disables IPv6 on wlan0 using a new Bourne Shell (for some reason)

#route add -net 0 metric 1000 dev wlo0
#echo 0 > /proc/sys/net/ipv4/conf/all/send_redirects
#echo 0 > /proc/sys/net/ipv4/conf/wlo0/send_redirects
exit