import serial
import matplotlib.pyplot as plt
import numpy as np
from collections import deque

# --------------------------
# USER CONFIG
# --------------------------
SERIAL_PORT = "/dev/tty.usbmodem11201"    # On Windows use "COM3"
BAUDRATE = 921600
CHANNELS = 1                    # CAPTURE_PIN_COUNT
BUFFER = 2000                  # Number of samples to display
MAX_DATA_POINTS = 8000
# --------------------------

ser = serial.Serial(SERIAL_PORT, BAUDRATE, timeout=1)

# storage buffer
logic_buffer = deque([0]*BUFFER, maxlen=BUFFER)

plt.ion()
fig, ax = plt.subplots()
line, = ax.plot(range(BUFFER), logic_buffer)
ax.set_ylim(-0.5, 1.5)
ax.set_title("RP2040 Logic Analyzer (Live)")
ax.set_xlabel("Sample Index")
ax.set_ylabel("Logic Level")

print("Reading data...")

while True:
    raw = ser.readline().strip()
    data_val = 0
    ts_us = 0
    try:
        # Expected format: 8-bit stream like 10101010,12345678
        line_str = raw.decode().strip()
        parts = line_str.split(',')
        if len(parts) == 2:
            ts_us = int(parts[1])
            data_val = int(parts[0],2)
        #print(bin(data_val))
        ch = []
        ts = []
        for i in range(8):
            # Extract the bit state: (data >> bit_index) & 1
            state = (data_val >> i) & 0x01
            
            # Append the data. Convert microseconds to seconds here.
            ts.append(ts_us / 1_000_000.0) 
            ch.append(state)
            
            # Maintain the plot buffer size
            if len(ts) > MAX_DATA_POINTS:
                del ts[:8]
                del ch[:8]

            ax.clear()
            ax.plot(ch)
            plt.pause(0.1)

        

 
    except Exception as e:
        print("ERR:", e)