from uuid import uuid4, UUID
from enum import Enum
from json import dumps, JSONEncoder
from re import compile as re_compile
from collections import OrderedDict
from functools import partial
try:
    from IPython.display import display_javascript, display_html
except ImportError:
    display_html, display_javascript = None, None
import vsp


regex_listname = re_compile(r"^(?P<name>[a-zA-Z]+)_*(?P<i>\d*)")
EXCLUDE_GROUPS = ('BBox',)
INCLUDE_GROUPS = ('Design',)  # Not currently used for filtering groups
PARAM_KEYS = ('upper', 'lower', 'value')

Enum0 = partial(Enum, start=0)
ParamType = Enum0('ParamType', 'PARM_DOUBLE_TYPE PARM_INT_TYPE PARM_BOOL_TYPE PARM_FRACTION_TYPE PARM_STRING_TYPE ')
ErrorCode = Enum0('ErrorCode', 'VSP_OK VSP_INVALID_PTR VSP_CANT_FIND_TYPE VSP_CANT_FIND_PARM VSP_CANT_FIND_NAME ' +
                  'VSP_INVALID_GEOM_ID VSP_FILE_DOES_NOT_EXIST VSP_FILE_WRITE_FAILURE VSP_WRONG_XSEC_TYPE ' +
                  'VSP_WRONG_FILE_TYPE VSP_INDEX_OUT_RANGE VSP_INVALID_XSEC_ID')
SymCode = Enum0('SymCode', 'SYM_XY SYM_XZ SYM_YZ SYM_ROT_X SYM_ROT_Y SYM_ROT_Z SYM_PLANAR_TYPES SYM_NUM_TYPES')
ExportCode = Enum0('ExportCode', 'EXPORT_FELISA EXPORT_XSEC EXPORT_STL EXPORT_AWAVE EXPORT_NASCART EXPORT_POVRAY ' +
                   'EXPORT_CART3D EXPORT_VORXSEC EXPORT_XSECGEOM EXPORT_GMSH EXPORT_X3D')


class VspEncoder(JSONEncoder):
    # TODO: remove keys that start with '_'
    def default(self, obj):
        if isinstance(obj, VspElement):
            return obj.json

        if isinstance(obj, Enum):
            return {"__enum__": str(obj)}

        if isinstance(obj, UUID):
            return {"__uuid__": str(obj)}

        return JSONEncoder.default(self, obj)


class VspElement(OrderedDict):
    """An object that can represent an manipulate OpenVSP elements."""

    def __init__(self, is_param=False, **kwargs):
        super().__init__()
        self.__uuid = uuid4()
        self.__is_param = is_param
        for key, value in kwargs.items():
            if isinstance(value, dict):
                value = VspElement(**value)
            self[key] = value

    def __hash__(self):
        return hash(id(self))

    def __getitem__(self, key):
        try:
            if key[0] == '_':
                return super().__getitem__(key)
        except TypeError:
            pass

        try:
            item = super().__getitem__(key)
        except KeyError:
            return None

        if isinstance(item, VspElement) and item.__is_param:
            return item.value
        else:
            return item

    def __lt__(self, other):
        if self.__is_param:
            return self.value < other
        else:
            return super().__lt__(other)

    def __gt__(self, other):
        if self.__is_param:
            return self.value > other
        else:
            return super().__gt__(other)

    def __eq__(self, other):
        if self.__is_param:
            return self.value == other
        else:
            return super().__eq__(other)

    def __setitem__(self, key, value):
        try:
            item = super().__getitem__(key)
        except KeyError:
            item = []
        if isinstance(item, VspElement) and item.__is_param:
            if value < item.lower or value > item.upper:
                msg = "{lower}<={name}<={upper}, value={_new}"
                raise ValueError(msg.format(_new=value, **item))
            item.value = value
            vsp.Update()
        else:
            super().__setitem__(key, value)

    __getattr__ = __getitem__
    __setattr__ = __setitem__
    __delattr__ = OrderedDict.__delitem__

    def __dir__(self):
        return super().__dir__() + list(self.keys())

    def __repr__(self):
        rep_dict = {'name': 'VSP Element'}
        rep_dict.update(**dict(self))
        return "{name}".format(**rep_dict)

    def _ipython_display_(self):
        if display_html is not None:
            display_html('<div id="{}" style="height: 600px; width:100%;"></div>'.format(self.__uuid), raw=True)

            display_javascript("""
                require(["https://rawgit.com/caldwell/renderjson/master/renderjson.js"], function() {
                  document.getElementById('%s').appendChild(renderjson(%s))
                });
                """ % (self.__uuid, self.json), raw=True)

    @property
    def json(self):
        return dumps(self, cls=VspEncoder, indent=2, sort_keys=True)


class VspModel(VspElement):
    """An object that can represent and manipulate OpenVSP aircraft."""

    def __init__(self, filename=None, threejs=False, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.__filename = filename
        self.__threejs = threejs
        self.__param_map = {}
        self.design = None

        if filename:
            self._load_from_file(filename)

    def load_design(self, filename):
        design_vars = VspElement()
        vsp.ReadApplyDESFile(filename)
        for i in range(vsp.GetNumDesignVars()):
            param_id = vsp.GetDesignVar(i)
            if param_id in self.__param_map:
                param = self.__param_map[param_id]
            else:
                param = self._make_parameter(param_id)
            design_vars[param.name] = param
        self.design = design_vars

    def _make_parameter(self, param_id):
        kind = vsp.GetParmType(param_id)
        if kind < len(ParamType):
            kind = ParamType(kind)
        param = VspElement(_id=param_id,
                           name=vsp.GetParmName(param_id),
                           kind=kind,
                           value=vsp.GetParmVal(param_id),
                           lower=vsp.GetParmLowerLimit(param_id),
                           upper=vsp.GetParmUpperLimit(param_id),
                           )
        self.__param_map[param_id] = param
        return param

    @staticmethod
    def _update_list(lst, idx):
        idx = int(idx)
        if len(lst) <= idx:
            lst.extend([None] * (1 + idx - len(lst)))
        if lst[idx] is None:
            lst[idx] = VspElement()
        return lst, lst[idx]

    def _load_from_file(self, filename):
        vsp.ClearVSPModel()
        vsp.ReadVSPFile(filename)

        for geom_id in vsp.FindGeoms():
            geom_name_raw = vsp.GetGeomName(geom_id)
            geom_name, geom_idx = regex_listname.findall(geom_name_raw)[0]

            if geom_name not in self:
                if geom_idx:
                    self[geom_name] = []
                else:
                    self[geom_name] = VspElement()

            if geom_idx != '':
                geom = self._update_list(self[geom_name], geom_idx)
            else:
                geom = self[geom_name]

            geom._id = geom_id

            for param_id in vsp.GetGeomParmIDs(geom_id):
                group_name_raw = vsp.GetParmDisplayGroupName(param_id)
                group_name, group_idx = regex_listname.findall(group_name_raw)[0]
                if group_name not in EXCLUDE_GROUPS:
                    if group_name not in geom:
                        if group_idx:
                            geom[group_name] = []
                        else:
                            geom[group_name] = VspElement()

                    if group_idx != '':
                        geom[group_name], group = self._update_list(geom[group_name], group_idx)
                    else:
                        group = geom[group_name]

                    param = self._make_parameter(param_id)

                    if param['name'] in group:
                        raise ValueError("{} already in <{}:{}>".format(param.name, geom_name, group_name))

                    group[param['name']] = param

    def export(self, filename=None, vsp_set=0, file_format=ExportCode.EXPORT_STL):
        if filename is None:
            # TODO: map extension to file_format
            filename = self.__filename.split('.')[0] + '.stl'
        if isinstance(file_format, ExportCode):
            file_format = file_format.value
        vsp.ExportFile(filename, vsp_set, file_format)

    @property
    def threejs_data(self):
        if '__stl_filename' not in self:
            self.__stl_filename = '_tmp_threejs.stl'
        self.export(filename=self.__stl_filename, file_format=ExportCode.EXPORT_STL.value)
        # read in STL file
        with open(self.__stl_filename) as stl_file:
            stl_data = stl_file.read()

        return dumps([{"id": 2, "polyData": stl_data, "x": 0.0, "y": 0.0, "z": 0.0}])

    @property
    def wetted_areas(self):
        vsp.DeleteAllResults()
        results = {}
        _ = vsp.ComputeCompGeom(0, False, 0)
        result_id = vsp.FindLatestResultsID('Comp_Geom')
        components = vsp.GetStringResults(result_id, 'Comp_Name')
        wetted_areas = vsp.GetDoubleResults(result_id, 'Wet_Area')
        for comp, area in zip(components, wetted_areas):
            results[comp] = area

        results['total'] = sum(wetted_areas)
        return results

    # TODO: add more methods to do the other calculations
