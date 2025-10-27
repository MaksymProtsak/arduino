import serial


def readserial(comport, baudrate):
    ser = serial.Serial(comport, baudrate, timeout=0.1)  # 1/timeout is the frequency at which the port is read

    while True:
        data = ser.readline().decode().strip()
        if data:
            print(data)


def writeserial(comport, baudrate):
    ser = serial.Serial(comport, baudrate, timeout=0.1)


if __name__ == '__main__':

    readserial('/dev/cu.usbserial-1240', 9600)