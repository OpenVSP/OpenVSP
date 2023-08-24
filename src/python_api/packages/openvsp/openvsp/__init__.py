import openvsp_config

if openvsp_config.LOAD_GRAPHICS:
  if openvsp_config.LOAD_FACADE:
    from .facade import *
  elif openvsp_config.LOAD_MULTI_FACADE:
	print("multi facade not supported")
    #from .vsp_g_facade_multi import *
  else:
    from .vsp_g import *
else:
  from .vsp import *

if openvsp_config._IGNORE_IMPORTS:
    pass

elif not (openvsp_config.LOAD_FACADE or openvsp_config.LOAD_MULTI_FACADE):

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
