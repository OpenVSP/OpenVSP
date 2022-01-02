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

import os
from charm.input_automation import *  # noqa
import numpy as np
import utilities.units as u
import matplotlib.pyplot as plt
import time
from collections import namedtuple
from enum import Enum
import pandas as pd
from typing import List
import utilities.simple_rotor_calcs as src
import fortranformat as ff

RotorTuple = namedtuple("RotorTuple", "shaft_thrust_total shaft_power_total shaft_h_force shaft_y_force \
                                       shaft_roll_mom shaft_pitch_mom shaft_yaw_mom \
                                       wind_thrust_total  wind_power_total  wind_x_force  wind_side_force \
                                       aircraft_roll_mom aircraft_pitch_mom aircraft_yaw_mom \
                                       collective tip_speed fom sigma \
                                       aircraft_x_force aircraft_y_force aircraft_z_force")


class ScanGridPlaneType(Enum):
    XY = 1
    XZ = 2
    YZ = 3


class CharmScanGridFrame:
    """
    Class to contain and plot results from a single scan grid
    """
    def __init__(self, x, y, z, u, v, w, cp, cp_prime, psis):
        self.x = x
        """
        x coordinates of grid points
        """

        self.y = y
        """
        y coordinates of grid points
        """

        self.z = z
        """
        z coordinates of grid points
        """

        self.u = u
        """
        x velocity component [ft/s]
        """

        self.v = v
        """
        y velocity component [ft/s]
        """

        self.w = w
        """
        z velocity component [ft/s]
        """

        self.cp = cp
        """
        pressure coefficient
        """

        self.cp_prime = cp_prime

        self.psis = psis
        """
        azimuth locations of each rotor
        """

        self.v_mag = np.sqrt(u**2.0 + v**2.0 + w**2.0)
        """
        velocity magnitude [ft/s]
        """

        # determine x/y plotting dimensions xy, xz, yz
        uniqx = np.unique(self.x)
        uniqy = np.unique(self.y)
        uniqz = np.unique(self.z)

        self._invert_x = False
        self._invert_y = False
        self._xlabel = "x"
        self._ylabel = "y"

        if len(uniqx) == 1:
            self.__plane_type = ScanGridPlaneType.YZ
            self._invert_y = True
            self._xlabel = "y"
            self._ylabel = "z"
        elif len(uniqy) == 1:
            self.__plane_type = ScanGridPlaneType.XZ
            self._invert_x = True
            self._invert_y = True
            self._ylabel = "z"
        else:
            self.__plane_type = ScanGridPlaneType.XY
            self._invert_x = True

    def filled_contour(self, z, ax=None, **kwargs):
        """
        Draws filled contour plot

        :param z: variable to contour
        :param ax: matplot lib axes object, if None one will be created
        :param kwargs: keyword arguments to pass to filled contour call
        :return: axes handle of plot
        """
        if ax is None:
            fig = plt.figure()
            ax = fig.gca()

        x, y, z_plot, u, v, w = self.__get_plot_coordinates()

        c = ax.contourf(x, y, z, **kwargs)
        self.__set_axes_properties(ax=ax)
        cbar = plt.colorbar(c)
        return ax, cbar

    def streamlines(self, ax=None, **kwargs):
        """
        Plot streamlines

        :param ax: optional handle to axis to use for streamline plot
        :param kwargs: keyword arguments to pass to streamline plot
        :return: axes handle of plot
        """

        if ax is None:
            fig = plt.figure()
            ax = fig.gca()

        x, y, z, u, v, w = self.__get_plot_coordinates()

        ax.streamplot(x, y, u, v, **kwargs)
        self.__set_axes_properties(ax=ax)

    def __get_plot_coordinates(self):
        if self.__plane_type == ScanGridPlaneType.XY:
            return self.x, self.y, self.z, self.u, self.v, self.w
        elif self.__plane_type == ScanGridPlaneType.XZ:
            return self.x, self.z, self.y, self.u, self.w, self.v
        elif self.__plane_type == ScanGridPlaneType.YZ:
            return self.y, self.z, self.x, self.v, self.w, self.u

    def __set_axes_properties(self, ax):
        if ax.xaxis_inverted() != self._invert_x:
            ax.invert_xaxis()
        if ax.yaxis_inverted() != self._invert_y:
            ax.invert_yaxis()

        ax.set_xlabel(self._xlabel)
        ax.set_ylabel(self._ylabel)
        ax.set_aspect('equal', adjustable='box')


class CharmScanGridResults:
    """
    Holds all scan grid results frames
    """
    def __init__(self, num_psi, num_grids, grids):
        """
        Initializes scan grid results object
        """

        self.num_psi = num_psi
        """
        Number of azimuthal points in the solution
        """

        self.num_grids = num_grids
        """
        Number of grids this solution contains
        """

        self.grids = grids
        """
        List of list of :class:`CharmScanGridFrame` objects.

        First index is the azimuthal index. Second index is the grid index.

        example: grids[iazimuth][igrid]
        """

        average_grids = []
        for igrid in range(num_grids):
            shape = grids[0][igrid].x.shape
            x = np.zeros(shape)
            y = np.zeros(shape)
            z = np.zeros(shape)
            u = np.zeros(shape)
            v = np.zeros(shape)
            w = np.zeros(shape)
            cp = np.zeros(shape)
            cp_prime = np.zeros(shape)
            for ipsi in range(num_psi):
                x += grids[ipsi][igrid].x
                y += grids[ipsi][igrid].y
                z += grids[ipsi][igrid].z
                u += grids[ipsi][igrid].u
                v += grids[ipsi][igrid].v
                w += grids[ipsi][igrid].w
                cp += grids[ipsi][igrid].cp
                cp_prime += grids[ipsi][igrid].cp_prime
            x /= num_psi
            y /= num_psi
            z /= num_psi
            u /= num_psi
            v /= num_psi
            w /= num_psi
            cp /= num_psi
            cp_prime /= num_psi
            average_grids.append(CharmScanGridFrame(x, y, z, u, v, w, cp, cp_prime, [0]))

        self.average_grids = average_grids
        """
        List of single :class:`CharmScanGridFrame` per grid where the data has been averaged over all of the
        azimuths.
        """


class RotorFrameResults:
    """
    Class to contain results for a single rotor in a particular frame of reference
    """
    def __init__(self, az, Fx, Fy, Fz, Mx, My, Mz):
        """
        Initializes rotor frame results
        :param az: azimuth
        :param Fx: x force
        :param Fy: y force
        :param Fz: z force
        :param Mx: moment about x axis
        :param My: moment about y axis
        :param Mz: moment about z axis
        """
        self.az = az
        self.Fx = Fx
        self.Fy = Fy
        self.Fz = Fz
        self.Mx = Mx
        self.My = My
        self.Mz = Mz


class CharmCQData:
    """
    Class to contain results from [name]cq.dat file
    """
    def __init__(self, ct, cqi, cqp, cqt, coll, cone, a1s, b1s):
        self.ct = ct
        self.cqi = cqi
        self.cqp = cqp
        self.cqt = cqt
        self.coll = coll
        self.cone = cone
        self.a1s = a1s
        self.b1s = b1s
        self.fom = src.simple_fom(ct, cqt)


class CharmPerfData:
    """
    Class that contains data from the [name]perf.dat file of a CHARM run
    """

    def __init__(self, filename):
        """
        Initializes a :class:`CharmPerfData` object from a [name]perf.dat file

        :param filename: full name of the [name]perf.dat file to read from
        """

        self.row_data = pd.DataFrame()
        """
        Pandas :class:`DataFrame` consisting of the raw data from the [name]perf.dat file.

        Additional columns of ``rotor` and ``revolution`` have been added to enable easy filtering
        """

        self.rotor_metadata = pd.DataFrame()
        """
        Pandas :class:`DataFrame` consisting of the metadata for each rotor

        Additional columns of ``rotor`` and ``revolution``
        """

        self._parse_perf_file(filename=filename)

    def line_plot(self, rotors, revolutions, variable, psis=None, ax=None, npsi=None, scaling=1.0):
        """
        Creates line plots versus r/R

        Some of the use cases for this function include the following:
            - Plot a distribution on a single rotor for multiple psi on a fixed revolution
            - Plot a distribution on a single rotor for a given psi for multiple revolutions
            - Plot a distributions of multiple rotors for a given psi and revolution

        :param rotors: single value or list of rotors to plot
        :param revolutions: single value or list of revolutions to plot
        :param variable: variable to plot
        :param psis: single value or list of psi's to plot
        :param ax: optional axes handle to plot on, if None a new figure and axis will be created
        :param npsi: optional single value or list of azimuthial calculation indices (useful for wings where psi is
        constant)
        :param scaling: optional scalar to convert non-dimensional plots to dimensional plots (i.e. dCQI/dx to dQI/dx)
        :return: axes handle of plot
        """

        # Convert all inputs to np arrays
        rotors = np.array([rotors])
        revolutions = np.array([revolutions])
        if psis is None and npsi is None:
            raise ValueError("Either psi or npsi must not be None")

        psi_col_name = 'psi'
        psi_label_name = 'Psi'
        if psis is None:
            psis = npsi
            psi_col_name = 'psi_ind'
            psi_label_name = 'NPSI'

        psis = np.array([psis])

        # Squeeze all extraneous dimensions
        if len(rotors.shape) > 1:
            rotors = rotors.squeeze(0)
        if len(revolutions.shape) > 1:
            revolutions = revolutions.squeeze(0)
        if len(psis.shape) > 1:
            psis = psis.squeeze(0)

        if ax is None:
            fig = plt.figure()
            ax = fig.gca()

        num_rotors = rotors.shape[0]
        num_rev = revolutions.shape[0]
        num_psi = psis.shape[0]

        # Convert negative revolutions to count backwards to allow easy plotting of last revolution
        revolutions[revolutions < 0] += self.row_data['revolution'].max() + 1

        for rotor in rotors:
            for revolution in revolutions:
                for psi in psis:
                    label = []
                    if num_rotors > 1:
                        label.append("Rotor = {}".format(rotor))
                    if num_rev > 1:
                        label.append("Rev. = {}".format(revolution))
                    if num_psi > 1:
                        label.append("{} = {} [deg]".format(psi_label_name, psi))
                    label = "\n".join(label)
                    if not label:
                        label = None
                    case_df = self.row_data.loc[(self.row_data['rotor'] == rotor) &
                                                (self.row_data['revolution'] == revolution) &
                                                (self.row_data[psi_col_name] == psi)]
                    case_df = case_df.sort_values('x=r/R')
                    ax.plot(case_df['x=r/R'], case_df[variable] * scaling, label=label)
        ax.set_xlabel('r/R')
        ax.set_ylabel(variable)
        return ax

    def polar_plot(self, rotor, revolution, variable, ax=None, scaling=1.0, **kwargs):
        """
        Creates a filled contour polar plot of the given variable on the specified rotor
        and revolution

        :param rotor: rotor on which to create a polar plot
        :param revolution: specify which blade revolution to use (if negative, count from end of list.
            For example, -1 would be the last revolution)
        :param variable: variable to contour
        :param ax: optional axes handle on which to plot, if None a new figure and axes will be created
        :param scaling: optional scalar to convert non-dimensional plots to dimensional plots (i.e. dCQI/dx to dQI/dx)
        :param kwargs: keyword arguments that will be passed to the contourf function
        :return: axes handle of the plot, handle to color bar
        """

        if revolution < 0:
            revolution = self.row_data['revolution'].max() + revolution + 1

        case_df = self.row_data.loc[(self.row_data['rotor'] == rotor)
                                    & (self.row_data['revolution'] == revolution)]
        case_df = case_df.pivot('x=r/R', 'psi', variable)
        psis = case_df.columns.values
        radi = case_df.index.values
        z = case_df.values * scaling

        psi, radi = np.meshgrid(psis, radi)

        # append first psi to end in order to connect the full circle
        dpsi = psi[0, 1] - psi[0, 0]
        psi = np.c_[psi, np.array([psi[0, -1]+dpsi]*psi.shape[0])]
        radi = np.c_[radi, radi[:, 0]]
        z = np.c_[z, z[:, 0]]

        if ax is None:
            fig, ax = plt.subplots(subplot_kw=dict(projection='polar'))

        c = ax.contourf(psi*u.deg2rad, radi, z, **kwargs)
        ax.set_ylim(bottom=0)
        cbar = plt.colorbar(c)
        cbar.ax.set_ylabel(variable)
        return ax, cbar

    def _parse_perf_file(self, filename):
        """
        Parses a [name]perf.dat file from a CHARM run

        :param filename: fullname of the [name]perf.dat file to parse
        :return:
        """

        # Open up the file
        rows = []
        rotor_num = []
        revolution_num = []
        variable_names = []
        meta_data_rows = []
        meta_data_variablenames = []
        psi_ind = []
        with open(filename, "r") as f:
            # Read in the number of rotors, and atmosphere properties
            line = f.readline()  # variable names
            line = f.readline()  # variable values
            data = line.split()
            num_rotors = int(data[0])
            rho = float(data[1])
            speed_of_sound = float(data[2])

            revolution = 0

            # arrays for blade level properties
            line = f.readline()  # variable names line
            while line:
                revolution += 1
                meta_data_variablenames = ['revolution'] + line.split()
                meta_data_variablenames[1] = 'rotor'
                for irotor in range(num_rotors):
                    line = f.readline()  # variable values line
                    reader = ff.FortranRecordReader("(I9, 2I5, F11.2, F10.2, E10.4, F12.4, F12.3, 2E11.3)")
                    data = reader.read(line)
                    charm_rotor_num = int(data[0])
                    num_psi = int(data[1])
                    num_radial_locs = int(data[2])

                    meta_data_row = [revolution, charm_rotor_num, num_psi, num_radial_locs] + list(map(float, data[3:]))
                    meta_data_rows.append(meta_data_row)
                    # Read in radial data
                    line = f.readline()  # variable names
                    variable_names = line.split()
                    skip = False
                    for ipsi in range(num_psi):
                        if skip:
                            break
                        for iradial in range(num_radial_locs):
                            cur_pos = f.tell()
                            line = f.readline()  # variable values
                            try:
                                row_data = [float(d) for d in line.split()]
                            except:
                                f.seek(cur_pos)
                                skip = True

                            if skip:
                                break
                            rotor_num.append(charm_rotor_num)
                            revolution_num.append(revolution)
                            psi_ind.append(ipsi)
                            rows.append(row_data)
                    line = f.readline()

        # Create a pandas data frame dictionary
        df_dict = {'rotor': rotor_num, 'revolution': revolution_num, 'psi_ind': psi_ind}
        rows = np.array(rows)
        for ivar, var_name in enumerate(variable_names):
            df_dict[var_name] = rows[:, ivar]

        self.row_data = pd.DataFrame(df_dict)

        # Create metadata pandas data frame
        meta_df = {}
        for ivar, var_name in enumerate(meta_data_variablenames):
            meta_df[var_name] = np.array([row[ivar] for row in meta_data_rows])
        self.rotor_metadata = pd.DataFrame(meta_df)


class CharmRotorResults:
    """
    Class to contain results for a rotor from charm and provide plotting helpers
    """

    def __init__(self, aircraft_frame: RotorFrameResults, rotor_frame: RotorFrameResults,
                 shaft_frame: RotorFrameResults, omega: float, rotor_log_results: List[RotorTuple],
                 cq_data: CharmCQData):
        """
        Initializes rotor results

        :param aircraft_frame: results data in aircraft frame
        :param rotor_frame: results data in rotor frame
        :param shaft_frame: results data in shaft frame
        :param omega: rotational rate (rads/s)
        :param cq_data: iteration history for this rotor from [name]cq.dat charm file
        """

        self.aircraft_frame = aircraft_frame
        """
        Forces/moments in the aircraft frame parsed from the [name]hubacr.dat file

            .. warning:: Currently not supported
        """

        self.rotor_frame = rotor_frame
        """
        Forces/moments in the rotor frame parsed from the [name]hubrot.dat file

            .. warning:: Currently not supported
        """

        self.shaft_frame = shaft_frame
        """
        Forces/moments in the shaft frame parsed from the [name]hub.dat file
        """

        self.omega = omega
        """
        Rotational rate of the rotor [rads/s]

        .. warning:: Parsed from the [name]perf.dat file. Not tested for cases where RPM is used to achieve desired
                     thrust
        """

        self.cq_data = cq_data
        """
        Data parsed from [name]cq.dat file
        """

        self.thrusts = np.array([result.shaft_thrust_total for result in rotor_log_results])
        """
        Total thrust in shaft frame [lb] parsed from the log file.

        First and last value in the log file for this rotor.
        """

        self.powers = np.array([result.shaft_power_total for result in rotor_log_results])
        """
        Total power in shaft frame [hp] parsed from the log file.

        First and last value in the log file for this rotor.
        """

        self.hforces = np.array([result.shaft_h_force for result in rotor_log_results])
        """
        Total H-force in shaft frame [lbs] parsed from the log file.

        First and last value in the log file for this rotor.
        """

        self.yforces = np.array([result.shaft_y_force for result in rotor_log_results])
        """
        Total Y-Force in shaft frame [lbs] parsed from the log file.

        First and last value in the log file for this rotor.
        """

        self.roll_mom_shaft = np.array([result.shaft_roll_mom for result in rotor_log_results])
        """
        Total Roll Moment in shaft frame [ft-lbs] parsed from the log file.

        First and last value in the log file for this rotor.
        """

        self.pitch_mom_shaft = np.array([result.shaft_pitch_mom for result in rotor_log_results])
        """
        Total Pitch Moment in shaft frame [ft-lbs] parsed from the log file.

        First and last value in the log file for this rotor.
        """

        self.yaw_mom_shaft = np.array([result.shaft_yaw_mom for result in rotor_log_results])
        """
        Total Yaw Moment in shaft frame [ft-lbs] parsed from the log file.

        First and last value in the log file for this rotor.
        """

        self.lifts_wind = np.array([result.wind_thrust_total for result in rotor_log_results])
        """
        Total Lift in wind frame [lbs] parsed from the log file.

        First and last value in the log file for this rotor.
        """

        self.powers_wind = np.array([result.wind_power_total for result in rotor_log_results])
        """
        Total Power in wind frame [hp] parsed from the log file.

        First and last value in the log file for this rotor.
        """

        self.xforces_wind = np.array([result.wind_x_force for result in rotor_log_results])
        """
        Total X-force in wind frame [lbs] parsed from the log file.

        First and last value in the log file for this rotor.
        """

        self.side_forces_wind = np.array([result.wind_side_force for result in rotor_log_results])
        """
        Total Side force in wind frame [lbs] parsed from the log file.

        First and last value in the log file for this rotor.
        """

        self.fx_aircraft = np.array([result.aircraft_x_force*-1.0 for result in rotor_log_results])
        """
        Total X-force in aircraft frame [lbs] parsed from the log file. Note the sign on this value is changed
        from the log file. Positive value aligned with positive x-body axis (forward) of aircraft
        
        First and last value in the log file for this rotor.
        """

        self.fy_aircraft = np.array([result.aircraft_y_force for result in rotor_log_results])
        """
        Total Y-force in aircraft frame [lbs] parsed from the log file.
        
        First and last value in the log file for this rotor.
        """

        self.fz_aircraft = np.array([result.aircraft_z_force*-1.0 for result in rotor_log_results])
        """
        Total Z-force in aircraft frame [lbs] parsed from the log file. Note the sign on this value is changed from log
        file. Positive value aligned with positive z-body axis (down) of the aircraft
        
        First and last value in the log file for this rotor.
        """

        self.roll_mom_aircraft = np.array([result.aircraft_roll_mom for result in rotor_log_results])
        """
        Total Roll Moment in wind frame [ft-lbs] parsed from the log file.

        First and last value in the log file for this rotor.
        """

        self.pitch_mom_aircraft = np.array([result.aircraft_pitch_mom for result in rotor_log_results])
        """
        Total Pitch Moment in wind frame [ft-lbs] parsed from the log file.

        First and last value in the log file for this rotor.
        """

        self.yaw_mom_aircraft = np.array([result.aircraft_yaw_mom for result in rotor_log_results])
        """
        Total Yaw Moment in wind frame [ft-lbs] parsed from the log file.

        First and last value in the log file for this rotor.
        """

        self.collectives = np.array([result.collective for result in rotor_log_results])
        """
        Collective [deg]

        First and last value in the log file for this rotor.
        """

        self.tip_speeds = np.array([result.tip_speed for result in rotor_log_results])
        """
        Tipspeed [ft/s] reported in the log file

        First and last value in the log file for this rotor.
        """

        self.solidity = rotor_log_results[0].sigma
        """
        Rotor solidity reported in the log file
        """

        self.foms = np.array([result.fom for result in rotor_log_results])
        """
        Figure out merits reported in the log file
        """


class CharmResults:
    """
    Class to hold the results of a charm run
    """
    def __init__(self, rotor_results: List[CharmRotorResults], log_file, scan_grid: CharmScanGridResults,
                 aircraft_velocity, perf_data: CharmPerfData):
        """
        Initializes a charm results object

        :param rotor_results: list of rotor results
        :param log_file: log file from run
        :param scan_grid: scan grid results object containing data for each grid and azimuth
        :param aircraft_velocity: u, v, w velocity in ft/s of aircraft
        """
        self.rotor_results = rotor_results
        """
        List of :class:`CharmRotorResults` objects, one for each rotor in the solution
        """

        self.log_file = log_file
        """
        String contents of this run's log file
        """

        self.scan_grid = scan_grid
        """
        :class:`CharmScanGridResults` object, containing velocity and pressure information on each scan grid
        """

        self.aircraft_velocity = aircraft_velocity
        """
        u, v, w velocity of the vehicle in ft/s.
        """

        self.alpha = 0.0
        """
        Angle of attack [deg]
        """

        self.perf_data = perf_data
        """
        :class:`CharmPerfData` object, which contains performance data out the radius of each blade,
        for each revolution
        """

        v_inf = np.linalg.norm(aircraft_velocity)
        if v_inf > 0.0:
            self.alpha = math.atan2(aircraft_velocity[2], aircraft_velocity[0]) * u.rad2deg

        cq_data = pd.DataFrame()
        for ir, res in enumerate(rotor_results):
            df_new = pd.DataFrame(vars(res.cq_data))
            df_new['rotor'] = ir+1
            df_new['rev'] = range(len(res.cq_data.a1s))
            cq_data = cq_data.append(df_new, ignore_index=True)

        cq_data = cq_data.pivot('rev', 'rotor')

        self.cq_data = cq_data
        """
        :class:`pd.DataFrame` object, which contains the data from the cq.dat file for all rotors in a single dataframe
        """


def parse_charm_run(charm_dir, case_name, ignore_perfdat_errors=True):
    """
    Parses a charm run

    :param charm_dir: directory where charm was run
    :param case_name: name of the charm case
    :param ignore_perfdat_errors: if True, exceptions raised while parsing [name]perf.dat file will be ignored
    :return: :class:`CharmResults` object
    """
    cwd = os.getcwd()
    try:
        os.chdir(charm_dir)

        # Parse the run characteristics file
        run_char_params = read_run_characteristics_template(template_filename=case_name + ".inp")

        num_rotors = abs(run_char_params['NROTOR'])

        # Parse the shaft frame values
        shaft_frames = _parse_shaft_frame_forces(case_name+"hub.dat")

        # If there are not the same number of shaft frame results as num_rotors,
        # append None to the list
        if len(shaft_frames) < num_rotors:
            shaft_frames += [None]*(num_rotors-len(shaft_frames))

        # Parse perf file
        perf_data = None
        omegas = np.ones(num_rotors)*np.nan
        try:
            perf_data = CharmPerfData(filename=case_name+"perf.dat")
            omegas = perf_data.rotor_metadata.pivot("rotor", "revolution", "OMEGA").values[:, -1]
        except Exception as e:
            if not ignore_perfdat_errors:
                raise e

        # parse convergence history file
        cq_results = __parse_cq_file(case_name + "cq.dat", num_rotors=num_rotors)

        # read log
        log_contents = []
        with open(case_name + ".log") as f:
            log_contents = f.read()

        rotor_log_results = __parse_log_file(case_name + ".log")

        rotor_results = []
        for i in range(num_rotors):
            rotor_results.append(CharmRotorResults(aircraft_frame=None, rotor_frame=None,
                                                   shaft_frame=shaft_frames[i], omega=omegas[i],
                                                   rotor_log_results=rotor_log_results[i],
                                                   cq_data=cq_results[i]))

        # Read scan grid
        scan_grid = None
        aircraft_velocity = np.array([run_char_params['U'], run_char_params['V'], run_char_params['W']])
        freestream = -1.0*aircraft_velocity
        try:
            scan_grid = parse_scan_grid_output(case_name + ".off", freestream_velocity=freestream,
                                               num_rotors=num_rotors)
        except FileNotFoundError:
            pass

        results = CharmResults(rotor_results=rotor_results, log_file=log_contents, scan_grid=scan_grid,
                               aircraft_velocity=aircraft_velocity, perf_data=perf_data)
        return results
    finally:
        os.chdir(cwd)


def parse_scan_grid_output(filename, freestream_velocity, num_rotors):
    """
    Parses a scan grid output file
    :param filename: name of the scan grid output file
    :param freestream_velocity: freestream that is added to the scan grid velocities
    :param num_rotors: number of rotors in the scan grid output
    :return: scan grid results object
    """
    with open(filename, "r") as f:
        line = f.readline()
        num_grids = int(line.split()[0])
        dim1s = np.empty(num_grids, dtype=int)
        dim2s = np.empty(num_grids, dtype=int)

        dim1_remaining = num_grids
        dim2_remaining = num_grids

        while dim1_remaining > 0 or dim2_remaining > 0:
            line = f.readline()
            dims = np.array([int(d) for d in line.split()])
            dims_available = len(dims)
            current_dim_start = 0

            if dim1_remaining > 0:
                dim1_start_ind = num_grids-dim1_remaining
                dim1_end_ind = dim1_start_ind + dim1_remaining if dims_available >= dim1_remaining else dim1_start_ind + dims_available
                dim1s[dim1_start_ind:dim1_end_ind] = dims[current_dim_start:(dim1_end_ind-dim1_start_ind)]
                dim1_remaining -= (dim1_end_ind-dim1_start_ind)
                current_dim_start += (dim1_end_ind-dim1_start_ind)
                dims_available -= (dim1_end_ind-dim1_start_ind)

            if dim2_remaining > 0:
                dim2_start_ind = num_grids-dim2_remaining
                dim2_end_ind = dim2_start_ind + dim2_remaining if dims_available >= dim2_remaining else dim2_start_ind + dims_available
                dim2s[dim2_start_ind:dim2_end_ind] = dims[current_dim_start:current_dim_start+(dim2_end_ind-dim2_start_ind)]
                dim2_remaining -= (dim2_end_ind-dim2_start_ind)

        line = f.readline()
        num_psi = int(line.split()[0])

        psi_grids = []
        for ipsi in range(num_psi):
            psis = np.empty(num_rotors, dtype=float)
            psi_remaining = num_rotors
            while psi_remaining > 0:
                line = f.readline()
                line_data = [float(d) for d in line.split()]
                for psi in line_data:
                    psis[num_rotors-psi_remaining] = psi
                    psi_remaining -= 1
            frames = []
            for igrid in range(num_grids):
                x = np.empty((dim1s[igrid], dim2s[igrid]))
                y = np.empty((dim1s[igrid], dim2s[igrid]))
                z = np.empty((dim1s[igrid], dim2s[igrid]))
                u = np.empty((dim1s[igrid], dim2s[igrid]))
                v = np.empty((dim1s[igrid], dim2s[igrid]))
                w = np.empty((dim1s[igrid], dim2s[igrid]))
                cp = np.empty((dim1s[igrid], dim2s[igrid]))
                cp_prime = np.empty((dim1s[igrid], dim2s[igrid]))
                for i in range(dim1s[igrid]):
                    for j in range(dim2s[igrid]):
                        line = f.readline()
                        data = [float(d) for d in line.split()]
                        x[i, j] = data[0]
                        y[i, j] = data[1]
                        z[i, j] = data[2]
                        u[i, j] = data[3] + freestream_velocity[0]
                        v[i, j] = data[4] + freestream_velocity[1]
                        w[i, j] = data[5] + freestream_velocity[2]
                        cp[i, j] = data[6]
                        cp_prime[i, j] = data[7]
                grid = CharmScanGridFrame(x, y, z, u, v, w, cp, cp_prime, psis)
                frames.append(grid)

            psi_grids.append(frames)

        return CharmScanGridResults(num_psi=num_psi, num_grids=num_grids, grids=psi_grids)


def run_charm(files_to_write, case_name, run=True, print_log_stream=False, run_cmd="runv6", timeout=None, **kwargs):
    """
    Runs a charm case. This function is blocking.

    In order for this function to work, ``run_cmd`` must be on the system path.

    :param files_to_write: dictionary of filenames and contents to write
    :param case_name: name to use for the charm case
    :param run: if False only files will be created, charm will not be run
    :param print_log_stream: if True, the contents of the log file will be read and printed while CHARM is running
    :param run_cmd: command to run charm, this defaults to ``runv6``
    :param timeout: timeout in seconds. If the process is still running after timeout seconds, it will be killed.
        If none, no timeout will be applied.
    :param kwargs: keyword args passed to :class:`utilities.files.RunManager`
    :return: CharmResults object
    """
    import subprocess
    import signal
    from utilities.files import RunManager
    with RunManager(**kwargs) as rd:
        for filename, file_contents in files_to_write.items():
            with open(filename, "w") as f:
                f.write(file_contents)

        if run:
            # Start CHARM
            p = subprocess.Popen([run_cmd, ".", case_name], start_new_session=True)
            start_time = time.time()
            pgrp = os.getpgid(p.pid)

            if print_log_stream:
                # Try opening log file, max attempts of 10 times
                attempts = 0
                max_attempts = 10
                while attempts < max_attempts:
                    try:
                        with open(os.path.join(case_name + "dir", case_name + ".log"), "r") as f:
                            line = "a"
                            while p.poll() is None or line:
                                location = f.tell()
                                line = f.readline()
                                if not line:
                                    time.sleep(0.1)
                                    f.seek(location)
                                else:
                                    print(line.rstrip())

                                if timeout is not None and (time.time() - start_time) > timeout:
                                    p.kill()
                                    raise TimeoutError()

                    except FileNotFoundError:
                        attempts += 1
                        time.sleep(0.5)
            else:
                try:
                    p.wait(timeout=timeout)
                except subprocess.TimeoutExpired:
                    print("attempting to kill group {}".format(pgrp))
                    os.killpg(pgrp, signal.SIGTERM)

            if p.poll() is None:
                try:
                    p.wait(timeout=timeout)
                except subprocess.TimeoutExpired:
                    print("attempting to kill group {}".format(pgrp))
                    os.killpg(pgrp, signal.SIGTERM)

            # Process output
            results = parse_charm_run(".", case_name=case_name)
            return results
        return None


def _parse_shaft_frame_forces(filename):
    """
    Parses the shaft frame forces file
    :param filename: name of the shaft frame forces file
    :return: frame results for each rotor
    """

    shaft_frames = []
    with open(filename, "r") as f:
        frame_data = None
        for line_num, line in enumerate(f):
            if 'Rotor' in line:
                if frame_data is not None:
                    shaft_frames.append(__create_shaft_frame_from_array(frame_data))
                continue
            if 'Hub_Force' in line:
                frame_data = []
                continue
            data = [float(d) for d in line.split()]
            frame_data.append(data)
        if frame_data is not None:
            shaft_frames.append(__create_shaft_frame_from_array(frame_data))
    return shaft_frames


def __create_shaft_frame_from_array(frame_data):
    np_data = np.array(frame_data)
    frame = RotorFrameResults(np_data[:, 0],
                              np_data[:, 1], np_data[:, 2], np_data[:, 3],
                              np_data[:, 4], np_data[:, 5], np_data[:, 6])
    return frame


def __parse_omega(rw_filename):
    """
    Parses omega from rotor wake file
    :param rw_filename: name of rotor wake file
    :return: omega (rad/s)
    """
    expr = re.compile(r"\s+OMEGA(\s|$)")
    omega = 0.0
    with open(rw_filename, "r") as f:
        parse_omega = False
        for line in f:
            if parse_omega:
                data = line.split()
                return float(data[1])
            if re.search(expr, line):
                parse_omega = True

    return omega


def __parse_log_file(log_filename):
    """
    Parses integrated rotor quantities from log file
    :param log_filename: name of the log file
    :return: history or quantities for each rotor
    """

    with open(log_filename, "r") as f:
        # Count the number of rotors
        rotor_expr = re.compile(r"(^|\s)Rotor:\s+([0-9]+)")
        first_rotor = __find_line(f, rotor_expr)
        num_rotors = 1
        while True:
            # Skip 7 lines
            line = ""
            current_position = f.tell()
            for i in range(7):
                line = f.readline()
            if re.search(rotor_expr, line):
                num_rotors += 1
            else:
                f.seek(current_position)
                break

        tip_speed_expr = re.compile(r"(^|\s)Tip speed \(OMEGAR\):\s+(.*)\s")

        shaft_axes_expr = re.compile(r"(^|\s)SHAFT AXES:")
        shaft_thrust_expr = re.compile(r"(^|\s)Thrust \(\+up\)\s+(\S*)\s+(\S*)")
        shaft_h_force_expr = re.compile(r"(^|\s)H-force \(\+back\)\s+(\S*)\s+(\S*)")
        shaft_y_force_expr = re.compile(r"(^|\s)Y-force \(\+adv side\)\s+(\S*)\s+(\S*)")
        shaft_power_expr = re.compile(r"(^|\s)Shaft Power\s+(\S*)\s+(\S*)")
        shaft_roll_mom_expr = re.compile(r"(^|\s)Roll moment  \(about \+x\)\s+(\S*)\s+(\S*)")
        shaft_pitch_mom_expr = re.compile(r"(^|\s)Pitch moment \(about \+y\)\s+(\S*)\s+(\S*)")
        shaft_yaw_mom_expr = re.compile(r"(^|\s)Yaw moment   \(about \+z\)\s+(\S*)\s+(\S*)")
        shaft_angle_expr = re.compile(r"(^|\s+)Shaft angle (ALPHAS):")

        wind_axes_expr = re.compile(r"(^|\s)WIND AXES:")
        wind_thrust_expr = re.compile(r"(^|\s)Lift \(\+up\)\s+(\S*)\s+(\S*)")
        wind_x_force_expr = re.compile(r"(^|\s)X-force \(\+back\)\s+(\S*)\s+(\S*)")
        wind_side_force_expr = re.compile(r"(^|\s)Side force \(\+adv side\)\s+(\S*)\s+(\S*)")
        wind_power_expr = re.compile(r"(^|\s)Total Power \(energy balance\)\s+(\S*)\s+(\S*)")

        aircraft_loads_expr = re.compile(r"(^|\s)Hub loads \(aircraft frame\)")
        aircraft_x_force_expr = re.compile(r"(^|\s)Rearward force \(-x-dir\)\s+(\S*)\s+(\S*)")
        aircraft_y_force_expr = re.compile(r"(^|\s)Sideward force \(\+y-dir\)\s+(\S*)\s+(\S*)")
        aircraft_z_force_expr = re.compile(r"(^|\s)Vertical force \(-z-dir\)\s+(\S*)\s+(\S*)")
        aircraft_roll_mom_expr = re.compile(r"(^|\s)Roll moment  \(about \+x\)\s+(\S*)\s+(\S*)")
        aircraft_pitch_mom_expr = re.compile(r"(^|\s)Pitch moment \(about \+y\)\s+(\S*)\s+(\S*)")
        aircraft_yaw_mom_expr = re.compile(r"(^|\s)Yaw moment   \(about \+z\)\s+(\S*)\s+(\S*)")

        collective_expr = re.compile(r"(^|\s)Collective Pitch:\s+(\S*)")
        fom_expr = re.compile(r"(^|\s)Figure of Merit \(FM\)\.+\s+(\S+)")
        sigma_expr = re.compile(r"(^|\s)Solidity\s+=\s+\S+\s+(\S+)")

        rotor_results = []
        for irotor in range(num_rotors):
            rotor_results.append([])

        line = "a"
        while line:
            # Find Integrated Performance for each rotor
            for irotor in range(num_rotors):
                integrated_perf_expr = re.compile(r"(^|\s)INTEGRATED PERFORMANCE THIS REVOLUTION - ROTOR ([0-9]+)")
                line = __find_line(f, integrated_perf_expr)

                if not line:
                    break

                # Parse individual quantities

                line = __find_line(f, tip_speed_expr, integrated_perf_expr)
                tip_speed = float(re.search(tip_speed_expr, line).groups()[1].split()[0])

                # Shaft axes
                line = __find_line(f, shaft_axes_expr, integrated_perf_expr)
                line = __find_line(f, shaft_thrust_expr, integrated_perf_expr)
                shaft_thrust = float(re.search(shaft_thrust_expr, line).groups()[1])

                line = __find_line(f, shaft_h_force_expr, integrated_perf_expr)
                shaft_h_force = float(re.search(shaft_h_force_expr, line).groups()[1])

                line = __find_line(f, shaft_y_force_expr, integrated_perf_expr)
                shaft_y_force = float(re.search(shaft_y_force_expr, line).groups()[1])

                line = __find_line(f, shaft_power_expr, integrated_perf_expr)
                shaft_power = float(re.search(shaft_power_expr, line).groups()[1])

                line = __find_line(f, shaft_roll_mom_expr, integrated_perf_expr)
                shaft_roll_mom = float(re.search(shaft_roll_mom_expr, line).groups()[1])

                line = __find_line(f, shaft_pitch_mom_expr, integrated_perf_expr)
                shaft_pitch_mom = float(re.search(shaft_pitch_mom_expr, line).groups()[1])

                line = __find_line(f, shaft_yaw_mom_expr, integrated_perf_expr)
                shaft_yaw_mom = float(re.search(shaft_yaw_mom_expr, line).groups()[1])

                # Wind Axes
                line = __find_line(f, wind_axes_expr, integrated_perf_expr)
                line = __find_line(f, wind_thrust_expr, integrated_perf_expr)
                wind_thrust = float(re.search(wind_thrust_expr, line).groups()[1])

                line = __find_line(f, wind_x_force_expr, integrated_perf_expr)
                wind_x_force = float(re.search(wind_x_force_expr, line).groups()[1])

                line = __find_line(f, wind_side_force_expr, integrated_perf_expr)
                wind_side_force = float(re.search(wind_side_force_expr, line).groups()[1])

                line = __find_line(f, wind_power_expr, integrated_perf_expr)
                wind_power = float(re.search(wind_power_expr, line).groups()[1])

                line = __find_line(f, aircraft_loads_expr, integrated_perf_expr)
                if line != '':
                    line = __find_line(f, aircraft_x_force_expr, integrated_perf_expr)
                    aircraft_x_force = float(re.search(aircraft_x_force_expr, line).groups()[1])

                    line = __find_line(f, aircraft_y_force_expr, integrated_perf_expr)
                    aircraft_y_force = float(re.search(aircraft_y_force_expr, line).groups()[1])

                    line = __find_line(f, aircraft_z_force_expr, integrated_perf_expr)
                    aircraft_z_force = float(re.search(aircraft_z_force_expr, line).groups()[1])

                    line = __find_line(f, aircraft_roll_mom_expr, integrated_perf_expr)
                    aircraft_roll_mom = float(re.search(aircraft_roll_mom_expr, line).groups()[1])

                    line = __find_line(f, aircraft_pitch_mom_expr, integrated_perf_expr)
                    aircraft_pitch_mom = float(re.search(aircraft_pitch_mom_expr, line).groups()[1])

                    line = __find_line(f, aircraft_yaw_mom_expr, integrated_perf_expr)
                    aircraft_yaw_mom = float(re.search(aircraft_yaw_mom_expr, line).groups()[1])

                line = __find_line(f, sigma_expr, integrated_perf_expr)
                sigma = float(re.search(sigma_expr, line).groups()[1])

                line = __find_line(f, fom_expr, integrated_perf_expr)
                if line == '':
                    fom = 0.0
                else:
                    fom = float(re.search(fom_expr, line).groups()[1])

                line = __find_line(f, collective_expr, integrated_perf_expr)
                collective = float(re.search(collective_expr, line).groups()[1])

                rotor_results[irotor].append(
                    RotorTuple(
                        shaft_thrust, shaft_power, shaft_h_force, shaft_y_force, shaft_roll_mom,
                        shaft_pitch_mom, shaft_yaw_mom, wind_thrust, wind_power, wind_x_force, wind_side_force,
                        aircraft_roll_mom, aircraft_pitch_mom, aircraft_yaw_mom, collective, tip_speed, fom,
                        sigma,
                        aircraft_x_force, aircraft_y_force, aircraft_z_force,
                    )
                )

        return rotor_results


def __parse_cq_file(filename, num_rotors):
    """
    Parses the [name]cq.dat file which contains coefficients for each rotor as a history of iteration
    :param filename: [name]cq.dat file name
    :param num_rotors: number of rotors
    :return: convergence history for each rotor
    """
    data = np.genfromtxt(filename)
    rotor_data = []
    for irotor in range(num_rotors):
        rotor_data.append(CharmCQData(*data[irotor::num_rotors, :8].T))
    return rotor_data


def __find_line(file: io.IOBase, expr, stop_expr=None):
    """
    Moves file forward until expression is matched
    :param file: file object
    :param expr: regex
    :param stop_expr: optional expression used to stop search. If this expression is found, then fail to find line
    :return: line
    """
    line = "a"
    current_position = file.tell()
    while line:
        line = file.readline()
        if re.search(expr, line):
            return line
        if stop_expr is not None and re.search(stop_expr, line):
            break
    file.seek(current_position)
    return ''
