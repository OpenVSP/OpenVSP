import subprocess
import sys
from pathlib import Path

def main():
    scriptpath = str(Path(__file__).parent.resolve())
    print(scriptpath)
    p1 = subprocess.Popen([sys.executable, scriptpath+'/SuperDeltaTest.py'],start_new_session=True)
    p2 = subprocess.Popen([sys.executable, scriptpath+'/EllipseTest.py'],start_new_session=True)
    p3 = subprocess.Popen([sys.executable, scriptpath+'/VKTTest.py'],start_new_session=True)
    p4 = subprocess.Popen([sys.executable, scriptpath+'/WarrenTest.py'],start_new_session=True)
    p5 = subprocess.Popen([sys.executable, scriptpath+'/BertinSmithTest.py'],start_new_session=True)
    p6 = subprocess.Popen([sys.executable, scriptpath+'/SweptTest.py'],start_new_session=True)
    p7 = subprocess.Popen([sys.executable, scriptpath+'/HersheyTest.py'],start_new_session=True)
    plist = [p1,p2,p3,p4,p5,p6,p7]
    exit_codes = [p.wait() for p in plist]
    print(exit_codes)
    return


if __name__ == '__main__':
    main()