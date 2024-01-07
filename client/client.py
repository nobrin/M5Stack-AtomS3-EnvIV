#!/usr/bin/env python3
# BLEアドバタイズを受信する

import sys
sys.path.insert(0, "lib")

import struct
import asyncio
from bleak import BleakScanner

DEVICE_NAME = "m5stack"

last_seq = -1

def callback(device, data):
    global last_seq
    if device.name != DEVICE_NAME: return
    body = data.manufacturer_data[0xffff]

    seq = body[0]
    if seq == last_seq: return
    last_seq = seq
    tpl = struct.unpack("hhh", body[1:])
    temp = tpl[0] / 100
    hm = tpl[1] / 100
    atm = tpl[2] / 10
    print(f"温度: {temp:.1f}℃、湿度: {hm:.1f}％、気圧: {atm:.1f}hPa, RSSI: {data.rssi}")


async def run():
    print("Searching devices...")
    bs = BleakScanner(callback)
    await bs.start()
    while True:
        await asyncio.sleep(1.0)

asyncio.run(run())