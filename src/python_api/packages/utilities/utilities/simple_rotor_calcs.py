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
import utilities.atmosphere as uat
import utilities.units as uu


def ct(thrust_lb, dens_alt_ft, rpm, radius_ft, atm=None):
    """
    Computes thrust coefficient

    :param thrust_lb: thrust [lb]
    :param dens_alt_ft: density altitude [ft]
    :param rpm: revolutions per minute of the rotor [rpm]
    :param radius_ft: radius [ft]
    :param atm: atmosphere object, if None the 1976 standard atmosphere will be used
    :return: thrust coefficient
    """

    if atm is None:
        atm = uat.stdatm1976(0.0)

    omega_rads = rpm * uu.rpm2rad_s
    rho_slugcf = atm.rho(dens_alt_ft * uu.ft2m) * uu.kgcm2slugcf
    area_ft2 = np.pi*(radius_ft**2.0)
    ct = thrust_lb/(rho_slugcf*area_ft2*(omega_rads*radius_ft)**2.0)

    return ct


def prop_ct(thrust_lb, dens_alt_ft, rpm, radius_ft, atm=None):
    """
    Computes propeller thrust coefficient

    :param thrust_lb: thrust [lb]
    :param dens_alt_ft: density altitude [ft]
    :param rpm: revolutions per minute of the rotor [rpm]
    :param radius_ft: radius [ft]
    :param atm: atmosphere object, if None the 1976 standard atmosphere will be used
    :return: thrust coefficient
    """

    if atm is None:
        atm = uat.stdatm1976(0.0)

    n = rpm*uu.rpm2n
    rho_slugcf = atm.rho(dens_alt_ft * uu.ft2m) * uu.kgcm2slugcf
    diameter_ft = radius_ft*2.0
    ct_prop = thrust_lb/(rho_slugcf*n**2.0*diameter_ft**4.0)
    return ct_prop


def cq(torque_ft_lb, dens_alt_ft, rpm, radius_ft, atm=None):
    """
    Computes torque coefficient

    :param torque_ft_lb: torque [ft-lb]
    :param dens_alt_ft: density altitude [ft]
    :param rpm: revolutions per minute of the rotor [rpm]
    :param radius_ft: radius [ft]
    :param atm: atmosphere object, if None the 1976 standard atmosphere will be used
    :return: torque coefficient
    """

    if atm is None:
        atm = uat.stdatm1976(0.0)

    omega_rads = rpm * uu.rpm2rad_s
    rho_slugcf = atm.rho(dens_alt_ft * uu.ft2m) * uu.kgcm2slugcf
    area_ft2 = np.pi*(radius_ft**2.0)
    cq = torque_ft_lb/(rho_slugcf * area_ft2 * radius_ft * (omega_rads * radius_ft)**2)

    return cq


def prop_cq(torque_ft_lb, dens_alt_ft, rpm, radius_ft, atm=None):
    """
    Computes propeller torque coefficient

    :param torque_ft_lb: torque [ft-lb]
    :param dens_alt_ft: density altitude [ft]
    :param rpm: revolutions per minute of the rotor [rpm]
    :param radius_ft: radius [ft]
    :param atm: atmosphere object, if None the 1976 standard atmosphere will be used
    :return: torque coefficient
    """

    if atm is None:
        atm = uat.stdatm1976(0.0)

    n = rpm * uu.rpm2n
    rho_slugcf = atm.rho(dens_alt_ft * uu.ft2m) * uu.kgcm2slugcf
    diameter_ft = radius_ft*2.0
    cq = torque_ft_lb/(rho_slugcf*n**2.0*diameter_ft**5.0)

    return cq


def cp(power_ftlb_s, dens_alt_ft, rpm, radius_ft, atm=None):
    """
    Computes power coefficient

    :param power_ftlb_s: power [ft-lb/s]
    :param dens_alt_ft: density altitude [ft]
    :param rpm: revolutions per minute of the rotor [rpm]
    :param radius_ft: radius [ft]
    :param atm: atmosphere object, if None the 1976 standard atmosphere will be used
    :return: power coefficient
    """

    if atm is None:
        atm = uat.stdatm1976(0.0)

    omega_rads = rpm * uu.rpm2rad_s
    rho_slugcf = atm.rho(dens_alt_ft * uu.ft2m) * uu.kgcm2slugcf
    area_ft2 = np.pi*(radius_ft**2.0)
    cp = power_ftlb_s/(rho_slugcf*area_ft2*(omega_rads*radius_ft)**3.0)

    return cp


def prop_cp(power_ftlb_s, dens_alt_ft, rpm, radius_ft, atm=None):
    """
    Computes propeller power coefficient

    :param power_ftlb_s: power [ft-lb/s]
    :param dens_alt_ft: density altitude [ft]
    :param rpm: revolutions per minute of the rotor [rpm]
    :param radius_ft: radius [ft]
    :param atm: atmosphere object, if None the 1976 standard atmosphere will be used
    :return: power coefficient
    """

    if atm is None:
        atm = uat.stdatm1976(0.0)

    n = rpm * uu.rpm2n
    rho_slugcf = atm.rho(dens_alt_ft * uu.ft2m) * uu.kgcm2slugcf
    diameter_ft = radius_ft*2.0
    cp_prop = power_ftlb_s/(rho_slugcf*n**3.0*diameter_ft**5.0)
    return cp_prop


def fom(thrust_lb, power_ftlb_s, dens_alt_ft, rpm, radius_ft, atm=None):
    """
    Computes figure of merit

    :param thrust_lb: thrust [lb]
    :param power_ftlb_s: power [ft-lb/s]
    :param dens_alt_ft: density altitude [ft]
    :param rpm: revolutions per minute of the rotor [rpm]
    :param radius_ft: radius [ft]
    :param atm: atmosphere object, if None the 1976 standard atmosphere will be used
    :return: power coefficient
    """

    ct_calc = ct(thrust_lb, dens_alt_ft, rpm, radius_ft, atm)
    cp_calc = cp(power_ftlb_s, dens_alt_ft, rpm, radius_ft, atm)
    fom = simple_fom(ct_calc, cp_calc)
    return fom


def simple_fom(ct, cp):
    """
    Computes figure of merit from ct and cp

    :param ct: thrust coefficient
    :param cp: power coefficient
    :return: figure of merit
    """
    fom = ct**1.5/(2.0**0.5*cp)
    return fom


def calc_cp_from_ct_fom(ct, fom):
    """
    Computes power coefficient from thrust coefficient and figure of merit

    :param ct: thrust coefficient
    :param fom: figure of merit
    :return: power coefficient
    """
    cp = ct**1.5/(np.sqrt(2.0)*fom)
    return cp


def dimensionalize_cp(cp, rpm, radius_ft, dens_alt_ft, atm=None):
    """
    Converts cp into power ft-lb/s

    :param cp: power coefficient
    :param rpm: revolutions per minute
    :param radius_ft: radius [ft]
    :param dens_alt_ft: density altitude [ft\
    :param atm: atmosphere object (optional, 1976 used if None)
    :return: power [ft-lb/s]
    """
    if atm is None:
        atm = uat.stdatm1976()

    rho_slugcf = atm.rho(dens_alt_ft*uu.ft2m)*uu.kgcm2slugcf
    area_ft2 = np.pi * radius_ft**2.0
    omega = rpm * uu.rpm2rad_s
    power_ftlbs = cp*rho_slugcf*area_ft2*(omega*radius_ft)**3.0
    return power_ftlbs


def dimensionalize_prop_cp(cp_prop, rpm, radius_ft, dens_alt_ft, atm=None):
    """
    Converts propeller cp into power ft-lb/s

    :param prop_cp: power coefficient
    :param rpm: revolutions per minute
    :param radius_ft: radius [ft]
    :param dens_alt_ft: density altitude [ft]
    :param atm: atmosphere object (optional, 1976 used if None)
    :return: power [ft-lb/s]
    """
    if atm is None:
        atm = uat.stdatm1976(0.0)

    n = rpm * uu.rpm2n
    rho_slugcf = atm.rho(dens_alt_ft * uu.ft2m) * uu.kgcm2slugcf
    diameter_ft = radius_ft*2.0
    power_ftlbs = cp_prop*(rho_slugcf*n**3.0*diameter_ft**5.0)
    return power_ftlbs


def tipspeed2rpm(tip_speed, diam=None, radius=None, inflow_speed=0):
    '''
    Calculates an RPM based on a tip speed and either diameter or radius. Either diameter or radius should be entered, but not both.

    :param tip_speed: total helical tip speed
    :param diam: diameter
    :param radius: radius
    :param inflow_speed: inflow speed of the rotor. Used to convert between radial tip speed and helical tip speed. (optional, default to 0.)
    :return: RPM
    '''
    if (diam is None) and (radius is None):
        raise ValueError("Either 'diam' or 'radius' must be specified.")
    elif (diam is not None) and (radius is not None):
        raise ValueError("Only one of 'diam' and 'radius' may be specified.")

    if diam is not None:
        radius = diam/2

    radial_speed = (tip_speed**2 - inflow_speed**2)**0.5

    rad_s = radial_speed/radius

    return rad_s * uu.rad_s2rpm


def rpm2tipspeed(rpm, diam=None, radius=None, inflow_speed=0):
    '''
    Calculates a tip speed based on an RPM and either diameter or radius. Either diameter or radius should be entered, but not both.

    :param rpm: RPM
    :param diam: diameter
    :param radius: radius
    :param inflow_speed: inflow speed of the rotor. Used to convert between radial tip speed and helical tip speed. (optional, default to 0.)
    :return: total helical tip speed
    '''
    if (diam is None) and (radius is None):
        raise ValueError("Either 'diam' or 'radius' must be specified.")
    elif (diam is not None) and (radius is not None):
        raise ValueError("Only one of 'diam' and 'radius' may be specified.")

    if diam is not None:
        radius = diam/2

    rad_s = rpm*uu.rpm2rad_s

    radial_speed = rad_s * radius

    tip_speed = (radial_speed**2 + inflow_speed**2)**0.5
    return tip_speed


def power2torque(power_kw,rpm):
    '''
    Calculates torque based on power and RPM
    :param power_kw: power (units=kW)
    :param rpm: RPM
    :return: torque (units=N-m)
    '''
    rad_s = rpm * uu.rpm2rad_s
    return power_kw * 1e3 / rad_s


def torque2power(torque_nm, rpm):
    '''
    Calculates power in kW based on an input torque and RPM

    :param torque_nm: torque (units=N-m)
    :param rpm: RPM
    :return: power (units=kW)
    '''
    rad_s = rpm*uu.rpm2rad_s
    return torque_nm*rad_s / 1e3


def powertorque2rpm(power_kw, torque_nm):
    """
    Calculates rpm from a known power and torque

    :param power_kw: power (units=kW)
    :param torque_nm: torque (units=N-m)
    :return: rpm
    """
    rad_s = power_kw*uu.kw2w/torque_nm
    rpm = rad_s * uu.rad_s2rpm
    return rpm


def hover_power_calc(thrust_lb, fom, density_alt_ft, radius_ft, atm=None):
    """
    Compute hover power in watts

    :param thrust_lb: required thrust
    :param fom: figure of merit
    :param density_alt_ft: density altitude in feet
    :param radius_ft: rotor radius in feet
    :param atm: atmosphere object, if None use the 1976 standard atmosphere
    :return: power [watts]
    """

    if atm is None:
        atm = uat.stdatm1976(0.0)

    area_m2 = np.pi*(radius_ft*uu.ft2m)**2.0
    rho_kgcm = atm.rho(density_alt_ft*uu.ft2m)
    thrust_N = thrust_lb * uu.lb2n

    power_watts = thrust_N/(fom*np.sqrt(2.0*rho_kgcm/(thrust_N/area_m2)))
    return power_watts

def tipspeed_delta(radius_mult, orig_tipspeed=1.0):
    """
    Computes a new tip speed based on a radius multiplier and original tip speed

    :param radius_mult: radius multiplier
    :param orig_tipspeed: original tip speed
    :return: new tip speed assuming constant Ct/Sigma and geometrically scaling blade by radius multiplier
    """

    new_tipspeed = (1.0/radius_mult)*orig_tipspeed
    return new_tipspeed


def const_ctsigma_area_ratio(tipspeed_ratio):
    """
    Computes an area ratio from a tip speed ratio
    :param tipspeed_ratio: new tip speed / original tip speed
    :return: new blade area/original blade area assuming constant diameter rotor
    """

    area_ratio = tipspeed_ratio**(-2.0)
    return area_ratio

def thrust(ct, dens_alt_ft, rpm, radius_ft, atm=None):
    """
    Computes thrust from a thrust coefficient

    :param ct: thrust coefficient
    :param dens_alt_ft: density altitude [ft]
    :param rpm: revolutions per minute of the rotor [rpm]
    :param radius_ft: radius [ft]
    :param atm: atmosphere object, if None the 1976 standard atmosphere will be used
    :return: thrust [lb]
    """

    if atm is None:
        atm = uat.stdatm1976(0.0)

    omega_rads = rpm * uu.rpm2rad_s
    rho_slugcf = atm.rho(dens_alt_ft * uu.ft2m) * uu.kgcm2slugcf
    area_ft2 = np.pi*(radius_ft**2.0)
    thrust_lb = ct * rho_slugcf*area_ft2*(omega_rads*radius_ft)**2.0

    return thrust_lb


def prop_thrust(prop_ct, dens_alt_ft, rpm, radius_ft, atm=None):
    """
    Computes thrust from a thrust coefficient

    :param prop_ct: propeller thrust coefficient
    :param dens_alt_ft: density altitude [ft]
    :param rpm: revolutions per minute of the rotor [rpm]
    :param radius_ft: radius [ft]
    :param atm: atmosphere object, if None the 1976 standard atmosphere will be used
    :return: thrust [lb]
    """

    if atm is None:
        atm = uat.stdatm1976(0.0)

    n = rpm*uu.rpm2n
    rho_slugcf = atm.rho(dens_alt_ft * uu.ft2m) * uu.kgcm2slugcf
    diameter_ft = radius_ft*2.0
    thrust_lb = prop_ct*(rho_slugcf*n**2.0*diameter_ft**4.0)

    return thrust_lb


def torque(cq, dens_alt_ft, rpm, radius_ft, atm=None):
    """
    Computes torque from a torque coefficient

    :param cq: torque coefficient
    :param dens_alt_ft: density altitude [ft]
    :param rpm: revolutions per minute of the rotor [rpm]
    :param radius_ft: radius [ft]
    :param atm: atmosphere object, if None the 1976 standard atmosphere will be used
    :return: torque [ft-lb]
    """

    if atm is None:
        atm = uat.stdatm1976(0.0)

    omega_rads = rpm * uu.rpm2rad_s
    area_ft2 = np.pi*(radius_ft**2.0)
    rho_slugcf = atm.rho(dens_alt_ft * uu.ft2m) * uu.kgcm2slugcf

    torque_ft_lb = cq * rho_slugcf * area_ft2 * radius_ft * (omega_rads * radius_ft)**2

    return torque_ft_lb


def prop_torque(prop_cq, dens_alt_ft, rpm, radius_ft, atm=None):
    """
    Computes torque from a torque coefficient

    :param prop_cq: propeller torque coefficient
    :param dens_alt_ft: density altitude [ft]
    :param rpm: revolutions per minute of the rotor [rpm]
    :param radius_ft: radius [ft]
    :param atm: atmosphere object, if None the 1976 standard atmosphere will be used
    :return: torque [ft-lb]
    """

    if atm is None:
        atm = uat.stdatm1976(0.0)

    n = rpm * uu.rpm2n
    rho_slugcf = atm.rho(dens_alt_ft * uu.ft2m) * uu.kgcm2slugcf
    diameter_ft = radius_ft*2.0
    torque_ft_lb = prop_cq*rho_slugcf*n**2.0*diameter_ft**5.0

    return torque_ft_lb


def induced_velocity(diskload_lb_ft2, dens_alt_ft, atm=None):
    """
    Computes the induced velocity at the rotor plane.

    :param diskload_lb_ft2: diskloading, lb/ft2
    :param dens_alt_ft: density altitude [ft]
    :param atm: atmosphere object, if None the 1976 standard atmosphere will be used
    :return: induced velocity at the rotor plane, in ft/s
    """

    if atm is None:
        atm = uat.stdatm1976(0.0)

    rho_slugcf = atm.rho(dens_alt_ft * uu.ft2m) * uu.kgcm2slugcf

    iv = (diskload_lb_ft2 / (2* rho_slugcf)) ** 0.5
    return iv


def radius_from_dl_thrust(diskload_lb_ft2, thrust_lb):
    """
    Computes disk radius from a known disk loading and thrust

    :param diskload_lb_ft2: disk loading (units=lb/ft^2)
    :param thrust_lb: thrust (units=lb)
    :return: radius (units=ft)
    """
    radius_ft = np.sqrt(thrust_lb/(np.pi*diskload_lb_ft2))
    return radius_ft


def climb_power_ratio(v_climb_fts, diskload_lb_ft2, dens_alt_ft, atm=None):
    """
    Computes climb power ratio

    From Eq. 2.98 in Leishman

    :param v_climb_fts: climb speed [ft/s]
    :param diskload_lb_ft2: disk loading [lb/ft2]
    :param dens_alt_ft: density altitude [ft]
    :param atm: optional atmosphere object, defaults to 1976 standard atmosphere
    :return: climb power/hover power
    """
    vh = induced_velocity(diskload_lb_ft2, dens_alt_ft, atm)
    power_ratio = v_climb_fts/(2*vh)+np.sqrt((v_climb_fts/(2*vh))**2.0 + 1)
    return power_ratio
