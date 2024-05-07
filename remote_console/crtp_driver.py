#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
#     ||          ____  _ __
#  +------+      / __ )(_) /_______________ _____  ___
#  | 0xBC |     / __  / / __/ ___/ ___/ __ `/_  / / _ \
#  +------+    / /_/ / / /_/ /__/ /  / /_/ / / /_/  __/
#   ||  ||    /_____/_/\__/\___/_/   \__,_/ /___/\___/
#
#  Copyright (C) 2011-2013 Bitcraze AB
#
#  Crazyflie Nano Quadcopter Client
#
#  This program is free software; you can redistribute it and/or
#  modify it under the terms of the GNU General Public License
#  as published by the Free Software Foundation; either version 2
#  of the License, or (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
#  MA  02110-1301, USA.
""" CRTP UDP Driver. Work either with the UDP server or with an UDP device
See udpserver.py for the protocol"""
import re
import struct
import sys
import socket

if sys.version_info < (3,):
    import Queue as queue
else:
    import queue

__author__ = "Bitcraze AB"
__all__ = ["UdpDriver"]


class UdpDriver:

    def __init__(self):
        None

    def connect(self):

        self.queue = queue.Queue()
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.socket.settimeout(30)
        self.addr = ("192.168.43.42", 2390)
        # self.socket.connect(self.addr)

        # Add this to the server clients list
        self.socket.sendto(b"\xFF\x01\x01\x01", self.addr)

    def pid_update(self, pid_num, kp, ki, kd):
        data = struct.pack("<BBBfff", 0x40, 0x51, pid_num, kp, ki, kd)
        self.send_packet(data)

    def receive_packet(self, raw=False, time=0):
        data, addr = self.socket.recvfrom(64)
        if raw:
            return data

        if data:
            msg = self.decode_pkg(data)
            return msg

        try:
            if time == 0:
                return self.rxqueue.get(False)
            elif time < 0:
                while True:
                    return self.rxqueue.get(True, 10)
            else:
                return self.rxqueue.get(True, time)
        except queue.Empty:
            return None

    def decode_pkg(self, data):
        if data[0] == 0x3E:
            recv = bytes(self.driver.receive_packet(raw=True)[1:])
            recv = int.from_bytes(recv, byteorder="little")
        elif data[0] == 0x60:
            recv = struct.unpack("<Bdd", data[0:-1])
        elif data[0] == 0x01:
            recv = struct.unpack("B" * len(data), data)
            recv = bytes(recv[1:-1]).decode("utf-8")
        else:
            recv = "not understood"

        return recv

    def send_packet(self, pk):
        self.socket.sendto(pk, self.addr)

    def send_packet_cksum(self, pk):
        raw = struct.unpack("B" * len(pk), pk)

        cksum = 0
        for i in raw:
            cksum += i

        cksum %= 256

        data = "".join(chr(v) for v in (raw + (cksum,)))

        # print tuple(data)
        self.socket.sendto(data.encode(), self.addr)

    def close(self):
        # Remove this from the server clients list
        self.socket.sendto(b"\xFF\x01\x02\x02", self.addr)

    def get_name(self):
        return "udp"

    def scan_interface(self, address):
        return []
