# Copyright (c) 2018 Uber Technologies, Inc.

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

import math

# distances

ft2in = 12.0

in2ft = 1/ft2in

in2cm = 2.54

cm2in = 1/in2cm

m2cm = 100.0

cm2m = 1/m2cm

mi2ft = 5280.0

ft2mi = 1/mi2ft

km2m = 1000.0

m2km = 1/km2m

nmi2m = 1852.0  # nautical miles to meters

m2nmi = 1.0/nmi2m

in2m = in2cm * cm2m

m2in = 1/in2m

ft2m = ft2in * in2m

m2ft = 1/ft2m

km2mi = km2m * m2ft * ft2mi

mi2km = 1/km2mi

km2ft = km2mi*mi2ft

ft2km = ft2mi * mi2km

mi2nmi = mi2ft*ft2m*m2nmi

nmi2mi = 1.0 / mi2nmi

ft2nmi = ft2mi * mi2nmi

# area

ft22in2 = ft2in * ft2in

in22ft2 = 1/ft22in2

ft22m2 = ft2m * ft2m

m22ft2 = 1/ft22m2

# time
s2m = 1/60.0

m2s = 1/s2m

m2hr = 1/60.0

hr2m = 1/m2hr

s2hr = s2m * m2hr

hr2s = 1/s2hr

# mass
kg2lb = 2.20462234

lb2kg = 1/kg2lb

slug2kg = 14.59390

kg2slug = 1/slug2kg

slug2lb = slug2kg * kg2lb

lb2slug = 1/slug2lb

# forces
lb2n = 4.4482216152605

n2lb = 1/lb2n

# speeds
rpm2rad_s = (1.0/m2s)*(2.0*math.pi)

rad_s2rpm = 1/rpm2rad_s

mph2fts = mi2ft/(hr2m*m2s)

fts2mph = 1/mph2fts

ms2fts = m2ft

fts2ms = 1/ms2fts

kt2fts = nmi2mi*mph2fts  # knots true to ft/s

fts2kt = 1.0/kt2fts

fts2ftm = m2s

ftm2fts = 1.0 / fts2ftm

mph2kts = mph2fts * fts2kt

def tas2eas(tas, atmosphere, z_m):
    import numpy as np
    _, _, s = atmosphere.getRatios(z_m)
    return tas * np.sqrt(s)


def eas2tas(eas, atmosphere, z_m):
    import numpy as np
    _, _, s = atmosphere.getRatios(z_m)
    return eas / np.sqrt(s)


# accels
g2fps2 = 32.174

fps22g = 1/g2fps2

g2mps2 = 9.80665

mps22g = 1/g2mps2


# power
kw2w = 1000.0

w2kw = 1 / kw2w

ftlb_s2kw = lb2n * ft2m * w2kw

kw2ftlb_s = 1/ftlb_s2kw

w2ftlb_s = kw2ftlb_s * kw2w

ftlb_s2w = 1/w2ftlb_s

hp2ftlb_s = 550.0

ftlb_s2hp = 1/hp2ftlb_s

kw2hp = kw2ftlb_s * ftlb_s2hp

hp2kw = 1/kw2hp

w2hp = w2kw * kw2hp

hp2w = 1/w2hp

# torque
nm2ftlb = n2lb*m2ft

ftlb2nm = 1/nm2ftlb

# energy

j2ftlb = n2lb * m2ft

ftlb2j = 1/j2ftlb

wh2j = hr2m * m2s

kwh2j = kw2w * wh2j

kwh2kj = wh2j

kwh2ftlb = kwh2j * j2ftlb

ftlb2kwh = 1/kwh2ftlb

# Angles
deg2rad = math.pi/180.0

rad2deg = 1/deg2rad


# temperatures
k2r = 9./5.
r2k = 1/k2r


def k2degC(k):
    return k - 273.15


def degC2k(c):
    return c+273.15


def r2degF(r):
    return r-459.67


def degF2r(f):
    return f+459.67


def k2degF(k):
    return degF2r(k2r * k)


def degF2degC(f):
    return k2degC(r2k*degF2r(f))


def degC2degF(c):
    return r2degF(degC2k(c) * k2r)


# pressure
psi2psf = ft22in2
psf2psi = 1/psi2psf

pa2psf = n2lb / m22ft2
psf2pa = 1/pa2psf
pa2psi = pa2psf * psf2psi
psi2pa = 1/pa2psi

atm2pa = 101325.
pa2atm = 1/atm2pa

# densities
kgcm2slugcf = kg2slug / (m2ft ** 3.0)
slugcf2kgcm = 1/kgcm2slugcf
