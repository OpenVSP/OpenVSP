import wx
import wx.propgrid as pg

from pyvsp.results_panel import ResultsDialog

class AnalysisPanel(wx.Panel):
    '''
    Analysis Panel

    '''
    def __init__(self, parent, vsp):
        """
        Initializes the analysis panel.

        Parameters
        ----------
        parent : wx.Object
            The parent object for the analysis panel
        vsp : vsp
            the active vsp module
        """

        self.parent = parent
        self.vsp = vsp
        self.has_defaults_list = []

        # Create wx panel
        wx.Panel.__init__(self, self.parent, -1,size=wx.Size(-1,800))

        # Add sizer
        self.sizer = wx.FlexGridSizer(0, 1, 0, 0)
        self.sizer.AddGrowableRow(1)
        self.sizer.SetFlexibleDirection(wx.BOTH)
        self.sizer.SetNonFlexibleGrowMode(wx.FLEX_GROWMODE_ALL)
        self.SetSizer(self.sizer)

        # Adds box for selecting an analysis
        analysis_sizer = wx.StaticBoxSizer(wx.StaticBox(self, wx.ID_ANY), wx.HORIZONTAL)
        self.analysis_choice = wx.Choice(analysis_sizer.GetStaticBox(), -1, size=(1000, -1), choices=self.vsp.ListAnalysis())
        label = wx.StaticText(analysis_sizer.GetStaticBox(), wx.ID_ANY, u"Select Analysis: ", wx.DefaultPosition, (-1,-1), 0)
        analysis_sizer.Add(label, 0, wx.ALL|wx.ALIGN_CENTER_VERTICAL, 5)
        analysis_sizer.Add(self.analysis_choice, 0, wx.ALL|wx.EXPAND, 5)

        #binds event whenever the user chooses a new analysis
        self.Bind(wx.EVT_CHOICE, self.on_select_analysis, self.analysis_choice)

        # Create a Property Grid to contain analysis inputs
        self.analysis_grid = pg.PropertyGrid(
            self, wx.ID_ANY,
            wx.DefaultPosition,
            wx.Size(-1, -1),
            pg.PG_DEFAULT_STYLE | pg.PG_BOLD_MODIFIED | wx.EXPAND)
        self.analysis_grid.SetExtraStyle(pg.PG_EX_HELP_AS_TOOLTIPS)

        # binds event whenever the user changes a property
        self.analysis_grid.Bind(pg.EVT_PG_CHANGED, self.on_analysis_grid_change)

        #updates and fits property grid
        self.vsp_update()
        self.analysis_grid.FitColumns()
        self.analysis_grid.SetSize(-1,-1)

        # Create a run analysis button
        run_button = wx.Button(self, wx.ID_ANY, u"Run Analysis", wx.DefaultPosition, wx.DefaultSize, 0)
        self.Bind(wx.EVT_BUTTON, self.on_run_analysis, run_button)

        # Adds all previous objects to main analysis panel sizer
        self.sizer.Add(analysis_sizer, 1, wx.ALL | wx.EXPAND, 5)
        self.sizer.Add(self.analysis_grid, 1, wx.ALL | wx.EXPAND, 5)
        self.sizer.Add(run_button, 1, wx.ALL | wx.EXPAND, 5)



    def on_select_analysis(self, event):
        """
        Function that gets called when a user selects a new analysis from the drop down.
        It populates the analysis property grid with the correct name-value pairs

        Parameters
        ----------
        event : wx.Event
            The event that triggered this function
        """

        print("analysis selected")

        # Clears property grid
        self.analysis_grid.Clear()

        # Gets name of selected analysis
        analysis_index = self.analysis_choice.GetSelection()
        analysis_name = self.vsp.ListAnalysis()[analysis_index]

        # I don't think the has_defaults list was implemented
        # I imagine it was supposed to save user inputs
        if not analysis_name in self.has_defaults_list:
            self.vsp.SetAnalysisInputDefaults(analysis_name)
            self.has_defaults_list.append(analysis_name)

        #gets analysis input names
        input_names = self.vsp.GetAnalysisInputNames(analysis_name)

        # loops through every input
        for input_name in input_names:
            input_type = self.vsp.GetAnalysisInputType(analysis_name, input_name)
            input_num = self.vsp.GetNumAnalysisInputData(analysis_name, input_name)

            # GUI does not handle analysis inputs that accept more than one value
            if input_num > 1:
                print("input_num > 1 not handled")
                continue

            # Create property grid items based on input type
            if input_type == self.vsp.INT_DATA:
                default_value = self.vsp.GetIntAnalysisInput(analysis_name, input_name)[0]
                new_pg = pg.IntProperty(input_name, input_name, default_value)
            elif input_type == self.vsp.DOUBLE_DATA:
                default_value = self.vsp.GetDoubleAnalysisInput(analysis_name, input_name)[0]
                new_pg = pg.FloatProperty(input_name, input_name, default_value)
            elif input_type == self.vsp.STRING_DATA:
                default_value = self.vsp.GetStringAnalysisInput(analysis_name, input_name)[0]
                new_pg = pg.StringProperty(input_name, input_name, default_value)
            elif input_type == self.vsp.VEC3D_DATA:
                print("vec data type not handled")
                continue

            #adds documentation as a tool tip
            try:
                doc = self.vsp.GetAnalysisInputDoc(analysis_name, input_name)
            except:
                doc = "tom: default doc text"
            self.analysis_grid.Append(new_pg)
            self.analysis_grid.SetPropertyHelpString(new_pg, doc)

        #resizes property grid columns
        self.analysis_grid.FitColumns()

    def on_run_analysis(self,event):
        """
        Function that gets called when the user clicks the run analysis button.
        It runs the selected analysis in OpenVSP

        Parameters
        ----------
        event : wx.Event
            The event that triggered this function
        """

        # Gets currently selected analysis
        analysis_index = self.analysis_choice.GetSelection()
        analysis_name = self.vsp.ListAnalysis()[analysis_index]

        #executes the analysis
        result_id = self.vsp.ExecAnalysis(analysis_name)
        print("finished analysis")

        # Create and show the results dialog
        diag = ResultsDialog(self.parent, result_id, self.vsp)
        diag.Show()

    def on_analysis_grid_change(self, event=None):
        """
        Called when one property changes in the property grid.
        It takes the GUI user-entered value and feeds it into VSP

        Parameters
        ----------
        event : wx.Event
            The event that triggered this function

        """

        # gets the property that was edited from the event
        edited_pg = event.GetProperty()
        var_name = edited_pg.GetName()

        # Get the currently selected analysis
        analysis_index = self.analysis_choice.GetSelection()
        analysis_name = self.vsp.ListAnalysis()[analysis_index]

        #sets the value depending on the input type
        if isinstance(edited_pg, pg.IntProperty):
            self.vsp.SetIntAnalysisInput(analysis_name, var_name, [edited_pg.GetValue()])
        elif isinstance(edited_pg, pg.FloatProperty):
            self.vsp.SetDoubleAnalysisInput(analysis_name, var_name, [edited_pg.GetValue()])
        elif isinstance(edited_pg, pg.StringProperty):
            self.vsp.SetStringAnalysisInput(analysis_name, var_name, [edited_pg.GetValue()])
        else:
            print("unrecognized property type edited")
            return

        print("edited value")

    def vsp_update(self):
        #should be called whenever the openvsp model changes
        self.analysis_choice.Set(self.vsp.ListAnalysis())
        self.analysis_choice.SetSelection(wx.NOT_FOUND)