import json, requests
import time
import serial#导入串口通信库
import time_online
from time import sleep
import struct

#define NEED_DATA 1
NEED_DATA=1
ser = serial.Serial()

url = 'http://127.0.0.1:8085'
#别加后面的斜杠。还需要注意LibreHardware的地址变化！
url='http://192.168.221.1:20000'

def port_open_recv():#对串口的参数进行配置
    ser.port='com4'
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

def send_string_serial(str_main):
	port_open_recv()
	while True:
		if (ser.isOpen()):
			rec = (int)(ser.read())
			if (rec == NEED_DATA):
				ser.write(str_main.encode(encoding='ascii'))
				print('Sent\n')
				break

def send_string_serial_2(str_main):
	while True:
		if (ser.isOpen()):
			rec = (int)(ser.read())
			if (rec == NEED_DATA):
				ser.write(str_main.encode(encoding='ascii'))
				print('Sent\n')
				break

def findSensors(node):
	sensors = {}

	if len(node["Children"]) > 0:
		for child in node["Children"]:
			sensors.update(findSensors(child))
	else:
		if "Type" in node:
			sensors[node["SensorId"]] = node

	return sensors

def getValue(sensorId):
	params=dict(id=sensorId, action="Get")
	resp = requests.post(url=url + "/Sensor", params = params, timeout=10);
	result = json.loads(resp.text);

	if result["result"] != "ok":
		raise Exception("Server returned error:\n " + result["message"].replace("\\n", "\n").replace("\\r", ""))
	if result["value"] == None:
		return None;
	else:
		return float(result["value"])

def setValue(sensorId, sensorValue):
	if sensorValue == None:
		sensorValue = "null"
	params=dict(id=sensorId, action="Set", value=sensorValue)
	resp = requests.post(url=url + "/Sensor", params = params, timeout=10);
	result = json.loads(resp.text)
	if result["result"] != "ok":
		raise Exception("Server returned error:\n " + result["message"].replace("\\n", "\n").replace("\\r", ""))

#整个程序的思路，首先获取所有传感器的名称 findSensors(该函数返回字典。使用var.item()遍历字典所有内容) 如/intelcpu/0/temperature/1
#然后根据传感器名称获得值 getValue 此时需要再使用requests获取
def main():
	#该库用于获取错误信息
	import traceback
	params = dict()

	print("Fetching all sensor ids:")
	#主要还是用get获取json
	resp = requests.get(url=url + "/data.json", params=params, timeout=10)

	data = json.loads(resp.text)
	# try:
	# 	with open('test_json.txt','w+',encoding='UTF-8') as json_file:
	# 		json_file.write(resp.text)
	# 		json_file.write("\n")
	# 		json_file.write(str(data))
	# except IOError:
	# 	traceback.print_exc()
	sensors = findSensors(data)
	print("*"*20)
	print(sensors)
	#sensors是一个字典，字典的值又对应字典
	# 如：'/intelcpu/0/temperature/1': {'id': 18, 'Text': 'CPU Core #2', 'Min': '48.0 °C', 'Value': '53.0 °C', 'Max': '79.0 °C',
	print("*"*20)

	sensors_names=list()
	for key,value in sensors.items():
		sensors_names.append(key)
	for i in sensors_names:
		print("--"*10)
		print(i)
	#返回'/intelcpu/0/voltage/0', '/intelcpu/0/power/0', '/intelcpu/0/power/1', '/intelcpu/0/power/3', 等
	sensors_names_need={'CPU Temperature Avg':'/intelcpu/0/temperature/10',
						'CPU Load Total':'/intelcpu/0/load/0',
						'RAM Load':'/ram/load/0',
						'RAM Used':'/ram/data/0',
						'RAM Free':'/ram/data/1',
						'GPU Temp':'/gpu-nvidia/0/temperature/0',
						'GPU Load':'/gpu-nvidia/0/load/0',
						}
	# sensors_names_need=['/intelcpu/0/temperature/10',]

	#这里有几个问题就是 每次烧录完固件之后会出现一次串口传输错误，需要手动重新启动脚本
	#还有串口和ip地址会变的问题
	main_str=""
	main_str2=""
	port_open_recv()
	while True:
		#清空上一次内容，否则会叠加
		main_str2 = ""
		#main_str是正常数值，2是x100之后的内容
		for print_name,sensor_name in sensors_names_need.items():
			# print("-"*30)
			temp_val=getValue(sensor_name)
			# print(print_name+"\t"+str(temp_val))

			temp_str2=str(temp_val*100)
			main_str2= main_str2+temp_str2[0:4]+","

			temp_val=str(temp_val)
			main_str=main_str+temp_val[0:5]+","
		# print(main_str2)
		# print(main_str)
		if (ser.isOpen()):
			rec = (int)(ser.read())
			if (rec == NEED_DATA):
				print(main_str2)
				ser.write(main_str2.encode(encoding='ascii'))

				print('Sent\n')
		# send_string_serial_2(main_str2)


	#以下是官方代码
	# for key, value in sensors.items():
	# 	v = getValue(key)
	# 	print(key, ":", v);

	# # Change the id to one of yours
	# print("Setting GPU Fan to full speed!")
	# setValue("/nvidiagpu/0/control/0", "100.0")
	# time.sleep(10);
	# print("Returning GPU Fan speed to default")
	# setValue("/nvidiagpu/0/control/0", None)


if __name__ == '__main__':
    main()