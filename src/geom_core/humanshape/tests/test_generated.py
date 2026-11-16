#
# This file is released under the terms of the NASA Open Source Agreement (NOSA)
# version 1.3 as detailed in the LICENSE file which accompanies this software.
#
# The generated headers are current: each records the SHA-1 of its inputs, which must be those of
# the headers it was made from.  Regenerate in order (human_bezier.py, then human_tets.py) when one
# fails.
#
#     python3 -m pytest src/geom_core/humanshape/tests
#

import hashlib
import os
import re

HERE = os.path.dirname(os.path.abspath(__file__))
GEOM_CORE = os.path.dirname(os.path.dirname(HERE))


def check_inputs(generated):
    with open(os.path.join(GEOM_CORE, generated)) as f:
        head = f.read(8192)
    found = re.findall(r'// Input (\S+) SHA-1 ([0-9a-f]{40})', head)
    assert found, generated + ' records no inputs'
    for name, digest in found:
        with open(os.path.join(GEOM_CORE, name), 'rb') as f:
            assert hashlib.sha1(f.read()).hexdigest() == digest, '%s is stale: %s changed' % (generated, name)


def test_the_bezier_data_are_current():
    check_inputs('HumanBezierData.h')


def test_the_tet_data_are_current():
    check_inputs('HumanTetData.h')
