import os
import sys
HEADER = """
# Facade Code
# **********************************************************************************
import os
import sys
import socket
from time import sleep
import subprocess
import pickle
"""

START_SERVER_CODE = """
# Starting the server
HOST = 'localhost'
PORT = 6000"
server_file = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'facade_server.py')
proc = subprocess.Popen([sys.executable, server_file])

sleep(1)

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect((HOST, PORT))
"""

DECORATOR_CODE = """
# decorator for wrapping every function
def test_wrap(func):
    def wrapper(*args, **kwargs):
        return send_recieve(func.__name__, args, kwargs) #I think we might need something for kwargs
    return wrapper
"""



SEND_RECIEVE_START = """
# function to send and recieve data from the facade server
def send_recieve(func_name, args, kwargs):
    b_data = pickle.dumps([func_name, args, kwargs])
    sock.sendall(b_data)
    result = None
    b_result = []
    while True:
        packet = sock.recv(202400)
        if not packet: break
        b_result.append(packet)
        try:
            result = pickle.loads(b"".join(b_result))
"""

SEND_RECIEVE_END = """
            break
        except:
            pass
    return result
"""

def make_facade(file_path,
    classes_to_remove = [],
    custom_code = "",
    non_serializable_code = "",

    ):
    module_name = file_path.split(".")[0]

    in_header_comment = True

    in_class_list = {}
    removed_class_list = {}
    for class_name in classes_to_remove:
        in_class_list[class_name] = False
        removed_class_list[class_name] = False

    new_facade_string = ''

    with open(file_path, 'r') as f:
        for line in f.readlines():
            # adds comment to header
            if in_header_comment and not "#" in line:
                in_header_comment = False
                new_facade_string += "# This file has been modified by the OpenVSP automated facade script\n"


            #removing classes
            if "class" in line:
                class_name = line.split()[1].split("(")[0]
                if class_name in in_class_list:
                    in_class_list[class_name] = True

            #second part of removing classes
            if any(in_class_list.values()):
                #removes vec3d and matrix4d
                if "swigregister" in line:
                    class_name = line.split("(")[-1].strip()[:-1]
                    if class_name in in_class_list:
                        in_class_list[class_name] = False
                        removed_class_list[class_name] = True
                continue

            #adding function decorator to every function
            if (
                "def " in line
                and not "   def" in line
                and not "swig" in line
            ):
                new_facade_string += "@client_wrap\n"

            new_facade_string += line


    #adding required code
    new_facade_string += f"del _{module_name}\n"
    for class_name in classes_to_remove:

        new_facade_string += f"from {module_name} import {class_name}\n"


    new_facade_string += HEADER
    new_facade_string += START_SERVER_CODE
    new_facade_string += DECORATOR_CODE
    new_facade_string += SEND_RECIEVE_START
    new_facade_string += non_serializable_code
    new_facade_string += SEND_RECIEVE_END
    new_facade_string += custom_code

    with open('facade.py', 'w') as f:
        f.write(new_facade_string)

def make_server(module_name,
    custom_server_func = "",
    lock_func_name = None,
    unlock_func_name = None,
    second_thread_code = "",
    non_serializable_code = ""

):
    server_string = f"""
from threading import Thread, Event
import pickle

from . import {module_name} as module

HOST = 'localhost'
PORT = 6000

event = Event()

def start_server():
    import socket
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.bind((HOST,PORT))
        socket_open = True
        while socket_open:
            print("listening...")
            s.listen()
            conn, addr = s.accept()
            with conn:
                print("Connected by %s" % addr)
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
                    {custom_server_func}
                    func_name = data[0]
                    args = data[1]
                    kwargs = data[2]
                    foo = getattr(module, func_name)
"""
    if lock_func_name:
        server_string += f"                    module.{lock_func_name}()\n"
    server_string += f"                    result = foo(*args, **kwargs)\n"
    if unlock_func_name:
        server_string += f"                    module.{unlock_func_name}()\n"
    server_string+= "                    new_result = []"
    server_string += non_serializable_code
    server_string += """
                    b_result = pickle.dumps(result)
                    conn.sendall(b_result)
    print("socket closed")
t = Thread(target=start_server, args=())
t.start()
"""
    server_string += second_thread_code
    with open('facade_server.py', 'w') as f:
        f.write(server_string)



non_serializable_code = """
            # tailored code to handle non-serializable data
            try:
                if result['name'] == 'vec3d':
                    new_result = vec3d(result['x'],result['y'],result['z'])
                elif result['name'] == 'vec3d_list':
                    new_result = []
                    for r in result['list']:
                        new_result.append(vec3d(r['x'],r['y'],r['z']))
                result = new_result
            except:
                pass
"""

custom_code = """
# special function to open the OpenVSP GUI
def open_gui():
    b_data = pickle.dumps(['opengui'])
    sock.sendall(b_data)
    result = None
    b_result = []
    while True:
        packet = sock.recv(202400)
        if not packet: break
        b_result.append(packet)
        try:
            result = pickle.loads(b"".join(b_result))
            break
        except:
            pass
    return result
"""

classes_to_remove = [
    "ErrorObj",
    "ErrorMgrSingleton",
    "vec3d",
    "Matrix4d"
]

custom_server_func = """
                    if data[0] == 'opengui':
                        result = 0
                        b_result = pickle.dumps(result)
                        conn.sendall(b_result)
                        event.set()
                        continue
"""

server_non_serializable_code = """
                    try:
                        if isinstance(result, vsp.vec3d):
                            result = {"name":'vec3d',
                                "x":result.x(),
                                "y":result.y(),
                                "z":result.z(),
                            }
                        else:
                            new_result = {
                                "name": "vec3d_list",
                                "list": []
                            }
                            for p in result:
                                thing = {
                                    "x":p.x(),
                                    "y":p.y(),
                                    "z":p.z(),
                                }
                                new_result['list'].append(thing)
                            result = new_result
                    except:
                        pass
"""
second_thread_code = """
event.wait()
module.InitGui()
module.StartGui()
"""


def make_vsp_facade(directory):

    old_cwd = os.getcwd()
    os.chdir(directory)
    make_facade(r"vsp_g.py", classes_to_remove = classes_to_remove, custom_code = custom_code, non_serializable_code = non_serializable_code)
    make_server("vsp_g",custom_server_func,"Lock","Unlock",second_thread_code, server_non_serializable_code)
    os.chdir(old_cwd)
    
if __name__ == "__main__":
    make_vsp_facade(sys.argv[1])
