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

if load_graphics:
    if load_facade:
        from .facade import *
    elif load_multi_facade:
        print("multi facade not supported")
        #from .vsp_g_facade_multi import *
    else:
        from .vsp_g import *
else:
    from .vsp import *

if ignore_imports:
    pass

elif not (load_facade or load_multi_facade):

    from .degen_geom_parse import *
    from .parasite_drag import *
    from .surface_patches import *
    from .utilities import *
    import os

    def setup_vspaero_path():
        base_dir = os.path.dirname(__file__)

        if  CheckForVSPAERO( base_dir ):
            SetVSPAEROPath( base_dir )

    setup_vspaero_path()
