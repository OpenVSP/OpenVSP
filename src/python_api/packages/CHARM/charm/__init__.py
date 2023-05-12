# Copyright (c) 2023 United States Government as represented by the
# Administrator of the National Aeronautics and Space Administration.  All Other
# Rights Reserved.

# Licensed under the Apache License, Version 2.0 (the "License"); you may not
# use this file except in compliance with the License. You may obtain a copy of
# the License at

#      http://www.apache.org/licenses/LICENSE-2.0

# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
# License for the specific language governing permissions and limitations under
# the License.

_shortcopyright = """
================================================================================
The OpenVSP-to-CHARM Automation software is a custom Python package
that acts as an interface between OpenVSP, an open-source geometry tool, and
CHARM, a commercially available analysis tool for rotorcraft.

Copyright (c) 2023 United States Government as represented by the Administrator
of the National Aeronautics and Space Administration. No copyright is claimed in
the United States under Title 17, U.S. Code. All Other Rights Reserved.
================================================================================
"""

__copyright__ = """
================================================================================
The OpenVSP-to-CHARM Automation software is a custom Python package
that acts as an interface between OpenVSP, an open-source geometry tool, and
CHARM, a commercially available analysis tool for rotorcraft.

Copyright (c) 2023 United States Government as represented by the Administrator
of the National Aeronautics and Space Administration. No copyright is claimed in
the United States under Title 17, U.S. Code. All Other Rights Reserved.

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

(https://github.com/OpenVSP/OpenVSP/blob/1b60491d1f5454ce9bbfc3163fe84361a4ed3d65/src/python_api/packages/README.md)

*This software calls the following third-party software, which is subject to the
terms and conditions of its licensor, as applicable

Title-                    License (License URL)

SciPy-                    BSD (https://scipy.org/)
NumPy-                    BSD (https://numpy.org/)
MatPlotLib-               MatPlotLib (https://matplotlib.org/stable/users/project/license.html)(PSF)
SciPy-                    BSD (https://scipy.org/)
Pandas-                   BSD (https://pandas.pydata.org/)
FortranFormat-            MIT (https://github.com/brendanarnold/py-fortranformat/blob/master/LICENSE)

*Note about OpenVSP and CHARM:

OpenVSP is available from the U.S. Government, NASA under NOSA 1.3
(https://openvsp.org/license.shtml)

CHARM (Comprehensive Hierarchical Aeromechanics Rotorcraft Model) is available
from Continuum Dynamics, Inc. (https://continuum-dynamics.com/product/#charm)
================================================================================
"""


def copyright(long=False):
    if not long:
        print(_shortcopyright)
        print("\nTo see full copyright information, run `python -m charm --copyright`\n")
        print("\n***  OpenVSP-to-CHARM  Automation  ***\n")
    else:
        print(__copyright__)


copyright()
