import serial
import time

arduino = serial.Serial(port='/dev/cu.usbserial-1240', baudrate=9600, timeout=.1)


def write_read(x):
   arduino.write(bytes(x, 'utf-8'))
   time.sleep(0.05)
   data = arduino.readline()
   return data


if __name__ == '__main__':
    while True:
        num = input("Enter a number: ") # Taking input from user
        value = write_read(num)
        print(value.decode()) # printing the value
