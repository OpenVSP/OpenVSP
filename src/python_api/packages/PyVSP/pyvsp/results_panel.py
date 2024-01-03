
import wx
import wx.propgrid as pg

class ResultsDialog(wx.Dialog):
    size = wx.Size(800, 600)
    title = "Results"
    sash = .5

    def __init__(self, parent, results_id, vsp):
        """
        Initialize the results dialog

        Parameters
        ----------
        parent : wx.Window
            The parent window
        results_id : str
            The id of the results to display
        vsp : _type_
            the active vsp module
        """
        self.results_id = results_id
        self.vsp = vsp
        wx.Dialog.__init__(self, parent, id=wx.ID_ANY, title=self.title, size=self.size, style=wx.DEFAULT_DIALOG_STYLE)
        self.sizer = wx.FlexGridSizer(0, 1, 0, 0)
        self.sizer.AddGrowableRow(0)
        self.sizer.SetFlexibleDirection(wx.BOTH)
        self.sizer.SetNonFlexibleGrowMode(wx.FLEX_GROWMODE_ALL)
        self.SetSizer(self.sizer)

        # Create a Property Grid
        self.results_grid = pg.PropertyGrid(
            self, wx.ID_ANY,
            wx.DefaultPosition,
            wx.Size(-1, -1),
            pg.PG_DEFAULT_STYLE | pg.PG_BOLD_MODIFIED | wx.EXPAND)
        self.results_grid.SetExtraStyle(pg.PG_EX_HELP_AS_TOOLTIPS)

        self.update_grid()
        self.results_grid.FitColumns()
        self.results_grid.SetSize(-1,-1)

        self.sizer.Add(self.results_grid, 1, wx.ALL | wx.EXPAND, 5)

    def update_grid(self):
        #makes a read only property grid
        #makes all results strings for simplification
        #probably crash the program if large matrix is read
        print("analysis selected")
        results_name = self.vsp.GetResultsName(self.results_id)
        output_names = self.vsp.GetAllDataNames(self.results_id)

        for output_name in output_names:
            output_type = self.vsp.GetResultsType(self.results_id, output_name)
            output_num = self.vsp.GetNumData(self.results_id, output_name)

            if output_num > 1:
                print("output_num > 1 not handled")
                continue

            if output_type == self.vsp.INT_DATA:
                data_value = str(self.vsp.GetIntResults(self.results_id, output_name))
            elif output_type == self.vsp.DOUBLE_DATA:
                data_value = str(self.vsp.GetDoubleResults(self.results_id, output_name))
            elif output_type == self.vsp.STRING_DATA:
                data_value = str(self.vsp.GetStringResults(self.results_id, output_name))

            elif output_type == self.vsp.VEC3D_DATA:
                data_value = "VEC3D data not displayed"
            else:
                data_value = "data not displayed"
            if len(data_value) > 100:
                data_value = "Data too large to display"
            new_pg = pg.StringProperty(output_name, output_name, data_value)
            try:
                doc = self.vsp.GetResultsEntryDoc(self.results_id, output_name)
            except:
                doc = "default doc text"

            self.results_grid.Append(new_pg)
            self.results_grid.SetPropertyReadOnly(new_pg)
            self.results_grid.SetPropertyHelpString(new_pg, doc)

        self.results_grid.FitColumns()
