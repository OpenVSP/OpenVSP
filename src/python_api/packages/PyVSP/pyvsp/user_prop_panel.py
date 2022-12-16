
import wx
import wx.propgrid as pg

class UserPropertyPanel(wx.Panel):
    '''
    Property Panel for editing user params

    '''
    def __init__(self, parent, vsp):
        """


        Parameters
        ----------
        parent : wx.Object
            parent object of this panel
        vsp : vsp
            the data from the branch selected
        """
        wx.Panel.__init__(self, parent, -1,size=wx.Size(-1,800))
        self.parent = parent
        self.vsp = vsp
        # Panel sizer
        self.sizer = wx.BoxSizer(wx.VERTICAL)
        self.SetSizer(self.sizer)

        # Creates a Property Grid
        self.prop_grid = pg.PropertyGrid(
            self, wx.ID_ANY,
            wx.DefaultPosition,
            wx.Size(300, -1),
            pg.PG_DEFAULT_STYLE | pg.PG_BOLD_MODIFIED)

        # binds event when the user edits the property grid
        self.prop_grid.Bind(pg.EVT_PG_CHANGED, self.OnPropGridChange)

        self.p_fields = []

        #set default values for property grid
        self.vsp_update()
        self.prop_grid.FitColumns()
        self.prop_grid.SetSize(-1,-1)
        self.sizer.Add(self.prop_grid, 1, wx.ALL | wx.EXPAND, 0)
        self.sizer.Fit(self)


    def OnPropGridChange(self, event=None, prop_name=None):
        """
        Called when one property changes in the property grid

        Parameters
        ----------
        event : wx.Event
            a wx event
        prop_name : str
            if called manually, this function needs a prop_name instead of an event
            Tom - This is left over from what I copied this from

        """

        #some of this is unnecessary for this script, but i just copied stuff
        if not prop_name:
            p = event.GetProperty()
            pName = p.GetName()
        else:
            pName = prop_name

        edited_pg = None
        for pg in self.p_fields:
            if pg.GetName() == pName:
                edited_pg = pg
        if edited_pg == None:
            print("could not find edited property")
            return
        self.vsp.SetParmVal(edited_pg.GetName(), edited_pg.GetValue())
        print("edited value")

    def vsp_update(self):
        #this function should be called when ever the openvsp model changes
        self.Freeze()
        self.prop_grid.Clear()
        self.p_fields = []

        usr_container_id = self.vsp.FindContainer("UserParms", 0)
        parm_id_list = self.vsp.FindContainerParmIDs(usr_container_id)

        for parm_id in parm_id_list:
            parm_type = self.vsp.GetParmType(parm_id)
            parm_name = self.vsp.GetParmName(parm_id)
            if parm_type == self.vsp.PARM_INT_TYPE:
                parm_value = self.vsp.GetIntParmVal(parm_id)
                new_pg = pg.IntProperty(parm_name, parm_id, parm_value)
            else:
                parm_value = self.vsp.GetParmVal(parm_id)
                new_pg = pg.FloatProperty(parm_name, parm_id, parm_value)
            self.p_fields.append(new_pg)
            self.prop_grid.Append(new_pg)


        self.Thaw()
