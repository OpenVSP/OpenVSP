from .vsp_g import *
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
