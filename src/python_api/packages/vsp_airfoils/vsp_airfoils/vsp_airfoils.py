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

import os
import matplotlib.pyplot as plt
from math import pi, cos, sin, factorial
from enum import Enum
import numpy as np
import copy
import math

class Segment(np.ndarray):
    def __new__(cls, *args, **kwargs):
        return super(Segment, cls).__new__(cls, *args, **kwargs)

    @property
    def direction(self):
        dV = self[1,:] - self[0,:]
        if (dV[0] == 0) or (dV[1] == 0): # direction is undefined
            return float("nan")
        # if both positive or both negative,, going clockwise
        elif np.sign(dV[0]) == np.sign(dV[1]):
            return 1
        # if x is negative and y is positive or  x is positive and y is negative, counterclockwise
        elif np.sign(dV[0]) != np.sign(dV[1]):
            return -1
        else:
            raise ValueError("Direction could not be identified")
class AirfoilType(Enum):
    lednicer = 1
    selig = 2
    vsp_sym = 3
    vsp_nonsym = 4
    pointwise = 5
class airfoil():
    # def __init__(self, file=None, coords=None, type=None)
    # if type is none, try to infer what the type is
    # airfoil should fundamentally be a series of line segments.

    def __init__(self, file=None,segments=None, type=None, **kwargs):
        """

        :param str file: path to the airfoil coordinate file to load (optional)
        :param ndaaray coords: nx2 numpy array containing the (x,y) pairs of airfoil coordinates (optional)
        :param float maxX: the maximum x value for the coordinates (default 1.1)
        """

        self.type = type
        self.segments = []
        self.file = file

        if self.file is not None:
            if self.type is not None:
                self.importAirfoil(self.type, **kwargs)
            else:
                for type in AirfoilType:
                    try:
                        self.importAirfoil(type, **kwargs)
                        break
                    except ValueError:
                        pass

        else:
            self.segments = segments
    @property
    def x(self):
        array = np.array([])
        for s in self.segments:
            array = np.hstack( (array, s[:,0]) )

        return array

    @property
    def y(self):
        array = np.array([])
        for s in self.segments:
            array = np.hstack((array, s[:,1]))
        return array

    @property
    def coords(self):
        return np.array([self.x, self.y])

    def importAirfoil(self, type, delimiter=" ", **kwargs):
        '''

        :param type: AirfoilType airfoil type
        :param delimiter: file delimiter
        :param kwargs: (optional) can pass a maxX parameter to trim x outputs past a value (default 1.1)
        :return: None
        '''
        maxX = kwargs.get('maxX', 1.1)

        if type == AirfoilType.selig:
            d = np.genfromtxt(self.file, skip_header=1, delimiter=delimiter)
            if d[:, 0].max() > maxX:
                raise ValueError(
                    "Airfoil coordinates outside maxX value. Airfoil Max X = {}, maxX = {}".format(d[:, 0].max(), maxX))

            dx = np.diff(d[:,0])
            switch_indx = np.where(np.sign(dx) != np.sign(dx[0]))[0][0]

            self.segments.append(d[0:switch_indx+1,:])
            self.segments.append(d[switch_indx:,:])

        elif type == AirfoilType.lednicer:
            d = np.genfromtxt(self.file, comments="#", skip_header=1)

            #usually the only point > 1 is the number of points, which indicates a two-segment file
            # remove points outside of 1+tol
            d = d[d[:, 0] <= maxX, :]

            switches = self.findSegments(d[:,0])
            self.segments.append( d[0:switches[0],:] )
            self.segments.append( d[switches[0]:,:] )

        elif type == AirfoilType.vsp_sym:
            d = np.genfromtxt(self.file, skip_header=4)

            self.segments.append( d)
            d2 = copy.deepcopy(d)
            d2[:,1] = -d2[:,1]
            self.segments.append(d2)

        elif type == AirfoilType.vsp_nonsym:
            d = np.genfromtxt(self.file, skip_header=5)
            switches = self.findSegments(d[:,0])
            self.segments.append( d[0:switches[0],:] )
            self.segments.append( d[switches[0]:,:] )

        else:
            raise ValueError("Unknown airfoil file type")

        return

    def plot(self, ax=None):
        """Plot the airfoil

        :param ax: optional axis handle on which to plot
        :return: matplotlib.pyplot axis handle of the plot
        """
        if ax is None:
            fig = plt.figure()
            ax = plt.gca()
        ax.plot(self.x, self.y)
        ax.axis('equal')
        ax.set_xlabel('X')
        ax.set_ylabel('Y')
        ax.set_title('Airfoil')
        ax.grid(True)
        return ax

    def toVSP(self):
        """Converts the airfoil to VSP format

        :return: none
        """
        raise NotImplementedError("not yet implemented")

    def toXfoil(self, file, title="exported airfoil"):
        """

        :param str file: file path to which the coordinates will be saved
        :param str title: title to be inserted in the first line of the file (optional, default="exported airfoil")
        :return: absolute path to the resulting file
        """
        if not os.path.isabs(file):
            file = os.path.abspath(file)
        np.savetxt(file, self.coords.T, delimiter=" ", header=title, comments="")
        return file

    def toLednicer(self):
        """

        :param str file: file path to which the coordinates will be saved
        :return: absolute path to the resulting file
        """
        raise NotImplementedError("Not yet implemented")

    def seg2str(self, seg, writeNPts=False, z=None):
        '''

        :param seg: array containing the airfoil segment
        :param writeNPts: boolean, whether or not to output the number of points at the top of the file
        :param z: z value for this airfoil. If None, assumes a 2d airfoil
        :return: string array containing the segment.
        '''
        nPts = seg.shape[0]
        strArr = []
        # write number of pts in this segment
        if writeNPts:
            strArr.append( "{}\n".format(nPts))

        # write line by line, appending a zero
        for i in range(0, nPts):
            if z is not None:
                strArr.append( "{} {} {}\n".format(seg[i, 0], seg[i, 1], z) )
            else:
                strArr.append( "{} {}\n".format(seg[i, 0], seg[i, 1]) )
        return strArr


    def calcDir(self, v1, v2):
        '''
        Calculates the direction of two points. This is setup for airfoils, so cases like flat lines or vertical lines make guesses based on typical airfoils.
        :param v1: first (x,y) point
        :param v2: second (x,y) point
        :return: either +1 or -1
        '''
        dV = v2 - v1
        # direction is undefined, we'll do some ghetto math based on airfoil assumptions
        if (dV[0] == 0):
            return -np.sign(v1[0]) * np.sign(dV[1])
        # direction is undefined, we'll do some ghetto math based on airfoil assumptions
        elif (dV[1] == 0):
            return np.sign(v1[1]) * np.sign(dV[0])
        # if both positive or both negative,, going clockwise
        elif np.sign(dV[0]) == np.sign(dV[1]):
            return 1
        # if x is negative and y is positive or  x is positive and y is negative, counterclockwise
        elif np.sign(dV[0]) != np.sign(dV[1]):
            return -1
        else:
            raise ValueError("Direction could not be identified")

    def toPointwise(self, fname, z=0., dir=-1):
        '''
        writes the airfoil to pointwise format
        :param fname: file name
        :param z: z value (Default, 0)
        :param dir: airfoil direction (default, -1)
        :return: None
        '''
        with open(fname, "w") as f:
            for i, seg in enumerate(self.segments):
                angle = self.calcDir(seg[0,:], seg[1,:])
                if np.sign(angle) != dir:
                    seg = np.flipud(seg)
                strArr = self.seg2str(seg, writeNPts=True, z=z)

                f.writelines(strArr)

    def invert(self):
        '''
        Inverts an airfoil's y coordinates
        :return: inverted airfoil object
        '''
        segs = copy.deepcopy(self.segments)
        for s in segs:
            s[:,1] = -s[:,1]

        return airfoil(segments=segs)

    def trimTE(self, maxX, nPts=9):
        '''
        Trims the trailing edge of an airfoil
        :param maxX: x at which to trim the airfoil
        :param nPts: number of points to place on the blunt trailing edge
        :return: new airfoil object with the blunt trailing edge
        '''
        from scipy.interpolate import interp1d
        segs = copy.deepcopy(self.segments)
        for i,s in enumerate( segs):
            f = interp1d(s[:,0], s[:,1], kind='cubic')
            mask = s[:,0] < maxX
            # check if we're cutting off the front or the back
            arr = s[mask,:]
            endPt = np.array([maxX, f(maxX)])
            if mask[0]:
                # we need to append
                segs[i] = np.concatenate([ arr, np.atleast_2d(endPt)])
            elif mask[-1]:
                # we need to prepend
                segs[i] = np.concatenate([ np.atleast_2d(endPt), arr])


        # add segment for flat TE
        segs.append( np.empty((nPts,2)))
        indx0 = np.where( segs[0][:,0] == maxX)[0]
        indx1 = np.where( segs[1][:,0] == maxX)[0]
        dy = np.linspace( segs[1][indx1,1], segs[0][indx0,1],nPts).T
        dx = np.ones(dy.shape) * maxX
        segs[-1][:,0] = dx
        segs[-1][:,1] = dy

        newCopy = copy.deepcopy(self)
        newCopy.segments = segs
        return newCopy

    def findSegments(self, x):
        '''
        Finds the segments in the airfoil
        :param x:
        :return:
        '''
        delta = np.sign(x[0:-1] - x[1:])
        switches = np.where(delta != delta[0])[0]
        switches = [s+1 for s in switches]
        return switches

class kulfanAirfoil(airfoil):
    def __init__(self, wl=[-1, -1, -1], wu=[1, 1, 1], dz=0, N=200):
        """

        :param wl: weights for the lower surface (default=[-1,-1,-1])
        :param wu: weights for the upper surface (default=[1,1,1])
        :param dz:
        :param N: number of output points
        """
        self.wl = wl
        self.wu = wu
        self.dz = dz
        self.N = N
        # self.coordinate = np.zeros(N)

        # generate the airfoil coordinates
        x, y, coords = self.__airfoil_coor()
        self.segments = [coords]

    def __airfoil_coor(self):
        """

        :return: (x,y,coords) tuple containing the x coordinates, the y coordinates, and the concatenated coordinates
        """
        wl = self.wl
        wu = self.wu
        dz = self.dz
        N = self.N

        # Create x coordinate
        x = np.ones((N, 1))
        y = np.zeros((N, 1))
        zeta = np.zeros((N, 1))

        for i in range(0, N):
            zeta[i] = 2 * pi / N * i
            x[i] = 0.5 * (cos(zeta[i]) + 1)

        # N1 and N2 parameters (N1 = 0.5 and N2 = 1 for airfoil shape)
        N1 = 0.5
        N2 = 1

        center_loc = np.where(x == 0)  # Used to separate upper and lower surfaces
        center_loc = center_loc[0][0]

        xl = np.zeros(center_loc)
        xu = np.zeros(N - center_loc)

        for i in range(len(xl)):
            xl[i] = x[i]  # Lower surface x-coordinates
        for i in range(len(xu)):
            xu[i] = x[i + center_loc]  # Upper surface x-coordinates

        yl = self.__ClassShape(wl, xl, N1, N2, -dz)  # Call ClassShape function to determine lower surface y-coordinates
        yu = self.__ClassShape(wu, xu, N1, N2, dz)  # Call ClassShape function to determine upper surface y-coordinates

        y = np.hstack((yl, yu))  # Combine upper and lower y coordinates
        x = x.reshape(-1)
        coords = np.vstack( (x, y) ).T # Combine x and y into single output
        return x, y, coords

    # Function to calculate class and shape function
    def toCST(self):
        """

        :return:
        """
        return self
    def __ClassShape(self, w, x, N1, N2, dz):
        """

        :param w: weight function
        :param x: x locations at which y will be calculated
        :param N1:
        :param N2:
        :param dz:
        :return:
        """
        # Class function; taking input of N1 and N2
        C = np.zeros(len(x))
        for i in range(len(x)):
            C[i] = x[i] ** N1 * ((1 - x[i]) ** N2)

        # Shape function; using Bernstein Polynomials
        n = len(w) - 1  # Order of Bernstein polynomials

        K = np.zeros(n + 1)
        for i in range(0, n + 1):
            K[i] = factorial(n) / (factorial(i) * (factorial((n) - (i))))

        S = np.zeros(len(x))
        for i in range(len(x)):
            S[i] = 0
            for j in range(0, n + 1):
                S[i] += w[j] * K[j] * x[i] ** (j) * ((1 - x[i]) ** (n - (j)))

        # Calculate y output
        y = np.zeros(len(x))
        for i in range(len(y)):
            y[i] = C[i] * S[i] + x[i] * dz

        return y


class Naca4(airfoil):
    def __init__(self, max_camber=0, camb_loc=0, max_thick=10, **kwargs):
        """Generates a NACA 4-series airfoil (eg 2412)

        :param max_camber: max camber value (eg 2)
        :param camb_loc: location of max camber (eg 4)
        :param max_thick: max thickness (eg 12)
        """
        self.max_camber = max_camber
        self.camb_loc = camb_loc
        self.max_thick = max_thick

        self.segments = self.genSegs(**kwargs)

    def genSegs(self,  nPts=100, finite_TE=False, half_cosine_spacing=False):
        """
        Returns 2*n+1 points in [0 1] for the given 4 digit NACA number string
        """

        m = self.max_camber / 100.0
        p = self.camb_loc / 10.0
        t = self.max_thick / 100.0

        a0 = +0.2969
        a1 = -0.1260
        a2 = -0.3516
        a3 = +0.2843

        if finite_TE:
            a4 = -0.1015  # For finite thick TE
        else:
            a4 = -0.1036  # For zero thick TE

        if half_cosine_spacing:
            beta = np.linspace(0.0, pi, nPts + 1)
            x = [(0.5 * (1.0 - cos(xx))) for xx in beta]  # Half cosine based spacing
        else:
            x = np.linspace(0.0, 1.0, nPts + 1)

        yt = [5 * t * (a0 * math.sqrt(xx) + a1 * xx + a2 * pow(xx, 2) + a3 * pow(xx, 3) + a4 * pow(xx, 4)) for xx in x]

        xc1 = [xx for xx in x if xx <= p]
        xc2 = [xx for xx in x if xx > p]

        if p == 0:
            xu = x
            yu = yt

            xl = x
            yl = [-xx for xx in yt]

            xc = xc1 + xc2
            zc = [0] * len(xc)
        else:
            yc1 = [m / pow(p, 2) * xx * (2 * p - xx) for xx in xc1]
            yc2 = [m / pow(1 - p, 2) * (1 - 2 * p + xx) * (1 - xx) for xx in xc2]
            zc = yc1 + yc2

            dyc1_dx = [m / pow(p, 2) * (2 * p - 2 * xx) for xx in xc1]
            dyc2_dx = [m / pow(1 - p, 2) * (2 * p - 2 * xx) for xx in xc2]
            dyc_dx = dyc1_dx + dyc2_dx

            theta = [math.atan(xx) for xx in dyc_dx]

            xu = [xx - yy * sin(zz) for xx, yy, zz in zip(x, yt, theta)]
            yu = [xx + yy * cos(zz) for xx, yy, zz in zip(zc, yt, theta)]

            xl = [xx + yy * sin(zz) for xx, yy, zz in zip(x, yt, theta)]
            yl = [xx - yy * cos(zz) for xx, yy, zz in zip(zc, yt, theta)]

        upper = np.array( [xu, yu]).T
        lower = np.array( [xl, yl]).T

        return [upper, lower]

    def to_naca_string(self):
        """Saves airfoil to a NACA string

        :return:
        """
        string_list = [
            '%d' % self.max_camber,
            '%d' % self.camb_loc,
            '%02d' % self.max_thick
            ]
        return ''.join(string_list)


class VspSeligExport(airfoil):
    def __init__(self, file):
        '''
        VSP Selig airfoil class
        :param file: VSP selig airfoil file
        '''
        super().__init__(file, type=AirfoilType.selig, delimiter=" ")

        self.geom_name = None
        self.geom_id = None
        self.airfoil_index = None
        self.xsec_flag = None
        self.xsec_index = None
        self.xsec_surf_id = None
        self.foil_surf_u_value = None
        self.global_u_value = None
        self.le_point = np.empty(3)
        self.te_point = np.empty(3)
        self.chord = None

        #self.segments = self.importAirfoil(type=AirfoilType.selig, delimiter=",")

class Naca16(airfoil):
    def __init__(self, t_ov_c, cl, xMaxThick=0.5, nPts=150):
        '''

        :param t_ov_c: thickness to chord ratio of this airfoil
        :param cl: design cl
        :param xMaxThick: x/C location of max thickness
        :param nPts: number of points
        '''
        super().__init__(segments=[])
        self.xMaxThick = xMaxThick
        self.nPts = nPts
        self.cl = cl
        self.t_ov_c = t_ov_c

        x = np.linspace(0,1,self.nPts)
        yc = -0.079577 * self.cl * (x*np.log(x) + (1-x)*np.log(x)*(1-x))
        yc[0] = 0

        yt = np.empty(x.shape)
        yb = np.empty(yt.shape)

        for i in range(0,x.shape[0]):
            if x[i] <= self.xMaxThick:
                yt[i] = yc[i] + 0.01*self.t_ov_c * (0.989665*np.power(x[i],0.5) - 0.239250*x[i] - 0.041000*np.power(x[i],2) - 0.559400*np.power(x[i],3))
                yb[i] = yc[i] - 0.01*self.t_ov_c * (0.989665*np.power(x[i],0.5) - 0.239250*x[i] - 0.041000*np.power(x[i],2) - 0.559400*np.power(x[i],3))

            else:
                yt[i] = yc[i] + 0.01*self.t_ov_c * (0.01 + 2.325*(1-x[i]) - 3.42 * np.power(1-x[i],2) + 1.46*np.power(1-x[i],3))
                yb[i] = yc[i] - 0.01*self.t_ov_c * (0.989665*np.power(x[i],0.5) - 0.239250*x[i] - 0.041000*np.power(x[i],2) - 0.559400*np.power(x[i],3))


        plt.plot(x,yc)
        plt.figure()
        plt.plot(x,yt)
        print(yt.max())
        plt.figure()
        plt.plot(x,yb)
        plt.show()
        self.LERadius = np.power(0.004897,2)*np.power(self.t_ov_c,2)
        #yt[0] = yc[0] + self.LERadius
        #yb[0] = yc[0] - self.LERadius
        self.segments.append( np.vstack((x,yt)).T)
        self.segments.append( np.vstack((x,yb)).T)