from threading import Thread, Event
from time import sleep

import openvsp as vsp



event = Event()


def start_gui():
    print("Before InitStartGui() in Python")
    # vsp.InitStartGui()
    vsp.InitGui()
    #sleep(5)
    print("before start gui")
    vsp.StartGui()
    print("after start gui")
    #sleep(1)
    event.set()

print("before thread")
t = Thread(target=start_gui, args=())
t.start()

sleep(5)

#start_gui()
print("after thread")
#vsp.ReadVSPFile(r'ElectricGA.vsp3')
i = 0
while True:
    if event.is_set():
        print("ending main thread")
        break
    else:
        sleep(1)
        i += 1
        # if i > 10:
            #geom_list = vsp.FindGeoms()
            #vsp.SetParmVal(geom_list[0], 'X_Location', 'XForm', i*3)
            #vsp.Update()
            #vsp.UpdateGui()
        print("main thread continues")
# t.join()
# print(my_var)