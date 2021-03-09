# CHARM Automation Python Package (using Python 3.9)

## Setup Instructions

1. Follow the installation instructions in README.md located in the `python`
directory of the OpenVSP distribution.  You will know you are in the correct
location if you see the files environment.yml, requirements-dev.txt, and
requirements-uninstall.txt.  A link to these instructions can be found in the
section below.

	NOTE: `...` in a directory path refers to the `python` directory location.

2. After completing Step 1, navigate to
`.../charm/charm/charm_fortran_utilities`

3. Create a new directory called "bin" at the same level as the "src" directory
found within the charm_fortran_utilities directory.

4. Change directories into the newly created "bin" directory.

5. Compile bg2charm_thick.f using your favorite compiler, such as gfortran.
	- For Windows (recommend using MINGW64)
	  - `gfortran -static -o bg2charm_thick.exe ../src/bg2charm_thick.f`
	- For MacOS/Linux
	  - `gfortran -o bg2charm_thick ../src/bg2charm_thick.f`

6. Confirm the newly compiled executable is in the recently created "bin" directory
and that it has executable rights.

7. Return to the `python` directory and then navigate to `.../openvsp/openvsp` and
ensure that you have `_vsp.pyd` (Windows) or `_vsp.so` (Mac/Linux), and `vsp.py`
in the directory.

8. Navigate to `.../charm/charm/test`

9. Ensure the new "vsppytools" conda environment is active using `conda activate
vsppytools`.

10. Execute `python testcharminput.py` If `testfile.vsp3` is generated
and it ran without a Python error then the setup is complete.

       Note: An Error Code echo that includes GetStringResults is expected.  This is
       a response from the OpenVSP Python API as it is searching each OpenVSP component
       for all the various types of geometry meta data.  Not all components contain the
       same types of geometry meta data.  For example, a wing cannot get reduced down to
       a disk like a propeller or rotor can.

## OpenVSP Python Packages Installation

Instructions for setting up the OpenVSP Python Packages can be found in
[README](../README.md).
