from marklin import *
from arduino import *
import time
from time import sleep


def millis():
    return round(time.time() * 1000)


def accel_until_trigger(train, speed, trigger, com, ardu):
    com.set_train(train, speed+16)
    sleep(5)
    if trigger[0] == 'L':
        laser = int(trigger[1])
        tripped = ardu.laser_tripped(laser)
        hit_count = 0
        hit_max = 1
        state = 'hi'
        while hit_count < hit_max:
            tripped = ardu.laser_tripped(laser)
            if tripped and state == 'hi':
                state = 'lo'
                hit_count += 1
                print(tripped)
            elif not tripped and state == 'lo':
                state = 'hi'


def set_track(straight, curved, com):
    for s in straight:
        print("Switch {0} straight".format(s))
        com.switch_track(s, 0)
        sleep(0.15)
    for c in curved:
        print("Switch {0} curved".format(c))
        com.switch_track(c, 1)
        sleep(0.15)
    print("Done")


def time_difference(c0,c1):
    dt = c0 - c1
    if (dt == 0):
        return None
    return 2/dt


def loop_test(loops, stop_trigger, com, ardu):
    runs = 0
    times = []
    laser_times = []
    last_sensors = []
    t1 = millis()
    t2 = t1
    c0 = 0
    c1 = 0
    sensors = com.dump_sensors()

    while runs < loops:
        t1 = millis()
        if ardu.laser_tripped(1):
            c1 = ardu.get_clock(1)
            laser_times.append(('L1', c1))
            print('L1, {0}'.format(c1))
            if (stop_trigger) == 'L1':
                runs += 1
                if (runs >= loops):
                    break

        t1 = millis()
        if ardu.laser_tripped(0):
            # assume then L0 follows L1
            c0 = ardu.get_clock(0)
            d = time_difference(c0, c1)
            laser_times.append(('L0', c0))
            laser_times.append(('L0-L1 (2cm spacing)', c0-c1))
            laser_times.append(('d(L0-L1) (cm/ms)', d))
            print('L0, {0}'.format(c0))
            print('L0-L1, {0}'.format(c0-c1))
            print('d(L0-L1) (cm/ms), {0}'.format(d))
            if (stop_trigger) == 'L0':
                runs += 1
                if (runs >= loops):
                    break

        t1 = millis()
        sensors = com.dump_sensors()

        if (len(sensors)) > 1:
            print(sensors)
            raise ValueError

        if sensors == last_sensors or len(sensors) == 0:
            continue
        last_sensors = sensors

        s = sensors[0]
        times.append((s, t1))
        print("{0}: {1}".format(s, t1))
        if (s == stop_trigger):
            runs += 1

    return (times, laser_times)


def track_b_middle_loop_and_stop(train, speed, loops, stop_trigger, com=COM1(), ardu=Arduino()):
    loop_straight = [15, 16, 6, 9]
    loop_curved = [11]

    stop_straight = [11, 12]
    stop_curved = []

    set_track(loop_straight, loop_curved, com)
    accel_until_trigger(train, speed, stop_trigger, com, ardu)

    times, laser_times = loop_test(loops, stop_trigger, com, ardu)
    com.set_train(train, 0)
    set_track(stop_straight, stop_curved, com)

    f = open("raw_times.txt", 'a')
    f.write('Track B Loop and Stop, Train: {0}, Speed: {1}, Loops: {2}, Trigger: {3}\n'.format(train, speed, loops, stop_trigger))
    for i in times:
        f.write("{0}, {1}\n".format(i[0], i[1]))
    for i in laser_times:
        f.write("{0}, {1}\n".format(i[0], i[1]))
    f.write('\n')
    f.close()


def track_a_outer_loop_and_stop_straight(train, speed, loops, stop_trigger, com=COM1(), ardu=Arduino()):
    loop_straight = [6, 9, 15]
    loop_curved = [5, 11]

    stop_straight = [11, 12]
    stop_curved = []

    set_track(loop_straight, loop_curved, com)
    accel_until_trigger(train, speed, stop_trigger, com, ardu)

    times, laser_times = loop_test(loops, stop_trigger, com, ardu)
    com.set_train(train, 0)
    set_track(stop_straight, stop_curved, com)

    f = open("raw_times.txt", 'a')
    f.write('Track A Loop and Stop, Train: {0}, Speed: {1}, Loops: {2}, Trigger: {3}\n'.format(train, speed, loops, stop_trigger))
    for i in times:
        f.write("{0}, {1}\n".format(i[0], i[1]))
    for i in laser_times:
        f.write("{0}, {1}\n".format(i[0], i[1]))
    f.write('\n')
    f.close()


