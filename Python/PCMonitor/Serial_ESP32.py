import serial#导入串口通信库
from time import sleep
import struct

#define NEED_DATA 1
NEED_DATA=1

ser = serial.Serial()

def port_open_recv():#对串口的参数进行配置
    ser.port='com13'
    ser.baudrate=115200
    ser.bytesize=8
    ser.stopbits=1
    ser.parity="N"#奇偶校验位
    ser.open()
    if(ser.isOpen()):
        print("串口打开成功！")
    else:
        print("串口打开失败！")
#isOpen()函数来查看串口的开闭状态



def port_close():
    ser.close()
    if(ser.isOpen()):
        print("串口关闭失败！")
    else:
        print("串口关闭成功！")

def send(send_data):
    if(ser.isOpen()):
        ser.write(send_data.encode('utf-8'))#编码
        print("发送成功",send_data)
    else:
        print("发送失败！")

def read():
    size_10=ser.read(size=10)
    ser.read_all()

if __name__ == '__main__':
    port_open_recv()
    float_serial=[1.1,1.23,1.55,6.32,7.77,9,99.9]
    while True:
        # a=input("输入要发送的数据：")
        # send(a)
        # sleep(0.5)#起到一个延时的效果，这里如果不加上一个while True，程序执行一次就自动跳出了

        # data_received=ser.readline()
        # print(data_received.decode())
        # sleep(0.5)

        # #测试struct
        # i=struct.pack('f',float_serial[1])
        # ser.writelines(i)
        # # for i in float_serial:
        # #     i=struct.pack('f',i)
        # #     ser.write(i)
        # #     sleep(0.01)
        # #     print("Sending",str(i))
        # sleep(10)
        data="1.11,2.22,3.33,4.44,5.55,6.66,7.666"
        if(ser.isOpen()):
            rec=(int)(ser.read())
            if(rec==NEED_DATA):
                ser.write(data.encode(encoding='ascii'))
                print('ok')