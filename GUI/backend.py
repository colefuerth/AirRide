"""
 * This is the backend for integration with arduino embedded system
 * This python program communicates over Serial CLI, with commands to request data over serial from the arduino
"""

# port at dmesg | grep "tty"

# make sure any incoming messages with prefix "log" are printed

import serial, time
port = '/dev/ttyACM0'  # default arduino port
baudrate = 115200  # serial baud with arduino


if __name__ == '__main__':
    with serial.Serial(port, baudrate, timeout=1) as arduino:
        time.sleep(0.1) # wait for port to open
        arduino.flush() # flush the boi
        if (arduino.isOpen()):
            print("{} connected!".format(arduino.port))
            try:
                while True:
                    # need to make sure messages sent end in \n
                    # this is to separate commands when sent quickly
                    arduino.write(str(input("enter msg here: ") + "\n").encode()) # write a byte, add newline on for end of command
                    time.sleep(0.1)
                    while arduino.in_waiting > 0:
                        line = arduino.read().decode()
                        print(line, end="")
            except KeyboardInterrupt:
                print("\nkeyboard interrupt ancountered, exiting")
