import numpy as np
import openvsp as vsp
import matplotlib.pyplot as plt
import os


def test_rst():
    # Open VSP geometry
    vsp_file = 'tests/wing.vsp3'
    vsp.ReadVSPFile(vsp_file)
    all_geoms = vsp.FindGeoms()
    geom_id = all_geoms[0]

    # Create a grid of test points in rst for wing
    nr = 25
    ns = 5
    small = .01
    r_array = np.linspace(0.25+small, 0.75-small, nr)
    s_array = np.linspace(0.0, 0.5, ns)
    rst0 = np.zeros([nr, ns, 3])
    rst0[:, :, 0], rst0[:, :, 1] = np.meshgrid(r_array, s_array, indexing='ij')
    rst0[:, :, 2] = 0.5
    rst0 = rst0.reshape(-1, 3)
    # Find the test points xyz locations
    pts0 = vsp.CompVecPntRST(geom_id, 0, rst0[:, 0], rst0[:, 1], rst0[:, 2])
    xyz0 = np.array([[pnt.x(), pnt.y(), pnt.z()] for pnt in pts0])

    r1 = np.zeros([nr*ns, 1])
    s1 = np.zeros([nr*ns, 1])
    t1 = np.zeros([nr*ns, 1])
    d1 = np.zeros([nr*ns, 1])

    # Preform RST projection on test points (this should give the rst points we defined in the step above)
    r1, s1, t1, d1 = vsp.FindRSTVec(geom_id, 0, pts0)
    # Evaluate the new rst projections in xyz (again, these should be the same points we found above)
    pts1 = vsp.CompVecPntRST(geom_id, 0, r1, s1, t1)
    xyz1 = np.array([[pnt.x(), pnt.y(), pnt.z()] for pnt in pts1])

    assert all( di < 1.0e-6 for di in d1 )

    if not "PYTEST_CURRENT_TEST" in os.environ:
        # Plot the test points vs projected points in xy
        plt.figure(0)
        plt.title('Wing projection test')
        plt.scatter(xyz0[:, 0], xyz0[:, 1])
        plt.scatter(xyz1[:, 0], xyz1[:, 1])
        plt.xlabel('x')
        plt.ylabel('y')
        plt.legend(['Original points', 'rst Projections'])
        plt.show()

        # Plot the test points vs projected points in xy
        plt.figure(1)
        plt.axes(projection ="3d")
        plt.title('Wing projection test')
        plt.scatter(xyz0[:, 0], xyz0[:, 1], xyz0[:,2])
        plt.scatter(xyz1[:, 0], xyz1[:, 1], xyz1[:,2])
        plt.xlabel('x')
        plt.ylabel('y')
        plt.ylabel('z')
        plt.legend(['Original points', 'rst Projections'])
        plt.show()

        plt.figure(2)
        plt.title('Wing projection test')
        plt.scatter(rst0[:,0],d1)
        plt.xlabel('r')
        plt.ylabel('dist')
        plt.show()

        plt.figure(3)
        plt.title('Wing projection test')
        plt.scatter(rst0[:,1],d1)
        plt.xlabel('s')
        plt.ylabel('dist')
        plt.show()


if __name__ == "__main__":
    test_rst()
