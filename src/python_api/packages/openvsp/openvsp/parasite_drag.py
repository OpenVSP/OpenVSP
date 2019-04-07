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

import openvsp as vsp


class ParasiteDragResults:
    """
    Class that contains all of the results from a parasite drag sweep in
    OpenVSP
    """
    def __init__(self, results):
        self.Alt_Label = [r.Alt_Label[0] for r in results]
        self.Comp_CD = [r.Comp_CD for r in results]
        self.Comp_Cf = [r.Comp_Cf for r in results]
        self.Comp_FFEqn = [r.Comp_FFEqn for r in results]
        self.Comp_FFEqnName = [r.Comp_FFEqnName for r in results]
        self.Comp_FFIn = [r.Comp_FFIn for r in results]
        self.Comp_FFOut = [r.Comp_FFOut for r in results]
        self.Comp_FineRat = [r.Comp_FineRat for r in results]
        self.Comp_ID = [r.Comp_ID for r in results]
        self.Comp_Label = [r.Comp_Label for r in results]
        self.Comp_Lref = [r.Comp_Lref for r in results]
        self.Comp_PercLam = [r.Comp_PercLam for r in results]
        self.Comp_PercTotalCD = [r.Comp_PercTotalCD for r in results]
        self.Comp_Q = [r.Comp_Q for r in results]
        self.Comp_Re = [r.Comp_Re for r in results]
        self.Comp_Roughness = [r.Comp_Roughness for r in results]
        self.Comp_SurfNum = [r.Comp_SurfNum for r in results]
        self.Comp_Swet = [r.Comp_Swet for r in results]
        self.Comp_TawTwRatio = [r.Comp_TawTwRatio for r in results]
        self.Comp_TeTwRatio = [r.Comp_TeTwRatio for r in results]
        self.Comp_f = [r.Comp_f for r in results]
        self.Excres_Amount = [r.Excres_Amount for r in results]
        self.Excres_CD_Total = [r.Excres_CD_Total[0] for r in results]
        self.Excres_Input = [r.Excres_Input for r in results]
        self.Excres_Label = [r.Excres_Label for r in results]
        self.Excres_PercTotalCD = [r.Excres_PercTotalCD for r in results]
        self.Excres_Perc_Total = [r.Excres_Perc_Total for r in results]
        self.Excres_Type = [r.Excres_Type for r in results]
        self.Excres_f_Total = [r.Excres_f_Total[0] for r in results]
        self.Alt = [r.FC_Alt[0] for r in results]
        self.Mach = [r.FC_Mach[0] for r in results]
        self.Pres = [r.FC_Pres[0] for r in results]
        self.Rho = [r.FC_Rho[0] for r in results]
        self.Sref = [r.FC_Sref[0] for r in results]
        self.Temp = [r.FC_Temp[0] for r in results]
        self.Vinf = [r.FC_Vinf[0] for r in results]
        self.Geom_CD_Total = [r.Geom_CD_Total for r in results]
        self.Geom_Perc_Total = [r.Geom_Perc_Total for r in results]
        self.Geom_f_Total = [r.Geom_f_Total for r in results]
        self.LamCfEqnName = [r.LamCfEqnName for r in results]
        self.Lref_Label = [r.Lref_Label for r in results]
        self.Num_Comp = [r.Num_Comp for r in results]
        self.Num_Excres = [r.Num_Excres for r in results]
        self.Pres_Label = [r.Pres_Label for r in results]
        self.Rho_Label = [r.Rho_Label for r in results]
        self.Sref_Label = [r.Sref_Label for r in results]
        self.Swet_Label = [r.Swet_Label for r in results]
        self.Temp_Label = [r.Temp_Label for r in results]
        self.CD_Total = [r.Total_CD_Total[0] for r in results]
        self.Total_Perc_Total = [r.Total_Perc_Total[0] for r in results]
        self.Total_f_Total = [r.Total_f_Total[0] for r in results]
        self.TurbCfEqnName = [r.TurbCfEqnName for r in results]
        self.Vinf_Label = [r.Vinf_Label[0] for r in results]
        self.f_Label = [r.f_Label[0] for r in results]

    def plot(self):
        """
        Creates plot of CD0 vs speed and altitude
        """
        import matplotlib.pyplot as plt
        import numpy as np

        data = np.column_stack((self.Vinf, self.Alt, self.CD_Total, self.Rho))

        speeds = np.unique(data[:, 0])
        alts = np.unique(data[:, 1])

        plt.figure()
        leg_str = []
        for alt in alts:
            d_filtered = data[data[:, 1] == alt, :]
            cd = d_filtered[:, 2]
            rho = d_filtered[:, 3]
            # d = [cd[i]*(speeds[i]*1.46667)**2*self.Sref[0]*0.5*rho[i] for i in range(len(cd))]
            plt.plot(speeds, cd)
            leg_str.append("Alt = %d" % alt)

        plt.xlabel(self.Vinf_Label[0])
        plt.ylabel('CD_0')
        plt.xlim(xmin=0.0)
        plt.ylim(ymin=0.0)
        plt.legend(leg_str)
        plt.show()

    def build_interpolator(self):
        """ Returns interpolator to interpolate results as a function of speed and altitude """
        from scipy.interpolate import LinearNDInterpolator, interp1d
        import numpy as np

        alts = np.unique(self.Alt)
        if len(alts) > 1:
            return LinearNDInterpolator((self.Vinf, self.Alt), (self.CD_Total))
        else:
            intrp = interp1d(self.Vinf, self.CD_Total)
            return lambda v, a: intrp(v)


def parasitedrag_sweep(speeds, alts_ft, sref=None, length_unit=None,
                       speed_unit=vsp.V_UNIT_MACH, set=None):
    """
    Runs a parasite drag sweep over a range of speeds and altitudes. For subsonic data only.

    :param speeds: list of speeds to sweep over
    :param alts_ft: list of altitudes to sweep over
    :param speed_unit: units of speed array
    :param set: vsp geometry set to use for build up
    :param length_unit: length unit of the vsp model
    :param sref: reference area
    :return: named tuple with results
    """
    # Reset default values to ensure values that have been read from a vsp file are used by default
    vsp.SetAnalysisInputDefaults('ParasiteDrag')

    # inputs that don't change during a sweep

    vsp.SetIntAnalysisInput("ParasiteDrag", "VelocityUnit", [speed_unit])
    if length_unit is not None:
        vsp.SetIntAnalysisInput("ParasiteDrag", "LengthUnit", [length_unit])

    vsp.SetStringAnalysisInput("ParasiteDrag", "FileName", ["/dev/null"])

    if set is not None:
        vsp.SetIntAnalysisInput("ParasiteDrag", "GeomSet", [set])

    if sref is not None:
        vsp.SetIntAnalysisInput("ParasiteDrag", "RefFlag", [0])
        vsp.SetDoubleAnalysisInput("ParasiteDrag", "Sref", [float(sref)])

    vsp.SetVSP3FileName('/dev/null')

    results = []
    first_val = True
    for speed in speeds:
        for alt in alts_ft:
            vsp.SetDoubleAnalysisInput("ParasiteDrag", "Vinf", [float(speed)])
            vsp.SetDoubleAnalysisInput("ParasiteDrag", "Altitude", [float(alt)])
            if first_val:
                vsp.SetIntAnalysisInput("ParasiteDrag", "RecomputeGeom", [True])
                first_val = False
            else:
                vsp.SetIntAnalysisInput("ParasiteDrag", "RecomputeGeom", [False])
            results.append(vsp.parse_results_object(vsp.ExecAnalysis("ParasiteDrag")))

    return ParasiteDragResults(results)
