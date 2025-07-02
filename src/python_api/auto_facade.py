import os
import sys

IS_FACADE_DOC = """
    \"\"\"
    Returns True if the facade API is in use.


    .. code-block:: python

        is_facade = IsFacade()

    \"\"\"
"""
IS_GUI_RUNNING_DOC = """
    \"\"\"
    Returns True if the GUI event loop is running.


    .. code-block:: python

        is_gui_active = IsGUIRunning()

    \"\"\"
"""

PLACEHOLDER_FUNCS = "# **Placeholder start**\n" + "def IsFacade():" + IS_FACADE_DOC + "\n    return False\n"
PLACEHOLDER_FUNCS += "def IsGUIRunning():" + IS_GUI_RUNNING_DOC + "\n    return False\n"

CLIENT_HEAD = r"""
# Facade Code
# **********************************************************************************
import os
import sys
import socket
from time import sleep, time
import subprocess
import pickle
from openvsp.facade_server import pack_data, unpack_data
from traceback import format_exception
import openvsp_config
# Import the low-level C/C++ module
if __package__ or "." in __name__:
    from . import _vsp
else:
    import _vsp
# decorator for wrapping every function
def client_wrap(func):
    def wrapper(self, *args, **kwargs):
        return self._send_receive(func.__name__, args, kwargs)
    return wrapper
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

    print("This error occurred while using the facade API")
    print("Facade Traceback:")
    for line in facade_traceback:
        print(line)
    print("")
    print("Regular Traceback:")
    for line in regular_traceback:
        print(line)

class _vsp_server():
    def __init__(self, name, funcs=[], port=-1):
        self.server_name = name
        HOST = 'localhost'
        if port>0:
            self.port = port
        else:
            self.port = 0
        python_exe = None
        if "python" in os.path.basename(sys.executable):
            python_exe = sys.executable
        elif "python" in os.path.basename(os.__file__):
            python_exe = os.__file__
        else:
            python_exe = "python"
        server_file = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'facade_server.py')
        for i in range(openvsp_config.FACADE_SERVER_ATTEMPTS):
            try:
                self._proc = subprocess.Popen([python_exe, server_file, str(self.port), str(openvsp_config.LOAD_GRAPHICS)], stderr=subprocess.PIPE)
                start_time = time()
                timeout = openvsp_config.FACADE_SERVER_TIMEOUT  # seconds
                while True:
                    line = self._proc.stderr.readline().strip().decode()
                    # print(line)
                    if not line:
                        if self._proc.poll() is not None:
                            raise RuntimeError("Server process exited unexpectedly")
                        if time() - start_time > timeout:
                            raise TimeoutError("Waiting for server timed out")
                        continue  # No line, but process still alive wait more

                    if line.startswith("Server Socket Thread: Bound to"):
                        self.port = int(line.split()[6][:-2])
                        break
                break
            except Exception as e:
                print(f'Failed to start server, attempt {i+1}, trying again; "{str(e)}"')
                try:
                    self._proc.close()
                    self._proc = None
                except:
                    pass
        if self._proc is None:
            raise RuntimeError("Facade failed to start the server")
        self._sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self._sock.connect((HOST, self.port))

        for func in funcs:
            setattr(self, func.__name__, (lambda  *args, func=func, **kwargs: self._run_func(func, *args, **kwargs)))

"""

CLIENT_END = """
    # function to send and receive data from the facade server
    def _send_receive(self, func_name, args, kwargs):
        b_data = pack_data([func_name, args, kwargs], True)
        self._sock.sendall(b_data)
        result = None
        b_result = []
        while True:
            packet = self._sock.recv(202400)
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

    def IsFacade(self):
        \"\"\"
        Returns True if the facade API is in use.


        .. code-block:: python

            is_facade = IsFacade()

        \"\"\"

        return True
    def IsGUIRunning(self):
        \"\"\"
        Returns True if the GUI event loop is running.


        .. code-block:: python

            is_gui_active = IsGUIRunning()

        \"\"\"

        return self._send_receive('IsGUIRunning', [], {})

    def _run_func(self, func, *args, **kwargs):
        try:
            kwargs['vsp_instance'] = self
            return func(*args, **kwargs)
        except TypeError:
            kwargs.pop("vsp_instance")
            return func(*args, **kwargs)

    def _close_server(self):
        try:
            self._proc.terminate()
        except:
            pass
        try:
            self._sock.shutdown(socket.SHUT_RDWR)
            self._sock.close()
            self._sock.detach()
            del self._sock
        except:
            pass
        try:
            del self.t
        except:
            pass
    def __del__(self):
        try:
            self._proc.terminate()
        except:
            pass
        try:
            self._sock.shutdown(socket.SHUT_RDWR)
            self._sock.close()
            self._sock.detach()
            del self._sock
        except:
            pass
        try:
            del self.t
        except:
            pass


class _server_controller():
    def __init__(self) -> None:
        print("server controller initialized")
        self._name_to_server = {}
        self._port_to_name = {}
        self._name_to_port = {}
        self.name_num = 1
        self.funcs = []
    def start_vsp_instance(self, name=None, port=-1) -> _vsp_server:

        if not name:
            name = f"default_name_{self.name_num}"
            while name in self._name_to_server:
                self.name_num += 1
                name = f"default_name_{self.name_num}"

        assert isinstance(name,str), "Name must be a string"
        assert not name in self._name_to_server, f"Server with name {name} already exists"
        assert not port in self._port_to_name, f"Server with port {port} already exists"
        self._name_to_server[name] = new_server = _vsp_server(name, self.funcs, port=port)
        self._port_to_name[new_server.port] = name
        self._name_to_port[name] = new_server.port

        return new_server

    def get_vsp_instance(self, server):
        try:
            if isinstance(server, str):
                return self._name_to_server[server]
            elif server in self._name_to_server.values():
                    return server
        except:
            pass
        print("Warning: Could not find vsp_instance, returning singleton")
        return None

    def stop_vsp_instance(self, name=None, port=None):
        assert name or port, "please specify a name or a port"
        if port:
            server_name = self._port_to_name[port]
            server_port = port
        elif name:
            server_port = self._name_to_port[name]
            server_name = name
        if server_name == "vsp_singleton":
            print("Can't close vsp_singleton")
            return
        self._port_to_name.pop(server_port)
        self._name_to_port.pop(server_name)
        self._name_to_server[server_name]._close_server()
        del self._name_to_server[server_name]

    def set_functions(self, funcs):
        self.funcs = funcs

from openvsp.vsp import ErrorObj
from openvsp.vsp import ErrorMgrSingleton
from openvsp.vsp import vec3d
from openvsp.vsp import Matrix4d
vsp_servers = _server_controller()
"""
def write_facade(file_path):
    module_name = file_path.split(".")[0]

    print( "Writing facade for " + module_name + "." )

    new_facade_string = ''
    new_facade_string += CLIENT_HEAD

    with open(file_path, 'r') as f:
        in_swig_header = True
        in_class = False
        previous_indent = ''
        for line in f.readlines():
            if 'class ' in line:
                in_class = True
            elif in_class and len(line) > 0 and not line[0] in [' ', '\n', '\t']:
                in_class = False

            if in_class:
                continue
            elif 'swigregister' in line:
                continue
            #updating constants
            elif " = _vsp." in line and not "swig" in line:
                in_swig_header = False
                new_facade_string += '        self.' + line
                previous_indent = '        '

            #adding function decorator to every function
            elif (
                "def " in line
                and not "   def" in line
                and not "swig" in line
            ):
                new_facade_string += "    @client_wrap\n"
                ind = line.index("(") + 1
                new_facade_string += '    ' + line[:ind] + "self, " + line[ind:]
                previous_indent = '    '

            elif not in_swig_header:
                new_facade_string += previous_indent + line
    new_facade_string += CLIENT_END
    with open('facade.py', 'w') as f:
        f.write(new_facade_string)

def make_server():
    server_string = r"""
from threading import Thread, Event
import pickle
import traceback
import sys
from time import sleep

#special code that is not generalizable
import openvsp_config
openvsp_config._IGNORE_IMPORTS = True
try:
    openvsp_config.LOAD_GRAPHICS = (sys.argv[2] == 'True')
except IndexError:
    pass
import openvsp as module

HOST = 'localhost'
try:
    PORT = int(sys.argv[1])
except IndexError:
    PORT = 0
event = Event()
global gui_wait
gui_wait = True
global gui_active
gui_active = False
debug = False

def pack_data(data, is_command_list=False):
    def sub_pack(sub_data):
        new_data = sub_data
        if isinstance(sub_data, module.vec3d):
            new_data = {"name":'vec3d',
                "x":sub_data.x(),
                "y":sub_data.y(),
                "z":sub_data.z(),
            }
        elif isinstance(sub_data, list) or isinstance(sub_data, tuple):
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

        return new_data

    if is_command_list:

        #commands look like this: [func_name (str), args (list [arg1, arg2, argn]), kwargs (dict keyword1: arg1, kw2: arg2)  ]
        # example
        #                               [comp_name,     args,       dict]
        # vsp.compvecpnt01(uv_array) -> ["compvepnt01", [uv_array], {}  ]
        #
        new_data = [data[0], [], {}]
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
                n_data = module.vec3d(sub_data['x'], sub_data['y'], sub_data['z'])
            elif sub_data['name'] == 'vec3d_list':
                n_data = []
                for r in sub_data['list']:
                    n_data.append(module.vec3d(r['x'], r['y'], r['z']))
        return n_data

    data = pickle.loads(b"".join(b_data))
    if is_command_list:
        new_data = [data[0], [], {}]
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
    global gui_active
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        s.bind((HOST, PORT))
        s.listen()
        print(f"Server Socket Thread: Bound to {s.getsockname()}. Listening...", file=sys.stderr)
        conn, addr = s.accept()
        with conn:
            print("Server Socket Thread: Connected by %s, %s"%(addr[0], addr[1]))
            while True:
                b_data = []
                data = []
                # Wait for command
                while True:
                    try:
                        packet = conn.recv(1024)
                    except ConnectionResetError:
                        print("Socket ConnectionResetError")
                        break
                    if not packet: break
                    b_data.append(packet)
                    try:
                        data = unpack_data(b_data, is_command_list=True)
                        break
                    except (pickle.UnpicklingError, EOFError):
                        pass
                if b_data == [] or data == []:
                    print("Server Socket Thread: Unable to receive data from socket, closing server.")
                    break

                # Special functionality for StartGUI
                if data[0] == 'StartGUI':
                    if debug:
                        print("Server Socket Thread: StartGUI called")
                    if debug and event.is_set():
                        print("Server Socket Thread: The OpenVSP GUI should already be running")
                    result = 0
                    b_result = pack_data(result)
                    event.set()
                    if module.IsGUIBuild():
                        while not module.IsEventLoopRunning():
                            sleep(.01)

                # Special functionality for StopGUI
                elif data[0] == 'StopGUI':
                    if debug and not event.is_set():
                        print("Server Socket Thread: The OpenVSP GUI is not running")
                    if debug:
                        print("Server Socket Thread: About to call StopGUI()")
                    if module.IsEventLoopRunning():
                        module.StopGUI()
                    gui_active = False
                    if debug:
                        print("Server Socket Thread: After StopGUI() called")
                    result = 0
                    b_result = pack_data(result)

                # Special functionality for IsGUIRunning
                elif data[0] == 'IsGUIRunning':
                    result = gui_active
                    b_result = pack_data(result)

                # Regular functionality
                else:
                    func_name = data[0]
                    args = data[1]
                    kwargs = data[2]
                    foo = getattr(module, func_name)
                    try:
                        if debug:
                            print("Server Socket Thread: A1 Waiting for Lock")
                        if gui_active:
                            module.Lock()
                            if debug:
                                print("Server Socket Thread: A2 Lock obtained")
                        result = foo(*args, **kwargs)
                        if debug:
                            print("Server Socket Thread: A3 VSP function called")
                        if gui_active:
                            module.Unlock()
                            if debug:
                                print("Server Socket Thread: A4 Lock released")
                    except Exception as e:
                        exc_info = sys.exc_info()
                        result = ["error", ''.join(traceback.format_exception(*exc_info))]
                    b_result = pack_data(result)

                # Try to send response back
                try:
                    if debug:
                        print("Server Socket Thread: sending data back")
                    conn.sendall(b_result)
                except (ConnectionResetError, BrokenPipeError) as e:
                    print("Server Socket Thread: Unable to send data to socket, closing server.")
                    break

    print("Server Socket Thread: server closing")
    global gui_wait
    gui_wait = False
    event.set()
    module.StopGUI()
    print("Server Socket Thread: End of thread")


if __name__ == "__main__":
    t = Thread(target=start_server, args=())
    t.start()
    module.Lock()
    module.Unlock()
    while gui_wait:
        event.wait()
        if debug:
            print("Server GUI Thread: Starting GUI")
        if gui_wait: #makes sure this didnt change while waiting
            gui_active = True
            module.StartGUI()
        if debug:
            print("Server GUI Thread: GUI stopped")
        event.clear()
    print("Server GUI Thread: End of thread")

"""
    with open('facade_server.py', 'w') as f:
        f.write(server_string)

def modify_vsp_py(filepath):
    with open(filepath, 'r') as vsp_py:
        if PLACEHOLDER_FUNCS in vsp_py.read():
            return
    with open(filepath, 'a') as vsp_py:
        vsp_py.write(PLACEHOLDER_FUNCS)

def make_vsp_facade(source_file):
    directory = os.path.dirname(source_file)
    file_name = os.path.basename(source_file)

    old_cwd = os.getcwd()
    os.chdir(directory)
    write_facade(file_name)
    make_server()
    os.chdir(old_cwd)

def add_placeholder_funcs(source_file):
    print( "Writing placeholders to " + source_file + "." )
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
