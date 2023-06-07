# Copyright (c) 2023 Rob McDonald

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

import f90nml
import numpy as np
import matplotlib.pyplot as plt
import matplotlib
from dataclasses import dataclass
from pydantic import BaseModel, Field


@dataclass
class loadsData:
    ntstps: int = 0
    dt: float = 0
    aldeg: float = 0
    yawdeg: float = 0
    cbar: float = 0
    sref: float = 0
    sspan: float = 0
    mx: float = 0
    my: float = 0
    mz: float = 0
    npatch: int = 0
    nsectional: int = 0
    maxsect: int = 0
    patno = []
    patid = []
    komp = []
    klass = []
    nrow = []
    patncol = []
    ipan = []
    lpan = []
    npans = []
    patname = []
    xle_sect = []
    yle_sect = []
    zle_sect = []
    chord_sect = []
    circ_sect = []
    eta_sect = []
    Clift_sect = []
    Cdrag_sect = []
    Cside_sect = []
    Cpitch_sect = []
    Cyaw_sect = []
    Croll_sect = []
    Clift_patch: np.ndarray = Field(default_factory=lambda: np.zeros())  # shape=<int>, dtype=<type>
    Cdrag_patch: np.ndarray = Field(default_factory=lambda: np.zeros())
    Cside_patch: np.ndarray = Field(default_factory=lambda: np.zeros())
    Cpitch_patch: np.ndarray = Field(default_factory=lambda: np.zeros())
    Cyaw_patch: np.ndarray = Field(default_factory=lambda: np.zeros())
    Croll_patch: np.ndarray = Field(default_factory=lambda: np.zeros())
    Sfrac_patch: np.ndarray = Field(default_factory=lambda: np.zeros())
    Clift: float = 0
    Cdrag: float = 0
    Cside: float = 0
    Cpitch: float = 0
    Cyaw: float = 0
    Croll: float = 0
    CdragTrefftz: float = 0
    CliftTrefftz: float = 0


@dataclass
class surfData:
    ntstep: int = 0
    npatch: int = 0
    pnames = []
    ncol: np.ndarray = Field(default_factory=lambda: np.zeros())
    nrow: np.ndarray = Field(default_factory=lambda: np.zeros())
    initpan: np.ndarray = Field(default_factory=lambda: np.zeros())
    finalpan: np.ndarray = Field(default_factory=lambda: np.zeros())
    x_corner = []
    y_corner = []
    z_corner = []
    x_center = []
    y_center = []
    z_center = []
    x_normal = []
    y_normal = []
    z_normal = []
    nwake: int = 0
    wnames = []
    nwcol: np.ndarray = Field(default_factory=lambda: np.zeros())
    nwrow: np.ndarray = Field(default_factory=lambda: np.zeros())
    iwpan: np.ndarray = Field(default_factory=lambda: np.zeros())
    lwpan: np.ndarray = Field(default_factory=lambda: np.zeros())
    xw_corner = []
    yw_corner = []
    zw_corner = []
    dub_corner = []
    vx_corner = []
    vy_corner = []
    vz_corner = []
    cp_corner = []
    dub_center = []
    vx_center = []
    vy_center = []
    vz_center = []
    cp_center = []


def pos_file_str(file, target_string):
    # Read the file character by character
    while True:
        char = file.read(1)
        lastpos = file.tell()

        if not char:
            # End of file reached without finding the target string
            return ld
            break

        if char == target_string[0]:

            # Check if the remaining characters match the target string
            remaining_chars = file.read(len(target_string) - 1)

            if remaining_chars == target_string[1:]:
                # remaining_contents = file.readline()
                # print("Remaining contents of line:")
                # print(remaining_contents)
                return file.tell()
                break
            else:
                file.seek(lastpos)


def ReadPMARC12LoadData(fname, plotflag=True):
    ld = loadsData()

    with open(fname, 'r') as fp:
        if pos_file_str(fp, "NTSTPS") == -1: return ld
        fp.readline()  # skip to end of line.
        dat = fp.readline().split()
        ld.ntstps = int(dat[0])
        ld.dt = float(dat[1])

        if pos_file_str(fp, "ALDEG") == -1: return ld
        fp.readline()  # skip to end of line.
        dat = fp.readline().split()
        ld.aldeg = float(dat[0])
        ld.yawdeg = float(dat[1])

        if pos_file_str(fp, "CBAR") == -1: return ld
        fp.readline()  # skip to end of line.
        dat = fp.readline().split()
        ld.cbar = float(dat[0])
        ld.sref = float(dat[1])
        ld.sspan = float(dat[2])
        ld.mx = float(dat[3])
        ld.my = float(dat[4])
        ld.mz = float(dat[5])

        if pos_file_str(fp, "BASIC PATCH DATA") == -1: return ld
        fp.readline()  # skip to end of line.
        fp.readline()  # skip line.
        fp.readline()  # skip line.

        dat = fp.readline().split()

        while dat[0].isdigit():
            pn, pid, ko, kl, nr, pnc, ip, lp, npa = map(int, dat[0:9])
            ld.patno.append(pn)
            ld.patid.append(pid)
            ld.komp.append(ko)
            ld.klass.append(kl)
            ld.nrow.append(nr)
            ld.patncol.append(pnc)
            ld.ipan.append(ip)
            ld.lpan.append(lp)
            ld.npans.append(npa)
            ld.patname.append(dat[9].strip())

            if pnc > ld.maxsect:
                ld.maxsect = pnc

            if pid == 1:
                ld.nsectional += 1

            ld.npatch += 1
            dat = fp.readline().split()

        # Move to final time step
        if pos_file_str(fp, "TIME STEP %3d" % ld.ntstps) == -1: return ld
        fp.readline()  # skip to end of line.

        for ipat in range(ld.npatch):
            if ld.patid[ipat] == 1:  # Wing type patch, section data provided

                xle = np.zeros(ld.patncol[ipat])
                yle = np.zeros(ld.patncol[ipat])
                zle = np.zeros(ld.patncol[ipat])
                chord = np.zeros(ld.patncol[ipat])
                circ = np.zeros(ld.patncol[ipat])
                eta = np.zeros(ld.patncol[ipat])
                Clift = np.zeros(ld.patncol[ipat])
                Cdrag = np.zeros(ld.patncol[ipat])
                Cside = np.zeros(ld.patncol[ipat])
                Cpitch = np.zeros(ld.patncol[ipat])
                Cyaw = np.zeros(ld.patncol[ipat])
                Croll = np.zeros(ld.patncol[ipat])

                for icol in range(ld.patncol[ipat]):

                    if pos_file_str(fp, "SECTION PARAMETERS") == -1: return ld
                    fp.readline()  # skip to end of line.
                    fp.readline()  # skip line.

                    xle[icol], yle[icol], zle[icol], chord[icol], circ[icol], eta[icol] = map(float,
                                                                                              fp.readline().split())

                    if pos_file_str(fp, "WIND") == -1: return ld

                    Clift[icol], Cdrag[icol], Cside[icol], Cpitch[icol], Cyaw[icol], Croll[icol] = map(float,
                                                                                                       fp.readline().split())

                ld.xle_sect.append(xle)
                ld.yle_sect.append(yle)
                ld.zle_sect.append(zle)
                ld.chord_sect.append(chord)
                ld.circ_sect.append(circ)
                ld.eta_sect.append(eta)
                ld.Clift_sect.append(Clift)
                ld.Cdrag_sect.append(Cdrag)
                ld.Cside_sect.append(Cside)
                ld.Cpitch_sect.append(Cpitch)
                ld.Cyaw_sect.append(Cyaw)
                ld.Croll_sect.append(Croll)

        if pos_file_str(fp, "FORCE AND MOMENT COEFFICIENTS") == -1: return ld
        fp.readline()  # skip to end of line.

        if pos_file_str(fp, "WIND AXES") == -1: return ld
        fp.readline()  # skip to end of line.

        ld.Clift_patch = np.zeros(ld.npatch)
        ld.Cdrag_patch = np.zeros(ld.npatch)
        ld.Cside_patch = np.zeros(ld.npatch)
        ld.Cpitch_patch = np.zeros(ld.npatch)
        ld.Cyaw_patch = np.zeros(ld.npatch)
        ld.Croll_patch = np.zeros(ld.npatch)
        ld.Sfrac_patch = np.zeros(ld.npatch)

        for ipat in range(ld.npatch):
            if pos_file_str(fp, ld.patname[ipat].strip()) == -1: return ld

            ld.Clift_patch[ipat], ld.Cdrag_patch[ipat], ld.Cside_patch[ipat], ld.Cpitch_patch[ipat], ld.Cyaw_patch[
                ipat], ld.Croll_patch[ipat], ld.Sfrac_patch[ipat] = map(float, fp.readline().split())

        if pos_file_str(fp, "TOTAL COEFFICIENTS") == -1: return ld
        fp.readline()  # skip to end of line.

        if pos_file_str(fp, "CL") == -1: return ld
        fp.readline()  # skip to end of line.
        fp.readline()  # skip line.

        ld.Clift, ld.Cdrag, ld.Cside, ld.Cpitch, ld.Cyaw, ld.Croll = map(float, fp.readline().split())

        if pos_file_str(fp, "TREFFTZ") == -1: return ld
        fp.readline()  # skip to end of line.

        if pos_file_str(fp, "TOTAL INDUCED DRAG COEFFICIENT CDI =") == -1: return ld

        ld.CdragTrefftz = float(fp.readline())

        if pos_file_str(fp, "TOTAL LIFT COEFFICIENT CL =") == -1: return ld

        ld.CliftTrefftz = float(fp.readline())

    # Plot load distributions
    if plotflag:
        fig = plt.figure()
        ax = fig.add_subplot(111)

        for isect in range(ld.nsectional):
            ax.plot(ld.yle_sect[isect], ld.Clift_sect[isect] * ld.chord_sect[isect])

        plt.show()

    return ld


def ReadPMARC12SurfData(fname='DATA22', plotflag=True):
    sd = surfData()

    # Open the file for parsing.
    with open(fname, 'r') as fp:
        # Read in three integers from first line. Not sure what they are.
        a = [int(x) for x in fp.readline().split()]
        sd.ntstep = a[1]

        # print( sd.ntstep )

        # Read in number of patches
        sd.npatch = int(fp.readline())

        sd.ncol = np.zeros(sd.npatch, dtype=int)
        sd.nrow = np.zeros(sd.npatch, dtype=int)
        sd.initpan = np.zeros(sd.npatch, dtype=int)
        sd.finalpan = np.zeros(sd.npatch, dtype=int)

        for ipatch in range(sd.npatch):
            # fp.readline()  # read to next line
            sd.pnames.append(fp.readline().strip())
            a = [int(x) for x in fp.readline().split()]
            sd.ncol[ipatch] = a[0]  # Number of columns in patch
            sd.nrow[ipatch] = a[1]  # Number of rows in patch
            sd.initpan[ipatch] = a[2]  # Initial panel of patch
            sd.finalpan[ipatch] = a[3]  # Last panel of patch

        # print(sd.pnames)
        # print(sd.ncol)
        # print(sd.nrow)
        # print(sd.initpan)
        # print(sd.finalpan)

        for ipatch in range(sd.npatch):

            nr = sd.nrow[ipatch]
            nc = sd.ncol[ipatch]

            ncenter = nr * nc
            ncorner = (nr + 1) * (nc + 1)

            # print(ncenter)
            # print(ncorner)

            x = np.zeros(ncorner)
            y = np.zeros(ncorner)
            z = np.zeros(ncorner)

            xc = np.zeros(ncenter)
            yc = np.zeros(ncenter)
            zc = np.zeros(ncenter)
            xn = np.zeros(ncenter)
            yn = np.zeros(ncenter)
            zn = np.zeros(ncenter)

            k = 0
            icp = 0
            for icol in range(sd.ncol[ipatch] + 1):
                for irow in range(sd.nrow[ipatch] + 1):
                    if irow < sd.nrow[ipatch] and icol < sd.ncol[ipatch]:
                        a = [float(x) for x in fp.readline().split()]  # Read in 9 floats
                        # print(a)
                        x[icp] = a[0]  # Corner points
                        y[icp] = a[1]
                        z[icp] = a[2]
                        xc[k] = a[3]  # Center points
                        yc[k] = a[4]
                        zc[k] = a[5]
                        xn[k] = a[6]  # Normal vectors
                        yn[k] = a[7]
                        zn[k] = a[8]
                        k += 1
                        icp += 1
                    else:
                        a = [float(x) for x in fp.readline().split()]  # Read in 3 floats
                        # print(a)
                        x[icp] = a[0]  # Corner points
                        y[icp] = a[1]
                        z[icp] = a[2]
                        icp += 1

            sd.x_corner.append(np.reshape(x, (nr + 1, nc + 1), order='F'))
            sd.y_corner.append(np.reshape(y, (nr + 1, nc + 1), order='F'))
            sd.z_corner.append(np.reshape(z, (nr + 1, nc + 1), order='F'))

            sd.x_center.append(np.reshape(xc, (nr, nc), order='F'))
            sd.y_center.append(np.reshape(yc, (nr, nc), order='F'))
            sd.z_center.append(np.reshape(zc, (nr, nc), order='F'))
            sd.x_normal.append(np.reshape(xn, (nr, nc), order='F'))
            sd.y_normal.append(np.reshape(yn, (nr, nc), order='F'))
            sd.z_normal.append(np.reshape(zn, (nr, nc), order='F'))

        # Read and ignore panel neighbor data
        for ipanel in range(sd.finalpan[-1]):
            a = [int(x) for x in fp.readline().split()]  # Read in 8 int

        for itstep in range(sd.ntstep):
            # print(itstep)

            sd.nwake = int(fp.readline())
            # print(sd.nwake)

            sd.nwcol = np.zeros(sd.nwake, dtype=int)
            sd.nwrow = np.zeros(sd.nwake, dtype=int)
            sd.iwpan = np.zeros(sd.nwake, dtype=int)
            sd.lwpan = np.zeros(sd.nwake, dtype=int)

            for iwake in range(sd.nwake):
                sd.wnames.append(fp.readline().strip())
                a = [int(x) for x in fp.readline().split()]
                # print(a)
                sd.nwcol[iwake] = a[0]  # Number of columns in patch
                sd.nwrow[iwake] = a[1]  # Number of rows in patch
                sd.iwpan[iwake] = a[2]  # Initial panel of patch
                sd.lwpan[iwake] = a[3]  # Last panel of patch

            # print(sd.wnames)
            # print(sd.nwcol)
            # print(sd.nwrow)
            # print(sd.iwpan)
            # print(sd.lwpan)

            xw_corner = []
            yw_corner = []
            zw_corner = []

            for iwake in range(sd.nwake):

                nr = sd.nwrow[iwake]
                nc = sd.nwcol[iwake]

                nwcenter = nr * nc
                nwcorner = (nr + 1) * (nc + 1)

                xw = np.zeros(nwcorner)
                yw = np.zeros(nwcorner)
                zw = np.zeros(nwcorner)

                k = 0
                icp = 0
                for icol in range(sd.nwcol[iwake] + 1):
                    for irow in range(sd.nwrow[iwake] + 1):
                        if irow < sd.nwrow[iwake] and icol < sd.nwcol[iwake]:
                            a = [float(x) for x in fp.readline().split()]  # Read in 9 floats
                            # print(a)
                            xw[icp] = a[0]  # Corner points
                            yw[icp] = a[1]
                            zw[icp] = a[2]
                            k += 1
                            icp += 1
                        else:
                            a = [float(x) for x in fp.readline().split()]  # Read in 3 floats
                            # print(a)
                            xw[icp] = a[0]  # Corner points
                            yw[icp] = a[1]
                            zw[icp] = a[2]
                            icp += 1

                xw_corner.append(np.reshape(xw, (nr + 1, nc + 1), order='F'))
                yw_corner.append(np.reshape(yw, (nr + 1, nc + 1), order='F'))
                zw_corner.append(np.reshape(zw, (nr + 1, nc + 1), order='F'))

            sd.xw_corner.append(xw_corner)
            sd.yw_corner.append(yw_corner)
            sd.zw_corner.append(zw_corner)

            dub_corner = []
            vx_corner = []
            vy_corner = []
            vz_corner = []
            cp_corner = []

            dub_center = []
            vx_center = []
            vy_center = []
            vz_center = []
            cp_center = []

            for ipatch in range(sd.npatch):

                nr = sd.nrow[ipatch]
                nc = sd.ncol[ipatch]

                ncenter = nr * nc
                ncorner = (nr + 1) * (nc + 1)

                dub = np.zeros(ncorner)
                vx = np.zeros(ncorner)
                vy = np.zeros(ncorner)
                vz = np.zeros(ncorner)
                cp = np.zeros(ncorner)

                dubc = np.zeros(ncenter)
                vxc = np.zeros(ncenter)
                vyc = np.zeros(ncenter)
                vzc = np.zeros(ncenter)
                cpc = np.zeros(ncenter)

                k = 0
                icp = 0
                for icol in range(sd.ncol[ipatch] + 1):
                    for irow in range(sd.nrow[ipatch] + 1):
                        if irow < sd.nrow[ipatch] and icol < sd.ncol[ipatch]:
                            a = [float(x) for x in fp.readline().split()]  # Read in 7 floats
                            # print(a)
                            dub[icp] = a[0]  # Corner points
                            vx[icp] = a[1]
                            vy[icp] = a[2]
                            vz[icp] = a[3]
                            # Skip 4
                            cp[icp] = a[5]
                            # Skip 6
                            a = [float(x) for x in fp.readline().split()]  # Read in 7 floats
                            # print(a)
                            dubc[k] = a[0]  # Center points
                            vxc[k] = a[1]
                            vyc[k] = a[2]
                            vzc[k] = a[3]
                            # Skip 4
                            cpc[k] = a[5]
                            # Skip 6
                            k += 1
                            icp += 1
                        else:
                            a = [float(x) for x in fp.readline().split()]  # Read in 7 floats
                            # print(a)
                            dub[icp] = a[0]  # Corner points
                            vx[icp] = a[1]
                            vy[icp] = a[2]
                            vz[icp] = a[3]
                            # Skip 4
                            cp[icp] = a[5]
                            # Skip 6
                            icp += 1

                dub_corner.append(np.reshape(dub, (nr + 1, nc + 1), order='F'))
                vx_corner.append(np.reshape(vx, (nr + 1, nc + 1), order='F'))
                vy_corner.append(np.reshape(vy, (nr + 1, nc + 1), order='F'))
                vz_corner.append(np.reshape(vz, (nr + 1, nc + 1), order='F'))
                cp_corner.append(np.reshape(cp, (nr + 1, nc + 1), order='F'))

                dub_center.append(np.reshape(dubc, (nr, nc), order='F'))
                vx_center.append(np.reshape(vxc, (nr, nc), order='F'))
                vy_center.append(np.reshape(vyc, (nr, nc), order='F'))
                vz_center.append(np.reshape(vzc, (nr, nc), order='F'))
                cp_center.append(np.reshape(cpc, (nr, nc), order='F'))

            sd.dub_corner.append(dub_corner)
            sd.vx_corner.append(vx_corner)
            sd.vy_corner.append(vy_corner)
            sd.vz_corner.append(vz_corner)
            sd.cp_corner.append(cp_corner)

            sd.dub_center.append(dub_center)
            sd.vx_center.append(vx_center)
            sd.vy_center.append(vy_center)
            sd.vz_center.append(vz_center)
            sd.cp_center.append(cp_center)

        # Read whatever remains in the file and dump it out to screen.
        print(fp.readlines())

    # Visualize surface solution and wake.
    if plotflag:
        fig = plt.figure()
        ax = fig.add_subplot(111, projection='3d')

        norm = matplotlib.colors.Normalize(vmin=-3, vmax=1)

        for ipatch in range(sd.npatch):
            ax.plot_surface(sd.x_corner[ipatch], sd.y_corner[ipatch], sd.z_corner[ipatch],
                            facecolors=matplotlib.cm.jet(norm(sd.cp_center[-1][ipatch])), shade=False)

        for iwake in range(sd.nwake):
            ax.plot_wireframe(sd.xw_corner[-1][iwake], sd.yw_corner[-1][iwake], sd.zw_corner[-1][iwake])

        fig.colorbar(matplotlib.cm.ScalarMappable(norm=norm, cmap=matplotlib.cm.jet), ax=ax)
        ax.axis('off')
        ax.set_aspect('equal')

        plt.show()

    return sd



def modNamelist(fin, fout, grp, var, val):
    """
    Writes a new namelist file based on a baseline file and some new values
    :param fin: path to input namelist file
    :param fout: path to output namelist file
    :param grp: list of group names for changes
    :param var: list of variable names for changes
    :param val: list of values for changes
    :return:
    """
    nchange = len(grp)

    nml_pat = f90nml.Namelist()
    for i in range(nchange):
        if grp[i] not in nml_pat:
            nml_pat[grp[i]] = f90nml.Namelist()

        nml_pat[grp[i]][var[i]] = val[i]

    f90nml.patch(fin, nml_pat, fout)
