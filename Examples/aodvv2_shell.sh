#!/bin/sh
# xt_NFQUEUE
echo "Stopping the firewall and inserting ruler"
/sbin/iptables –F
#/sbin/iptables -I INPUT -s 192.168.1.7 -j DROP
#/sbin/iptables -t mangle -A PREROUTING -m mac --mac-source 48:5d:60:7e:bb:8f -j DROP
 
#/sbin/iptables -A INPUT -p UDP --sport 269 --dport 269 -j NFQUEUE --queue-num 0

#/sbin/iptables -I OUTPUT -p UDP --sport 269 --dport 269 -j ACCEPT
#/sbin/iptables -I OUTPUT -s 127.0.0.1/8 -j ACCEPT
    # currently still routes packets from ethernet interface as well, which we dont want
#/sbin/iptables -A OUTPUT -j NFQUEUE --queue-num 1 # queues packets leaving user space
#/sbin/iptables -L –n
#/sbin/ip -s -s neigh flush all # DONT DO
echo 1 > /proc/sys/net/ipv4/ip_forward
sh -c 'echo 1 > /proc/sys/net/ipv6/conf/wlan0/disable_ipv6'
#ip route flush table main # DONT DO
route add -net 0 metric 1000 dev wlo0
#echo 0 > /proc/sys/net/ipv4/conf/all/send_redirects
#echo 0 > /proc/sys/net/ipv4/conf/wlo0/send_redirects
exit