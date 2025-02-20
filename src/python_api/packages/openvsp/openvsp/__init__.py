import os
import sys
import importlib

try:
    import openvsp_config
    load_graphics = openvsp_config.LOAD_GRAPHICS
    load_facade = openvsp_config.LOAD_FACADE
    load_multi_facade = openvsp_config.LOAD_MULTI_FACADE
    ignore_imports = openvsp_config._IGNORE_IMPORTS
except ModuleNotFoundError:
    # if openvsp_config is not setup, loads default vsp
    load_graphics = False
    load_facade = False
    load_multi_facade = False
    ignore_imports = False

if load_facade:
    from .facade import _vsp_server, vec3d, Matrix4d, ErrorMgrSingleton, ErrorObj
    _single = _vsp_server("vsp_singleton")
    thismodule = sys.modules[__name__]
    for attr_name in dir(_single):
        try:
            getattr(_single, attr_name)
            setattr(thismodule, attr_name, getattr(_single, attr_name))
        except :
            pass
elif load_multi_facade:
    from .facade import _vsp_server, vec3d, Matrix4d, ErrorMgrSingleton, ErrorObj, vsp_servers
    _single = _vsp_server("vsp_singleton")
    thismodule = sys.modules[__name__]
    for attr_name in dir(_single):
        try:
            getattr(_single, attr_name)
            setattr(thismodule, attr_name, getattr(_single, attr_name))
        except :
            pass
elif load_graphics:
    from .vsp_g import *
    InitGUI()
else:
    from .vsp import *

def setup_vspaero_path():
    base_dir = os.path.dirname(__file__)
    if  CheckForVSPAERO( base_dir ):
        SetVSPAEROPath( base_dir )

if ignore_imports or not (load_facade or load_multi_facade):
    setup_vspaero_path()

if not ignore_imports:
    if load_multi_facade:
        import inspect
        import openvsp.degen_geom_parse as degen_geom_parse
        import openvsp.parasite_drag as parasite_drag
        import openvsp.surface_patches as surface_patches
        import openvsp.utilities as utilities

        functions = []

        for mod in [degen_geom_parse, parasite_drag, surface_patches, utilities]:

            for attribute in dir(mod):
                if (
                    inspect.isfunction(getattr(mod, attribute))
                    or inspect.isclass(getattr(mod, attribute))
                ):
                    functions.append(getattr(mod, attribute))

        vsp_servers.set_functions(functions + [vec3d, Matrix4d, ErrorMgrSingleton, ErrorObj])

    else:
        from .degen_geom_parse import *
        from .parasite_drag import *
        from .surface_patches import *
        from .utilities import *

def get_instance(vsp_instance):
    if load_multi_facade:
        if not vsp_instance:
            return _single
        instance = vsp_servers.get_vsp_instance(vsp_instance)
        if instance:
            return instance
        return _single
    else:
        return importlib.import_module("openvsp")
