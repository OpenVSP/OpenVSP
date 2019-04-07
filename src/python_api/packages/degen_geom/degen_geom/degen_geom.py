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

"""Degen_Geom defines common structures and plotting methods for degenerate geometry types"""
from enum import Enum
import numpy as np
from utilities.transformations import TransMatrix


class DegenTypeEnum(Enum):
    """
    Enum indicating degen geom component type
    """

    LIFTING = 1
    BODY = 2
    DISK = 3


class DegenGeom:
    def __init__(self, res):
        self.name = res.name[0]
        self.geom_id = res.geom_id[0]
        self.surf_index = res.surf_index[0]
        self.main_surf_index = res.main_surf_index[0]
        self.sym_copy_index = res.sym_copy_index[0]
        self.type = DegenTypeEnum.BODY
        if res.type[0] == 'LIFTING_SURFACE':
            self.type = DegenTypeEnum.LIFTING
        elif res.type[0] == 'BODY':
            self.type = DegenTypeEnum.BODY
        elif res.type[0] == 'DISK':
            self.type = DegenTypeEnum.DISK
        self.flip_normal = res.flip_normal[0]
        self.transmat = TransMatrix(mat=np.reshape(res.transmat, (4, 4), order='F'))
        self.surf = None
        self.plates = []
        self.sticks = []
        self.hinge_lines = []
        self.point = None
        self.disk = None


class DegenSurf:
    def __init__(self,results):
        self.num_secs = results.nxsecs[0]
        self.num_pnts = results.num_pnts[0]
        self.x = results.x
        self.y = results.y
        self.z = results.z
        self.u = results.u
        self.w = results.w
        self.nx = results.nx
        self.ny = results.ny
        self.nz = results.nz
        self.area = results.area


class DegenStick:
    def __init__(self, results):
        self.num_secs = results.nxsecs[0]
        self.le = results.le
        self.te = results.te
        self.cgShell = results.cgShell
        self.cgSolid = results.cgSolid
        self.toc = results.toc
        self.tLoc = results.tLoc
        self.chord = results.chord
        self.Isolid = results.Isolid
        self.Ishell = results.Ishell
        self.sectArea = results.sectArea
        self.sectNormal = results.sectNormal
        self.perimTop = results.perimTop
        self.perimBot = results.perimBot
        self.u = results.u
        self.transmat = results.transmat
        self.invtransmat = results.invtransmat
        self.toc2 = results.toc2
        self.tLoc2 = results.tLoc2
        self.anglele = results.anglele
        self.anglete = results.anglete
        self.radleTop = results.radleTop
        self.radleBot = results.radleBot
        self.sweeple = results.sweeple
        self.sweepte = results.sweepte
        self.areaTop = results.areaTop
        self.areaBot = results.areaBot


class DegenHinge:
    def __init__(self, results):
        self.name = results.name
        self.uEnd = results.uEnd
        self.uStart = results.uStart
        self.wEnd = results.wEnd
        self.wStart = results.wStart
        self.xEnd = results.xEnd
        self.xStart = results.xStart


class DegenPoint:
    def __init__(self, results):
        self.vol = results.vol[0]
        self.volWet = results.volWet[0]
        self.area = results.area[0]
        self.areaWet = results.areaWet[0]
        self.Ishell = results.Ishell
        self.Isolid = results.Isolid
        self.cgShell = results.cgShell[0]
        self.cgSolid = results.cgSolid[0]


class DegenDisk:
    def __init__(self, results):
        self.diameter = results.diameter
        self.pos = results.pos
        self.n = results.n


class DegenComponent:
    """
    Class that organizes all degen geom objects associated
    with a single geom id
    """

    def __init__(self):
        self.copies = {}
        """
        Mapping of
        """

        self.max_copies = 0

    def add_degen_geom(self, degen_geom: DegenGeom):
        if degen_geom.sym_copy_index in self.copies:
            self.copies[degen_geom.sym_copy_index].append(degen_geom)
        else:
            self.copies[degen_geom.sym_copy_index] = [degen_geom]
            self.max_copies += 1

    def plot_copies(self):
        import matplotlib.pyplot as plt
        from mpl_toolkits import mplot3d
        import numpy as np

        fig = plt.figure()
        ax = plt.axes(projection='3d')

        max_x = -1e6
        min_x = 1e6
        max_y = max_x
        min_y = min_x
        max_z = max_x
        min_z = min_x
        for key, val in self.copies.items():
            get_color = True
            cmap = plt.get_cmap("tab10")
            color = cmap(key)
            for dg in val:
                surf = dg.surf

                x = np.array(surf.x)
                y = np.array(surf.y)
                z = np.array(surf.z)

                max_x = max(max_x, x.max())
                min_x = min(min_x, x.min())
                max_y = max(max_y, y.max())
                min_y = min(min_y, y.min())
                max_z = max(max_z, z.max())
                min_z = min(min_z, z.min())
                ax.plot_surface(x, y, z, color=color)

        x_range = max_x - min_x
        y_range = max_y - min_y
        z_range = max_z - min_z
        x_cent = (max_x + min_x)/2.0
        y_cent = (max_y + min_y)/2.0
        z_cent = (max_z + min_z)/2.0

        max_range = max(x_range, y_range, z_range)
        margin = 0.6
        plt.xlim(x_cent-margin*max_range, x_cent+margin*max_range)
        plt.ylim(y_cent-margin*max_range, y_cent+margin*max_range)
        ax.set_zlim(z_cent-margin*max_range, z_cent+margin*max_range)
        ax.set_xlabel("x")
        ax.set_ylabel("y")
        ax.set_zlabel("z")
        plt.show()
        return fig


class DegenGeomMgr():
    """
    Helper class to make it easy to access degen geom objects by
    geom id

    """

    def __init__(self, degen_objs):
        """
        Class constructor

        :param degen_objs: list of :class:`DegenGeom` objects
        :type degen_objs: list[:class:`DegenGeom`]
        """

        self.degen_objs = dict()
        """
        Dictionary of degen geom objects. Geom ID used as dictionary key
        """

        for d in degen_objs:
            if d.geom_id in self.degen_objs:
                self.degen_objs[d.geom_id].add_degen_geom(d)
            else:
                degen_comp = DegenComponent()
                degen_comp.add_degen_geom(d)
                self.degen_objs[d.geom_id] = degen_comp
