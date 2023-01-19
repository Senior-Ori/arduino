import serial
from tkinter import *
import tkinter as tk
import time

import json
ser = serial.Serial('COM5', baudrate=57600, timeout=.01)
time.sleep(1)
def writeJsonHandler(var,dataStr):

    ser.write("".encode())
DataDict = {"command": "", "leds": [-1, -1, -1, -1]}


# DataDict = {"command": "", "leds": [-1, -1, -1, -1]}
# DataDict["command"]="triggerLed"
# print(f"{DataDict}\r\n".encode())

def writeJsonHandler(name,values):
    DataDict[name] = values
    # if name=="triggerLed": DataDict["leds"] = [-1,-1,-1,-1]
    # {"command":"triggerLed","leds":[-1,-1,-1,-1]}
    ser.write(f"{DataDict}\r\n".encode())
    DataDict["command"]=""
    return


def turnOnLED():
    writeJsonHandler("","")
    # i=0
    writeJsonHandler("leds",[1,0,1,0])
    while True:
    #     i+=2
    #     print(f"Loding... number of checks for serial input:~=[{i}]")
        if ser.in_waiting:
            text1 = ser.readline().decode('utf')
@ -24,63 +33,61 @@


def turnOffLED():
    ser.write("off\r\n".encode())
    # i=0
    writeJsonHandler("leds",[0,0,0,0])

    while True:
    #     i += 2
    #     print(f"Loding... number of checks for serial input:~=[{i}]")
        if ser.in_waiting:
            text1 = ser.readline().decode('utf')
            break
    my_var.set(str(text1))
def turnOnOffLED():
    ser.write("triggerLed\r\n".encode())
    # i=0
    DataDict["leds"] = [-1,-1,-1,-1]
    writeJsonHandler("command","triggerLed")
    while True:
    #     i += 2
    #     print(f"Loding... number of checks for serial input:~=[{i}]")
        if ser.in_waiting:
            text1 = ser.readline().decode('utf')
            break
    my_var.set(str(text1))

# creating TK-inter window
root = Tk()
root.title('Blink GUI')
text1="initial value"
my_var = tk.StringVar()
my_var.set("First click")

btn_On = tk.Button(root,name="btn_On1", text="on", command=turnOnLED)
btn_On.grid(row=0,column=0)
btn_Off = tk.Button(root, text="off", command=turnOffLED)
btn_Off.grid(row=0,column=1)
btn_Trigger = tk.Button(root, text="Trigger", command=turnOnOffLED)
btn_Trigger.grid(row=0,column=2)

label = tk.Label(root, textvariable=my_var, fg="red")
label.grid(row=1,column=0)


root.geometry("350x350")
root.mainloop()

## firstCode
# numPoints = 5
# dataList = [0] * numPoints
#
#
# def getValues():
#     ser.write(b'g')
#     arduinoData = ser.readline().decode('ascii')
#     return arduinoData
#
#
# while True:
#     userInput = input('Get data point?')
#     if userInput == 'y':
#         for i in range(0,len(dataList),1):
#             data = getValues()
#             dataList[i]=data
#         print(dataList)
