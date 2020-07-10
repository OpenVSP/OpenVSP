# Copyright (c) 2020 Uber Technologies, Inc.

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


import numpy as np
from scipy.interpolate import interp1d
import utilities.units as u
from collections import namedtuple

'''
All altitude inputs are geometric altitude

'''
class Atmosphere():
    output = namedtuple('output', 'T,P,rho,sos,nu,eta,theta,delta,sigma')
    rEarth_km = 6378.0
    t0 = 288.15 # rankine
    p0 = 101325.0 # pascals
    rho0 = 1.2250 # kg/m3
    sos0 = 340.294 #m/s

    def __init__(self,  deltaT_degC=0):
        '''

        :param deltaT_degC: delta temperature from standard day, in degC (default=0)
        '''
        self.deltaT_degC = deltaT_degC

    def geometric2geopotential(self,z_m):
        '''
        convert geometric altitude to geo-potential altitude

        :param z_m: geometric altitude, in m
        :return:
        '''
        rEarth_m = self.rEarth_km * 1000.
        return z_m*rEarth_m/(z_m+rEarth_m)

    def sutherlands(self,theta):
        '''
        sutherland's equation for absolute viscosity

        :param theta: temperature ratio
        :return: absolute viscosity, in kg/(m-sec)
        '''
        temp = self.t0 * theta
        suthConst = 110.4  # kelvin
        betavisc = 1.458e-6  # N-sec/ (m^2 * sqrt(K))
        return betavisc * np.power(temp,3.0/2)/(temp+suthConst)   #kg/(m-sec)

    def getRatios(self,z_m):
        raise NotImplementedError("getRatios method must be implemented by sub-class.")

    def T(self,z_m):
        '''

        :param z_m:  geometric altitude, in m
        :return: temperature, in Kelvin
        '''
        theta, _, _ = self.getRatios(z_m)
        return theta * self.t0

    def P(self,z_m):
        '''

        :param z_m:  geometric altitude, in m
        :return: pressure, in Pascals
        '''
        _, delta, _ = self.getRatios(z_m)
        return delta * self.p0

    def rho(self,z_m):
        '''

        :param z_m:  geometric altitude, in m
        :return: density, in kg/m^3
        '''
        _, _, sigma = self.getRatios(z_m)
        return sigma * self.rho0

    def sos(self,z_m):
        '''

        :param z_m: geometric altitude, in m
        :return:
        '''
        theta, _, _ = self.getRatios(z_m)
        return np.sqrt(theta) * self.sos0

    def nu(self,z_m):
        '''

        :param z_m: geometric altitude, in m
        :return: absolute viscosity, in kg/(m-sec)
        '''
        theta, _, _ = self.getRatios(z_m)
        return self.sutherlands(theta)

    def eta(self,z_m):
        '''

        :param z_m: geometric altitude, in m
        :return: dynamic viscosity, in Pa-sec
        '''
        nu = self.nu( z_m)
        return nu/self.rho(z_m)

    def calc(self,z_m):
        '''

        :param z_m: geometric altitude, in m
        :return: namedtuple containing all quantities at this z_m
        '''
        t, d, s = self.getRatios(z_m)
        return self.output(self.T(z_m), self.P(z_m),self.rho(z_m), self.sos(z_m), self.nu(z_m),self.eta(z_m),t,d,s)

class stdatm1976(Atmosphere):
    htab = np.array([0.0, 11.0, 20.0, 32.0, 47.0, 51.0, 71.0, 84.852]) # geopotential altitude, km
    ttab = np.array([288.15, 216.65, 216.65, 228.65, 270.65, 270.65, 214.65, 186.946]) # corresponding temperatures, kelvin
    ptab = np.array([1.0, 2.233611E-1, 5.403295E-2, 8.5666784E-3, 1.0945601E-3, 6.6063531E-4, 3.9046834E-5, 3.68501E-6]) # corresponding pressures, atmospheres
    gtab = np.array([-6.5, 0.0, 1.0, 2.8, 0.0, -2.8, -2.0, 0.0]) # corresponding temperature gradients, kelvin/ft
    gmr = 34.163195 #gas constant,  TODO: unknown units on this, need to figure this out

    def __init__(self, deltaT_degC=0):
        super().__init__(deltaT_degC=deltaT_degC)

    def getRatios(self,z_m):
        '''

        :param z_m:  geometric altitudes, in m
        :return: tuple of arrays with temperature, pressure, and density ratios
        '''
        if isinstance(z_m, np.ndarray):
            dtype = z_m.dtype
            shape = z_m.shape
            theta = np.empty(shape, dtype=dtype)
            delta = np.empty(shape, dtype=dtype)
            sigma = np.empty(shape, dtype=dtype)

            for i,z in enumerate(z_m):
                t,d,s = self.getRatio(z)
                theta[i] = t
                delta[i] = d
                sigma[i] = s
        else:
            theta,delta,sigma = self.getRatio(z_m)

        #TODO: support lists
        return theta, delta, sigma
    def getRatio(self,z_m):
        '''

        :param z_m:  geometric altitude, in m
        :return: tuple with temperature, pressure, and density ratios
        '''
        h = self.geometric2geopotential(z_m) # convert from geometric to geopotential
        h = h / 1000. # convert to km
        if (h > self.htab.max()) or (h<self.htab.min()):
            raise ValueError("Geopotential altitude must be between {} m and {} m".format(self.htab.min(),self.htab.max()))

        if h == self.htab[0]:
            i = 0
        else:
            i = np.searchsorted( self.htab, h)-1 # move back one so the calcs are base + delta

        tgrad = self.gtab[i]
        tbase = self.ttab[i]
        deltah = h - self.htab[i]
        tlocal = tbase + tgrad * deltah

        if (tgrad == 0.0):
            delta = self.ptab[i] * np.exp(-self.gmr * deltah / tbase)
        else:
            delta = self.ptab[i] * (tbase / tlocal) ** (self.gmr / tgrad)

        theta = (tlocal + self.deltaT_degC) / self.ttab[0]

        sigma = delta / theta

        return theta,  delta, sigma

class Mil210Atmosphere(Atmosphere):
    def __init__(self):
        super().__init__()
        self.Tinterp_rank = None
    def getRatios(self, z_m):
        '''

        :param z_m:  geometric altitudes, in m
        :return: tuple of arrays with temperature, pressure, and density ratios
        '''
        # source data is already in geometric altitude, so just interpolate

        # get pressure curve from std atm
        a = stdatm1976(deltaT_degC=0)
        _, delta, _ = a.getRatios(z_m)

        # get temperature from interpolation
        tRank = self.Tinterp_rank(z_m*u.m2ft)
        theta = u.r2k * tRank / self.t0

        # get density ratio
        sigma = delta / theta

        return theta, delta, sigma

class Hot(Mil210Atmosphere):
    def __init__(self):
        super().__init__()
        z = np.array([0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0,
                      10.0, 11.0, 12.0, 13.0, 14.0, 15.0, 16.0, 17.0, 18.0, 19.0,
                      20.0, 21.0, 22.0, 23.0, 24.0, 25.0, 26.0, 27.0, 28.0, 29.0,
                      30.0, 31.0, 32.0, 33.0, 34.0, 35.0, 36.0, 37.0, 38.0, 39.0,
                      40.0, 41.0, 42.0, 43.0, 44.0, 45.0, 46.0, 47.0, 48.0, 49.0,
                      50.0, 51.0, 52.0, 53.0, 54.0, 55.0, 56.0, 57.0, 58.0, 60.0,
                      62.0, 64.0, 66.0, 68.0, 70.0, 72.0, 74.0, 76.0, 78.0, 80.0,
                      82.0, 84.0, 86.0, 88.0, 90.0, 92.0, 94.0, 96.0, 98.0, 100.0])*1000  # geometric altitude, in ft

        temp_rank = np.array([562.7, 558.9, 555.1, 551.2, 547.3, 543.4, 539.5, 535.5, 531.5, 527.5,
                                523.6, 519.9, 516.1, 512.3, 508.5, 504.6, 500.7, 496.8, 492.8, 488.9,
                                485.2, 481.5, 477.7, 474.0, 470.2, 466.4, 462.6, 458.7, 454.8, 451.0,
                                447.4, 443.8, 440.2, 436.5, 432.9, 429.6, 426.3, 423.0, 419.6, 416.2,
                                414.9, 415.4, 415.8, 416.2, 416.6, 417.1, 417.6, 418.0, 418.5, 419.0,
                                419.5, 419.8, 420.0, 420.2, 420.4, 420.6, 420.7, 420.9, 421.1, 421.5,
                                421.9, 422.3, 422.6, 423.6, 425.0, 426.4, 427.8, 429.2, 430.6, 432.0,
                                433.6, 435.1, 436.7, 438.4, 439.9, 441.4, 442.9, 444.6, 446.3, 448.1 ])
        self.Tinterp_rank = interp1d(z,temp_rank)

class Cold(Mil210Atmosphere):
    def __init__(self):
        super().__init__()
        z = np.array([0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0,
                      10.0, 11.0, 12.0, 13.0, 14.0, 15.0, 16.0, 17.0, 18.0, 19.0,
                      20.0, 21.0, 22.0, 23.0, 24.0, 25.0, 26.0, 27.0, 28.0, 29.0,
                      30.0, 31.0, 32.0, 33.0, 34.0, 35.0, 36.0, 37.0, 38.0, 39.0,
                      40.0, 41.0, 42.0, 43.0, 44.0, 45.0, 46.0, 47.0, 48.0, 49.0,
                      50.0, 51.0, 52.0, 53.0, 54.0, 55.0, 56.0, 57.0, 58.0, 60.0,
                      62.0, 64.0, 66.0, 68.0, 70.0, 72.0, 74.0, 76.0, 78.0, 80.0,
                      82.0, 84.0, 86.0, 88.0, 90.0, 92.0, 94.0, 96.0, 98.0, 100.0])*1000  # geometric altitude, in ft

        temp_rank = np.array([399.7, 413.2, 426.7, 440.4, 444.7, 444.7, 444.7, 444.7, 444.7, 444.7,
                                444.7, 443.9, 440.6, 437.3, 434.0, 430.6, 427.3, 423.9, 420.5, 417.0,
                                413.6, 410.1, 406.5, 403.0, 399.4, 395.8, 392.2, 388.6, 384.9, 381.1,
                                377.4, 374.7, 374.7, 374.7, 374.7, 374.7, 374.7, 374.7, 374.7, 374.7,
                                374.7, 374.7, 374.7, 371.5, 366.4, 361.1, 355.8, 350.4, 345.0, 340.5,
                                336.8, 334.7, 334.7, 334.7, 334.7, 334.7, 334.7, 334.7, 334.7, 334.7,
                                337.6, 343.7, 349.3, 354.4, 359.2, 363.6, 365.4, 364.9, 364.4, 363.8,
                                363.1, 362.3, 361.5, 360.8, 360.0, 359.2, 358.4, 357.6, 356.7, 355.8 ])
        self.Tinterp_rank = interp1d(z,temp_rank)

class Tropical(Mil210Atmosphere):
    def __init__(self):
        super().__init__()
        z = np.array([0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0,
                      10.0, 11.0, 12.0, 13.0, 14.0, 15.0, 16.0, 17.0, 18.0, 19.0,
                      20.0, 21.0, 22.0, 23.0, 24.0, 25.0, 26.0, 27.0, 28.0, 29.0,
                      30.0, 31.0, 32.0, 33.0, 34.0, 35.0, 36.0, 37.0, 38.0, 39.0,
                      40.0, 41.0, 42.0, 43.0, 44.0, 45.0, 46.0, 47.0, 48.0, 49.0,
                      50.0, 51.0, 52.0, 53.0, 54.0, 55.0, 56.0, 57.0, 58.0, 60.0,
                      62.0, 64.0, 66.0, 68.0, 70.0, 72.0, 74.0, 76.0, 78.0, 80.0,
                      82.0, 84.0, 86.0, 88.0, 90.0, 92.0, 94.0, 96.0, 98.0, 100.0])*1000  # geometric altitude, in ft

        temp_rank = np.array([549.5, 545.6, 541.7, 537.8, 534.0, 530.1, 526.2, 522.3, 518.4, 514.6,
                                510.7, 506.8, 502.9, 499.1, 495.2, 491.3, 487.5, 483.6, 479.7, 475.8,
                                472.0, 468.1, 464.2, 460.4, 456.5, 452.7, 448.8, 444.9, 441.7, 437.2,
                                433.4, 429.5, 425.6, 421.8, 417.9, 414.1, 410.2, 406.4, 402.6, 398.8,
                                395.1, 391.4, 387.7, 384.1, 380.5, 376.9, 373.4, 369.9, 366.5, 363.0,
                                359.6, 356.3, 352.9, 349.6, 348.6, 350.7, 352.9, 355.1, 357.2, 361.7,
                                366.1, 370.7, 375.3, 379.9, 384.2, 386.8, 389.4, 392.1, 394.7, 397.4,
                                400.1, 402.8, 405.5, 408.2, 410.9, 413.6, 416.3, 418.9, 421.6, 424.3 ])
        self.Tinterp_rank = interp1d(z,temp_rank)

class Polar(Mil210Atmosphere):
    def __init__(self):
        super().__init__()
        z = np.array([0.0,  1.0,  2.0,  3.0,  4.0,  5.0,  6.0,  7.0,  8.0,  9.0,
                    10.0, 11.0, 12.0, 13.0, 14.0, 15.0, 16.0, 17.0, 18.0, 19.0,
                    20.0, 21.0, 22.0, 23.0, 24.0, 25.0, 26.0, 27.0, 28.0, 29.0,
                    30.0, 31.0, 32.0, 33.0, 34.0, 35.0, 36.0, 37.0, 38.0, 39.0,
                    40.0, 41.0, 42.0, 43.0, 44.0, 45.0, 46.0, 47.0, 48.0, 49.0,
                    50.0, 51.0, 52.0, 53.0, 54.0, 55.0, 56.0, 57.0, 58.0, 60.0,
                    62.0, 64.0, 66.0, 68.0, 70.0, 72.0, 74.0, 76.0, 78.0, 80.0,
                    82.0, 84.0, 86.0, 88.0, 90.0, 92.0, 94.0, 96.0, 98.0,100.0])*1000 # geometric altitude, in ft

        temp_rank = np.array([444.0, 447.0, 450.1, 453.1, 453.5, 453.0, 452.4, 451.9, 451.3, 450.8,
                                450.0, 447.2, 444.3, 441.5, 438.7, 435.9, 433.0, 430.2, 427.4, 424.5,
                                421.7, 418.8, 416.0, 413.1, 410.3, 407.4, 404.5, 401.7, 398.8, 395.9,
                                393.0, 392.5, 392.2, 392.0, 391.7, 391.4, 391.2, 390.9, 390.7, 390.4,
                                390.1, 389.9, 389.6, 389.4, 389.1, 388.8, 388.6, 388.3, 388.1, 387.8,
                                387.5, 387.3, 387.0, 386.8, 386.5, 386.2, 386.0, 385.7, 385.5, 385.0,
                                384.4, 383.9, 383.4, 382.9, 382.4, 381.9, 381.4, 380.9, 380.3, 379.8,
                                379.3, 378.8, 378.3, 378.3, 378.3, 378.3, 378.3, 378.3, 378.3, 378.3 ])
        self.Tinterp_rank = interp1d(z,temp_rank)

def geomAlt2DensAlt(atmos, geom_alt):
    """

    :param atmos: the non-standard atmosphere we're using
    :param geom_alt: the geometric altitude
    :return: density altitude (eg, the altitude at which we'd have the same density if we had a standard day)
    """
    from scipy.optimize import newton
    stdatm = stdatm1976()

    # calculate current density
    rho = atmos.rho(geom_alt)

    # solve on standard atmosphere for altitude at which rho = rho_da
    rhoSolve = lambda z_m : stdatm.rho(z_m) - rho
    da = newton(rhoSolve, 1000)

    return da

def densAlt2GeomAlt(atmos, densityAlt):
    """

    :param atmos: the non-standard atmosphere we're using
    :param densityAlt: the equivalent altitude for a standard day
    :return: the geometric altitude during the non-standard atmosphere
    """
    from scipy.optimize import newton
    stdatm = stdatm1976()

    # calculate density based off of stdatm and density altitude
    rho = stdatm.rho(densityAlt)

    # find the geometric altitude that corresponds to that density, using the provided atmosphere
    rhoSolve = lambda geom_alt: atmos.rho(geom_alt) - rho
    z_m = newton(rhoSolve, 1000)

    return z_m
