#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Thu Apr 16 20:12:47 2020

@author: devikrishnan
"""

import socket
import struct
import time
import collections
import csv


#unpack ethernet frame
def eth_unpack(data):
    dest_mac, src_mac, protocol = struct.unpack("! 6s 6s H", data[:14])
    return socket.htons(protocol), data[14:]


#unpack IPv4 packet
def ipv4_unpack(data):
    version_hdr_len = data[0]
    header_len = (version_hdr_len & 15) * 4
    ttl, proto, src, target = struct.unpack('! 8x B B 2x 4s 4s', data[:20])
    return proto, data[header_len:]


#unpack tcp segment
def tcp_unpack(data):
    src_port, dest_port, sequence, ack, offset_resvd_flgs = struct.unpack('! H H L L H', data[:14])
    offset = (offset_resvd_flgs >> 12) * 4
    return src_port, dest_port, data[offset:]


#unpack udp packet
def udp_unpack(data):
    src_port, dest_port, size = struct.unpack('! H H 2x H', data[:8])
    return src_port, dest_port, data[8:]



#main function
def main():
    conn = socket.socket(socket.AF_PACKET, socket.SOCK_RAW, socket.ntohs(0x003))
    stoptime = time.time() + 60 * 0.5
    packets = []

    while time.time() < stoptime:
        pkt, addr = conn.recvfrom(65536)
        packets.append(pkt)


    #create placeholders to hold count of each packet type
    pktCounts = collections.defaultdict.fromkeys(['ip', 'tcp', 'udp', 'dns', 'icmp', 'http', 'https', 'quic'])
    for key in pktCounts:
        pktCounts[key] = 0

    #pcount = 0
    for p in packets:
        #pcount += 1
        protocol, data = eth_unpack(p)
        #print("\nEthernet frame: ")
        #print("Protocol: {}".format(protocol))

        if (protocol == 8):   #IPv4
            ip_proto, ip_data = ipv4_unpack(data)
            #print("IP protocol: " +str(ip_proto))
            print("IP ")
            pktCounts['ip'] += 1

            if ip_proto == 1:  #ICMP
                print("\nICMP")
                pktCounts['icmp'] += 1
                pktCounts['ip'] += 1

            if ip_proto == 6:   #TCP
                print("\tTCP")
                pktCounts['tcp'] += 1
                pktCounts['ip'] += 1
                srcport, dstport, tcp_data = tcp_unpack(ip_data)

                if len(tcp_data) > 0:
                    if (srcport == 80 or dstport == 80):   #HTTP
                        print("\t\tHTTP")
                        pktCounts['http'] += 1
                        pktCounts['ip'] += 1
                        pktCounts['tcp'] += 1

                    if (srcport == 443 or dstport == 443):   #HTTPS
                        print("\t\tHTTPS")
                        pktCounts['https'] += 1
                        pktCounts['ip'] += 1
                        pktCounts['tcp'] += 1

            if ip_proto == 17:   #UDP
                print("\tUDP")
                pktCounts['udp'] += 1
                pktCounts['ip'] += 1
                srcport, dstport, udp_data = udp_unpack(ip_data)

                if len(udp_data) > 0 and (srcport == 53 or dstport == 53):   #DNS
                    print("\t\tDNS")
                    pktCounts['dns'] += 1
                    pktCounts['ip'] += 1
                    pktCounts['udp'] += 1


    #print(pcount) 
    #write packet counts to csv file
    with open('sniffer_dhkrishn.csv', 'w') as f:
        writer = csv.writer(f, delimiter=',')
        writer.writerow(['protocol','counts'])
        for key in pktCounts.keys():
            f.write("%s,%s\n" %(key, pktCounts[key]))



if __name__ == "__main__":
    main()
