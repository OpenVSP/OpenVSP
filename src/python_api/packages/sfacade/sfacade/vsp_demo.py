import os

from sfacade import make_server, make_facade

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
