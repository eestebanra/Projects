import socket
import struct


def sendData(index, data):
    command = b"DREF\x00"
    dataRef = dataRefs[index].encode()
    package = struct.pack("<5sf500s", command, data, dataRef)
    assert (len(package) == 509)
    sock.sendto(package, (UDP_IP, UDP_PORT))


def requestData(index, freq):
    # for index, dataRef in enumerate(dataRefs):
    command = b"RREF\0x00"
    dataRef = dataRefs[index].encode()
    package = struct.pack("<5sii400s", command, freq, index, dataRef)
    assert(len(package) == 413)
    sock.sendto(package, (UDP_IP, UDP_PORT))


def decodeData(receivedData):
    receivedValues = receivedData[5:]
    lenValue = 8
    numValues = int(len(receivedValues)/lenValue)
    for j in range(0, numValues):
        indexValue = receivedData[(5 + lenValue*j):(5 + lenValue*(j + 1))]
        (index, value) = struct.unpack("<if", indexValue)
        return value


UDP_IP = "192.168.50.250"
UDP_PORT = 49000

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

dataRefs = ["sim/cockpit2/radios/actuators/hsi_obs_deg_mag_pilot", "sim/cockpit/autopilot/airspeed",
            "sim/cockpit2/autopilot/airspeed_dial_kts_mach", "sim/cockpit/autopilot/heading_mag",
            "sim/cockpit/autopilot/altitude", "sim/cockpit2/autopilot/vvi_dial_fpm",
            "sim/cockpit/misc/barometer_setting", "sim/cockpit2/radios/actuators/com1_frequency_hz_833",
            "sim/cockpit/radios/nav1_freq_hz"]