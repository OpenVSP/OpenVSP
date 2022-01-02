# Copyright (c) 2018-2020 Uber Technologies, Inc.

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

import utilities.units as u
from utilities.files import RunManager
from utilities.transformations import TransMatrix
import logging
import utilities.uberlogging as ul
import openvsp as vsp
import degen_geom as dg
import numpy as np
import os
from subprocess import Popen, PIPE, STDOUT
import io
from enum import Enum
import collections
import re
import math
import sys
import glob
from typing import Union

# Find default fortran executables
DEFAULT_BG2CHARM = os.path.join(os.path.dirname(__file__), "charm_fortran_utilities", "bin")
g = glob.glob(os.path.join(DEFAULT_BG2CHARM, "bg2charm*"))
if sys.platform in ['linux', 'win32', 'darwin']:  # Allowed system platform identifiers
    DEFAULT_BG2CHARM = g[0]
else:
    raise ValueError("Detected platform not tested for use with CHARM Automation.  Add sys.platform string output to "
                     "list of platform identifiers in charm_automation.py script and use at your own risk.")


class AirfoilDataLocation(Enum):
    """
    Defines an enum for where to retrieve an airfoil
    """
    INVALID = 0
    FILE_LOCATION = 1


class AirfoilReynoldsDataType(Enum):
    """
    Defines the type of reynolds number correction information this section contains
    """
    NONE = 0
    METHOD1 = 1
    METHOD2 = 2


class AirfoilFileLocationOpt:
    def __init__(self, **kwargs):
        """
        Specifies options associated with obtaining charm airfoil file
        :param kwargs:  base_directory = directory of airfoil database
                        filename = specifies name of the airfoil file to retrieve, path should be relative to
                                   base_directory
        """

        self.base_directory = kwargs.get('base_directory', os.path.join(os.path.dirname(__file__), "charm_airfoils"))
        """
        directory of airfoil database, if this is not specified the "charm_airfoils" directory of this package will
        be used
        """

        self.filename = kwargs.get('filename', None)
        """
        Specifies name of the airfoil file to retrieve, path should be relative to :attr:`base_directory`.

        The format of this file is the same as the CHARM sectional properties. See the
        *2-D AIRFOIL SECTION DATA INPUT FILE (nameaf.inp)* of the CHARM user manual for details. The file should
        contain everything below the *COMMENT#1* line in the :code:`K=1,NFOIL` loop. The only modification
        to the format is that the first line should be the maximum thickness/chord value of the airfoil.
        """

    def get_fullpath(self):
        """
        Gets full path of file to read

        :return: path to charm airfoil section file
        """
        return os.path.join(self.base_directory, self.filename)


class CharmAirfoil:
    def __init__(self, thickness, section_string, reynolds_data_type: AirfoilReynoldsDataType):
        """
        Contains data pertaining to a CHARM airfoil
        :param thickness: thickness of the airfoil
        :param section_string: string representing the section data
        :param reynolds_data_type: reynolds number correction type this airfoil has
        """
        self.thickness = thickness
        self.section_string = section_string
        self.reynolds_data_type = reynolds_data_type


class CharmAirfoilSection:
    def __init__(self, airfoil: CharmAirfoil, radius_frac):
        """
        Contains information related to an airfoil section in CHARM

        The difference between a CharmAirfoilSection and a CharmAirfoil is that a CharmAirfoil is airfoil data that is
        independent of the propeller it is related to, it simply contains airfoil data formatted for CHARM.
        CharmAirfoilSection contains a CharmAirfoil as well as any details specific to a rotor, such as radial location
        :param airfoil: CHARM airfoil data at this section
        :param radius_frac: radial location normalized by prop/rotor radius of this section
        """
        self.airfoil = airfoil
        self.radius_frac = radius_frac


class CharmRotorSettings:
    def __init__(self, rpm=0.0, rotor_wake_template=None, initial_collective=None, ct=None,
                 default_airfoil_opts=None, merge_wings=True, nspan_override=None,
                 airfoil_opts=None, iaero=1, irvflo=0, icoll=None, airfoil_r_o_Rs=None, nchord=1,
                 icnvct=None, flap_type=None, flap_length=None, flap_defl=None):
        """

        :param rpm: revolutions per minute of the rotor
        :param rotor_wake_template: list of string lines of a rotor wake template file
        :param initial_collective: initial rotor collective in degrees
        :param ct: thrust coefficient of rotor (T/(rho*pi*R^2*(R*Omega)^2))
        :param default_airfoil_opts: default airfoil options, used if airfoil_opts are not specified
        :param merge_wings: if true, touching symmetric wings are merged into one rotor component, otherwise they are
            treated as separate rotors. If merging is disabled, make sure the rotor wake template settings are specified
            to not shed root vortices
        :param nspan_override: if not none it will be used to override the nspan field
            in the generated blade geometry file
        :param airfoil_opts: if not none, this is a list of airfoil options corresponding to
            each cross-section specified on the VSP geometry
        :param iaero: value to set IAERO to in the blade dynamics file
        :param irvflo: value to set IVRFLO to in the blade dynamics file
        :param icoll: set ICOLL variable in CHARM rotor wake file. Controls how collective is adjusted
        :param airfoil_r_o_Rs: optional array of local radius/blade radius locations of airfoil sections
            specified in `airfoil_opts`. If not specified, an attempt to use VSP will be made to locate the airfoils
        :param nchord: number of chordwise elements in vortex latice method
        :param icnvct: controls downstream wake convection; =0 is a free wake, =1 is free up fixed down, =2 is fixed
        :param flap_type: flap type input for segment ISEG; =0 no flap, =1 plain flap (nicolai), =-1 plain flap
        :param flap_defl: flap deflection of segment ISEG in degrees (0 - 40)
        :param flap_length: flap length for segment ISEG, percent chord, (0.0 - 0.5)
        """
        self.__rpm = rpm
        self.__rotor_wake_template = rotor_wake_template
        self.__initial_collective = initial_collective
        self.__ct = ct
        self.__default_airfoil_opts = default_airfoil_opts
        self.__merge_wings = merge_wings
        self.__nspan_override = nspan_override
        self.__airfoil_opts = [] if airfoil_opts is None else airfoil_opts
        self.__airfoil_r_o_Rs = [] if airfoil_r_o_Rs is None else airfoil_r_o_Rs
        self.__iaero = iaero
        self.__irvflo = irvflo
        self.__icoll = icoll
        self.__nchord = nchord
        self.__icnvct = icnvct
        self.__flap_type = flap_type
        self.__flap_length = flap_length
        self.__flap_defl = flap_defl

    @property
    def rpm(self):
        return self.__rpm

    @rpm.setter
    def rpm(self, rpm):
        self.__rpm = rpm

    @property
    def rotor_wake_template(self):
        return self.__rotor_wake_template

    @rotor_wake_template.setter
    def rotor_wake_template(self, rotor_wake_template):
        self.__rotor_wake_template = rotor_wake_template

    @property
    def initial_collective(self):
        return self.__initial_collective

    @initial_collective.setter
    def initial_collective(self, initial_collective):
        self.__initial_collective = initial_collective

    @property
    def ct(self):
        return self.__ct

    @ct.setter
    def ct(self, ct):
        self.__ct = ct

    @property
    def default_airfoil_opts(self):
        return self.__default_airfoil_opts

    @default_airfoil_opts.setter
    def default_airfoil_opts(self, default_airfoil_opts):
        self.__default_airfoil_opts = default_airfoil_opts

    @property
    def merge_wings(self):
        return self.__merge_wings

    @merge_wings.setter
    def merge_wings(self, merge_wings):
        self.__merge_wings = merge_wings

    @property
    def nspan_override(self):
        return self.__nspan_override

    @nspan_override.setter
    def nspan_override(self, nspan_override):
        self.__nspan_override = nspan_override

    @property
    def airfoil_opts(self):
        return self.__airfoil_opts

    @airfoil_opts.setter
    def airfoil_opts(self, airfoil_opts):
        self.__airfoil_opts = [] if airfoil_opts is None else airfoil_opts

    @property
    def iaero(self):
        return self.__iaero

    @iaero.setter
    def iaero(self, iaero):
        self.__iaero = iaero

    @property
    def irvflo(self):
        return self.__irvflo

    @irvflo.setter
    def irvflo(self, irvflo):
        self.__irvflo = irvflo

    @property
    def icoll(self):
        """
        Corresponds to the ICOLL variable in CHARM rotor wake file. Controls how collective is trimmed

        From the CHARM Manual::

            ICOLL=0: The collective is fixed at COLLD degrees and the
                input CT value is simply an estimate used to initialize the wake
                induced velocity if IFREE=0.
            ICOLL=1: Collective adjusts until the thrust coefficient is
                equal to the input value of CT = T/R2(R)2 and COLLD is
                just an initial estimate.
            ICOLL=2: Collective adjusts until the torque coefficient is
                equal to the input value of CCTRIM = CQ = Q/R3(R)2.
            ICOLL=3: Shaft angle (s) adjusts until X-force/Lift is equal
                to the input value of CCTRIM = X-force/Lift (wind axes).
                Important note: For ICOLL=3, the code also adjusts collective
                to keep CTcos(s) constant as it adjusts s. This is only an
                option when SFRAME=0 (shaft frame solution).
            ICOLL=4: RPM adjusts until the thrust is equal to the input
                value of CCTRIM = thrust (lbf or Newtons). Important note:
                For large changes in RPM, repeat the calculation beginning
                with the new RPM value until the power converges.
            ICOLL=5: Shaft angle (s) adjusts until -X-force = parasite
                drag force = ½U2A where CCTRIM = A (ft or m2).
                Important note: For ICOLL=5, the code also adjusts collective
                to keep CTcos(s) constant as it adjusts s. This is only an
                option when SFRAME=0 (shaft frame solution).
            ICOLL=6: Same as ICOLL=5 only shaft angle is fixed and
                cyclic pitch is used to trim to –X-force.
            ICOLL=7: Same as ICOLL=5 only shaft angle is fixed and
                thrust is used to trim to –X-force.
            ICOLL = 21: Collective adjusts until the torque is equal to the
                CCTRIM value (ft-lbs or n-m).
            ICOLL = 22: Collective adjusts until the power is equal to the
                CCTRIM value (HP or watts).
        """
        return self.__icoll

    @icoll.setter
    def icoll(self, icoll):
        self.__icoll = icoll

    @property
    def airfoil_r_o_Rs(self):
        return self.__airfoil_r_o_Rs

    @airfoil_r_o_Rs.setter
    def airfoil_r_o_Rs(self, airfoil_r_o_Rs):
        self.__airfoil_r_o_Rs = [] if airfoil_r_o_Rs is None else airfoil_r_o_Rs

    @property
    def nchord(self):
        return self.__nchord

    @nchord.setter
    def nchord(self, nchord):
        self.__nchord = int(nchord)

    @property
    def icnvct(self):
        """
        Corresponds to the ICOLL variable in CHARM rotor wake file. Controls how collective is trimmed

        From the CHARM Manual::

            ICNVCT=0: Include all induced velocites in the calculation of wake geometry.
            ICNVCT=1: ICNVCT=0 upstream of x=-RADIUS(1), ICNVCT=2 downstream.
            ICNVCT=2: Wake elements convect at free stream plus uniform inflow.
            ICNVCT=3: Wake elements convect at free stream.
        """
        return self.__icnvct

    @icnvct.setter
    def icnvct(self, icnvct):
        self.__icnvct = icnvct

    @property
    def flap_type(self):
        """
        Flap type for segment ISEG

        From the CHARM Manual::

        KFLAP(ISEG)=0: Segment ISEG has no flaps
        KFLAP(ISEG)=1: Plain flaps (Nicolai model)
        KFLAP(ISEG)=-1: Plain flaps (Torenbeek model)
        KFLAP(ISEG)=2: Split flaps (Nicolai model)
        :return:
        """
        return self.__flap_type

    @flap_type.setter
    def flap_type(self, flap_type):
        self.__flap_type = flap_type

    @property
    def flap_length(self):
        return self.__flap_length

    @flap_length.setter
    def flap_length(self, flap_length):
        self.__flap_length = flap_length

    @property
    def flap_defl(self):
        return self.__flap_defl

    @flap_defl.setter
    def flap_defl(self, flap_defl):
        self.__flap_defl = flap_defl


class CharmRotorSettingsCollection(collections.abc.MutableMapping, CharmRotorSettings):
    """
    Allows a collection of rotors settings to be treated like
    a single rotor setting
    """

    def __init__(self, *args, **kwargs):
        self.store = dict()
        self.update(dict(*args, **kwargs))
        super().__init__(**kwargs)

    def __getitem__(self, key) -> Union[CharmRotorSettings, 'CharmRotorSettingsCollection']:
        return self.store[key]

    def __setitem__(self, key, value: Union[CharmRotorSettings, 'CharmRotorSettingsCollection']):
        self.store[key] = value

    def __delitem__(self, key):
        del self.store[key]

    def __iter__(self):
        return iter(self.store)

    def __len__(self):
        return len(self.store)

    @property
    def rpm(self):
        rpm = 0.0
        for key, rotor_setting in self.store.items():
            rpm += rotor_setting.rpm
        if len(self.store) > 0:
            rpm /= len(self.store)
        return rpm

    @rpm.setter
    def rpm(self, rpm):
        for key, rotor_setting in self.store.items():
            rotor_setting.rpm = rpm

    @property
    def rotor_wake_template(self):
        return ""

    @rotor_wake_template.setter
    def rotor_wake_template(self, rotor_wake_template):
        for key, rotor_setting in self.store.items():
            rotor_setting.rotor_wake_template = rotor_wake_template

    @property
    def initial_collective(self):
        initial_collective = 0.0
        for key, rotor_setting in self.store.items():
            initial_collective += rotor_setting.initial_collective
        if len(self.store) > 0:
            initial_collective /= len(self.store)
        return initial_collective

    @initial_collective.setter
    def initial_collective(self, initial_collective):
        for key, rotor_setting in self.store.items():
            rotor_setting.initial_collective = initial_collective

    @property
    def ct(self):
        ct = 0.0
        for key, rotor_setting in self.store.items():
            ct += rotor_setting.ct
        if len(self.store) > 0:
            ct /= len(self.store)
        return ct

    @ct.setter
    def ct(self, ct):
        for key, rotor_setting in self.store.items():
            rotor_setting.ct = ct

    @property
    def default_airfoil_opts(self):
        return None

    @default_airfoil_opts.setter
    def default_airfoil_opts(self, default_airfoil_opts):
        for key, rotor_setting in self.store.items():
            rotor_setting.default_airfoil_opts = default_airfoil_opts

    @property
    def merge_wings(self):
        for key, rotor_setting in self.store.items():
            if not rotor_setting.merge_wings:
                return False
        return True

    @merge_wings.setter
    def merge_wings(self, merge_wings):
        for key, rotor_setting in self.store.items():
            rotor_setting.merge_wings = merge_wings

    @property
    def nspan_override(self):
        nspan_override = 0.0
        num_rotors = 0.0
        for key, rotor_setting in self.store.items():
            if rotor_setting.nspan_override is not None:
                nspan_override += rotor_setting.nspan_override
                num_rotors += 1
        if num_rotors < 1:
            return None
        else:
            return int(round(nspan_override/num_rotors))

    @nspan_override.setter
    def nspan_override(self, nspan_override):
        for key, rotor_setting in self.store.items():
            rotor_setting.nspan_override = nspan_override

    @property
    def airfoil_opts(self):
        for key, rotor_setting in self.store.items():
            if len(rotor_setting.airfoil_opts) > 0:
                return rotor_setting.airfoil_opts
        return []

    @airfoil_opts.setter
    def airfoil_opts(self, opts):
        for key, rotor_setting in self.store.items():
            rotor_setting.airfoil_opts = opts

    @property
    def iaero(self):
        iaero = 0
        for key, rotor_setting in self.store.items():
            iaero += rotor_setting.iaero
        if len(self.store) > 0:
            iaero /= len(self.store)
        return int(iaero)

    @iaero.setter
    def iaero(self, iaero):
        for key, rotor_setting in self.store.items():
            rotor_setting.iaero = iaero

    @property
    def irvflo(self):
        irvflo = 0
        for key, rotor_setting in self.store.items():
            irvflo += rotor_setting.irvflo
        if len(self.store) > 0:
            irvflo /= len(self.store)
        return int(irvflo)

    @irvflo.setter
    def irvflo(self, irvflo):
        for key, rotor_setting in self.store.items():
            rotor_setting.irvflo = irvflo

    @property
    def icoll(self):
        return None

    @icoll.setter
    def icoll(self, icoll):
        for key, rotor_setting in self.store.items():
            rotor_setting.icoll = icoll

    @property
    def airfoil_r_o_Rs(self):
        return None

    @airfoil_r_o_Rs.setter
    def airfoil_r_o_Rs(self, airfoil_r_o_Rs):
        for key, rotor_setting in self.store.items():
            rotor_setting.airfoil_r_o_Rs = airfoil_r_o_Rs

    @property
    def nchord(self):
        nchord = 0
        for key, rotor_setting in self.store.items():
            nchord += rotor_setting.nchord
        if len(self.store) > 0:
            nchord /= len(self.store)
        return int(nchord)

    @nchord.setter
    def nchord(self, nchord):
        for key, rotor_setting in self.store.items():
            rotor_setting.nchord = nchord

    @property
    def icnvct(self):
        icnvct = 0
        for key, rotor_settings in self.store.items():
            icnvct += rotor_settings.icnvct
        if len(self.store) > 0:
            icnvct /= len(self.store)
        return int(icnvct)

    @icnvct.setter
    def icnvct(self, icnvct):
        for key, rotor_setting in self.store.items():
            rotor_setting.icnvct = icnvct

    @property
    def flap_type(self):
        flap_type = []
        for key, rotor_settings in self.store.items():
            flap_type.append(rotor_settings.flap_type)
        return flap_type

    @flap_type.setter
    def flap_type(self, flap_type):
        for key, rotor_setting in self.store.items():
            rotor_setting.flap_type = flap_type

    @property
    def flap_length(self):
        flap_length = []
        for key, rotor_settings in self.store.items():
            flap_length.append(rotor_settings.flap_length)
        return flap_length

    @flap_length.setter
    def flap_length(self, flap_length):
        for key, rotor_settings in self.store.items():
            rotor_settings.flap_length = flap_length

    @property
    def flap_defl(self):
        flap_delf = []
        for key, rotor_settings in self.store.items():
            flap_delf.append(rotor_settings.flap_defl)
        return flap_delf

    @flap_defl.setter
    def flap_defl(self, flap_defl):
        for key, rotor_settings in self.store.items():
            rotor_settings.flap_defl = flap_defl


class CharmTrimSettings:
    def __init__(self, forces_moments=None, targets=None, tol=None, pilot_cntrls=None, attitude_cntrls=None,
                 num_aero_comps=None, aero_comp_cntrls=None, cntrl_gains=None, init_cntrls=None, template=None):
        """
        Initializes a CHARM trim information object.  All params default to None so default template file can be used
        without modification.

        :param forces_moments: integer list [Fx, Fy, Fz, Mx, My, Mz], select forces and moments to trim
        :param targets: float list [Fx, Fy, Fz, Mx, My, Mz], target values to trim to
        :param tol: float list [Fx, Fy, Fz, Mx, My, Mz], trim tolerance (fraction of weight)
        :param pilot_cntrls: integer list (see CHARM Manual)
        :param attitude_cntrls: integer list [Roll, Pitch, Sideslip]
        :param num_aero_comps: integer, number of aerodynamic components (rotors and wings) used to trim
        :param aero_comp_cntrls: integer matrix, component control capabilities, rows must equal num_aero_comps
        :param cntrl_gains: float matrix, control gains connecting pilot controls to component controls
        :param template: string, template file name
        :param init_cntrls: float list, initial pilot control inputs
        """
        self.forces_moments = forces_moments
        self.targets = targets
        self.tol = tol
        self.pilot_cntrls = pilot_cntrls
        self.attitude_cntrls = attitude_cntrls
        self.num_aero_comps = num_aero_comps
        self.aero_comp_cntrls = aero_comp_cntrls
        self.cntrl_gains = cntrl_gains
        self.init_cntrls = init_cntrls
        self.template = template


class CharmWingInfo:
    def __init__(self, charm_le, charm_te, toc, vsp_wing_origin, vsp_origin_2_wing_origin: TransMatrix,
                 span, airfoils, geom_id, num_syms, flip_rotation_direction):
        """
        Initializes a CHARM Wing Information object

        :param charm_le: leading edge coordinates in charm rotor frame
        :param charm_te: trailing edge coordinates in charm rotor frame
        :param toc: thickness/chord
        :param vsp_wing_origin: x, y, z location of root chord in VSP coordinates
        :param vsp_origin_2_wing_origin: transformation matrix from default vsp origin to the wing origin
        :param span: projected wing span
        :param airfoils: list of CharmAirfoilSections
        :param geom_id: id of the wing geometry in VSP
        :param num_syms: number of planar symmetries for this wing
        :param flip_rotation_direction: boolean to indicate if the rotation direction of this wing should be reversed
        reversed

        """
        self.charm_le = charm_le
        self.charm_te = charm_te
        self.vsp_wing_origin = vsp_wing_origin
        self.vsp_origin_2_wing_origin = vsp_origin_2_wing_origin
        self.toc = toc
        self.span = span
        self.airfoils = airfoils
        self.geom_id = geom_id
        self.num_syms = num_syms
        self.flip_rotation_direction = flip_rotation_direction


class CharmAtmosphereData:
    def __init__(self, speed_of_sound: float, density: float):
        """
        Class to hold charm atmosphere data
        :param speed_of_sound: speed of sound in charm unit system (either m/s or ft/s)
        :param density: density of air in charm unit system (either slugs/ft^3 or kg/m^3)
        """
        self.speed_of_sound = speed_of_sound
        self.density = density


def create_charm_blade_geom_file_from_propeller(prop_dg: dg.DegenComponent, unit_factor=u.in2ft,
                                                bg2charm_path=DEFAULT_BG2CHARM,
                                                nspan_override=None, nchord=1, **kwargs):
    """
    Create charm blade geometry file (*bg file)
    :param prop_dg: degen object of propeller to create bg file from
    :param unit_factor: unit conversion factor to go from units in vsp to feet
    :param bg2charm_path: path to bg2charm executable
    :param nspan_override: number of spanwise segments for the vortext lattice override
    :param nchord: number of chordwise segments for the vortex lattice panels
    :return: string of bg file contents
    """

    # Use first surface from first copy since all blades should be the same
    prop_blade = prop_dg.copies[0][0]
    thickness = np.array(prop_blade.sticks[0].toc)

    # Transform degen geom output into charm coordinates
    leading_edge_charm, trailing_edge_charm = remove_le_te_transformations(prop_blade)

    # Create input settings for CHARM te and le transformation utilities
    # For CHARM x is out the radius and y is leading edge to trailing edge
    # The untransformed vsp blade has y out the radius and x going leading edge to trailing edge. Therefore, we need
    # to swap the x/z axes before running the CHARM utility
    # z is positive in the down direction

    data = []  # lex ley lez tex tey tez toc
    for i in range(leading_edge_charm.shape[1]):
        row = [leading_edge_charm[1, i], leading_edge_charm[2, i], leading_edge_charm[0, i],
               trailing_edge_charm[1, i], trailing_edge_charm[2, i], trailing_edge_charm[0, i],
               thickness[i]]
        data.append(row)

    data = np.array(data)

    # Run the utility in a temporary directory
    return create_charm_blade_geom_file(data[:, 0:3].T, data[:, 3:6].T, thickness, unit_factor=unit_factor,
                                        bg2charm_path=bg2charm_path, nspan_override=nspan_override, nchord=nchord)


def create_charm_blade_geom_file_from_wing(wing_info: CharmWingInfo, unit_factor=u.in2ft,
                                           bg2charm_path=DEFAULT_BG2CHARM,
                                           nspan_override=None, **kwargs):
    """

    :param wing_info: wing info object
    :param unit_factor: unit conversion factor to go from units in vsp to feet
    :param bg2charm_path: path to bg2charm executable
    :param nspan_override: number of spanwise segments for the vortext lattice override
    :return: string of bg file contents
    """
    return create_charm_blade_geom_file(wing_info.charm_le, wing_info.charm_te, wing_info.toc, unit_factor=unit_factor,
                                        bg2charm_path=bg2charm_path, nspan_override=nspan_override, **kwargs)


def create_charm_blade_geom_file(leading_edge, trailing_edge, thickness, unit_factor=u.in2ft,
                                 bg2charm_path=DEFAULT_BG2CHARM,
                                 nspan_override=None, nchord=1, flap_type=None, flap_length=None, flap_defl=None,
                                 **kwargs):
    """
    Creates a CHARM blade geometry input file

    :param leading_edge: leading points (x,y,z) each point is a column
    :param trailing_edge: trailing edge points (x,y,z) each point is a column
    :param thickness: thickness/chord array
    :param unit_factor: unit conversion factor to go from units in vsp to feet
    :param bg2charm_path: path to bg2charm executable
    :param nspan_override: number of spanwise segments for the vortext lattice override
    :param nchord: number of chordwise segments for the vortex lattice panels
    :param flap_type: flap type input for segment ISEG; =0 no flap, =1 plain flap (nicolai), =-1 plain flap
    (Torenbeek), =2 split flap (Nicolai)
    :param flap_length: flap length for segment ISEG, percent chord, (0.0 - 0.5)
    :param flap_defl: flap deflection of segment ISEG in degrees (0 - 40)
    :return: string of blade geometry file
    """

    logger = ul.StyleAdapter(logging.getLogger(__name__), default_style='{')

    data = np.concatenate((leading_edge.T, trailing_edge.T, thickness.reshape(len(thickness), 1)), 1)

    nseg = data.shape[0]-1
    if nseg > 50:
        raise ValueError("Number of segments exceeds 50, incompatible with CHARM. Reduce tesselation.")

    bg_file_contents = None
    with RunManager() as rd:
        np.savetxt("blade.inp", data, delimiter=' ', header=' lex ley lez tex tey tez toc', comments='*', fmt='%.10f')

        # build up command inputs for utility
        cmd = "blade.inp\n"
        cmd += "bladebg.inp\n"
        cmd += "4\n"
        cmd += "0\n"
        cmd += "{:10f}\n".format(unit_factor)

        p = Popen([bg2charm_path], stdout=PIPE, stdin=PIPE, stderr=STDOUT, universal_newlines=True)
        stdout = p.communicate(input=cmd)[0]

        # Read contents into memory
        with open("bladebg.inp", "r") as f:
            bg_file_contents = f.read()

        # If the nspan override is not none, modified the file contents
        if nspan_override is not None:
            # Warn if override is not a valid value
            if abs(nspan_override) < 2 or abs(nspan_override) > 100 or abs(nspan_override) < nseg:
                logger.warning("nspan override is invalid. using nspan=50")
                nspan_override = 50

            re_expr = re.compile(r"(^NCHORD\s+NSPAN\s+ICOS\s?\n\s+[0-9]+\s+)([0-9]+)\s", re.MULTILINE)
            bg_file_contents = re.sub(re_expr, r"\g<1>{}".format(nspan_override), bg_file_contents)

        # Modify nchord
        re_expr = re.compile(r"(^NCHORD\s+NSPAN\s+ICOS\s?\n\s+)([0-9])", re.MULTILINE)
        bg_file_contents = re.sub(re_expr, r"\g<1>{}".format(nchord), bg_file_contents)

        if flap_type is not None:
            if len(flap_type) != nseg:
                raise ValueError(f"The flap_type list length must equal number of wing segments.  "
                                 f"len(flap_type)={len(flap_type)}, nseg={nseg}.")
            re_expr_type = re.compile(r"((^|\s)KFLAP\(ISEG\)\n\s*)([0-9]+\*0)")
            kflapstr = "  ".join(map(str, flap_type))
            bg_file_contents = re.sub(re_expr_type, r"\g<1>{}".format(kflapstr), bg_file_contents)

            if len(flap_length) != nseg:
                raise ValueError(f"The flap_length list length must equal number of wing segments.  len("
                                 f"flap_length)={len(flap_length)}, nseg={nseg}.")
            re_expr_length = re.compile(r"((^|\s)FLAPND\(ISEG\)\n\s*)([0-9]+\*0.0)")
            flapndstr = "  ".join(map(lambda x: f"{x:0.4f}", flap_length))
            bg_file_contents = re.sub(re_expr_length, r"\g<1>{}".format(flapndstr), bg_file_contents)

            if len(flap_defl) != nseg:
                raise ValueError(f"The flap_defl list length must equal number of wing segments.  len("
                                 f"flap_defl)={len(flap_defl)}, nseg={nseg}.")
            re_expr_defl = re.compile(r"((^|\s)FLDEFL\(ISEG\)\n\s*)([0-9]+\*0.0)")
            fldeflstr = "  ".join(map(lambda x: f"{x:0.4f}", flap_defl))
            bg_file_contents = re.sub(re_expr_defl, r"\g<1>{}".format(fldeflstr), bg_file_contents)

    return bg_file_contents


def create_single_wing_info(wing_dg: dg.DegenComponent, settings: CharmRotorSettingsCollection):
    """
    Creates wing information object from degen geom component

    :param wing_dg: degen object of wing to create bg file from
    :param settings: rotor settings for this wing component
    :return: wing information objects
    """

    # Determine if the wing components should be connected as one
    leading_edges = []
    trailing_edges = []
    tocs = []
    transmats = []
    u_vals = []
    connected = False
    if wing_dg.max_copies == 2 and settings.merge_wings:
        wing1 = wing_dg.copies[0][0]  # Get first wing
        wing2 = wing_dg.copies[1][0]  # Get second wing

        # See if both leading edge and trailing edge are connected
        le1 = wing1.sticks[0].le
        te1 = wing1.sticks[0].te
        toc1 = wing1.sticks[0].toc
        u1 = wing1.sticks[0].u
        le2 = wing2.sticks[0].le
        te2 = wing2.sticks[0].te
        toc2 = wing2.sticks[0].toc
        u2 = wing2.sticks[0].u

        arr1 = np.append(np.array(le1), np.array([toc1, u1]).T, 1)
        arr2 = np.append(np.array(le2), np.array([toc2, u2]).T, 1)

        connected_le = _connect_arrays(arr1, arr2)
        connected_te = _connect_arrays(np.array(te1), np.array(te2))

        connected = connected_le is not None and connected_te is not None

        if connected:
            leading_edges.append(connected_le[:, 0:3])
            tocs.append(connected_le[:, 3])
            trailing_edges.append(connected_te)
            transmats.append(wing1.transmat)
            u_vals.append(connected_le[:, 4])

    if not connected:
        for i in range(wing_dg.max_copies):
            wing = wing_dg.copies[i][0]
            leading_edges.append(np.array(wing.sticks[0].le))
            trailing_edges.append(np.array(wing.sticks[0].te))
            transmats.append(wing.transmat)
            tocs.append(np.array(wing.sticks[0].toc))
            u_vals.append(np.array(wing.sticks[0].u))

    wing_infos = []
    for i in range(len(leading_edges)):
        le_vsp = leading_edges[i]
        te_vsp = trailing_edges[i]
        toc = tocs[i]
        inverse_vsp_tmat = transmats[i].get_inverse_transform()

        le_vsp_untrans = inverse_vsp_tmat.apply_transformation(le_vsp.T)

        # Check to see if the root le the origin, if not transform it to be so
        t_vsp2origin = TransMatrix(inverse_vsp_tmat.mat)
        if not _is_same_point(np.zeros(3), le_vsp_untrans[:, 0]):
            wing_origin2vsp_origin = TransMatrix()
            wing_origin2vsp_origin.set_translations(-1.0*le_vsp_untrans[:, 0])
            t_vsp2origin.mat = np.dot(wing_origin2vsp_origin.mat, t_vsp2origin.mat)

        t_vsp2charm_blade = TransMatrix()
        t_vsp2charm_blade.mat[0, 0:3] = np.array([0.0, 1.0, 0.0])
        t_vsp2charm_blade.mat[1, 0:3] = np.array([1.0, 0.0, 0.0])
        t_vsp2charm_blade.mat[2, 0:3] = np.array([0.0, 0.0, -1.0])

        le_charm = t_vsp2charm_blade.apply_transformation(t_vsp2origin.apply_transformation(le_vsp.T))
        te_charm = t_vsp2charm_blade.apply_transformation(t_vsp2origin.apply_transformation(te_vsp.T))
        vsp_origin = le_vsp.T[:, 0]

        span = abs(le_vsp_untrans[1, 0] - le_vsp_untrans[1, -1])

        # Get airfoils
        airfoils = []
        specified_airfoils = settings[i].airfoil_opts
        airfoil_r_o_Rs = settings[i].airfoil_r_o_Rs

        if len(airfoil_r_o_Rs) < 2:
            airfoil_r_o_Rs = []
            for u_index in range(len(u_vals[i])):
                u = u_vals[i][u_index]
                if math.isclose(u, round(u)):
                    local_span = 0.75 * le_charm[0, u_index] + 0.25 * te_charm[0, u_index]
                    airfoil_r_o_Rs.append(local_span / span)

        for irad, roR in enumerate(airfoil_r_o_Rs):
            af_opts = settings[i].default_airfoil_opts if irad >= len(specified_airfoils) else specified_airfoils[irad]
            airfoil = get_airfoil(AirfoilDataLocation.FILE_LOCATION, af_opts)
            airfoils.append(CharmAirfoilSection(airfoil=airfoil, radius_frac=roR))

        num_syms = _determine_num_symm(trans_orig=wing_dg.copies[0][0].transmat,
                                       trans_copy=wing_dg.copies[i][0].transmat)
        flip_rotation = False
        if not wing_dg.copies[i][0].flip_normal:
            flip_rotation = True
        wing_info = CharmWingInfo(charm_le=le_charm, charm_te=te_charm, toc=toc, vsp_wing_origin=vsp_origin,
                                  vsp_origin_2_wing_origin=t_vsp2origin, span=span, airfoils=airfoils,
                                  geom_id=wing_dg.copies[i][0].geom_id,
                                  num_syms=num_syms, flip_rotation_direction=flip_rotation)
        wing_infos.append(wing_info)

    return wing_infos


def create_all_wing_infos(degen_mgr: dg.DegenGeomMgr, settings: CharmRotorSettingsCollection):
    """
    Creates a list of wing info objects from all wing objects in the degen geom manager
    :param degen_mgr: degen geom manager
    :param settings: settings for all wing objects
    :return: list of wing info objects
    """
    wing_infos = []
    for wing_id, degen_component in degen_mgr.degen_objs.items():
        typ_name = vsp.GetGeomTypeName(wing_id)
        if typ_name != "Wing":
            continue
        wing_infos.extend(create_single_wing_info(degen_component, settings[wing_id]))
    return wing_infos


def remove_le_te_transformations(degen_blade: dg.DegenGeom):
    """
    Removes transformations from leading and trailing edges of a blade
    :param degen_blade: degen geom object of a single blade
    :return: leading edge and trailing edge coordinates without transformations
    """

    transform_inverse = degen_blade.transmat.get_inverse_transform()
    leading_edge_original = np.array(degen_blade.sticks[0].le)
    trailing_edge_original = np.array(degen_blade.sticks[0].te)

    leading_edge_charm = transform_inverse.apply_transformation(leading_edge_original.T)
    trailing_edge_charm = transform_inverse.apply_transformation(trailing_edge_original.T)

    rotation_deg = vsp.GetParmVal(degen_blade.geom_id, "Rotate", "Design")
    reverse_flag = vsp.GetParmVal(degen_blade.geom_id, "ReverseFlag", "Design")
    axis = np.array([-1.0, 0.0, 0.0])
    if reverse_flag > 0.5:
        axis[0] *= -1.0
    rot_mat = TransMatrix.create_from_axis_angle(axis, rotation_deg * u.deg2rad).get_inverse_transform()

    # if reversed, make y negative y and rotate 180 degrees about x
    if reverse_flag:
        ynegy = TransMatrix()
        ynegy.mat[1, 1] = -1.0
        rotx = TransMatrix.create_from_axis_angle(np.array([1.0, 0.0, 0.0]), angle=180*u.deg2rad)
        rot_mat.mat = np.dot(ynegy.mat, rot_mat.mat)
        rot_mat.mat = np.dot(rotx.mat, rot_mat.mat)

    leading_edge_charm = rot_mat.apply_transformation(leading_edge_charm)
    trailing_edge_charm = rot_mat.apply_transformation(trailing_edge_charm)

    return leading_edge_charm, trailing_edge_charm


def create_rigid_blade_dynamics_file(radius, unit_factor=u.in2ft, iaero=1, irvflo=0, isv66=False, isrotor=True,
                                     **kwargs):
    """
    Creates a rigid blade geometry file for degenerate propeller component
    :param radius: radius of propeller (span of wing)
    :param unit_factor: conversion factor to convert to feet
    :param iaero: value of IAERO variable in blade dynamics file. Controls used for computing lift on rotor.
    :param irvflo: value of IRVFLO var in blade dynamics file. Controls used for computing reverse flow lift on rotor.
    :param isv66: boolean, set to True if running CHARM v6.6
    :param isrotor: boolean, True if rotor, False if wing, used to determine ISTFLO input value
    :return:
    """

    # Get propeller info
    with io.StringIO() as bd_file:

        def write_array(arr):
            for i, el in enumerate(arr):
                if i > 0 and i % 10 == 0:
                    bd_file.write("\n")
                bd_file.write("{:8.3f}".format(el))
            bd_file.write("\n")

        # Write out title line
        bd_file.write("bladebd.inp\n")

        # For rigid blade set ISTRM = 0
        bd_file.write(" ISTRM\n")
        bd_file.write("   0\n")

        # Not sure what these do, but copying Dan
        bd_file.write("ISTRIP   IFPC  IAERO\n")
        bd_file.write("  -1      0      {:d}\n".format(iaero))
        bd_file.write("ICOMP   IRVFLO  ISTFLO\n")
        if isrotor:
            istflo_val = "2"
        else:
            istflo_val = "3"
        bd_file.write("  1       {:d}       ".format(irvflo) + istflo_val + "\n")
        bd_file.write(" IART   HINGE  PRECONE\n")
        bd_file.write("  0    0.000   0.000\n")
        bd_file.write(" NMODE\n")
        bd_file.write("   1\n")
        bd_file.write(" NMDFLP   NMDTOR  NMDLAG  NMDELG\n")
        bd_file.write("   1        0       0       0\n")
        if not isv66:
            bd_file.write(" IFXMDE\n")
            bd_file.write("   1\n")
            bd_file.write("IMDFIX  NPSIFX\n")
            bd_file.write("  1      20\n")
            bd_file.write("AMP\n")
            bd_file.write(" 20*0.0\n")
        else:
            bd_file.write(" IFXMDE IRIGID\n")
            bd_file.write("   0      1\n")

        bd_file.write(" FREQMD   GMASS     for Mode 1\n")
        bd_file.write("  1.000   1000.0\n")
        bd_file.write(" NRBARS\n")
        bd_file.write("   51\n")

        bd_file.write(" RBARS(IR), IR=1,NRBARS\n")

        # Compute rbars
        nrbars = 51
        rbars = np.array([float(i)/(nrbars-1) for i in range(nrbars)])
        write_array(rbars)

        bd_file.write("  Rigid flap mode\n")
        write_array(np.zeros(nrbars))
        write_array(np.zeros(nrbars))

        write_array(rbars*radius*unit_factor)
        write_array(np.zeros(nrbars))
        write_array(np.ones(nrbars))
        write_array(np.zeros(nrbars))
        return bd_file.getvalue()


def create_airfoil_file(airfoils):
    """
    Creates a CHARM *af.inp file given a list of airfoils
    :param airfoils: airfoil list
    :return: string of airfoil file contents
    """

    # Check that type of reynolds data is consistent across all airfoils
    if len(airfoils) == 0:
        return
    reyn_type = airfoils[0].airfoil.reynolds_data_type
    for af in airfoils:
        if af.airfoil.reynolds_data_type != reyn_type:
            raise ValueError("All airfoils must have same reynolds number correction information type")

    with io.StringIO() as af_file:
        num_airfoils = len(airfoils)
        if num_airfoils < 2:
            raise ValueError("must have at least two airfoils in order to be valid")
        if num_airfoils > 20:
            raise ValueError("CHARM cannot handle more than 20 airfoils")

        ithick = 0  # Assuming no thickness profiles for sections
        af_file.write("{:4} {:3} {:3}\n".format(num_airfoils, ithick, reyn_type.value))

        # Write out radial locations
        af_file.write("  ")
        for af in airfoils:
            af_file.write(" {:6.4f}".format(af.radius_frac))
        af_file.write("\n")

        # Write out thickness
        af_file.write("  ")
        for af in airfoils:
            af_file.write(" {:6.4f}".format(af.airfoil.thickness))
        af_file.write("\n")

        # Write out section data
        for af in airfoils:
            af_file.write("COMMENT#1\n")
            af_file.write(af.airfoil.section_string)

        return af_file.getvalue()


def get_airfoil(af_data_location: AirfoilDataLocation, af_options=None):
    """
    Gets an airfoil from the specified data location
    :param af_data_location: enum indicating the type of location from which the data should be retrieved
    :param af_options: options related to retrieving airfoil, if None is specified the default options will be used
    :return: Charm airfoil section
    """

    if af_data_location == AirfoilDataLocation.FILE_LOCATION:
        if af_options is None:
            af_options = AirfoilFileLocationOpt(filename='0012.inp')
        return _get_airfoil_from_file(af_options)
    else:
        raise ValueError("Unhandled data location: {}".format(af_data_location))


def create_airfoil_sections(prop_id, rotor_setting: CharmRotorSettingsCollection):
    """
    Creates a set CHARM airfoil sections based off of prop id
    :param prop_id: vsp geom id for propeller of interest
    :param rotor_setting: rotor settings for a single rotor. This is a :class:`CharmRotorSettingsCollection` object
    because it includes settings for symmetric copies
    :return: charm airfoil sections
    """
    xsec_surf_id = vsp.GetXSecSurf(prop_id, 0)
    num_xsec = vsp.GetNumXSec(xsec_surf_id)

    airfoils = []

    specified_airfoils = rotor_setting[0].airfoil_opts
    default_af_opts = rotor_setting[0].default_airfoil_opts
    airfoil_r_o_Rs = rotor_setting[0].airfoil_r_o_Rs

    if len(airfoil_r_o_Rs) < 2:
        airfoil_r_o_Rs = []
        for i in range(num_xsec):
            xsec_id = vsp.GetXSec(xsec_surf_id, i)
            rad_frac = vsp.GetParmVal(vsp.GetXSecParm(xsec_id, "RadiusFrac"))
            airfoil_r_o_Rs.append(rad_frac)

    for irad, roR in enumerate(airfoil_r_o_Rs):
        af_opts = default_af_opts if irad >= len(specified_airfoils) else specified_airfoils[irad]
        charm_af = get_airfoil(AirfoilDataLocation.FILE_LOCATION, af_options=af_opts)
        charm_af_sect = CharmAirfoilSection(airfoil=charm_af, radius_frac=roR)
        airfoils.append(charm_af_sect)

    return airfoils


def create_rotor_wake_file_from_template(blade_degens, rotor_settings: CharmRotorSettings, num_psi,
                                         num_syms, unit_factor=u.in2ft):
    """
    Modifies rotor wake template file and updates it with proper position of propeller/rotor along with
    input settings

    :param blade_degens: list of degen geoms for all of the blades associated with this prop/rotor
    :param rotor_settings: settings for this propeller/rotor
    :param num_psi: number of azimuthal points in CHARM computation, required to determine precision of azimuthal offset
    :param num_syms: number of planar symmetries from original copy to this copy
    :param unit_factor: factor to convert from vsp units to CHARM units
    :return: string containing modified rotor wake file
    """

    # Get number of blades
    num_blades = len(blade_degens)

    # Compute radians per second
    omega = rotor_settings.rpm * u.rpm2rad_s

    # Compute positioning
    prop_info = vsp.get_single_propeller_info(blade_degens[0])

    vsp_rot_angle = vsp.GetParmVal(blade_degens[0].geom_id, "Rotate", "Design")

    # CHARM rotor to propeller transformation
    phi_offset = -90.0
    phi_offset_sym_dir = 1.0
    if num_syms % 2 == 1:
        phi_offset_sym_dir = -1.0
    charmrotor2propeller = TransMatrix(np.dot(TransMatrix.create_from_axis_angle([1.0, 0.0, 0.0],
                                                                                 phi_offset_sym_dir*phi_offset*u.deg2rad).mat,
                                              TransMatrix.create_from_axis_angle(np.array([0, 1.0, 0]), -90.0*u.deg2rad).mat))

    # CHARM Body Frame to vsp Frame
    charm2vsp = TransMatrix()
    charm2vsp.mat[0, 0] = -1.0
    charm2vsp.mat[2, 2] = -1.0

    # VSP Frame to CHARM Body Frame
    vsp2charm = charm2vsp.get_inverse_transform()

    # Compute transform from charm blade frame to vsp coordinates
    charm2vspprop = TransMatrix(np.dot(charm2vsp.mat, charmrotor2propeller.mat))

    # Apply vsp transformation matrix
    vsp_trans = TransMatrix(np.dot(prop_info.transmat.mat,
                                   TransMatrix.create_from_axis_angle(
                                       np.array([1.0*prop_info.rotation_direction, 0.0, 0.0]),
                                       vsp_rot_angle*u.deg2rad*phi_offset_sym_dir).mat))
    final_trans_mat = TransMatrix(np.dot(vsp_trans.mat, charm2vspprop.mat))

    # Convert back to charm aircraft coordinates
    final_trans_mat.mat = np.dot(vsp2charm.mat, final_trans_mat.mat)

    # Get rotation angles
    xrot, yrot, zrot = final_trans_mat.get_angles_zxy()

    # Limit zrotation to an acceptable increment
    dpsi = 360.0/float(num_psi)
    zrot = round(zrot/dpsi) * dpsi

    itilt = 2
    irotat = prop_info.rotation_direction

    charm_trans = TransMatrix()
    charm_trans.mat[0, 0] = -1.0
    charm_trans.mat[2, 2] = -1.0

    # Create positioning string
    pos_string = "{:2d} {:8.3f} {:8.3f} {:8.3f} {xrot:8.3f} {yrot:8.3f} {zrot:8.3f} {itilt:3d}\n".format(irotat,
                                    *charm_trans.apply_transformation(prop_info.hub_center*unit_factor).reshape(3),
                                                                                       xrot=xrot, yrot=yrot,
                                                                                       zrot=zrot,
                                                                                       itilt=itilt)

    rw_file = rotor_settings.rotor_wake_template
    rw_file[2] = "{:3d}     {:8.4f}\n".format(num_blades, omega)
    rw_file[4] = pos_string

    rw_file = __modify_rotor_wake_template(rw_file, rotor_settings)

    with io.StringIO() as f:
        f.writelines(rw_file)
        return f.getvalue()


def create_wing_rotor_wake_file_from_template(wing_info: CharmWingInfo, wing_settings: CharmRotorSettings, num_psi,
                                              unit_factor=u.in2ft):
    """
    Creates a rotor wake file for a wing described by a wing info object and a rotor settings
    :param wing_info: wing information object for this wing
    :param wing_settings: settings for this wing
    :param num_psi: number of azimuthal points in CHARM computation, required to determine precision of azimuthal offset
    :param unit_factor: factor to convert from vsp units to CHARM units
    :return: string containing modified rotor wake file
    """

    # Get number of blades
    num_blades = 1

    # Compute radians per second
    omega = wing_settings.rpm * u.rpm2rad_s

    # Compute positioning
    phi_offset_dir = 1.0
    if wing_info.num_syms % 2 == 1:
        phi_offset_dir = -1.0
    charmrotor2wing = TransMatrix.create_from_axis_angle([0.0, 0.0, 1.0], 90.0*phi_offset_dir*u.deg2rad)

    # CHARM Body Frame to vsp Frame
    charm2vsp = TransMatrix()
    charm2vsp.mat[0, 0] = -1.0
    charm2vsp.mat[2, 2] = -1.0

    # VSP Frame to CHARM Body Frame
    vsp2charm = charm2vsp.get_inverse_transform()

    # Compute transform from charm blade frame to vsp coordinates
    charm2vspwing = TransMatrix(np.dot(charm2vsp.mat, charmrotor2wing.mat))

    # Apply vsp transformation matrix
    vsp_trans = wing_info.vsp_origin_2_wing_origin.get_inverse_transform()
    final_trans_mat = TransMatrix(np.dot(vsp_trans.mat, charm2vspwing.mat))

    # Convert back to charm aircraft coordinates
    final_trans_mat.mat = np.dot(vsp2charm.mat, final_trans_mat.mat)

    # Get rotation angles
    xrot, yrot, zrot = final_trans_mat.get_angles_zxy()

    # Limit zrotation to an acceptable increment
    dpsi = 360.0/float(num_psi)
    zrot = round(zrot/dpsi) * dpsi

    itilt = 2
    irotat = 1
    if wing_info.flip_rotation_direction:
        irotat = -1

    charm_trans = TransMatrix()
    charm_trans.mat[0, 0] = -1.0
    charm_trans.mat[2, 2] = -1.0

    # Create positioning string
    pos_string = "{:2d} {:8.3f} {:8.3f} {:8.3f} {xrot:8.3f} {yrot:8.3f} {zrot:8.3f} {itilt:3d}\n".format(irotat,
                                    *charm_trans.apply_transformation(wing_info.vsp_wing_origin*unit_factor).reshape(3),
                                                                                       xrot=xrot, yrot=yrot,
                                                                                       zrot=zrot,
                                                                                       itilt=itilt)

    rw_file = wing_settings.rotor_wake_template
    rw_file[2] = "{:3d}     {:8.4f}\n".format(num_blades, omega)
    rw_file[4] = pos_string

    rw_file = __modify_rotor_wake_template(rw_file, wing_settings)

    with io.StringIO() as f:
        f.writelines(rw_file)
        return f.getvalue()


def build_default_rotor_settings(degen_mgr: dg.DegenGeomMgr, default_rpm=0.0, default_template=None)\
        -> CharmRotorSettingsCollection:
    """
    Builds up a default collection of rotor settings

    :param degen_mgr: degen_mgr with objects desired for this charm run
    :param default_rpm: this value will be used as the rpm value for all rotors
    :param default_template: this will used as the template file by default for all rotors/propellers
    :return: CharmRotorsSettingsCollection object
    """

    # provided default template current template is empty
    if default_template is None:
        with open(os.path.join(os.path.dirname(__file__), "test", "prop_rw.inp")) as f:
            default_template = f.readlines()

    # if the default template is a string, assume it is a filename that should be read in
    if isinstance(default_template, str):
        with open(default_template, "r") as f:
            default_template = f.readlines()

    # Loop over each geom id
    default_settings = CharmRotorSettingsCollection()
    for geom_id, dg_component in degen_mgr.degen_objs.items():
        # If this geom is not a prop, move on to next item
        typename = vsp.GetGeomTypeName(geom_id)
        if typename != "Propeller":
            continue
        # Create a collection of settings for this geom
        settings_collection = CharmRotorSettingsCollection()

        # Create a new setting for each copy in the geom
        for copy_num in dg_component.copies.keys():
            settings_collection[copy_num] = CharmRotorSettings()

        default_settings[geom_id] = settings_collection

    default_settings.rpm = default_rpm
    default_settings.rotor_wake_template = default_template

    return default_settings


def build_default_wing_settings(degen_mgr: dg.DegenGeomMgr, default_template=None):
    """
    Builds a default settings collection of rotor settings for wing components

    :param degen_mgr: degen geom manager with objects desired for this charm run
    :param default_template: this will be used as the default template for all wing like objects
    :return: CharmRotorSettingsCollection object with settings for wing components
    """
    # provided default template current template is empty
    if default_template is None:
        with open(os.path.join(os.path.dirname(__file__), "test", "prop_rw.inp")) as f:
            default_template = f.readlines()

    # if the default template is a string, assume it is a filename that should be read in
    if isinstance(default_template, str):
        with open(default_template, "r") as f:
            default_template = f.readlines()

    # Loop over each geom id
    default_settings = CharmRotorSettingsCollection()
    for geom_id, dg_component in degen_mgr.degen_objs.items():
        # If this geom is not a prop, move on to next item
        typename = vsp.GetGeomTypeName(geom_id)
        if typename != "Wing":
            continue
        # Create a collection of settings for this geom
        settings_collection = CharmRotorSettingsCollection()

        # Create a new setting for each copy in the geom
        for copy_num in dg_component.copies.keys():
            settings_collection[copy_num] = CharmRotorSettings()

        default_settings[geom_id] = settings_collection

    default_settings.rpm = 0.0
    default_settings.rotor_wake_template = default_template

    return default_settings


def build_default_trim_settings(trim_template=None) -> CharmTrimSettings:

    trim_template_name = None  # Initialize
    # provided default template current template is empty
    if trim_template is None:
        trim_template_name = "trim.inp"  # tracking template filename
        with open(os.path.join(os.path.dirname(__file__), "test", trim_template_name)) as f:
            trim_template = f.readlines()

    # if the default template is a string, assume it is a filename that should be read in
    if isinstance(trim_template, str):
        trim_template_name = trim_template
        with open(trim_template, "r") as f:
            trim_template = f.readlines()

    # These line numbers will not change
    line_forces_moments = 2
    line_trim_targets = 4
    line_trim_tolerance = 6
    line_pilot_controls = 8
    line_attitude_controls = 10
    line_num_aero_comps = 12
    line_comp_cntrls_start = 14  # This is the start of the component controls section
    line_init_controls = -1  # The last line of file will always be this

    # ToDo: would be nice to define a function to make this not so repetitive, would need to loop over the object
    #  variable assignments, which I don't know how to do
    default_settings = CharmTrimSettings()
    data = trim_template[line_forces_moments].split()
    default_settings.forces_moments = list(map(int, data))
    data = trim_template[line_trim_targets].split()
    default_settings.targets = list(map(float, data))
    data = trim_template[line_trim_tolerance].split()
    default_settings.tol = list(map(float, data))
    data = trim_template[line_pilot_controls].split()
    default_settings.pilot_cntrls = list(map(int, data))
    data = trim_template[line_attitude_controls].split()
    default_settings.attitude_cntrls = list(map(int, data))
    default_settings.num_aero_comps = int(trim_template[line_num_aero_comps])
    data = trim_template[line_init_controls].split()
    default_settings.init_cntrls = list(map(float, data))
    default_settings.template = trim_template_name

    ctrl_expr = re.compile(r"(^|\s)Ctrl\s")
    init_expr = re.compile(r"(^|\s)Initial\s")
    line_cntrl_gains = 0  # Initialize variable
    line_init_labels = 0  # Initiailize variable
    for linenum in range(len(trim_template)):
        line = trim_template[linenum]
        if re.search(ctrl_expr, line):
            line_cntrl_gains = linenum + 1
        if re.search(init_expr, line):
            line_init_labels = linenum
    shape_comp_cntrls = (line_cntrl_gains - 1 - line_comp_cntrls_start, 10)
    shape_cntrl_gains = (line_init_labels - line_cntrl_gains, 13)

    default_settings.aero_comp_cntrls = np.zeros(shape_comp_cntrls, dtype=int)
    default_settings.cntrl_gains = np.zeros(shape_cntrl_gains, dtype=float)
    for i in range(line_comp_cntrls_start, line_cntrl_gains-1):
        if line_cntrl_gains == 0:
            raise ValueError("RegEx for 'Ctrl' not found in trim input file.")
        data = trim_template[i].split()
        default_settings.aero_comp_cntrls[i-line_comp_cntrls_start, :] = np.array(data[0:10])
    for i in range(line_cntrl_gains, line_init_labels):
        if line_init_labels == 0:
            raise ValueError("RegEx for 'Initial' not found in trim input file.")
        data = trim_template[i].split()
        default_settings.cntrl_gains[i-line_cntrl_gains, :] = np.array(data[0:13])

    return default_settings


def create_rotor_file_list(degen_mgr: dg.DegenGeomMgr, settings: CharmRotorSettingsCollection, num_psi,
                           unit_factor=u.in2ft, **kwargs):
    """
    Creates a list of input files for charm based off of degen geom objects and rotor settings
    :param degen_mgr: degen objects containing components to export to charm
    :param settings: rotor settings relating to each rotor to be exported to charm
    :param num_psi: number of azimuthal points in CHARM computation
    :param unit_factor: unit conversion factor to get from vsp units to feet
    :return: dictionary of filename, file contents to write to disk, dictionary of rotorname to input filenames to be
    written in the run characteristics input file
    """

    rotor_files = {}
    files_to_write = {}
    for geom_count, (rotor_id, rotor_settings) in enumerate(settings.items()):
        # Create BG File
        bg_file = create_charm_blade_geom_file_from_propeller(degen_mgr.degen_objs[rotor_id], unit_factor=unit_factor,
                                                              nspan_override=rotor_settings.nspan_override,
                                                              nchord=rotor_settings.nchord, **kwargs)

        # Create BD File
        prop_info = vsp.get_single_propeller_info(degen_mgr.degen_objs[rotor_id].copies[0][0])
        bd_file = create_rigid_blade_dynamics_file(prop_info.diameter/2.0, unit_factor=unit_factor,
                                                   iaero=rotor_settings.iaero, irvflo=rotor_settings.irvflo,
                                                   isrotor=True, **kwargs)

        # Create Airfoil Files
        airfoils = create_airfoil_sections(rotor_id, rotor_settings)
        af_file = create_airfoil_file(airfoils)

        # Add blade specific files to files to write list based off of geom name
        basename = vsp.GetGeomName(rotor_id) + "_{:03}".format(geom_count)
        bg_file_name = _cleanup_filename_for_charm(basename + "bg.inp")
        bd_file_name = _cleanup_filename_for_charm(basename + "bd.inp")
        af_file_name = _cleanup_filename_for_charm(basename + "af.inp")

        files_to_write[bg_file_name] = bg_file
        files_to_write[bd_file_name] = bd_file
        files_to_write[af_file_name] = af_file

        # Create individual rotor specific files
        orig_trans_mat = degen_mgr.degen_objs[rotor_id].copies[0][0].transmat
        for rotor_count, rotor_setting in rotor_settings.items():
            # Create the rotor wake file
            copy_trans_mat = degen_mgr.degen_objs[rotor_id].copies[rotor_count][0].transmat
            num_syms = _determine_num_symm(trans_orig=orig_trans_mat, trans_copy=copy_trans_mat)
            rw_file = create_rotor_wake_file_from_template(degen_mgr.degen_objs[rotor_id].copies[rotor_count],
                                                           rotor_settings=rotor_setting, num_psi=num_psi,
                                                           unit_factor=unit_factor, num_syms=num_syms)

            rotor_base_name = _cleanup_filename_for_charm(basename + "_{:03}".format(rotor_count))
            rw_file_name = _cleanup_filename_for_charm(basename + "_{:03}rw.inp".format(rotor_count))
            files_to_write[rw_file_name] = rw_file

            rotor_files[rotor_base_name] = [rw_file_name, bg_file_name, bd_file_name, af_file_name, "none"]

    return files_to_write, rotor_files


def create_wing_file_list(wing_infos, settings: CharmRotorSettings, num_psi, unit_factor=u.in2ft, **kwargs):
    """

    :param wing_infos: wing info objects
    :param settings: wing settings
    :param num_psi: azimuthal resolution
    :param unit_factor: unit conversion factor from vsp units to charm units
    :param kwargs:
    :return: files to write, and file list for run characteristics file
    """

    wing_infos_dict = {}
    for wing_info in wing_infos:
        if wing_info.geom_id in wing_infos_dict:
            wing_infos_dict[wing_info.geom_id].append(wing_info)
        else:
            wing_infos_dict[wing_info.geom_id] = [wing_info]

    rotor_files = {}
    files_to_write = {}
    for geom_count, (wing_id, wing_info_list) in enumerate(wing_infos_dict.items()):
        for wing_count, wing_info in enumerate(wing_info_list):
            # Create BG File
            bg_file = create_charm_blade_geom_file_from_wing(wing_info, unit_factor=unit_factor,
                                                             nspan_override=settings[wing_id][wing_count].nspan_override,
                                                             nchord=settings[wing_id][wing_count].nchord,
                                                             flap_type=settings[wing_id][wing_count].flap_type,
                                                             flap_length=settings[wing_id][wing_count].flap_length,
                                                             flap_defl=settings[wing_id][wing_count].flap_defl,
                                                             **kwargs)

            # Create BD File
            bd_file = create_rigid_blade_dynamics_file(wing_info.span, unit_factor=unit_factor,
                                                       iaero=settings[wing_id][wing_count].iaero,
                                                       irvflo=settings[wing_id][wing_count].irvflo,
                                                       isrotor=False, **kwargs)

            # Create Airfoil Files
            af_file = create_airfoil_file(wing_info.airfoils)

            # Add blade specific files to files to write list based off of geom name
            basename = vsp.GetGeomName(wing_id) + "_{:03}_{:03}".format(geom_count, wing_count)
            bg_file_name = _cleanup_filename_for_charm(basename + "bg.inp")
            bd_file_name = _cleanup_filename_for_charm(basename + "bd.inp")
            af_file_name = _cleanup_filename_for_charm(basename + "af.inp")

            files_to_write[bg_file_name] = bg_file
            files_to_write[bd_file_name] = bd_file
            files_to_write[af_file_name] = af_file

            # Create the rotor wake file
            rw_file = create_wing_rotor_wake_file_from_template(wing_info, settings[wing_id][wing_count],
                                                                num_psi=num_psi, unit_factor=unit_factor)

            rw_file_name = _cleanup_filename_for_charm(basename + "rw.inp")
            files_to_write[rw_file_name] = rw_file

            rotor_files[basename] = [rw_file_name, bg_file_name, bd_file_name, af_file_name, "none"]

    return files_to_write, rotor_files


def create_trim_file_from_template(trim_template, trim_settings: CharmTrimSettings) -> dict:
    """
    Creates CHARM trim input file
    :param trim_template:
    :param trim_settings:
    :return:
    """

    if trim_template is None:
        with open(os.path.join(os.path.dirname(__file__), "test", "trim.inp")) as f:
            trim_file = f.readlines()

    # if the default template is a string, assume it is a filename that should be read in
    if isinstance(trim_template, str):
        with open(trim_template, "r") as f:
            trim_file = f.readlines()

    # These line numbers will not change
    line_forces_moments = 2
    line_trim_targets = 4
    line_trim_tolerance = 6
    line_pilot_controls = 8
    line_attitude_controls = 10
    line_num_aero_comps = 12
    line_comp_cntrls_start = 14  # This is the start of the component controls section
    line_init_controls = -1  # The last line of file will always be this
    init_line_string = trim_file[-2]

    # Find the line string for the control gains matrix
    expr = re.compile(r"(^|\s)Ctrl\s")
    cntrl_gain_string = None
    for linenum, line in enumerate(trim_file):
        if re.search(expr, line):
            cntrl_gain_string = line
    if cntrl_gain_string is None:
        raise ValueError("'Cntl' string was not found in trim input file")

    # ToDo: create a loop that uses trim_settings.__dict__.items() and iterate to reduce repeated code
    trim_file[line_forces_moments] = "  ".join(map(str, trim_settings.forces_moments)) + "\n"
    trim_file[line_trim_targets] = "  ".join(map(str, trim_settings.targets)) + "\n"
    trim_file[line_trim_tolerance] = "  ".join(map(str, trim_settings.tol)) + "\n"
    trim_file[line_pilot_controls] = "  ".join(map(str, trim_settings.pilot_cntrls)) + "\n"
    trim_file[line_attitude_controls] = "  ".join(map(str, trim_settings.attitude_cntrls)) + "\n"
    trim_file[line_num_aero_comps] = str(trim_settings.num_aero_comps) + "\n"
    # I need to create a loop here
    nrows_comp = trim_settings.aero_comp_cntrls.shape[0]
    for i in range(nrows_comp):
        trim_file[line_comp_cntrls_start+i] = "  ".join(map(str, trim_settings.aero_comp_cntrls[i, :])) + "\n"
    trim_file[line_comp_cntrls_start+nrows_comp] = cntrl_gain_string
    nrows_gain = trim_settings.cntrl_gains.shape[0]
    for i in range(nrows_gain):
        # ToDo: create a second, string creation loop
        trim_file.insert(line_comp_cntrls_start+nrows_comp+1+i, f"{int(trim_settings.cntrl_gains[i, 0]):d}  "
                                                                f"{trim_settings.cntrl_gains[i, 1]:.2f}  "
                                                                f"{trim_settings.cntrl_gains[i, 2]:.2f}  "
                                                                f"{trim_settings.cntrl_gains[i, 3]:.2f}  "
                                                                f"{trim_settings.cntrl_gains[i, 4]:.2f}  "
                                                                f"{trim_settings.cntrl_gains[i, 5]:.2f}  "
                                                                f"{trim_settings.cntrl_gains[i, 6]:.2f}  "
                                                                f"{trim_settings.cntrl_gains[i, 7]:.2f}  "
                                                                f"{trim_settings.cntrl_gains[i, 8]:.2f}  "
                                                                f"{trim_settings.cntrl_gains[i, 9]:.2f}  "
                                                                f"{trim_settings.cntrl_gains[i, 10]:.2f}  "
                                                                f"{trim_settings.cntrl_gains[i, 11]:.2f}  "
                                                                f"{trim_settings.cntrl_gains[i, 12]:.2f}\n"
                         )
    trim_file = trim_file[:line_comp_cntrls_start+nrows_comp+nrows_gain+1]
    trim_file.append(init_line_string)
    trim_file.append("  ".join(map(str, trim_settings.init_cntrls)) + "\n")
    files_to_write = {}

    with io.StringIO() as f:
        f.writelines(trim_file)
        files_to_write['trim.inp'] = f.getvalue()
    return files_to_write


def build_run_characteristics_file_from_template(rotor_files, template_filename=None, template_file=None, num_psi=None,
                                                 velocity=None, rotation_rates=None, pitch=None, weight=None, cg=None,
                                                 fuselage_data=None, atmos_data: CharmAtmosphereData = None, nrev=None,
                                                 convgVec=None, sframe=None, smode=None, az_sym=None, **kwargs):
    """
    Uses a template run characteristics file to create a run characteristic file that is updated with
    all of the rotor filenames that are input

    :param rotor_files: dictionary with key of rotor name and value of list of input files for that rotor
    :param template_filename: filename of run characteristics file template (will be open in read mode)
    :param template_file: file like object (something from io.IOBase) that can be read
    :param num_psi: number of azimuthal points for charm calculation
    :param velocity: velocity vector (ft/s)
    :param rotation_rates: rotational rates (P, Q, R)
    :param pitch: pitch in degrees, vector length matching the number of aircraft
    :param weight: aircraft weight in lb, vector length matching the number of aircraft
    :param cg: vector [x, y, z] for cg, number of vectors matching the number of aircraft
    :param fuselage_data: vector of fuselage reference areas/length for fuselage drag [FUSPLAN, FUSSIDEA, FUSREFL]
    :param atmos_data: atmosphere data (speed of sound and density) to use for this charm case
    :param nrev: maximum number of blade revolutions, use -NREV to output after each iteration
    :param convgVec: vector [CONVG1, CONVG2, CONVG3], converence criteria for the charm rotor solution
    :param sframe: solution frame; 0=shaft rotor frame with ADVs and ALPHAS; 1=body frame with U,V,W,P,Q,R;
    2=inertial frame
    :param smode: solution mode; 0=normal operations (wind tunnel); 1=full aircraft trim (sframe=2)
    :param az_sym: if True azimuthal symmetry will be applied (NROTORs will be negative), if False then NROTORS
    will be positive. If None, sign in template file is used
    :return: CHARM run characteristics file updated with the input list of rotor files
    """

    # Check that both template file and filename are not None
    if template_file is None and template_filename is None:
        raise ValueError("template_file and template_filename cannot both be None")

    # Determine if a template file object exists, if not then open template filename
    close_file = False
    if template_file is None:
        close_file = True
        template_file = open(template_filename, 'r')

    run_char_file = []
    try:
        template_file.seek(0)
        run_char_file = template_file.readlines()
    finally:
        if close_file:
            template_file.close()

    # Create list of lines for all of the rotor files
    file_string_list = []
    for rotorname, file_list in rotor_files.items():
        file_string_list.append("INPUT FILENAMES for {}\n".format(rotorname))
        for filename in file_list:
            file_string_list.append("  {}\n".format(filename))

    # Modify the input for number of rotors
    num_rotors = len(rotor_files)

    # Find line with NROTOR
    for line_num in range(len(run_char_file)):
        line = run_char_file[line_num]
        if "NROTOR" in line:
            nrotor_line_num = line_num + 1
            data = run_char_file[nrotor_line_num].split()
            template_num_rotors = int(data[0])
            rotor_sign = np.sign(template_num_rotors)
            if az_sym is not None:
                rotor_sign = -1 if az_sym else 1
            template_num_rotors = abs(template_num_rotors)
            new_data_line = "   {:3}   ".format(rotor_sign*num_rotors)
            if len(data) > 1:
                new_data_line += data[1]
            new_data_line += "\n"
            run_char_file[nrotor_line_num] = new_data_line

        if "PATHNAME" in line:
            filenames_start_line_num = line_num + 2
            filenames_end_line_num = filenames_start_line_num + template_num_rotors*6
            del run_char_file[filenames_start_line_num:filenames_end_line_num]
            run_char_file[filenames_start_line_num:filenames_start_line_num] = file_string_list
            break

    # Set NPSI, velocity, and rotation rates, aircraft pitch
    expr = re.compile(r"(^|\s)NPSI\s+")
    sframe_expr = re.compile(r"(^|\s)SFRAME\s+")
    vel_expr = re.compile(r"(^|\s)U\s+V\s+W\s")
    pitch_expr = re.compile(r"(^|\s)YAW\s*\(\s*IAC\s*\)\s*,\s*PITCH\s*\(\s*IAC\s*\)\s*,\s*ROLL\s*\(\s*IAC\s*\)")
    weight_expr = re.compile(r"(^|\s)WEIGHT")
    cg_expr = re.compile(r"(^|\s)\(?CGAC")
    atmo_expr = re.compile(r"(^\s*)SSPD\s+RHO")
    num_pitches_found = 0
    for line_num in range(len(run_char_file)):
        line = run_char_file[line_num]
        if re.search(expr, line):
            npsi_line_num = line_num + 1
            current_values = [float(d) for d in run_char_file[npsi_line_num].split()]
            if num_psi is not None:
                current_values[0] = num_psi
            if nrev is not None:
                current_values[1] = nrev
            if convgVec is not None:
                current_values[2:-1] = convgVec[:]
            run_char_file[npsi_line_num] = ("{:1.0f}      "*2 + "{:.6f}   "*3 + "{:1.0f}" + "\n").format(
                *current_values)
        if re.search(sframe_expr, line):
            sframe_line_num = line_num + 1
            current_values = np.array([int(d) for d in run_char_file[sframe_line_num].split()])
            if sframe is not None:
                current_values[0] = sframe
            if smode is not None:
                if len(current_values) > 1:
                    current_values[1] = smode
                else:
                    current_values = np.append(current_values[0], smode)
            if len(current_values) > 1:
                run_char_file[sframe_line_num] = ("{}" + " "*10 + "{}" + "\n").format(*current_values)
            else:
                run_char_file[sframe_line_num] = ("{}" + "\n").format(*current_values)
        if re.search(vel_expr, line):
            vel_line_num = line_num + 1
            current_values = np.array([float(d) for d in run_char_file[vel_line_num].split()])
            if velocity is not None or rotation_rates is not None:
                if velocity is not None:
                    current_values[0:3] = velocity[:]
                if rotation_rates is not None:
                    current_values[3:] = rotation_rates[:]
                run_char_file[vel_line_num] = ("{:8.4f} "*6 + "\n").format(*current_values)
        if re.search(pitch_expr, line):
            pitch_line_num = line_num + 1
            ypr = np.array([float(d) for d in run_char_file[pitch_line_num].split()])
            if pitch is not None and num_pitches_found < len(pitch):
                ypr[1] = pitch[num_pitches_found]
                num_pitches_found += 1
                run_char_file[pitch_line_num] = ("{:8.4f} "*3 + "\n").format(*ypr)
        if re.search(weight_expr, line):
            weight_line_num = line_num + 1
            current_values = np.array([float(d) for d in run_char_file[weight_line_num].split()])
            if weight is not None:
                current_values[0] = weight
            if len(current_values) > 1:
                current_values[1:] = fuselage_data[:]
            run_char_file[weight_line_num] = ("{:8.4f} "*len(current_values) + "\n").format(*current_values)
        if re.search(cg_expr, line):
            cg_line_num = line_num + 1
            current_values = np.array([float(d) for d in run_char_file[cg_line_num].split()])
            if cg is not None:
                current_values[:] = cg[:]
            run_char_file[cg_line_num] = ("{:8.4f} "*len(current_values) + "\n").format(*current_values)
        if atmos_data is not None and re.search(atmo_expr, line):
            atmo_line_num = line_num + 1
            run_char_file[atmo_line_num] = f"{atmos_data.speed_of_sound:.1f}     {atmos_data.density:.6f}\n"

    with io.StringIO() as string_io:
        string_io.writelines(run_char_file)
        return string_io.getvalue()


def read_run_characteristics_template(template_filename=None, template_file=None):
    """
    Reads key inputs needed from a template run characteristics file

    :param template_filename: template file name, will be opened if template_file is None
    :param template_file: file like object (io.IOBase) containing the template file contents
    :return: dictionary of inputs
    """
    # Check that both template file and filename are not None
    if template_file is None and template_filename is None:
        raise ValueError("template_file and template_filename cannot both be None")

    # Determine if a template file object exists, if not then open template filename
    close_file = False
    if template_file is None:
        close_file = True
        template_file = open(template_filename, 'r')

    run_char_file = []
    try:
        template_file.seek(0)
        run_char_file = template_file.readlines()
    finally:
        if close_file:
            template_file.close()

    run_char_dict = {}

    npsi_expr = re.compile(r"(^|\s)NPSI\s+")
    nrotor_expr = re.compile(r"(^|\s)NROTOR\s+")
    file_names_expr = re.compile(r"(^|\s)INPUT FILENAMES\s+")
    sframe_expr = re.compile(r"(^|\s)SFRAME\s+")
    vel_expr = re.compile(r"(^|\s)U\s+V\s+W\s")
    weight_expr = re.compile(r"(^|\s)WEIGHT")
    cg_expr = re.compile(r"(^|\s)\(?CGAC")
    file_list = []
    for line_num in range(len(run_char_file)):
        line = run_char_file[line_num]
        if re.search(npsi_expr, line):
            npsi_line_num = line_num + 1
            data = run_char_file[npsi_line_num].split()
            npsi = int(data[0])
            nrev = int(data[1])
            convg1 = float(data[2])
            convg2 = float(data[3])
            convg3 = float(data[4])
            run_char_dict['NPSI'] = npsi
            run_char_dict['NREV'] = nrev
            run_char_dict['CONVG1'] = convg1
            run_char_dict['CONVG2'] = convg2
            run_char_dict['CONVG3'] = convg3
        if re.search(nrotor_expr, line):
            nrotor_line_num = line_num + 1
            data = run_char_file[nrotor_line_num].split()
            nrotor = int(data[0])
            run_char_dict['NROTOR'] = nrotor
        if re.search(file_names_expr, line):
            filenames = []
            for file_line in range(1, 6):
                file_line_num = line_num + file_line
                filename = run_char_file[file_line_num].split()[0]
                filenames.append(filename)
            file_list.append(filenames)
        if re.search(sframe_expr, line):
            sframe_line_num = line_num + 1
            data = run_char_file[sframe_line_num].split()
            sframe = int(data[0])
            run_char_dict['SFRAME'] = sframe
            if len(data) > 1 and data[1] is not None:
                smode = int(data[1])
                run_char_dict['SMODE'] = smode
        if re.search(vel_expr, line):
            vel_line_num = line_num + 1
            data = run_char_file[vel_line_num].split()
            run_char_dict['U'] = float(data[0])
            run_char_dict['V'] = float(data[1])
            run_char_dict['W'] = float(data[2])
            run_char_dict['P'] = float(data[3])
            run_char_dict['Q'] = float(data[4])
            run_char_dict['R'] = float(data[5])
        if re.search(weight_expr, line):
            weight_line_num = line_num + 1
            data = run_char_file[weight_line_num].split()
            weight = float(data[0])
            run_char_dict['WEIGHT'] = weight
            if len(data) > 1:
                fusplan = float(data[1])
                fussidea = float(data[2])
                fusrefl = float(data[3])
                run_char_dict.update({"FUSPLAN": fusplan, "FUSSIDEA": fussidea, "FUSREFL": fusrefl})
        if re.search(cg_expr, line):
            cg_line_num = line_num + 1
            data = run_char_file[cg_line_num].split()
            run_char_dict['XCG'] = float(data[0])
            run_char_dict['YCG'] = float(data[1])
            run_char_dict['ZCG'] = float(data[2])

    if len(file_list) > 0:
        run_char_dict['FILES'] = file_list

    return run_char_dict


def build_charm_input_files(degen_mgr: dg.DegenGeomMgr, case_name,
                            rotor_settings: CharmRotorSettingsCollection = None,
                            wing_settings=None,
                            unit_factor=u.in2ft, run_char_template=None, run_char_filename=None,
                            velocity=None, trim_settings=None, **kwargs):
    """
    Creates input files for charm case

    :param degen_mgr: degen geom manager object with desired components for charm
    :param rotor_settings: rotor settings objects corresponding with the degen geoms
    :param wing_settings: rotor settings objects corresponding to the wing degen geoms
    :param case_name: name of the case these input files are for
    :param unit_factor: unit conversion factor to go from vsp units to charm units (normally feet)
    :param run_char_template: template run characteristic file object (io.IOBase like object)
    :param run_char_filename: name of run characteristic input file template if run_char_template is None
    :param velocity: vehicle velocity vector [u, v, w] in ft/s
    :param trim_settings: trim settings object for running CHARM in trim mode (SFRAME=2, SMODE=1)
    :return: dictionary of name, file contents pairs
    """

    # Get key quantites from run characteristic file
    run_char_vars = read_run_characteristics_template(template_filename=run_char_filename,
                                                      template_file=run_char_template)
    num_psi = run_char_vars['NPSI']
    files_to_write = {}
    rotor_files = {}

    if rotor_settings is not None:
        # Determine appropriate num_psi value, needs to be divisible all blade counts
        num_blades_list = [len(degen_mgr.degen_objs[prop_id].copies[0]) for prop_id in rotor_settings.keys()]

        num_psi = _determine_num_psi(base_num_psi=run_char_vars['NPSI'], num_blades_list=num_blades_list)

        files_to_write, rotor_files = create_rotor_file_list(degen_mgr=degen_mgr,
                                                             settings=rotor_settings, num_psi=num_psi,
                                                             unit_factor=unit_factor, **kwargs)

    if wing_settings is not None:
        wing_infos = create_all_wing_infos(degen_mgr=degen_mgr, settings=wing_settings)
        wing_files_to_write, wing_files = create_wing_file_list(wing_infos, settings=wing_settings,
                                                                num_psi=num_psi, unit_factor=unit_factor, **kwargs)
        files_to_write = dict(files_to_write, **wing_files_to_write)
        rotor_files = dict(rotor_files, **wing_files)

    if trim_settings is not None:
        trim_files_to_write = create_trim_file_from_template(trim_template=trim_settings.template,
                                                             trim_settings=trim_settings)
        files_to_write = dict(files_to_write, **trim_files_to_write)

    run_char_file = build_run_characteristics_file_from_template(rotor_files=rotor_files,
                                                                 template_filename=run_char_filename,
                                                                 template_file=run_char_template,
                                                                 num_psi=num_psi, velocity=velocity, **kwargs)
    files_to_write[_cleanup_filename_for_charm(case_name + ".inp")] = run_char_file
    return files_to_write


def create_scan_grid(xpnts=None, ypnts=None, zpnts=None):
    """
    Creates scan grid file for input point vectors, one must be length of one

    :param xpnts: array of x points
    :param ypnts: array of y points
    :param zpnts: array of z points
    :return: string contents of scan.inp file
    """

    xlen = len(xpnts)
    ylen = len(ypnts)
    zlen = len(zpnts)
    shift = 0
    nd1 = xlen
    nd2 = ylen
    num_grids = zlen

    if xlen <= ylen and xlen <= zlen:
        shift = 2
        nd1 = ylen
        nd2 = zlen
        num_grids = xlen
    elif ylen <= xlen and ylen <= zlen:
        shift = 1
        nd1 = zlen
        nd2 = xlen
        num_grids = ylen

    with io.StringIO() as f:
        f.write("{:8d}\n".format(num_grids))
        for igrid in range(num_grids):
            f.write("{:8d}{:8d}\n".format(nd1, nd2))
        for igrid in range(num_grids):
            for i in range(nd1):
                for j in range(nd2):
                    indices = [i, j, igrid]
                    f.write("{:12.3f}{:12.3f}{:12.3f}\n".format(xpnts[indices[shift % 3]],
                                                                ypnts[indices[(shift+1) % 3]],
                                                                zpnts[indices[(shift+2) % 3]]))
        return f.getvalue()


def _determine_num_psi(base_num_psi, num_blades_list):
    """
    Determines the number of azimuthal computation points based on a template number of desired resolution
    and a list of all the number of blades
    :param base_num_psi: template number azimuthal computation points
    :param num_blades_list: list of blade counts
    :return: closest azimuthal resolution value that is divisible by all of the blade counts
    """

    # Compute the least common multiple of all of the numbers
    lcm = 1
    for number in num_blades_list:
        lcm = _compute_least_common_multiple(lcm, number)

    # Determine the closest number to the template number and the least common multiple
    remainder = base_num_psi % lcm
    num_psi = base_num_psi
    if remainder < lcm/2.0 and base_num_psi - remainder > 0:
        num_psi -= remainder
    else:
        num_psi += (lcm - remainder)

    return num_psi


def _compute_least_common_multiple(num1: int, num2: int):
    """
    Finds the least common multiple between the two input numbers
    :param num1: first number
    :param num2: second number
    :return: least common multiple
    """
    import math
    return int(num1*num2/math.gcd(num1, num2))


def _get_airfoil_from_file(af_opts: AirfoilFileLocationOpt):
    """
    Gets an airfoil section from a file location
    :param af_opts: Options associated with getting the file
    :return: Charm airfoil section
    """
    # Get full path to the file
    fullpath = af_opts.get_fullpath()

    thickness = 0.0
    with open(fullpath, 'r') as f:
        line = f.readline()
        # Assuming thickness is on the first line
        thickness = float(line)

        # Read file line by line
        with io.StringIO() as section_str_io:
            reynolds_data = AirfoilReynoldsDataType.NONE
            for iline, line in enumerate(f):
                if iline == 0 and 'COMMENT' not in line:
                    try:
                        data = [float(d) for d in line.split()]
                    except ValueError:
                        data = []

                    if len(data) == 4:
                        reynolds_data = AirfoilReynoldsDataType.METHOD1
                    elif len(data) == 2:
                        reynolds_data = AirfoilReynoldsDataType.METHOD2
                section_str_io.write(line)
            return CharmAirfoil(thickness=thickness, section_string=section_str_io.getvalue(),
                                reynolds_data_type=reynolds_data)


def _cleanup_filename_for_charm(name):
    """
    Cleans up a filename into something charm can handle
    :param name: unclean filename
    :return: filename suitable for charm
    """
    # Replacing spaces with "_"
    return name.replace(" ", "_")


def _determine_num_symm(trans_orig: TransMatrix, trans_copy: TransMatrix):
    """
    Determines the number of planar symmetries between an original transformation matrix and the transformation matrix
    of a copy
    :param trans_orig: original transformation matrix
    :param trans_copy: transformation matrix of a potentially symmetric copy
    :return: number of planar symmetries between the two matrices
    """

    orig_inv = trans_orig.get_inverse_transform()
    trans_diff = TransMatrix(np.dot(trans_copy.mat, orig_inv.mat))

    # if the difference is only planar symmetries, then off diagonal elements should zero
    num_syms = 0
    if not np.allclose(trans_diff.mat[0, 1:3], np.zeros(2)):
        return num_syms
    if not np.isclose(trans_diff.mat[1, 0], 0.0):
        return num_syms
    if not np.isclose(trans_diff.mat[1, 2], 0.0):
        return num_syms
    if not np.allclose(trans_diff.mat[2, 0:2], np.zeros(2)):
        return num_syms

    if np.isclose(trans_diff.mat[0, 0], -1.0):
        num_syms += 1
    if np.isclose(trans_diff.mat[1, 1], -1.0):
        num_syms += 1
    if np.isclose(trans_diff.mat[2, 2], -1.0):
        num_syms += 1

    return num_syms


def _connect_arrays(arr1, arr2):
    """
    Checks if two arrays could be connected
    :param arr1: np array, each row is a 3d point
    :param arr2: np array, each row is a 3d point
    :return: None if unable to connect, connected array if could connect
    """
    if _is_same_point(arr1[0], arr2[0]):
        connected = np.concatenate((np.flipud(arr2)[:-1, :], arr1))
        return connected
    if _is_same_point(arr1[0], arr2[-1]):
        connected = np.concatenate((arr2[:-1, :], arr1))
        return connected
    if _is_same_point(arr1[-1], arr2[-1]):
        connected = np.concatenate((arr2, np.flipud(arr1)[:-1, :]))
        return connected
    if _is_same_point(arr1[-1], arr2[0]):
        connected = np.concatenate(arr1[:-1, :], arr2)
        return connected
    return None


SAME_POINT_ATOL = 1.0E-8


def _is_same_point(point1, point2):
    """
    Checks if two points are the same
    :param point1: first point
    :param point2: second point
    :return: returns True if the are the same, False otherwise
    """
    from numpy.linalg import norm

    if abs(norm(point1-point2)) < SAME_POINT_ATOL:
        return True
    return False


def __modify_rotor_wake_template(rw_template, rotor_setting: CharmRotorSettings):
    """
    Modifies a rotor wake template given rotor settings object
    :param rw_template: array of rotor wake file lines
    :param rotor_setting: CharmRotorSettings object
    :return: modified template
    """

    # If there is a specified initial collective, then set that in the template
    if rotor_setting.initial_collective is not None or rotor_setting.ct is not None:
        # Collective line is index 6
        line_num = 6
        line = rw_template[line_num]
        data = line.split()
        icoll = int(data[0]) if rotor_setting.icoll is None else rotor_setting.icoll
        coll = float(data[1]) if rotor_setting.initial_collective is None else rotor_setting.initial_collective
        ct = float(data[2]) if rotor_setting.ct is None else rotor_setting.ct
        print_str = "{:4d} {:8.3f} {:8.6f}".format(icoll, coll, ct)
        if len(data) >= 4:
            icoax = int(data[3])
            print_str += " {:4d}".format(icoax)
        print_str += "\n"
        rw_template[line_num] = print_str

    if rotor_setting.icnvct is not None:
        # Wake settings line is index 10
        line_num = 10
        line = rw_template[line_num]
        data = line.split()
        nowake = int(data[0])
        icnvct = int(data[1]) if rotor_setting.icnvct is None else rotor_setting.icnvct
        nwakes = int(data[2])
        npwake = int(data[3])
        ifar = int(data[4])
        mbcve = int(data[5])
        print_str = f"{nowake:2d} {icnvct:8d} {nwakes:8d} {npwake:8d} {ifar:8d} {mbcve:8d}\n"
        rw_template[line_num] = print_str

    return rw_template
