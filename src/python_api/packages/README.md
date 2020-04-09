Copyright (c) 2018 Uber Technologies, Inc.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

# OpenVSP Python Packages


The python packages included extend the OpenVSP Python API.

### Installation Instructions
1. Make sure the vsp shared library (`_vsp.so` or `_vsp.pyd`) and python wrapper `vsp.py` from
the OpenVSP distribution built for your operating system and python environment have been placed in `openvsp\openvsp\`.

2. Build the `bg2charm_thick` fortran utility
    ```
    cd CHARM/charm/charm_fortran_utilities/build
    gfortran -o bg2charm_thick ../src/bg2charm_thick.f
    ```

    * On Windows with MinGW, you likely want to force the use of static libraries to prevent problems finding them later.
        ```
        gfortran -static -o bg2charm_thick.exe ../src/bg2charm_thick.f
        ```

3. Install the python packages (you should start in the directory this README file)
    ```bash
    pip install -r requirements.txt # If you are not going to modify the packages
    ```
    ```bash
    pip install -r requirements-dev.txt # If you want to modify the python packages
    ```
