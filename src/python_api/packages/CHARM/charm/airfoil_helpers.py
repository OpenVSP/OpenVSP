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
import pandas as pd
import utilities.units as uu
from collections import namedtuple


# Define some constants for data frame columns
AF_COL_NAMES_TYPE = namedtuple("AF_COL_NAMES", "mach aoa cl cd cm")
AF_COL_NAMES = AF_COL_NAMES_TYPE("Mach", "Alpha", "CL", "CD", "CM")

AF_META_COL_NAMES_TYPE = namedtuple("AF_META_COL_NAMES", "alpha0 cl_alpha cd_min cl_cd_min cdcl2 cl_max"
                                                         " cl_min stall_aoa neg_stall_aoa")
AF_META_COL_NAMES = AF_META_COL_NAMES_TYPE("alpha0", "cl_alpha", "cd_min", "cl_cd_min", "cdcl2", "cl_max", "cl_min",
                                           "stall_aoa", "neg_stall_aoa")


def create_charm_af_from_df(df: pd.DataFrame, charm_output_name, thickness, airfoil_name, re_data=None):
    """
    Create a text file suitable for the charm automation from a
    Pandas Dataframe

    The Data frame data should have rectangular data from CHARM.
    Columns named "Mach", "Alpha", "CL", "CD", "CM" should exist

    :param df: Pandas DataFrame containing the airfoil data
    :param charm_output_name: name of the file to export airfoil data to
    :param thickness: max thickness of the airfoil (thickness/chord)
    :param airfoil_name: name of the airfoil (will show up in the comment section)
    :param re_data: reynolds number of data. exact data depends on global charm rotor airfoil settings
    """

    # Check that all of the required columns can be found
    for req_col in AF_COL_NAMES:
        if req_col not in df.columns:
            raise ValueError("{} column not found".format(req_col))

    # Get number of mach and angle of attacks
    # Even though the CHARM format supports inputing cl, cd, and cm at different
    # mach numbers and angles of attack

    # Drop all rows with NaNs. Assuming that if one value is nan, there are not going
    # to be other values in the row (e.g. if cl is nan, assuming there isn't going to be a valid cd or cm)
    num_alpha = len(df[AF_COL_NAMES.aoa].unique())
    num_mach = len(df[AF_COL_NAMES.mach].unique())

    # Open up output file
    with open(charm_output_name, "w") as f:
        # Write out thickness value
        f.write("{}\n".format(thickness))
        # Write out Reynolds number or comment
        if re_data is None:
            f.write("COMMENT #2\n")
        else:
            f.write(("{:.8E} " * len(re_data) + "\n").format(*re_data))
        # Write out title and dimensions
        f.write(("{:28s}  " + "{:2d}" * 6 + "\n").format(airfoil_name, num_mach, num_alpha, num_mach, num_alpha, num_mach, num_alpha))

        # Write out mach numbers
        machs = df[AF_COL_NAMES.mach].unique()
        machs.sort()
        f.write("        ")
        for imach, mach in enumerate(machs):
            f.write("  {: 8.3f}".format(mach))
            if (imach+1) % 9 == 0 and (imach + 1) < num_mach:
                f.write("\n        ")

        # Write out alpha and cls
        for alpha, alpha_df in df.groupby(AF_COL_NAMES.aoa):
            f.write("\n{: 8.4f}".format(alpha))
            for imach, mach in enumerate(machs):
                cl = alpha_df.loc[alpha_df[AF_COL_NAMES.mach] == mach][AF_COL_NAMES.cl].unique()[0]
                f.write(" {: 8.6f}".format(cl))
                if (imach+1) % 9 == 0 and (imach + 1) < num_mach:
                    f.write("\n        ")

        # Write out mach
        f.write("\n        ")
        for imach, mach in enumerate(machs):
            f.write("  {: 8.3f}".format(mach))
            if (imach+1) % 9 == 0 and (imach + 1) < num_mach:
                f.write("\n        ")

        # Write out alpha and cds
        for alpha, alpha_df in df.groupby(AF_COL_NAMES.aoa):
            f.write("\n{: 8.4f}".format(alpha))
            for imach, mach in enumerate(machs):
                cd = alpha_df.loc[alpha_df[AF_COL_NAMES.mach] == mach][AF_COL_NAMES.cd].unique()
                f.write(" {: 8.6f}".format(cd[0]))
                if (imach+1) % 9 == 0 and (imach + 1) < num_mach:
                    f.write("\n        ")

        # Write out mach
        f.write("\n        ")
        for imach, mach in enumerate(machs):
            f.write("  {: 8.3f}".format(mach))
            if (imach+1) % 9 == 0 and (imach + 1) < num_mach:
                f.write("\n        ")

        # Write out alpha and cms
        for alpha, alpha_df in df.groupby(AF_COL_NAMES.aoa):
            f.write("\n{: 8.4f}".format(alpha))
            for imach, mach in enumerate(machs):
                cm = alpha_df.loc[alpha_df[AF_COL_NAMES.mach] == mach][AF_COL_NAMES.cm].unique()
                f.write(" {: 8.6f}".format(cm[0]))
                if (imach+1) % 9 == 0 and (imach + 1) < num_mach:
                    f.write("\n        ")
        f.write("\n")


def create_df_from_charm_af(filename):
    """
    Creates a pandas data from *.inp file
    :param filename: name of file to load data from
    :return: pandas data frame with mach, aoa, cl, cd, cm parsed from charm formatted airfoil file
    """
    from scipy.interpolate import interp1d

    def parse_machs(f, num_machs):
        parsed_machs = np.empty(num_machs)
        machs_read = 0
        while machs_read < num_machs:
            line = f.readline()
            machs = [float(m) for m in line.split()]
            parsed_machs[machs_read:machs_read + len(machs)] = np.array(machs)
            machs_read += len(machs)

        return parsed_machs

    def parse_inner_data(f, num_rows, num_cols):
        row_vals = np.empty(num_rows)
        data_vals = np.empty((num_rows, num_cols))
        for ir in range(num_rows):
            data_read = 0
            while data_read < num_cols:
                line = f.readline()
                data = [float(lift) for lift in line.split()]
                if data_read == 0:
                    row_vals[ir] = data[0]
                    del data[0]
                data_vals[ir, data_read:data_read + len(data)] = np.array(data)
                data_read += len(data)

        return row_vals, data_vals

    # Parse the file
    with open(filename, "r") as f:
        f.readline()
        f.readline()
        line = f.readline()

        dimensions = line[30:30 + 12]
        nlmach = int(dimensions[0:2])
        nlaoa = int(dimensions[2:4])
        ndmach = int(dimensions[4:6])
        ndaoa = int(dimensions[6:8])
        nmmach = int(dimensions[8:10])
        nmaoa = int(dimensions[10:12])

        # Parse lift values
        lift_machs = parse_machs(f, nlmach)
        lift_aoas, cls = parse_inner_data(f, nlaoa, nlmach)

        # Parse drag numbers
        drag_machs = parse_machs(f, ndmach)
        drag_aoas, cds = parse_inner_data(f, ndaoa, ndmach)

        # Parse Moment Values
        moment_machs = parse_machs(f, nmmach)
        moment_aoas, cms = parse_inner_data(f, nmaoa, nmmach)

    # Create data frame
    df_data = np.full((nlmach * nlaoa + ndmach * ndaoa + nmmach * nmaoa, 5), np.nan)

    irow = 0
    num_entries = nlmach * nlaoa
    df_data[irow:irow + num_entries, 0] = np.repeat(lift_aoas, nlmach).flatten()
    df_data[irow:irow + num_entries, 1] = np.reshape(np.repeat(lift_machs, nlaoa), (nlaoa, nlmach), order='F').flatten()
    df_data[irow:irow + num_entries, 2] = cls.flatten()

    irow += num_entries
    num_entries = ndmach * ndaoa
    df_data[irow:irow + num_entries, 0] = np.repeat(drag_aoas, ndmach).flatten()
    df_data[irow:irow + num_entries, 1] = np.reshape(np.repeat(drag_machs, ndaoa), (ndaoa, ndmach), order='F').flatten()
    df_data[irow:irow + num_entries, 3] = cds.flatten()

    irow += num_entries
    num_entries = nmmach * nmaoa
    df_data[irow:irow + num_entries, 0] = np.repeat(moment_aoas, nmmach).flatten()
    df_data[irow:irow + num_entries, 1] = np.reshape(np.repeat(moment_machs, nmaoa), (nmaoa, nmmach),
                                                     order='F').flatten()
    df_data[irow:irow + num_entries, 4] = cms.flatten()

    df = pd.DataFrame(df_data, columns=[AF_COL_NAMES.aoa, AF_COL_NAMES.mach, AF_COL_NAMES.cl,
                                        AF_COL_NAMES.cd, AF_COL_NAMES.cm])

    # Perform some pandas manipulations get cl and cd on common set of mach and alphas
    # Interpolate lift/drag coefficients into same set of alphas
    df_cl_pivot = df.dropna(subset=[AF_COL_NAMES.cl]).pivot(index=AF_COL_NAMES.aoa, columns=AF_COL_NAMES.mach)[AF_COL_NAMES.cl]
    df_cd_pivot = df.dropna(subset=[AF_COL_NAMES.cd]).pivot(index=AF_COL_NAMES.aoa, columns=AF_COL_NAMES.mach)[AF_COL_NAMES.cd]
    df_cl_c, df_cd_c = df_cl_pivot.align(df_cd_pivot)
    df_cl_c_dense = df_cl_c.interpolate(method='values').interpolate(method='values', axis=1).unstack().reset_index(name=AF_COL_NAMES.cl)
    df_cd_c_dense = df_cd_c.interpolate(method='values').interpolate(method='values', axis=1).unstack().reset_index(name=AF_COL_NAMES.cd)
    df_cl_cd = pd.concat([df_cl_c_dense, df_cd_c_dense[AF_COL_NAMES.cd]], axis=1)


    # Compute cl_alpha, cd_min, cl_cd_min, cl2cd, cl_max
    airfoil_meta = {AF_COL_NAMES.mach: [], AF_META_COL_NAMES.alpha0: [], AF_META_COL_NAMES.cl_alpha: [],
                    AF_META_COL_NAMES.cd_min: [], AF_META_COL_NAMES.cl_cd_min: [],
                    AF_META_COL_NAMES.cdcl2: [], AF_META_COL_NAMES.cl_max: [], AF_META_COL_NAMES.cl_min: [],
                    AF_META_COL_NAMES.stall_aoa: [], AF_META_COL_NAMES.neg_stall_aoa: []}
    for mach, df_mach in df_cl_cd.groupby(AF_COL_NAMES.mach):
        # Find index closest to alpha = 0
        alpha0_i = df_mach[AF_COL_NAMES.aoa].abs().idxmin()
        cl_diff = df_mach[AF_COL_NAMES.cl].diff()

        try:
            non_stall_max_ind = cl_diff.loc[alpha0_i:].loc[cl_diff.loc[alpha0_i:] < 0].index[0]-1
        except:
            non_stall_max_ind = cl_diff.index[-1]

        try:
            non_stall_min_ind = cl_diff.loc[:alpha0_i].loc[cl_diff.loc[:alpha0_i] < 0].index[-1]
        except:
            non_stall_min_ind = cl_diff.index[0]

        df_non_stall = df_mach.loc[non_stall_min_ind:non_stall_max_ind]
        cl_max = df_non_stall.loc[non_stall_max_ind, AF_COL_NAMES.cl]
        cl_min = df_non_stall.loc[non_stall_min_ind, AF_COL_NAMES.cl]
        stall_angle = df_non_stall.loc[non_stall_max_ind, AF_COL_NAMES.aoa]
        neg_stall_angle = df_non_stall.loc[non_stall_min_ind, AF_COL_NAMES.aoa]

        f_cl = interp1d(df_non_stall[AF_COL_NAMES.cl], df_non_stall[AF_COL_NAMES.aoa])
        alpha_0 = f_cl(0.0).item()
        alpha0_i = (df_mach[AF_COL_NAMES.aoa] - alpha_0).abs().idxmin()

        # fit line through five points around alpha0 to determine cl/alpha
        df_linear_lift = df_non_stall.loc[alpha0_i-2:alpha0_i+3]
        cl_alpha = np.polyfit(df_linear_lift[AF_COL_NAMES.aoa] * uu.deg2rad, df_linear_lift[AF_COL_NAMES.cl], 1)[0]

        # get cd min
        cd_min_ind = df_linear_lift[AF_COL_NAMES.cd].idxmin()
        cd_min = df_linear_lift.loc[cd_min_ind, AF_COL_NAMES.cd]
        cl_cd_min = df_linear_lift.loc[cd_min_ind, AF_COL_NAMES.cl]

        # Force dcdcl2 fit to go through cd_min, therefore not using polyfit
        df_linear_lift = df_non_stall.loc[alpha0_i - 4:alpha0_i + 4]
        y = df_linear_lift[AF_COL_NAMES.cd] - cd_min
        x = (cl_cd_min - df_linear_lift[AF_COL_NAMES.cl])**2.0
        cdcl21, _, _, _ = np.linalg.lstsq(x[:, np.newaxis], y, rcond=None)

        # get cl2cd
        cdcl2 = cdcl21[0]

        # Append data
        airfoil_meta[AF_COL_NAMES.mach].append(mach)
        airfoil_meta[AF_META_COL_NAMES.alpha0].append(alpha_0)
        airfoil_meta[AF_META_COL_NAMES.cl_alpha].append(cl_alpha)
        airfoil_meta[AF_META_COL_NAMES.cd_min].append(cd_min)
        airfoil_meta[AF_META_COL_NAMES.cl_cd_min].append(cl_cd_min)
        airfoil_meta[AF_META_COL_NAMES.cdcl2].append(cdcl2)
        airfoil_meta[AF_META_COL_NAMES.stall_aoa].append(stall_angle)
        airfoil_meta[AF_META_COL_NAMES.neg_stall_aoa].append(neg_stall_angle)
        airfoil_meta[AF_META_COL_NAMES.cl_max].append(cl_max)
        airfoil_meta[AF_META_COL_NAMES.cl_min].append(cl_min)

    df_airfoil_meta = pd.DataFrame(airfoil_meta)

    return df, df_cl_cd, df_airfoil_meta
