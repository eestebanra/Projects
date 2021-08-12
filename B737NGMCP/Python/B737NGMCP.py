import serial
import pydirectinput
import time
import XPlaneUDP

serialMonitor = serial.Serial("COM3", 115200, timeout=.1)
pydirectinput.PAUSE = 0

switchKeyboard = ["z", "x", "c", "v", "b", "n", "m", ","]
pushButtonKeyboard = ["0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "-", "=", "[", "]"]


def switchInput(switch):
    if switch == 3:
        pydirectinput.press(switchKeyboard[switch])
        time.sleep(2)
        pydirectinput.press(switchKeyboard[switch])

    else:
        pydirectinput.press(switchKeyboard[switch])


def pushButtonInput(pushButton):
    pydirectinput.press(pushButtonKeyboard[pushButton])


while True:
    rawdata = serialMonitor.readline()
    data = str(rawdata.decode("utf-8"))
    # Read switches
    if data.startswith("S"):
        switchInput(int(data[1]))
    # Read push buttons
    if data.startswith("B"):
        pushButtonInput(int(data[1:]))
        if int(data[1:]) == 10:
            XPlaneUDP.requestData(5, 1)
            value, address = XPlaneUDP.sock.recvfrom(1024)
            XPlaneUDP.requestData(5, 0)
            XPlaneVSV = XPlaneUDP.decodeData(value)
            serialMonitor.write(bytes(str(XPlaneVSV), "utf-8"))
    # Read encoder
    if data.startswith("E"):
        XPlaneUDP.sendData(int(data[1]), float(data[2:]))
