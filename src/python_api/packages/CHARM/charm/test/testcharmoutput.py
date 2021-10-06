import unittest
import os

path = os.path.dirname(__file__)


class Test(unittest.TestCase):

    def test_parse_perf_file(self):
        import charm.output as charmo

        perffile = os.path.join(path, "testperf.dat")

        perfdata = charmo.CharmPerfData(filename=perffile)

        print(perfdata.rotor_metadata)
        print(perfdata.row_data["CL2D"])


if __name__ == "__main__":
    t = Test()
    t.test_parse_perf_file()
