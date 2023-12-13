import os
import sys

CLASSES_TO_REMOVE = [
    "ErrorObj",
    "ErrorMgrSingleton",
    "vec3d",
    "Matrix4d"
]

OPEN_GUI_DOC = """
    \"\"\"
    Starts the GUI through the facade API. See InitGui and StartGui if not using the facade.


    .. code-block:: python

        if is_facade():
            open_gui()
    \"\"\"
"""

PLACEHOLDER_OPEN_GUI = "# **Placeholder start**\n" + "def open_gui():" + OPEN_GUI_DOC + r"""
    print("WARNING: open_gui only has functionality if the facade API is active. \nSee InitGui and StartGui for non-facade GUI")
"""


CLIENT = r"""
# Facade Code
# **********************************************************************************
import os
import sys
import socket
from time import sleep
import subprocess
import pickle
from openvsp.facade_server import pack_data, unpack_data
from traceback import format_exception

# Starting the server
HOST = 'localhost'
PORT = 6000
server_file = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'facade_server.py')
proc = subprocess.Popen([sys.executable, server_file])

sleep(1)

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect((HOST, PORT))

def _exception_hook(exc_type, exc_value, tb):
    regular_traceback = []
    facade_traceback = []
    for line in exc_value.args[0].split("\n")[:3]:
        facade_traceback.append(line)

    for line in format_exception(exc_type, exc_value, tb)[:-1]:
        if "facade.py" in line or "facade_server.py" in line:
            facade_traceback.append(line.strip("\n"))
        else:
            regular_traceback.append(line.strip("\n"))
    for line in exc_value.args[0].split("\n")[3:]:
        regular_traceback.append(line)

    print("This error occured while using the facade API")
    print("Facade Traceback:")
    for line in facade_traceback:
        print(line)
    print("")
    print("Regular Traceback:")
    for line in regular_traceback:
        print(line)

# function to send and recieve data from the facade server
def _send_recieve(func_name, args, kwargs):
    b_data = pack_data([func_name, args, kwargs], True)
    sock.sendall(b_data)
    result = None
    b_result = []
    while True:
        packet = sock.recv(202400)
        if not packet: break
        b_result.append(packet)
        try:
            result = unpack_data(b_result)
            break
        except:
            pass
    if isinstance(result, list) and result[0] == "error":
        sys.excepthook = _exception_hook
        raise Exception(result[1])
    return result

# special function to open the OpenVSP GUI
def open_gui():""" + OPEN_GUI_DOC + r"""
    b_data = pack_data(['opengui', [], {}], True)
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

DECORATOR_CODE = """
# decorator for wrapping every function
def client_wrap(func):
    def wrapper(*args, **kwargs):
        return _send_recieve(func.__name__, args, kwargs)
    return wrapper
"""


def write_facade(file_path):
    module_name = file_path.split(".")[0]

    in_header_comment = True

    in_class_list = {}
    removed_class_list = {}
    for class_name in CLASSES_TO_REMOVE:
        in_class_list[class_name] = False
        removed_class_list[class_name] = False

    new_facade_string = ''
    new_facade_string += DECORATOR_CODE

    with open(file_path, 'r') as f:
        for line in f.readlines():
            if "**Placeholder start**" in line:
                break
            # adds comment to header
            if in_header_comment and not "#" in line:
                in_header_comment = False
                new_facade_string += "\n#\n# This file has been modified by the OpenVSP automated facade script\n"


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
    for class_name in CLASSES_TO_REMOVE:
        #special code that has not been made generalized
        new_facade_string += f"from openvsp.{module_name} import {class_name}\n"


    new_facade_string += CLIENT

    with open('facade.py', 'w') as f:
        f.write(new_facade_string)

def make_server():
    server_string = r"""
from threading import Thread, Event
import pickle
import traceback
import sys

#special code that is not generalizable
import openvsp_config
openvsp_config._IGNORE_IMPORTS = True
openvsp_config.LOAD_GRAPHICS = True
import openvsp as module

HOST = 'localhost'
PORT = 6000
event = Event()

def pack_data(data, is_command_list=False):
    def sub_pack(sub_data):
        new_data = sub_data
        if isinstance(sub_data, module.vec3d):
            new_data = {"name":'vec3d',
                "x":sub_data.x(),
                "y":sub_data.y(),
                "z":sub_data.z(),
            }
        elif isinstance(sub_data,list) or isinstance(sub_data, tuple):
            if len(sub_data) > 0:
                if isinstance(sub_data[0], module.vec3d):
                    new_data = {
                        "name": "vec3d_list",
                        "list": []
                    }
                    for p in sub_data:
                        thing = {
                            "x":p.x(),
                            "y":p.y(),
                            "z":p.z(),
                        }
                        new_data['list'].append(thing)
                elif sub_data[0] == "error":
                    pass
                # else:
                #     print(data)
                #     raise RuntimeError( f"unable to pack data of list containing type {type(data[0])}")

        return new_data

    if is_command_list:

        #commands look like this: [func_name (str), args (list [arg1, arg2, argn]), kwargs (dict keyword1: arg1, kw2: arg2)  ]
        # example
        #                               [comp_name,     args,       dict]
        # vsp.compvecpnt01(uv_array) -> ["compvepnt01", [uv_array], {}  ]
        #
        new_data = [data[0],[],{}]
        for value in data[1]:
            new_data[1].append(sub_pack(value))
        for key, value in data[2].items():
            new_data[2][key] = sub_pack(value)
        new_data[1] = tuple(new_data[1])
    else:
       new_data = sub_pack(data)
    b_data = pickle.dumps(new_data)
    return b_data

def unpack_data(b_data, is_command_list=False):
    def sub_unpack(sub_data):
        n_data = sub_data
        if isinstance(sub_data, dict):
            if sub_data['name'] == 'vec3d':
                n_data = module.vec3d(sub_data['x'],sub_data['y'],sub_data['z'])
            elif sub_data['name'] == 'vec3d_list':
                n_data = []
                for r in sub_data['list']:
                    n_data.append(module.vec3d(r['x'],r['y'],r['z']))
        return n_data

    data = pickle.loads(b"".join(b_data))
    if is_command_list:
        new_data = [data[0],[],{}]
        for value in data[1]:
            new_data[1].append(sub_unpack(value))
        for key, value in data[2].items():
            new_data[2][key] = sub_unpack(value)
        new_data[1] = tuple(new_data[1])
    else:
       new_data = sub_unpack(data)

    return new_data

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
                print("Connected by %s, %s"%(addr[0], addr[1]))
                while True:
                    b_data = []
                    while True:
                        try:
                            packet = conn.recv(1024)
                        except ConnectionResetError:
                            socket_open = False
                            break
                        if not packet: break
                        b_data.append(packet)
                        try:
                            data = unpack_data(b_data, is_command_list=True)
                            break
                        except (pickle.UnpicklingError, EOFError):
                            pass
                    if b_data == []:
                        break
                    if data[0] == 'opengui':
                        result = 0
                        b_result = pickle.dumps(result)
                        conn.sendall(b_result)
                        event.set()
                        continue

                    func_name = data[0]
                    args = data[1]
                    kwargs = data[2]
                    foo = getattr(module, func_name)
                    try:
                        module.Lock()
                        result = foo(*args, **kwargs)
                        module.Unlock()
                    except Exception as e:
                        exc_info = sys.exc_info()
                        result = ["error", ''.join(traceback.format_exception(*exc_info))]
                    b_result = pack_data(result)
                    try:
                        conn.sendall(b_result)
                    except ConnectionResetError:
                        socket_open = False
                        break
    print("socket closed")
if __name__ == "__main__":
    t = Thread(target=start_server, args=())
    t.start()
    module.Lock()
    module.Unlock()

    event.wait()
    module.InitGui()
    module.StartGui()
"""
    with open('facade_server.py', 'w') as f:
        f.write(server_string)

def modify_vsp_py(filepath):

    with open(filepath, 'a') as vsp_py:
        vsp_py.write(PLACEHOLDER_OPEN_GUI)



def make_vsp_facade(source_file):
    directory = os.path.dirname(source_file)
    file_name = os.path.basename(source_file)

    old_cwd = os.getcwd()
    os.chdir(directory)
    write_facade(file_name)
    make_server()
    os.chdir(old_cwd)

def add_placeholder_funcs(source_file):
    directory = os.path.dirname(source_file)
    file_name = os.path.basename(source_file)
    old_cwd = os.getcwd()
    os.chdir(directory)
    modify_vsp_py(file_name)
    os.chdir(old_cwd)

if __name__ == "__main__":
    source_file = sys.argv[1]
    make_facade = sys.argv[2]
    if make_facade == "True":
        make_vsp_facade(source_file)
        add_placeholder_funcs(source_file)
    else:
        add_placeholder_funcs(source_file)
