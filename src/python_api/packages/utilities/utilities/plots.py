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


import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
import os
from matplotlib.backends.backend_pdf import PdfPages

def next_plot_color(ax=None):
    '''
    Gets the next color in the color cycler for the axis
    :param ax: axis in question
    :return: color rgb
    '''
    if ax is None:
        ax = plt.gca()
    return next(ax._get_lines.prop_cycler)['color']

def reset_color_cycler(ax=None):
    '''
    Resets the axis's color cycler to it's default state
    :param ax: axis in question
    :return: None
    '''
    if ax is None:
        ax = plt.gca()
    ax.set_prop_cycle(None)

def genBarChart(y, width=0.5, xlabels=None, legend=None, ygrid=False, ax=None, **kwargs):
    """

    :param y: y data to plot. Can either be a single array (eg [0,1,2,3]) or multiple variables (eg [[0,1,2,3],[4,5,6,7]] )
    :param width: total width of each bar group.
    This will be the bar width if the number of x variables is 1,
    or the total width of the group of bars if the number of x variables is greater than 1.
    :param xlabels: optional, labels for the x axis bars (eg ["Opt1","Opt2","Opt3","Opt4"])
    :param legend: optional, labels for the legend (eg ["Var1"] for single x variable or ["Var1","Var2"] for 2 x variables)
    :param ygrid: optional, boolean, turns ygrid on or off
    :param ax: optional pyplot axes handle to plot on
    :param kwargs: keyword arguments to be passed to bar plotting function
    :return: pyplot axes handle
    """

    y_data = np.array(y)
    if ax is None:
        fig = plt.figure()
        ax = fig.gca()
    if len(y_data.shape) == 1:
        spacing = np.arange(len(y_data))
        # assume y_data is 1-d data
        r = ax.bar(spacing, y_data, width=width, zorder=3, **kwargs)
        rectHandles = [r[0],]
    else:
        spacing = np.arange(len(y_data[0, :]))

        nVars = len(y_data)
        widthPerBar = width/nVars
        rectHandles = []
        for i in range(0,nVars):
            label=None
            if legend is not None:
                label = legend[i]
            r = ax.bar(spacing + i * widthPerBar - (nVars-1) * widthPerBar / 2, y_data[i, :], width=widthPerBar, zorder=3,
                       label=label, **kwargs)
            rectHandles.append(r[0])
    if xlabels is not None:
        ax.set_xticks(spacing)
        ax.set_xticklabels(xlabels)
    # if legend is not None:
    #     plt.legend(rectHandles,legend)

    if ygrid:
        ax.yaxis.grid(True, zorder=0)
    return ax

def genHBarChart( data, rows=None, lefts=None, cmaps=None, ax=None):
    '''
    Plots a horizontal stacked bar chart

    :param data: an iterable of arrays to plot (eg, ( (0,2,5), (1,2) )
    :param rows: an iterable of the row spacing (y-axis) (optional, default is every 1)
    :param lefts: an iterable of the starting point for each bar (optional, default is 0)
    :param ax: axis to plot on (optional)
    :return: axis
    '''
    if rows is None:
        rows = np.arange(len(data))

    if lefts is None:
        lefts = np.zeros( len(rows) )

    if ax is None:
        fig, ax = plt.subplots()

    for i,r in enumerate(rows):
        left = lefts[i]
        for d in data[i]:
            ax.barh(r, d, align='center', left=left)
            left += d

    return ax

def registerColorMaps():
    """

    Loads custom color maps.
    """

    cdict1 = {'red': ((0.0, 0.0, 0.0),
               (1 / 7., 0.0, 0.0),
               (2 / 7., 0.0, 0.0),
               (3 / 7., 0.0, 0.0),
               (4 / 7., 1.0, 1.0),
               (5 / 7., 1.0, 1.0),
               (6 / 7., 1.0, 1.0),
               (1.0, 1.0, 1.0)),
       'green': ((0.0, 0.0, 0.0),
                (1 / 7., 0.0, 0.0),
                (2 / 7., 1.0, 1.0),
                (3 / 7., 1.0, 1.0),
                (4 / 7., 1.0, 1.0),
                (5 / 7., 0.0, 0.0),
                (6 / 7., 0.0, 0.0),
                (1.0, 1.0, 1.0)),
       'blue': ((0.0, 0.0, 0.0),
               (1 / 7., 1.0, 1.0),
               (2 / 7., 1.0, 1.0),
               (3 / 7., 0.0, 0.0),
               (4 / 7., 0.0, 0.0),
               (5 / 7., 0.0, 0.0),
               (6 / 7., 1.0, 1.0),
               (1.0, 1.0, 1.0)),
      }

    cdict2 = {'red': ((0.0, 1.0, 1.0),
              (1 / 7., 1.0, 1.0),
              (2 / 7., 1.0, 1.0),
              (3 / 7., 1.0, 1.0),
              (4 / 7., 0.0, 0.0),
              (5 / 7., 0.0, 0.0),
              (6 / 7., 0.0, 0.0),
              (1.0, 0.0, 0.0)),
       'green': ((0.0, 1.0, 1.0),
                (1 / 7., 0.0, 0.0),
                (2 / 7., 0.0, 0.0),
                (3 / 7., 1.0, 1.0),
                (4 / 7., 1.0, 1.0),
                (5 / 7., 1.0, 1.0),
                (6 / 7., 0.0, 0.0),
                (1.0, 0.0, 0.0)),
       'blue': ((0.0, 1.0, 1.0),
               (1 / 7., 1.0, 1.0),
               (2 / 7., 0.0, 0.0),
               (3 / 7., 0.0, 0.0),
               (4 / 7., 0.0, 0.0),
               (5 / 7., 1.0, 1.0),
               (6 / 7., 1.0, 1.0),
               (1.0, 0.0, 0.0)),
      }

    plt.register_cmap(name='full_rainbow', data=cdict1)
    plt.register_cmap(name='full_rainbow_rev', data=cdict2)

    return


registerColorMaps()

def genAirfoilPlots(df: pd.DataFrame, ax=None, **kwargs):
    """
    Creates Cl vs Alpha, CM vs Alpha, and CD vs Alpha
    grouped by Mach

    :param df: Dataframe of airfoil data
    :type df: pd.DataFrame
    :param ax: Optional array of axes handles on which to plot (list [ax_cl, ax_cd, ax_cm, ax_cd_cl, ax_lod]).
                If None, new figures will be created for each plot
    :param kwargs: key word arguments passed to the plot command

    :return: Returns array of axes handles for the plots
    """

    from scipy.interpolate import interp1d

    # Parse the data into a pandas data frame
    mach_col = "Mach"
    aoa_col = "Alpha"
    cl_col = "CL"
    cd_col = "CD"
    cm_col = "CM"
    req_cols = [mach_col, aoa_col, cd_col, cl_col, cm_col]

    # Check that all of the required columns can be found
    for req_col in req_cols:
        if req_col not in df.columns:
            raise ValueError("{} column not found".format(req_col))

    ax_cl = None
    ax_cd = None
    ax_cm = None
    ax_cd_cl = None
    ax_lod = None
    if ax is not None:
        if len(ax) > 0:
            ax_cl = ax[0]
        if len(ax) > 1:
            ax_cd = ax[1]
        if len(ax) > 2:
            ax_cm = ax[2]
        if len(ax) > 3:
            ax_cd_cl = ax[3]
        if len(ax) > 4:
            ax_lod = ax[4]

    if ax_cl is None:
        ax_cl = plt.subplots()[1]
    if ax_cd is None:
        ax_cd = plt.subplots()[1]
    if ax_cm is None:
        ax_cm = plt.subplots()[1]
    if ax_cd_cl is None:
        ax_cd_cl = plt.subplots()[1]
    if ax_lod is None:
        ax_lod = plt.subplots()[1]

    # Create Plots
    for mach, df_mach in df.groupby(mach_col):
        # Sort by angle of attack
        df_aoa = df_mach.sort_values(by=aoa_col)
        label = "M = {:4.2f}".format(mach)
        df_cl = df_aoa.dropna(subset=[cl_col])
        df_cd = df_aoa.dropna(subset=[cd_col])
        df_cm = df_aoa.dropna(subset=[cm_col])
        ax_cl.plot(df_cl[aoa_col], df_cl[cl_col], label=label, **kwargs)
        ax_cd.plot(df_cd[aoa_col], df_cd[cd_col], label=label, **kwargs)
        ax_cm.plot(df_cm[aoa_col], df_cm[cm_col], label=label, **kwargs)

    # Create Drag polar plots, requires interpolating data onto same set of mach/alphas
    unique_machs = df[mach_col]
    # Interpolate lift/drag coefficientts into same set of alphas
    df_cl_pivot = df.dropna(subset=[cl_col]).pivot(index=aoa_col, columns=mach_col)[cl_col]
    df_cd_pivot = df.dropna(subset=[cd_col]).pivot(index=aoa_col, columns=mach_col)[cd_col]
    df_cl_c, df_cd_c = df_cl_pivot.align(df_cd_pivot)
    df_cl_c_dense = df_cl_c.interpolate(method='values').interpolate(method='values', axis=1)
    df_cd_c_dense = df_cd_c.interpolate(method='values').interpolate(method='values', axis=1)
    for mach in df_cl_c_dense.columns:
        label = "M = {:4.2f}".format(mach)
        ax_cd_cl.plot(df_cd_c_dense[mach].values, df_cl_c_dense[mach].values, label=label, **kwargs)
        ax_lod.plot(df_cl_c_dense[mach].values, df_cl_c_dense[mach].values/df_cd_c_dense[mach].values,
                    label=label, **kwargs)

    ax_cl.set_xlabel(r"$\alpha$")
    ax_cl.set_ylabel(r"$c_l$")

    ax_cd.set_xlabel(r"$\alpha$")
    ax_cd.set_ylabel(r"$c_d$")

    ax_cm.set_xlabel(r"$\alpha$")
    ax_cm.set_ylabel(r"$c_m$")

    ax_cd_cl.set_xlabel(r"$c_d$")
    ax_cd_cl.set_ylabel(r"$c_l$")
    ax_cd_cl.set_ylim(ymin=0)
    ax_cd_cl.set_xlim(xmin=0)

    ax_lod.set_xlabel(r"$c_l$")
    ax_lod.set_ylabel(r"$\frac{c_l}{c_d}$")

    return ax_cl, ax_cd, ax_cm, ax_cd_cl, ax_lod

def makeelip( xmin, xmax, ymin, ymax, npts=33):
    '''
    Makes an ellipse

    :param xmin: minimum x
    :param xmax: maximum x
    :param ymin: minimum y
    :param ymax: maximum y
    :param npts: number of points on the ellipse
    :return: (x,y)
    '''
    theta = np.linspace(0,2*np.pi, npts)

    xcir = np.cos(theta)
    ycir = np.sin(theta)

    xmid = np.mean((xmin,xmax))
    ymid = np.mean((ymin,ymax))

    xrng = (xmax - xmin) / 2.
    yrng = (ymax - ymin) / 2.

    x = xmid + xcir * xrng
    y = ymid + ycir * yrng

    return x,y
def makerect(xmin, xmax, ymin, ymax, npts=5):
    '''
    Makes a rectangle

    :param xmin: minimum x
    :param xmax: maximum x
    :param ymin: minimum y
    :param ymax: maximum y
    :param npts: number of points to interlace between corner points
    :return: (x,y)
    '''
    xc = np.array((xmin, xmax, xmax, xmin, xmin))
    yc = np.array((ymin,ymin,ymax,ymax,ymin))


    x = []
    y = []

    for i in range(0,len(xc)-1):
        x.append( np.linspace(xc[i],xc[i+1],npts))
        y.append(np.linspace(yc[i], yc[i+1], npts))

    x = [item for sublist in x for item in sublist]
    y = [item for sublist in y for item in sublist]

    return np.asarray(x), np.asarray(y)


def add_major_minor_gridlines(ax=None, major_kwargs=None, minor_kwargs=None):
    """
    Adds major an minor gridlines to a given axis
    :param ax: axis on which add grid lines, if None the current axis will be used
    :param major_kwargs: dictionary of keyword arguments to format major gridlines
    :param minor_kwargs: dictionary of keyword arguments to format minor gridlines, if none dashed and alpha=0.5 will be used
    :return: N/A
    """
    if ax is None:
        ax = plt.gca()

    if major_kwargs is None:
        major_kwargs = {}

    if minor_kwargs is None:
        minor_kwargs = {}

    minor_kwargs.setdefault("linestyle", "--")
    minor_kwargs.setdefault("alpha", 0.5)

    ax.grid(True, which="major", **major_kwargs)
    ax.grid(True, which="minor", **minor_kwargs)
    ax.minorticks_on()


class FigureSaver:
    def __init__(self, output_dir, dpi=300, transparent=True, tight_layout=True, pdf_filename=None, close_on_save=False,
                 show=False):
        """

        :param output_dir: directory to save plots to
        :param dpi: dpi used for saving pngs
        :param transparent: transparent flag for saving png
        :param tight_layout: if true, tight_layout will be called before saving a figure
        :param pdf_filename: if not none, name of pdf file to save images to
        :param close_on_save: if true, once a figured is saved, it will be closed
        :param show: if true, plt.show(block=False) and plt.pause(0.001) will be called before saving and closing figure
        """
        self.output_dir = output_dir
        self.dpi = dpi
        self.transparent = transparent
        self.tight_layout = tight_layout
        self.pdf_filename = pdf_filename
        self.close_on_save = close_on_save
        self.show = show
        os.makedirs(self.output_dir, exist_ok=True)

    def __enter__(self):
        if self.pdf_filename is not None:
            self.pdf = PdfPages(os.path.join(self.output_dir, self.pdf_filename))
            self.default_imagebase = os.path.basename(os.path.splitext(self.pdf_filename)[0])
            self.page_count = 0
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        if self.pdf_filename is not None:
            self.pdf.close()

    def save(self, fig=None, name="page.png"):
        """
        Saves a figure to pdf and to png
        :param fig: figure to save, if None, current figure is used
        :param name: name of png to save, this name will be prefixed with pdf page number
        :return:
        """
        if self.pdf_filename is not None:
            name = f"{self.page_count:03d}_{name}"
        self.save_png(name, fig, close=False)
        if self.pdf_filename is not None:
            self.save_to_pdf(fig)

    def save_to_pdf(self, fig=None, close=None):
        """
        Saves a figure to the pdf
        :param fig: figure to save, if None, current figure is used
        :param close: if true figure will be closed after save, if false, figure will not be closed.
            If none self.close_on_save will be used to determine if figure should be closed
        :return: n/a
        """
        if fig is None:
            fig = plt.gcf()
        if close is None:
            close = self.close_on_save
        if self.tight_layout:
            fig.tight_layout()
        if self.show:
            plt.show(block=False)
            plt.pause(0.001)
        self.pdf.savefig(fig)
        if close:
            plt.close(fig)
        self.page_count += 1

    def save_png(self, name, fig=None, close=None):
        """
        Save a figure to a png file in output directory
        :param name: filename
        :param fig: matplotlib figure, if None, current figure is used
        :param close: if true figure will be closed after save, if false, figure will not be closed.
            If none self.close_on_save will be used to determine if figure should be closed
        :return:
        """

        full_path = os.path.join(self.output_dir, name)
        if close is None:
            close = self.close_on_save
        if fig is None:
            fig = plt.gcf()
        if self.tight_layout:
            fig.tight_layout()
        if self.show:
            plt.show(block=False)
            plt.pause(0.001)
        fig.savefig(full_path, dpi=self.dpi, transparent=self.transparent)
        if close:
            plt.close(fig)
