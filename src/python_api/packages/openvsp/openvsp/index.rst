OpenVSP Python API Documentation
=================================

OpenVSP includes an API written in C++ that exposes all of the functionality of the GUI to a programming interface. This allows
OpenVSP modeling and analysis tools to be run on headless systems, directly integrated with external software programs, and 
automated for trade studies and optimization purposes. The OpenVSP API & MATLAB/Python Integration presentation from 
the `2020 OpenVSP Workshop`_ is a good resource to learn more about the API. 
For specific API questions, the `OpenVSP Google Group`_ is available.

.. _2020 OpenVSP Workshop: http://openvsp.org/wiki/doku.php?id=workshop2020
.. _OpenVSP Google Group: https://groups.google.com/forum/#!forum/openvsp

Examples
---------
OpenVSP API examples are available in the **scripts** directory of the distribution. These example scripts are written in 
AngelScript, but map very closely for the Python API. CustomGeom examples, also written in Angelscipt, are available in the 
**CustomScripts** directory. An example for using the Python API can be found in **python/openvsp/openvsp/tests**. The 
matlab_api directory includes examples for the MATLAB API in the form of test suites. 


Python API Instructions
-----------------------
View the **README** file in the **python** directory of the distribution for instructions on Python API installation. Note, the Python
version must be the same as what OpenVSP was compiled with. For instance OpenVSP 3.21.2 Win64 requires Python 3.6-x64. If a different 
version of Python is desired, the user must compile OpenVSP themselves. 

 
Improvements
============

Users
-----
Users are encouraged to make use of the `GitHub Issue Tracker`_ if they have a suggestions, 
feature request, or bug report for the OpenVSP developers. Please add an issue if an API function or capability is missing, 
not working correctly, or poorly documented.

.. _GitHub Issue Tracker: https://github.com/OpenVSP/OpenVSP

Links
-----
 - `Wiki`_
 - `OpenVSP Main Page`_
 - `Google Group`_
 - `Source Code on GitHub`_

.. _Wiki: http://openvsp.org/wiki/doku.php
.. _OpenVSP Main Page: http://openvsp.org/
.. _Google Group: https://groups.google.com/forum/#!forum/openvsp
.. _Source Code on Github: https://github.com/OpenVSP/OpenVSP

Contents
=========

.. toctree::
   :maxdepth: 2

   openvsp

Indices and tables
==================

* :ref:`genindex`
* :ref:`modindex`
* :ref:`search`

