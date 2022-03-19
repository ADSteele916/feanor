import time

import serial 


port = "COM4"


with serial.Serial(port,9600,timeout = 0.5) as ser:
    print(ser.name)
    while(True): #loop forever
        targets = int(input("Targets:\n"), 2) * 16
        data = int(input("Send byte:\n"))
        send = targets + data
        print(f"Sending {send}.")
        ser.write(bytes([send]))
        receive = ser.read(1)
        receive_int = int.from_bytes(receive, "big")
        print(f"sent {send}, read {receive_int}")
        time.sleep(2)
