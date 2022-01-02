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


import airfoils
import os
from subprocess import Popen, PIPE, STDOUT
import re
import warnings
from utilities import RunManager
import glob
import sys
import numpy as np
import pandas as pd
import io
import typing
import fortranformat as ff
import csv

# Try and find AVL binary
DEFAULT_AVL_BINARY = os.path.join(os.path.dirname(__file__), "bin")
for g in glob.glob(os.path.join(DEFAULT_AVL_BINARY, "avl*")):
    if sys.platform in g:
        DEFAULT_AVL_BINARY = g
        break


class AvlInput:
    def __init__(self, header, surfaces):
        """AVL input file class

        :param AvlHeader header: header to be prepended to the input file
        :param surfaces: array of AvlSurface objects to append to the input file
        """
        self.header = header
        self.surfaces = surfaces

    def toFile(self, fname):
        """Saves input to a file

        :param str fname: path to save the file
        :return: boolean indicating success
        """
        with open(fname, 'w') as f:
            f.write("%s\n" % self.header.generate())
            for surface in self.surfaces:
              f.write("%s\n" % surface.generate())
        return True


class AvlHeader:
    """AvlHeader class stores options pertaining the AvlHeader section of an AVL geometry input file

    Properties:
        configname: Name of the configuration
        Mach: default freestream Mach number for Prandtl-Glauert correction
        iYSym: controls case symmetry about X-Z plane
            1 - symmetric about Y=0
            -1 - case is antisymmetric about Y=0
            0 - no Y-symmetry is assumed
        iZsym: controls case symmetry about X-Y plane
            1 - symmetric about Z=Zsym
            -1 - antisymmetric about Z=Zsym
            0 = no Z-symmetry is assumed
        zSym: Z val of x-y symmetry plane
        Sref: reference area
        Cref: reference chord
        Bref: reference span
        CGref: x,y,z location for moment calculations
        CDp: default profile drag coefficient added to geometry

        See: http://web.mit.edu/drela/Public/web/avl/avl_doc.txt for more details

    """
    def __init__(self, configname='Default Name', Mach=0.0, iYsym=0, iZsym=0, Zsym=0.0,
                 Sref=1.0, Cref=1.0, Bref=1.0, CGref=(0.0, 0.0, 0.0), CDp=0.0):
        self.configname = configname
        self.Mach = Mach
        self.iYsym = iYsym
        self.iZsym = iZsym
        self.Zsym = Zsym
        self.Sref = Sref
        self.Cref = Cref
        self.Bref = Bref
        self.CGref = CGref
        self.CDp = CDp

    def generate(self):
        """ Generates a header string for an AVL geometry input file
        :return: String with AVL geometry headers
        """

        string_list = [
            '%s\n' % self.configname,
            '#Mach\n',
            '%f\n' % self.Mach,
            '#IYsym\tiZsym\tZsym\n',
            '%d %d %f\n' % (self.iYsym, self.iZsym, self.Zsym),
            '#Sref\tCref\tBref\n',
            '%f %f %f\n' % (self.Sref, self.Cref, self.Bref),
            '#Xref\tYref\tZref\n',
            '%f %f %f\n' % tuple(self.CGref)
        ]

        if self.CDp > 0.0:
            string_list.append('#CDp\n')
            string_list.append('%f\n' % self.CDp)

        string_list.append('#\n')
        return ''.join(string_list)[:-1]


class AvlSurface:
    """ AvlSurface wraps surface input type for an avl geometry file

    Properties:
        name: surface name
        Nchord: number of chordwise vortices
        Cspace: chordwise vortex spacing parameter
        Nspan: number of spanwise vortices
        Sspace: spanwise vortex spacing parameter
        Component: component number used to group multiple surfaces into components
        Ydupl: If set, specifies x-z plane to mirror surface about
        scale: x,y,z scale factors
        translate: x,y,z translation values, applied after scaling
        angle: angle added to all section incidence angles
        no_wake: set to true if wake is not shed from this surface
        no_able: set to true if this surface is unaffected by alpha,beta and p,q,r
        no_load: set to true if loads from this surface should not be totaled

        See: http://web.mit.edu/drela/Public/web/avl/avl_doc.txt for more details

    TODO: Add support for CDCL option
    """
    def __init__(self, name="surf name", Nchord=12, Cspace=1.0, Nspan=20, Sspace=-1.5,
                 Component=None, Ydupl=None, scale=None, translate=None, dAinc=None,
                 no_wake=False, no_able=False, no_load=False):
        """

        :param name:
        :param Nchord:
        :param Cspace:
        :param Nspan:
        :param Sspace:
        :param Component:
        :param Ydupl:
        :param scale:
        :param translate:
        :param dAinc:
        :param no_wake:
        :param no_able:
        :param no_load:
        """
        self.name = name
        self.Nchord = Nchord
        self.Cspace = Cspace
        self.Nspan = Nspan
        self.Sspace = Sspace
        self.Component = Component
        self.Ydupl = Ydupl
        self.scale = scale
        self.translate = translate
        self.dAinc = dAinc
        self.no_wake = no_wake
        self.no_able = no_able
        self.no_load = no_load
        self.sections = []

    def addSection(self, section):
        """Adds a section to the surface

        :param AvlSection section: section to add
        :return: surface's sections with the new section appended
        """
        self.sections.append(section)
        return self.sections

    def addSections(self, sections):
        """Adds an array of sections to the surface

        :param sections: array of AvlSection objects to append to the section
        :return: surface's sections with the new sections appended
        """
        for section in sections:
            self.addSection(section)
        return self.sections

    def generate(self):
        """Generates string for this surface
        :return: String with AVL Surface input
        """
        string_list = [
            '#'*50,
            '\n',
            'SURFACE\n',
            '%s\n' % self.name,
            '#Nchordwise\tCspace\tNspanwise\tSspace\n',
            '%d %f' % (self.Nchord, self.Cspace)
        ]

        if self.Nspan is not None and self.Sspace is not None:
            nspan = self.Nspan
            if self.Nspan < len(self.sections)*2:
                nspan = len(self.sections*2)
                warnings.warn("Spanwise paneling increased from {} in VSP to {} for AVL.\nRecommend increasing your paneling on VSP".format(self.Nspan,nspan))
            string_list.append(' %d %f\n' % (nspan, self.Sspace))
        else:
            string_list.append('\n')

        if self.Component is not None:
            string_list.append('COMPONENT\n')
            string_list.append('%d\n' % self.Component)
            string_list.append('#\n')

        if self.Ydupl is not None:
            string_list.append('YDUPLICATE\n')
            string_list.append('%f\n' % self.Ydupl)
            string_list.append('#\n')

        if self.scale is not None:
            string_list.append('SCALE\n')
            string_list.append('%f %f %f\n' % tuple(self.scale))
            string_list.append('#\n')

        if self.translate is not None:
            string_list.append('TRANSLATE\n')
            string_list.append('%f %f %f\n' % tuple(self.translate))
            string_list.append('#\n')

        if self.dAinc is not None:
            string_list.append('ANGLE\n')
            string_list.append('%f\n' % self.dAinc)
            string_list.append('#\n')

        if self.no_wake:
            string_list.append('NOWAKE\n')
            string_list.append('#\n')

        if self.no_able:
            string_list.append('NOABLE\n')
            string_list.append('#\n')

        if self.no_load:
            string_list.append('NOLOAD\n')
            string_list.append('#\n')

        for section in self.sections:
            string_list.append( section.generate())
            if self.Nspan is None:
                if section.Nspan is None:
                    raise Exception("Nspan needs to be defined for either the surface or for each section but is not currently defined for this section.")
            if self.Sspace is None:
                if section.Sspace is None:
                    raise Exception("Nspan needs to be defined for either the surface or for each section but is not currently defined for this section.")
        return ''.join(string_list)[:-1]


class AvlSection:
    """AvlSection class to wrap section input for avl geometry file

    Properties:
        le: x,y,z coordinates of the leading edge of the section
        chord: chord length of the section
        ainc: incidence angle (deg)
        Nspan: number of spanwise vortices
        Sspace: controls spacing of spanwise vorticies
        afile: airfoil object
        afile_x1: start of x/c range used for camber line
        afile_x2: end of x/c range used for camber line
        design: dictionary of design variable name and weighting pairs
        claf: Cl vs alpha slope correction factor
        control: list of control surfaces for this section

        See: http://web.mit.edu/drela/Public/web/avl/avl_doc.txt for more details

    TODO: Add support for CDCL
    """
    def __init__(self, le=(0.0, 0.0, 0.0), chord=1.0, ainc=0.0, Nspan=None, Sspace=None, afile=None, afile_x1=0.0,
                 afile_x2=1.0, design=None, controls=None, claf=None):
        """

        :param le:
        :param chord:
        :param ainc:
        :param Nspan:
        :param Sspace:
        :param afile:
        :param afile_x1:
        :param afile_x2:
        :param design:
        :param controls:
        :param claf:
        """
        self.le = le
        self.chord = chord
        self.ainc = ainc
        self.Nspan = Nspan
        self.Sspace = Sspace
        self.afile = afile
        self.afile_x1 = afile_x1
        self.afile_x2 = afile_x2
        self.design = design
        self.controls = controls
        self.claf = claf

    def add_control(self, control):
        """Adds control surface to section

        :param AvlControl control: AvlControl object to add to the section
        """
        if self.controls is None:
            self.controls = [control]
        else:
            self.controls.append(control)

    def generate(self):
        """Generates string for this section
                :return: String with AVL section input
        """
        string_list = [
            '#'*50,
            '\n',
            'SECTION\n',
            '#Xle\tYle\tZle\tChord\tAinc\tNspanwise\tSspace\n',
            '#\n',
            '%f %f %f' % tuple(self.le),
            ' %f %f' % (self.chord, self.ainc),
        ]

        if self.Nspan is not None and self.Sspace is not None:
            string_list.append(' %f %f' % (self.Nspan, self.Sspace))

        string_list.append('\n')

        if self.afile is not None:
            if isinstance(self.afile, airfoils.Naca4):
                string_list.append('NACA\t %f %f\n' % (self.afile_x1, self.afile_x2))
                string_list.append('%s\n' % self.afile.to_naca_string())
                string_list.append('#\n')
            else:
                string_list.append('AIRFOIL\t %f %f\n' % (self.afile_x1, self.afile_x2))
                for i in range(len(self.afile.coords)):
                    string_list.append('%f %f\n' % (self.afile.coords[i, 0], self.afile.coords[i, 1]))
            string_list.append('#\n')

        if self.design is not None:
            for key, val in self.design.items():
                string_list.append('DESIGN\n')
                string_list.append('%s %f\n' % (key, val))
                string_list.append('#\n')

        if self.controls is not None:
            for control in self.controls:
                string_list.append(control.generate())

        if self.claf is not None:
            string_list.append('CLAF\n')
            string_list.append('%f\n' % self.claf)
            string_list.append('#\n')

        return ''.join(string_list)


class AvlControl:
    """AvlControl class to control input for avl geometry file

    Properties:
        name: name of the control surface
        gain: gain of the control surface
        xhinge: x/c of hinge location. Positive is from trailing edge, negative is from leading edge
        xyz_hinge: hinge line of control surface (all zeros will cause it to be direction of leading edge
                   of the control surface
        sign_dup: For duplicate surface 1: same deflection, -1: opposite deflection

        See: http://web.mit.edu/drela/Public/web/avl/avl_doc.txt for more details
    """
    def __init__(self, name="con_surf", gain=1.0, xhinge=0.25, xyz_hinge=(0.0, 0.0, 0.0), sign_dup=1.0):
        """

        :param name:
        :param gain:
        :param xhinge:
        :param xyz_hinge:
        :param sign_dup:
        """
        self.name = name
        self.gain = gain
        self.xhinge = xhinge
        self.xyz_hinge = xyz_hinge
        self.sign_dup = sign_dup

    def generate(self):
        """Generates string for this section
            :return: String with AVL section input
        """
        string_list = [
            'CONTROL\n',
            '#name, gain,  Xhinge,  XYZhvec,  SgnDup\n',
            '%s ' % self.name,
            '%f %f ' % (self.gain, self.xhinge),
            '%f %f %f ' % tuple(self.xyz_hinge),
            '%f\n' % self.sign_dup
        ]

        return ''.join(string_list)


class AvlOutput:
    """AvlOutput class to collect avl results

    Properties:
        cases: name of the control surface
        plots: gain of the control surface
        plotFile: x/c of hinge location. Positive is from trailing edge, negative is from leading edge
        Surfaces: array containing the Surfaces for each AVL case
        Strips: array containing the Strips for each AVL case
        Vortices: array containing the Vortices for each AVL case
        Sref: array containing the Sref for each AVL case
        Cref: array containing the Cref for each AVL case
        Bref: array containing the Bref for each AVL case
        Xref: array containing the Xref for each AVL case
        Yref: array containing the Yref for each AVL case
        Zref: array containing the Zref for each AVL case
        Alpha: array containing the Alpha for each AVL case
        pb_2V: array containing the pb_2V for each AVL case
        pPrimeb_2V: array containing the pPrimeb_2V for each AVL case
        Beta: array containing the Beta for each AVL case
        qc_2V: array containing the qc_2V for each AVL case
        Mach: array containing the Mach for each AVL case
        rb_2V: array containing the rb_2V for each AVL case
        rPrimeb_2V: array containing the rPrimeb_2V for each AVL case
        CXtot: array containing the CXtot for each AVL case
        Cltot: array containing the Cltot for each AVL case
        ClPrimetot: array containing the ClPrimetot for each AVL case
        CYtot: array containing the CYtot for each AVL case
        Cmtot: array containing the Cmtot for each AVL case
        CZtot: array containing the CZtot for each AVL case
        Cntot: array containing the Cntot for each AVL case
        CnPrimetot: array containing the CnPrimetot for each AVL case
        CLtot: array containing the CLtot for each AVL case
        CDtot: array containing the CDtot for each AVL case
        CDvis: array containing the CDvis for each AVL case
        CDind: array containing the CDind for each AVL case
        CLff: array containing the CLff for each AVL case
        CDff: array containing the CDff for each AVL case
        CYff: array containing the CYff for each AVL case
        e: array containing the e for each AVL case
        Xnp: array containing the neutral point for each AVL case
        CLa: CL/alpha stability axis derivative
        CLb: CL/beta stability axis derivative
        CYa: CY/alpha stability axis derivative
        CYb: CY/beta stability axis derivative
        Cla: Cl/alpha stability axis derivative (x' moment)
        Clb: Cl/beta stability axis derivative (x' moment)
        Cma: Cm/alpha stability axis derivative (y moment)
        Cmb: Cm/beta stability axis derivative (y moment)
        Cna: Cn/alpha stability axis derivative (z' moment)
        Cnb: Cn/beta stability axis derivative (z' moment)
        CLp: CL/p stability axis derivative (z' force, p' roll rate)
        CLq: CL/q stability axis derivative (z' force, q' pitch rate)
        CLr: CL/r stability axis derivative (z' force, r' yaw rate)
        CYp: CY/p stability axis derivative (y force, p' roll rate)
        CYq: CY/q stability axis derivative (y force, q' pitch rate)
        CYr: CY/r stability axis derivative (y force, r' yaw rate)
        Clp: Cl/p stability axis derivative (x' mom, p' roll rate)
        Clq: Cl/q stability axis derivative (x' mom,, q' pitch rate)
        Clr: Cl/r stability axis derivative (x' mom,, r' yaw rate)
        Cmp: Cm/p stability axis derivative (y mom, p' roll rate)
        Cmq: Cm/q stability axis derivative (y mom, q' pitch rate)
        Cmr: Cm/r stability axis derivative (y mom, r' yaw rate)
        Cnp: Cn/p stability axis derivative (z' mom, p' roll rate)
        Cnq: Cn/q stability axis derivative (z' mom, q' pitch rate)
        Cnr: Cn/r stability axis derivative (z' mom, r' yaw rate)
    """

    def __init__(self, plotFile=None):
        """

        :param str plotFile: path to the generated plot file
        """
        self.cases = []
        self.plots = None
        self.plotFile = plotFile
        self.stripForces = None
        self.stabilityDerivs = None
        self.surfaceForces: typing.List[SurfaceForcesOutput] = []

    def parseFile(self,file):
        """

        :param str file: path to output file from AVL
        :return: none
        """
        data = dict()
        with open(file,'r') as f:
            lines = f.readlines()
        for line in lines:
            if "=" in line:
                splitLine = re.split('[\s|#|\n]+',line)
                indices = [i for i, j in enumerate(splitLine) if '=' in j ]
                for i in indices:
                    key = splitLine[i-1]
                    try:
                        val = float(splitLine[i+1])
                    except ValueError:
                        if '*****' in splitLine[i]:
                            continue
                        val = float(splitLine[i].replace("=",""))
                    if key not in data:
                        data[key] = val
                    else:
                        if key == 'Cnb':
                            key = "".join(splitLine[i-6:i])
                            data[key] = val
                        else:
                            warnings.warn(f"Duplicate quantity parsed for {key}, keeping first value")
        self.cases.append( data)

    def parseFiles(self,files):
        """

        :param files: array containing paths to output files
        :return: none
        """
        for f in files:
            self.parseFile(f)
        self.Surfaces = [case["Surfaces"] for case in self.cases]
        self.Strips = [case["Strips"] for case in self.cases]
        self.Vortices = [case["Vortices"] for case in self.cases]
        self.Sref = [case["Sref"] for case in self.cases]
        self.Cref = [case["Cref"] for case in self.cases]
        self.Bref = [case["Bref"] for case in self.cases]
        self.Xref = [case["Xref"] for case in self.cases]
        self.Yref = [case["Yref"] for case in self.cases]
        self.Zref = [case["Zref"] for case in self.cases]
        self.Alpha = [case["Alpha"] for case in self.cases]
        self.pb_2V = [case["pb/2V"] for case in self.cases]
        self.pPrimeb_2V = [case["p'b/2V"] for case in self.cases]
        self.Beta = [case["Beta"] for case in self.cases]
        self.qc_2V = [case["qc/2V"] for case in self.cases]
        self.Mach = [case["Mach"] for case in self.cases]
        self.rb_2V = [case["rb/2V"] for case in self.cases]
        self.rPrimeb_2V = [case["r'b/2V"] for case in self.cases]
        self.CXtot = [case["CXtot"] for case in self.cases]
        self.Cltot = [case["Cltot"] for case in self.cases]
        self.ClPrimetot = [case["Cl'tot"] for case in self.cases]
        self.CYtot = [case["CYtot"] for case in self.cases]
        self.Cmtot = [case["Cmtot"] for case in self.cases]
        self.CZtot = [case["CZtot"] for case in self.cases]
        self.Cntot = [case["Cntot"] for case in self.cases]
        self.CnPrimetot = [case["Cn'tot"] for case in self.cases]
        self.CLtot = [case["CLtot"] for case in self.cases]
        self.CDtot = [case["CDtot"] for case in self.cases]
        self.CDvis = [case["CDvis"] for case in self.cases]
        self.CDind = [case["CDind"] for case in self.cases]
        self.CLff = [case["CLff"] for case in self.cases]
        self.CDff = [case["CDff"] for case in self.cases]
        self.CYff = [case["CYff"] for case in self.cases]
        self.e = [case["e"] for case in self.cases]
        if "Xnp" in self.cases[0]:
            self.Xnp = [case["Xnp"] for case in self.cases]
        self.CLa = [case["CLa"] for case in self.cases]
        self.CLb = [case["CLb"] for case in self.cases]
        self.CYa = [case["CYa"] for case in self.cases]
        self.CYb = [case["CYb"] for case in self.cases]
        self.Cla = [case["Cla"] for case in self.cases]
        self.Clb = [case["Clb"] for case in self.cases]
        self.Cma = [case["Cma"] for case in self.cases]
        self.Cmb = [case["Cmb"] for case in self.cases]
        self.Cna = [case["Cna"] for case in self.cases]
        self.Cnb = [case["Cnb"] for case in self.cases]
        self.CLp = [case["CLp"] for case in self.cases]
        self.CLq = [case["CLq"] for case in self.cases]
        self.CLr = [case["CLr"] for case in self.cases]
        self.CYp = [case["CYp"] for case in self.cases]
        self.CYq = [case["CYq"] for case in self.cases]
        self.CYr = [case["CYr"] for case in self.cases]
        self.Clp = [case["Clp"] for case in self.cases]
        self.Clq = [case["Clq"] for case in self.cases]
        self.Clr = [case["Clr"] for case in self.cases]
        self.Cmp = [case["Cmp"] for case in self.cases]
        self.Cmq = [case["Cmq"] for case in self.cases]
        self.Cmr = [case["Cmr"] for case in self.cases]
        self.Cnp = [case["Cnp"] for case in self.cases]
        self.Cnq = [case["Cnq"] for case in self.cases]
        self.Cnr = [case["Cnr"] for case in self.cases]
        self.SM = -1.0*np.array(self.Cma)/np.array(self.CLa)

    def loadPlots(self,file=None):
        """Loads the plot files from AVL. This stores it as a binary file, so can be saved with a naive call to write()

        :param file: postscript file from avl to be read in
        :return: none
        """
        if file is not None:
            self.plotFile = file
            with open(self.plotFile, 'r') as psFile:
                self.plots = psFile.read()

    def savePlots(self,file):
        """

        :param file:
        :return:
        """
        with open(file, 'w') as plotFile:
            plotFile.write(self.plots)

    def build_interpolator_cl(self, beta_desired=0):
        """
        Builds a linear interpolator at beta closest to input beta
        :return: 1D linear interpolator of the form [alpha, CDi, Cm, e] = f(CL)
        """
        from scipy.interpolate import LinearNDInterpolator, interp1d
        import numpy as np
        beta = np.unique(self.Beta)
        beta_actual = beta[np.argmin((beta-beta_desired)**2.0)]
        beta_mask = np.array(self.Beta) == beta_actual
        return interp1d(np.array(self.CLtot)[beta_mask],
                        (np.array(self.Alpha)[beta_mask],
                         np.array(self.CDind)[beta_mask],
                         np.array(self.Cmtot)[beta_mask],
                         np.array(self.e)[beta_mask]), kind="cubic",
                        fill_value="extrapolate")

    def get_results_df(self):
        """
        Converts results to a pandas data frame
        :return:
        """
        import pandas as pd
        df = pd.DataFrame.from_records(self.cases)
        df['SM'] = self.SM
        return df

    def get_surface_forces_df(self):
        all_df = pd.DataFrame()
        for i, a in enumerate(self.Alpha):
            b = self.Beta[i]
            single_df = self.surfaceForces[i].get_df()
            single_df['Alpha'] = a
            single_df['Beta'] = b
            all_df = pd.concat((all_df, single_df), ignore_index=True)
        return all_df


class StripForcesOutput():
    def __init__(self, file):
        '''

        :param file: AVL output file
        '''
        self.cref = None # need to assign this later since it's not in this file
        with open(file,"r") as f:
            lines = f.readlines()

        self.lines = lines

    def parse_lines(self):
        lines = self.lines
        # find index for all beginnings of surfaces
        indx = [i for i, l in enumerate(lines) if "Surface #" in l]

        # add the strip output for each surface
        self.strips = []
        for i in range(0, len(indx)-1):
            self.strips.append( StripForcesSurface(lines[indx[i]:indx[i+1]-1]))

        self.strips.append( StripForcesSurface(lines[indx[-1]: len(lines)-1]))

    def dump_csvs(self, base_filename):
        for strip in self.strips:
            strip.dump_csv(base_filename + f"{strip.name}_{strip.surf_num}.csv", self.cref)


class StripForcesSurface():
    def __init__(self, str_array):
        '''

        :param str_array: string array of the strip forces on a surface, from AVL
        '''
        import numpy as np
        from io import BytesIO

        i = 0
        strs = str_array[i].split()
        self.surf_num = int(strs[2])
        self.name = strs[3]

        i = 1
        strs = str_array[i].split()
        self.nchord = int(strs[3])
        self.nspan = int(strs[7])

        i = 2
        strs = str_array[i].split()
        try:
            self.surf_area = float(strs[3])
            self.mac = float(strs[7])
        except ValueError:
            if strs[2][0] == "=":
                tempstr = strs[2][1:]
                self.surf_area = float(tempstr)
                self.mac = float(strs[-1])
            else:
                raise ValueError(f'Unable to convert "{str_array[0][2:22]}" to float.  Instead, found "{strs[2]}".')

        i = 3
        strs = str_array[i].split()
        self.CLsurf = float(strs[2])
        self.cl_surf = float(strs[5])

        i = 4
        strs = str_array[i].split()
        self.CY_surf = float(strs[2])
        self.cm_surf = float(strs[5])

        i = 5
        strs = str_array[i].split()
        self.CD_surf = float(strs[2])
        self.cn_surf = float(strs[5])

        i = 6
        strs = str_array[i].split()
        self.CDi_surf = float(strs[2])
        self.CDvsurf = float(strs[5])

        bio = BytesIO("".join(str_array[14:]).encode("utf8"))
        # data = np.genfromtxt(bio, )
        # Manually parse data since sometimes there are a different number of rows
        data = []
        for line in str_array[14:]:
            line = line.replace("*", " ")
            reader = ff.FortranRecordReader("(2X,I4,11(1X,F8.4),1X,F8.3)")
            line_data = reader.read(line)
            line_data = [np.nan if x is None else x for x in line_data]
            data.append(line_data)

        data = np.array(data)
        self.j = data[:,0]
        self.yle = data[:,1]
        self.chord = data[:,2]
        self.area = data[:,3]
        self.c_cl = data[:,4]
        self.ai = -data[:,5]
        self.cl_norm = data[:,6]
        self.cl = data[:,7]
        self.cd = data[:,8]
        self.cdv = data[:,9]
        self.cm_c4 = data[:,10]
        self.cm_le = data[:,11]
        try:
            self.cp_xc = data[:,12]
        except (IndexError):
            self.cp_xc = data[:, 11]
            self.cp_xc = np.nan
        self.data = data

    def dump_csv(self, filename, cref):
        with open(filename, "w") as csvfile:
            writer = csv.writer(csvfile)
            writer.writerow(["Name", "Surf Num", "Surf Area", "Cref", "NChord", "Nspan", "MAC", "CLsurf", "cl_surf", "CY_surf", "cm_surf", "CD_surf", "cn_surf", "CDi_surf", "CDv_surf"])
            writer.writerow([self.name, self.surf_num, self.surf_area, cref, self.nchord, self.nspan, self.mac, self.CLsurf, self.cl_surf, self.CY_surf, self.cm_surf, self.CD_surf, self.cn_surf, self.CDi_surf, self.CDvsurf])
            writer.writerow(['j', 'yle', 'chord', 'area', 'c_cl', 'ai', 'cl_norm', 'cl', 'cd', 'cdv', 'cm_c4', 'cm_le', 'cp_xc'])
            pd.DataFrame(self.data).to_csv(csvfile, header=None, index=None)

class SurfaceForcesOutput:
    """
    Output wrapper for surface forces
    """

    def __init__(self, fn_file_name):
        """

        :param fn_file_name: file name for the surface forces output file to parse
        """

        with open(fn_file_name, "r") as f:
            # Find start of main data
            for line in f:
                if "n      Area      CL      CD      Cm      CY      Cn      Cl     CDi     CDv" in line:
                    break

            data = []
            names = []
            for line in f:
                if not line.strip():
                    break
                split_data = line.split()
                line_data = split_data[:10]
                line_data = [float(d) for d in line_data]
                names.append(" ".join(split_data[10:]))
                data.append(line_data)

        mat = np.array(data)
        self.n = mat[:, 0]
        self.Area = mat[:, 1]
        self.CL = mat[:, 2]
        self.CD = mat[:, 3]
        self.Cm = mat[:, 4]
        self.CY = mat[:, 5]
        self.Cn = mat[:, 6]
        self.Cl = mat[:, 7]
        self.CDi = mat[:, 8]
        self.CDv = mat[:, 9]
        self.name = names

    def get_df(self):
        return pd.DataFrame(self.__dict__)



def create_input_from_degen_geom(degen_objects=None, degen_set=None, title="DegenAvl", mach=0.0, Sref=1.0, Bref=1.0,
                                 Cref=1.0, cgRef=(0.0, 0.0, 0.0), cdp=0.0):
    """
    Creates an AvlInput object from a list of degen geometry objects

    The degen objects can be created by passing the vsp set from which to create them

    :param degen_objects: List of degen geom objects, these can be created from openvsp. If degen_objects is None,
    degen objects will be created by using OpenVSP on the input degen set
    :param degen_set: OpenVSP set to create degen objects from if :param degen_objects is None
    :param title: title of the avl geometry
    :param mach: mach number
    :param Sref: reference area
    :param Bref: span
    :param Cref: reference chord
    :param cgRef: moment calculation location
    :param cdp: fixed parasite drag value to add to computed induced drag
    :return: AvlInput object
    """
    import degen_geom as dg
    import numpy as np
    header = AvlHeader(title, Mach=mach, Sref=Sref, Cref=Cref, Bref=Bref, CGref=cgRef, CDp=cdp)
    surfaces = []
    components = {}

    # Create degen objects from OpenVSP if no degen objects were passed in. Throw an exception if both degen_objects
    # and degen_set are None
    if degen_objects is None and degen_set is None:
        raise ValueError("degen_objects and degen_set cannot both be set to None")

    if degen_objects is None:
        # Import vsp locally to limit module dependency on openvsp
        import openvsp as vsp

        # Turn off file exports
        vsp.SetIntAnalysisInput("DegenGeom", "WriteCSVFlag", [0], 0)
        vsp.SetIntAnalysisInput("DegenGeom", "WriteMFileFlag", [0], 0)

        # Select the appropriate Set
        vsp.SetIntAnalysisInput("DegenGeom", "Set", [degen_set])

        # Run DegenGeom
        vsp.Update()
        degen_results_id = vsp.ExecAnalysis("DegenGeom")

        # Post process the results from the vsp api into more useful objects
        degen_objects = vsp.parse_degen_geom(degen_results_id)

    for degen_obj in degen_objects:
        component = len(components) + 1
        if degen_obj.name in components:
            component = components[degen_obj.name]
        else:
            components[degen_obj.name] = component

        for stick in degen_obj.sticks:
            surf = AvlSurface(name=degen_obj.name, Nchord=degen_obj.surf.num_pnts, Component=component,
                              Nspan=None, Sspace=None)

            # TODO: Add support for bodies
            if degen_obj.type == dg.DegenTypeEnum.LIFTING:
                # Sort sticks such that increase in y value. If y is constant (e.g. vertical tail) then order from z+ to z-
                # (top to bottom)
                le = np.array(stick.le)
                te = np.array(stick.te)
                chord = np.array(stick.chord)
                u = np.array(stick.u)

                sort_inds = []

                uniq_y = np.unique(le[:, 1])
                if len(uniq_y) > 1:
                    # y is not constant, so now sort based on y coordinate
                    sort_inds = le[:, 1].argsort()
                else:
                    sort_inds = le[:, 2].argsort()[::-1]

                le = le[sort_inds]
                te = te[sort_inds]
                chord = chord[sort_inds]
                u = u[sort_inds]

                span_axis = le[-1, :] - le[0, :]

                nspan = 1
                for i in range(len(le)):
                    nspan += 1
                    if abs(round(u[i]) - u[i]) > 1.0e-10:
                        continue

                    # Compute ainc
                    x_vec = np.array([1.0, 0.0, 0.0])
                    chord_vec = te[i, :] - le[i, :]
                    cos_theta = np.dot(chord_vec, x_vec) / np.linalg.norm(chord_vec)
                    rot_axis = np.cross(x_vec, chord_vec)
                    angle_sign = 1.0

                    cos_theta_span_axis = np.dot(span_axis, rot_axis) / (np.linalg.norm(span_axis)* np.linalg.norm(rot_axis))
                    if np.abs(cos_theta_span_axis) < np.pi/2.0:
                        angle_sign = -1.0

                    ainc = np.rad2deg(np.arccos(np.clip(cos_theta, -1, 1)))*angle_sign
                    sect = AvlSection(le=le[i, :], chord=chord[i], ainc=ainc, Nspan=nspan, Sspace=1.0)
                    surf.addSection(sect)
                    if len(surf.sections) > 1:
                        surf.sections[-2].Nspan = nspan
                    nspan = 2

            if degen_obj.type == dg.DegenTypeEnum.BODY:
                continue

            surfaces.append(surf)

    return AvlInput(header, surfaces)


def runAvl(avlInput, alpha=None, beta=None, savePlots=False,
           binPath=DEFAULT_AVL_BINARY, constraints=None, collect_surface_forces=False, **kwargs):
    """

    :param avlInput: AvlInput object to run or
    :param alpha: array of angles of attack to run
    :param savePlots: boolean to save plots or not (default=False)
    :param binPath: path to binary file (optional)
    :param constraints: constraints structure to be used for trimming (default=None)
    :param collect_surface_forces: True to have avl output surface forces
    :param kwargs: can pass cleanup_flag and change_dir to the RunManager class, if desired
    :return:
    """

    if alpha is None:
        alpha = list(range(-10,21,2))
    if beta is None:
        beta = [0]

    with RunManager(**kwargs) as r:
        cmd = io.StringIO()

        # save avlInput to that directory
        inputFile = "avlinput.txt"
        avlInput.toFile( inputFile)
        plotFile = "plot.ps"
        outputFiles = []
        fsFiles = []
        fnFiles = []
        cmd.write('plop\nG\n\n') # turn off plotting
        cmd.write('load {}\n'.format(inputFile))
        cmd.write('oper\n')
        cmd.write("g\n")  # go to geometry menu
        cmd.write("h\n")  # print current geometry
        cmd.write("no\n")  # turn normals on
        cmd.write("h\n")  # print
        cmd.write("no\n")  # turn normal off
        # Get front view
        cmd.write("v\n")
        cmd.write("0 0\n")
        cmd.write("h\n\n")

        if constraints is not None:
            for constraint in constraints:
                cmd.write("%s %s %f\n" % tuple(constraint))

        for a in alpha:
            for b in beta:
                cmd.write("+\n")  # add new run case
                cmd.write("a a {}\n".format(a))  # add this alpha to the run case
                cmd.write("b b {}\n".format(b))  # add this beta to the run case
                cmd.write("x\n")  # execute run case
                cmd.write("st\n")
                f = "results_alpha_{}_beta_{}.txt".format(a, b)
                outputFiles.append( f )
                cmd.write(f + "\n")

                cmd.write("g\n") # go to geometry menu
                cmd.write("lo\n")  # turn loading on
                cmd.write("v\n-45.0 20.0\n") # set viewpoint to iso
                cmd.write("h\n")  # print
                # Set viewpoint to front
                cmd.write("v\n")
                cmd.write("0 0\n")
                cmd.write("h\n")
                cmd.write("lo\n") # turn loading off
                cmd.write("\n")  #go back to oper menu

                # save out strip forces
                cmd.write("FS\n")
                fsFiles.append("strip_forces_{}_{}.txt".format(a, b))
                cmd.write("{}\n".format(fsFiles[-1]))

                cmd.write("T\n")  # go to trefftz plane menu
                cmd.write("h\n\n")  # print

                # save out surface forces
                if collect_surface_forces:
                    fn_file = "surface_forces_{}_{}.txt".format(a, b)
                    fnFiles.append(fn_file)
                    cmd.write("FN\n")
                    cmd.write(fn_file + "\n")


        cmd.write("\nquit\n")

        p = Popen([binPath], stdout=PIPE, stdin=PIPE, stderr=STDOUT, universal_newlines=True, cwd=r.wd)
        stdOut = p.communicate(input=cmd.getvalue())[0]

        cmd.close()

        results = AvlOutput()
        results.parseFiles(outputFiles)

        try:
            stripForces = []
            for f in fsFiles:
                s = StripForcesOutput(f)
                s.cref = avlInput.header.Cref
                stripForces.append(s)

            results.stripForces = stripForces
            
        except ValueError:
            warnings.warn("AVL strip force parsing failed.")


        try:
            surfaceForces = []
            for f in fnFiles:
                s = SurfaceForcesOutput(f)
                surfaceForces.append(s)
            results.surfaceForces = surfaceForces
        except ValueError:
            warnings.warn("AVL surface force parsing failed.")


        if savePlots:
            results.loadPlots(plotFile)

    return results, stdOut
