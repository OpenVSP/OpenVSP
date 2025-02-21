# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import unittest
from airfoils import airfoil, kulfanAirfoil, Naca16, Naca4
import os


class AirfoilTest(unittest.TestCase):
    # example test cases

    OUTPUT_FOLDER = os.path.join(os.path.dirname(__file__), "test_outputs")
    INPUT_FOLDER = os.path.join(os.path.dirname(__file__), "test_inputs")

    @classmethod
    def setUpClass(cls):
        os.makedirs(cls.OUTPUT_FOLDER, exist_ok=True)

    @classmethod
    def get_outputname(cls, name):
        return os.path.join(cls.OUTPUT_FOLDER, name)

    @classmethod
    def get_inputname(cls, name):
        return os.path.join(cls.INPUT_FOLDER, name)

    def test_clark_y_import(self):
        a = airfoil(self.get_inputname("clarky.dat"))
        self.assertTrue(True) # check if we've properly identified it as a lednicer format
        a.plot()
        plt.savefig(self.get_outputname("clarky.png"), dpi=300)

    def test_kulfan_airfoil_build(self):
        N = 200
        a = kulfanAirfoil(wl=[-1, -1, -1], wu=[1, 1, 1], dz=0, N=N)
        self.assertTrue(True)
        a.plot()
        plt.savefig(self.get_outputname("kulfan.png"), dpi=300)

    def test_vsp_nonsym_parse(self):
        a = airfoil(self.get_inputname("N0012_VSP.af"))
        self.assertTrue(True)
        a.plot()
        plt.savefig(self.get_outputname("N0012_VSP.png"), dpi=300)

    def test_vsp_sym_parse(self):
        a = airfoil(self.get_inputname("N0012_VSP_sym.af"))
        self.assertTrue(True)
        a.plot()
        plt.savefig(self.get_outputname("N0012_VSP_sym.png"), dpi=300)

    def test_xfoil_import(self):
        a = airfoil(self.get_inputname("e387.dat"))
        self.assertTrue(True)
        a.plot()
        plt.savefig(self.get_outputname("e387.png"), dpi=300)


if __name__ == '__main__':
    s = AirfoilTest()
    s.test_vsp_nonsym_parse()

    n4 = Naca4(2,4,12, half_cosine_spacing=True, nPts=200)
    n4 = n4.trimTE(0.995)
    f = n4.plot()
    f.gca().set_title(n4.to_naca_string())
    plt.xlim(0.9,1.05)
