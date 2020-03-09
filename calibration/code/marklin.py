import time
import serial
from threading import Thread
from sys import argv

ser = serial.Serial('/dev/cu.usbserial', 2400, serial.EIGHTBITS, parity=serial.PARITY_NONE, stopbits=serial.STOPBITS_TWO)

sensors = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
old_sensors = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0]

switches = [1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,153,154,155,156]


class COM1(Thread):
    def __init__(self):
        super().__init__()
        self.ser = serial.Serial('/dev/cu.usbserial', 2400, stopbits=serial.STOPBITS_TWO)
        self.switch_time = 0
        self.do_switch_off = False
        self.running = True
        self.start()


    def send_command(self, command):
        while not self.ser.getCTS():
            1+1
        self.ser.write(bytes([command]))
        time.sleep(0.05)

    def reset_solenoid(self):
        if self.do_switch_off and (time.time() >= self.switch_time + 0.3) and self.ser.getCTS():
            self.send_command(32)
            self.switch_time = time.time()
            self.do_switch_off = False


    def switch_track(self, addr, direction):
        if (direction > 2):
            raise Exception
        while time.time() < self.switch_time + 0.15 and not self.ser.getCTS():
            1 + 1
        self.send_command(direction+33)
        self.send_command(addr)
        self.switch_time = time.time()
        self.do_switch_off = True


    def set_train(self, train, speed):
        self.send_command(speed)
        self.send_command(train)


    def dump_raw_sensors(self):
        sensor_data = [None, None, None, None, None]
        self.ser.write(bytes([133]))
        for i in range(5):
            r1 = int.from_bytes(self.ser.read(), 'big')
            r2 = int.from_bytes(self.ser.read(), 'big')
            sensor_data[i] = (r1, r2)
        return tuple(sensor_data)


    def dump_sensors(self):
        raw = self.dump_raw_sensors()
        sensors = []
        for i in range(5):
            b = raw[i]
            s = chr(ord('A')+i)
            for j in range(2):
                d = b[j]
                for k in [7,6,5,4,3,2,1,0]:
                    if d == 0:
                        break
                    if 2 ** k <= d:
                        sensors.append(s + str(j*8 + (8-k)))
                        d -= (2 ** k)
        if ('C13' in sensors):
            sensors.remove('C13')
        return sensors


    def get_raw_sensors(self, box_id):
        sensor = ord(box_id) - ord('A') + 1
        self.ser.write(bytes([192+sensor]))
        r1 = int.from_bytes(self.ser.read(), 'big')
        r2 = int.from_bytes(self.ser.read(), 'big')
        return (r1, r2)


    def get_sensors(self, box_id):
        raw = self.get_raw_sensors(box_id)
        s = box_id
        sensors = []
        for j in range(2):
            d = raw[j]
            for k in [7,6,5,4,3,2,1,0]:
                if d == 0:
                    break
                if (2 ** k) <= d:
                    sensors.append(s + str(j*8 + (8-k)))
                    d -= (2 ** k)
        return sensors


    def run(self):
        while self.running:
            self.reset_solenoid()
            time.sleep(0.01)


def accelerate(com, train, speed, switch):
    a = decode_sensors(com)
    com.set_train(train, speed)
    time.sleep(0.1)
    done = False
    runs = 0
    t = millis()
    print('accelerating', train, speed)
    while runs < 2:
        a = decode_sensors(com)
        t = round(millis(), 4)
        if len(a) == 1 and a[0] == switch:
            runs += 1
    return t

def loop_test(com, train, speed, switch, loops):
    runs = 0
    t1 = round(millis(), 4)
    t2 = accelerate(com, train, speed, switch)
    times = []
    loop_times = {}

    print("running")
    while runs < loops:
        a = decode_sensors(com)
        t1 = millis()

        if (len(a) > 1):
            raise ValueError

        if (len(a) == 1):
            if a[0] in loop_times:
                diff = round(t1-t2, 4)
                loop_time = round(t1 - loop_times[a[0]], 4)
                times.append((a[0], t1, diff, loop_time))
                if (a[0] == switch):
                    print(loop_time)
                    runs += 1
                    print(runs)
            loop_times[a[0]] = t1
            t2 = t1
    com.set_train(train, 0)

    return times

def track_b_outer_loop_clockwise_test(com, trains, speed, loops):
    logfile = open("track-outer-loop-clockwise" + '-' + str(speed) + '.txt', 'a')
    sensor = 'C13'
    curved = [7, 3]
    straight = [2, 1, 8, 18]

    for i in curved:
        com.switch_track(i, 1)
        time.sleep(0.15)
    for i in straight:
        com.switch_track(i, 0)
        time.sleep(0.15)

    for train in trains:
        times = loop_test(com, train, speed, sensor, loops)
        logfile.write('Train: {0}, Speed: {1}, Sensor: {2}, Loops: {3}\n'.format(train, speed, sensor, loops))
        for i in times:
            logfile.write(str(i[0]) + ', ' + str(i[1]) + ', ' + str(i[2]) + ', ' + str(i[3]) + '\n')
        time.sleep(5)
    logfile.close()

def track_b_middle_loop_clockwise_test(com, trains, speed, loops):
    logfile = open("track-middle-loop-clockwise" + - str(speed) + 'txt', 'a')
    sensor = 'C13'
    straight = [11, 8, 7]

    for i in straight:
        com.switch_track(i, 0)
        time.sleep(0.15)

    for train in trains:
        times = loop_test(com, train, speed, sensor, loops)
        logfile.write('Train: {0}, Speed: {1}, Sensor: {2}, Loops: {3}\n'.format(train, speed, sensor, loops))
        for i in times:
            logfile.write(str(i[0]) + ', ' + str(i[1]) + ', ' + str(i[2]) + ', ' + str(i[3]) + '\n')
        time.sleep(5)
    logfile.close()

if __name__ == '__main__':
    c = COM1()

    trains = [1, 24, 58, 74, 78, 79]
    speed = 30
    track_b_outer_loop_clockwise_test(c, trains, speed, 16)

