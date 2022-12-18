import openvsp_config

if openvsp_config.LOAD_GRAPHICS:
  if openvsp_config.LOAD_FACADE:
    from .vsp_g_facade import *
  else:
    from .vsp_g import *
else:
  from .vsp import *

from .degen_geom_parse import *
from .parasite_drag import *
from .surface_patches import *
from .utilities import *
import os

def setup_vspaero_path():
  base_dir = os.path.dirname(__file__)

  if  vsp.CheckForVSPAERO( base_dir ):
    vsp.SetVSPAEROPath( base_dir )

setup_vspaero_path()
