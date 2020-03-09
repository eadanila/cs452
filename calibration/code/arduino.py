import time
import serial
from threading import Thread
from sys import argv


class Arduino():
    def __init__(self):
        self.ser = serial.Serial('/dev/cu.usbmodem1443201', 115200, timeout=0.05)
        while self.ser.readline() != b'':
            1+1


    def serial_query(self, msg):
        self.ser.write(bytes(msg))
        resp = self.ser.readline().decode('UTF-8')
        if resp == '':
            return -1


    def get_all_laser_states(self):
        self.ser.write(bytes([2]))
        l1 = self.get_laser_state(0)
        l2 = self.get_laser_state(1)
        return (l1, l2)


    def get_laser_state(self, laser_id):
        if (laser_id >= 2):
            return -1
        self.ser.write(bytes([laser_id]))
        self.ser.write(bytes([ord('r')]))
        l1 = self.ser.readline().decode('UTF-8')
        if l1 == '':
            return -1
        return int(l1.strip())


    def laser_tripped(self, laser_id):
        if (laser_id >= 2):
            return -1
        self.ser.write(bytes([laser_id,ord('t')]))
        l = self.ser.readline().decode('UTF-8')
        if l.strip() == '':
            return -1
        return int(l.strip()) == 1


    def get_velocity(self, laser_id):
        if (laser_id >= 2):
            return -1
        self.ser.write(bytes([laser_id,ord('v')]))
        l = self.ser.readline().decode('UTF-8')
        if l.strip() == '':
            return -1
        return int(l.strip())


    def get_clock(self, laser_id):
        if (laser_id >= 2):
            return -1
        self.ser.write(bytes([laser_id,ord('c')]))
        l = self.ser.readline().decode('UTF-8')
        if l == '':
            return -1
        return int(l.strip())


    def wait_for_next_trip(self, laser_id):
        if (laser_id >= 2):
            return -1
        self.laser_tripped(laser_id)

        while not self.laser_tripped(laser_id):
            1+1

        return self.get_clock(laser_id)

