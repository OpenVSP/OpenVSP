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

"""Degen_Geom defines common structures and plotting methods for degenerate geometry types"""
from enum import Enum
import numpy as np
from utilities.transformations import TransMatrix

import typing


class DegenTypeEnum(Enum):
    LIFTING = 1
    BODY = 2
    DISK = 3
    MESH = 4


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
        elif res.type[0] == 'MESH':
            self.type = DegenTypeEnum.MESH
        self.flip_normal = res.flip_normal[0]
        self.transmat = TransMatrix(mat=np.reshape(res.transmat, (4, 4), order='F'))
        self.surf = None
        self.plates: typing.List[DegenPlate] = []
        self.sticks: typing.List[DegenStick] = []
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


class DegenPlate:
    def __init__(self, results):
        self.n = results.n
        self.nCamber_x = results.nCamber_x
        self.nCamber_y = results.nCamber_y
        self.nCamber_z = results.nCamber_z
        self.num_pnts = results.num_pnts[0]
        self.num_secs = results.nxsecs[0]
        self.t = results.t
        self.u = results.u
        self.wBot = results.wBot
        self.wTop = results.wTop
        self.x = results.x
        self.y = results.y
        self.z = results.z
        self.zCamber = results.zCamber

    def _compute_areas_brute_force(self):
        num_i = len(self.x)
        num_j = len(self.x[0])
        areas = np.empty((num_i-1, num_j-1))
        for i in range(num_i-1):
            for j in range(num_j-1):
                a = np.array([self.x[i][j], self.y[i][j], self.z[i][j]])
                b = np.array([self.x[i][j+1], self.y[i][j+1], self.z[i][j+1]])
                c = np.array([self.x[i+1][j+1], self.y[i+1][j+1], self.z[i+1][j+1]])
                d = np.array([self.x[i+1][j], self.y[i+1][j], self.z[i+1][j]])
                areas[i, j] = 0.5*(np.linalg.norm(np.cross(b-a, c-b)) + np.linalg.norm(np.cross(d-c, a-d)))
        return areas

    def compute_areas(self):
        """
        Computes the areas of all the surface panels
        :return: numpy array of panel areas
        """
        v = np.stack((self.x, self.y, self.z), axis=2)
        d_i = np.diff(v, axis=0)
        d_j = np.diff(v, axis=1)
        areas = 0.5*(np.linalg.norm(np.cross(-d_j[:-1, :, :], d_i[:, 1:, :]), axis=2) +
                     np.linalg.norm(np.cross(-d_i[:, :-1, :], d_j[1:, :, :]), axis=2))
        return areas

    def compute_centroid(self):
        """
        Computes area weighted centroild of entire plate
        :return:
        """
        areas = self.compute_areas()

        # Brute force compute
        num_i = len(self.x)
        num_j = len(self.x[0])
        centroid = np.zeros(3)
        for i in range(num_i-1):
            for j in range(num_j-1):
                a = np.array([self.x[i][j], self.y[i][j], self.z[i][j]])
                b = np.array([self.x[i][j + 1], self.y[i][j + 1], self.z[i][j + 1]])
                c = np.array([self.x[i + 1][j + 1], self.y[i + 1][j + 1], self.z[i + 1][j + 1]])
                d = np.array([self.x[i + 1][j], self.y[i + 1][j], self.z[i + 1][j]])
                centroid = centroid + (a+b+c+d)/4.0*areas[i, j]

        centroid = centroid/areas.sum()
        return centroid


class DegenComponent:
    def __init__(self, name=None):
        self.copies = {}
        self.max_copies = 0
        self.name = name

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

    def plot_copies_plates(self, ax=None):
        """
        Plots plates, colored by copy
        :param ax: optional axis to plot on
        :return: ax
        """
        import matplotlib.pyplot as plt
        from mpl_toolkits import mplot3d
        import numpy as np

        if ax is None:
            fig = plt.figure()
            ax = plt.axes(projection='3d')

        max_x = -1e6
        min_x = 1e6
        max_y = max_x
        min_y = min_x
        max_z = max_x
        min_z = min_x
        for key, val in self.copies.items():
            cmap = plt.get_cmap("tab10")
            color = cmap(key)
            for dg in val:
                for plate in dg.plates:
                    x = np.array(plate.x)
                    y = np.array(plate.y)
                    z = np.array(plate.z)

                    centroid = plate.compute_centroid()

                    max_x = max(max_x, x.max())
                    min_x = min(min_x, x.min())
                    max_y = max(max_y, y.max())
                    min_y = min(min_y, y.min())
                    max_z = max(max_z, z.max())
                    min_z = min(min_z, z.min())
                    ax.plot_wireframe(x, y, z, color=color)
                    ax.scatter(centroid[0], centroid[1], centroid[2], marker='o', color='r', s=5)

        x_range = max_x - min_x
        y_range = max_y - min_y
        z_range = max_z - min_z
        x_cent = (max_x + min_x)/2.0
        y_cent = (max_y + min_y)/2.0
        z_cent = (max_z + min_z)/2.0

        max_range = max(x_range, y_range, z_range)
        margin = 0.6
        ax.set_xlim(x_cent-margin*max_range, x_cent+margin*max_range)
        ax.set_ylim(y_cent-margin*max_range, y_cent+margin*max_range)
        ax.set_zlim(z_cent-margin*max_range, z_cent+margin*max_range)
        ax.set_xlabel("x")
        ax.set_ylabel("y")
        ax.set_zlabel("z")
        return ax


class DegenGeomMgr():
    def __init__(self, degen_objs):
        self.degen_objs = dict()
        for d in degen_objs:
            if d.geom_id in self.degen_objs:
                self.degen_objs[d.geom_id].add_degen_geom(d)
            else:
                degen_comp = DegenComponent(name=d.name)
                degen_comp.add_degen_geom(d)
                self.degen_objs[d.geom_id] = degen_comp

    def get_all_objs(self):
        for comp in self.degen_objs.values():
            for copy in comp.copies.values():
                for dg in copy:
                    yield dg

    def FindGeomsWithName(self, name):
        geom_ids = []
        for geom_id, geom in self.degen_objs.items():
            if geom.name == name:
                geom_ids.append(geom_id)
        return geom_ids

    def FindGeom(self, name, index):
        geoms = self.FindGeomsWithName(name)
        return geoms[index]
