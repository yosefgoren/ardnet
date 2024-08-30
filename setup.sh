#!/usr/bin/bash
make && rmmod ardnet.ko && insmod ardnet.ko && ip l set up dev virtnC0 && ip l set mtu 1000 dev virtnC0 