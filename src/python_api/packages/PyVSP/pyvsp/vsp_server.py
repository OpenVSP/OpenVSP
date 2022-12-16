from threading import Thread, Event
from time import sleep
import pickle

import openvsp as vsp

HOST = 'localhost'
PORT = 6000

event = Event()

def start_server():
    import socket
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.bind((HOST,PORT))
        print("listening...")
        s.listen()
        conn, addr = s.accept()
        with conn:
            print(f"Connected by {addr}")
            while True:
                b_data = []
                while True:
                    packet = conn.recv(1024)
                    if not packet: break
                    b_data.append(packet)
                    try:
                        data = pickle.loads(b"".join(b_data))
                        break
                    except:
                        pass
                # data = pickle.loads(b"".join(b_data))
                print("recieved data", data)
                if data[0] == 'close':
                    break
                func_name = data[0]
                args = data[1]
                foo = getattr(vsp, func_name)
                print(foo)
                print(*args)
                result = foo(*args)
                b_result = pickle.dumps(result)
                conn.sendall(b_result)


    print("socket closed")

print("(main) before thread")
t = Thread(target=start_server, args=())
t.start()
print("(main) after thread")



#vsp.ReadVSPFile(r'C:\work\gmdao\test_app\ElectricGA.vsp3')
vsp.InitGui()
vsp.StartGui()
event.set()
