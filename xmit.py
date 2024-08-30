#!/usr/bin/python3

import socket as sc

s = sc.socket(sc.AF_PACKET, sc.SOCK_RAW)
s.bind(('virtnC0', 0))

def send(bs):
    s.send(bytes(bs))

send([0b10101010, 0b11001100, 0b11110000, 0b11111111])
# send([0xfe, 0x00])