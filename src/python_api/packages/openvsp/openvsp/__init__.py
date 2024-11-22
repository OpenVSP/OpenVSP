import os
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
    from .facade import *
elif load_multi_facade:
    print("multi facade not supporrted")
elif load_graphics:
    from .vsp_g import *
else:
    from .vsp import *

def setup_vspaero_path():
    base_dir = os.path.dirname(__file__)
    if  CheckForVSPAERO( base_dir ):
        SetVSPAEROPath( base_dir )

    if  CheckForVSPHelp( base_dir + "/help_vsp" ):
        SetVSPHelpPath( base_dir + "/help_vsp" )

if load_graphics:
    InitGUI()

if ignore_imports:
    setup_vspaero_path()


elif not (load_facade or load_multi_facade):
    from .degen_geom_parse import *
    from .parasite_drag import *
    from .surface_patches import *
    from .utilities import *

    setup_vspaero_path()
