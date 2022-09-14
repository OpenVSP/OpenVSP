import charm.input_automation as charm
import utilities.units as uu
import openvsp as vsp
import numpy as np

import os
import math
import matplotlib.pyplot as plt

path = os.path.dirname(__file__)

base_directory = "simple_prop"
template_directory = "templates"
geom_directory = "geom"
run_directory = "runs"

# Changed to "True" because I ran it twice and the second time it threw an error b/c it couldn't write the file.  Just a
# preference for this testing.
overwrite_files = True

vsp_filename = "prop.vsp3"
# This is one of your problems.  This VSP3 model was made inconsitently...the set name in the model is "Charm", not
# "CHARM."  
vsp_charm_set_name = "Charm"
# You don't need to have a pitch component name because this is a simple rotor, there is no Datum in the vsp3 model
# vsp_pitch_comp_name = "Datum"
vsp_prop_name = "Prop"

charm_prop_rw_template = "proprw.inp"
charm_rc_template = "prop.inp"
charm_casename = "test"

airspeed_fts = 20.0 * uu.mph2fps
# No pitch in this simple model (see vsp3 model)
# pitch_deg = 0.0
alpha_deg = 3.0
prop_collective = 5.0
prop_rpm = 1444.0
prop_ct = 0.0240

vsp.VSPRenew()
vsp.ClearVSPModel()
vsp.DeleteAllResults()

vsp.ReadVSPFile(os.path.join(path, base_directory, geom_directory, vsp_filename))

# vsp_pitch_comp_id = vsp.FindGeom(vsp_pitch_comp_name, 0)
vsp_prop_id = vsp.FindGeom(vsp_prop_name, 0)

# No longer need as we are not pitching the rotor in this model
# vsp.SetParmVal(vsp_pitch_comp_id, "Y_Rel_Rotation", "XForm", pitch_deg)

vsp.Update()

vsp_charm_set_index = vsp.GetSetIndex(vsp_charm_set_name)
dg_mgr = vsp.run_degen_geom(set_name=vsp_charm_set_name)
prop_info = vsp.get_propeller_thrust_vectors(prop_set=vsp.SET_FIRST_USER)

# Note: This block of code is not necessary and is just a fancy demo that Alex did.  This works better when in Jupyter
# Notebooks as I had to add the "waitforbuttonpress()" line to keep the window open.  I did discover that you **cannot**
# just hit the "X" to close the window and have it work.  You have to click anywhere on the window.
# prop_fig = vsp.plot_propeller_info(prop_info, vector_scale=20)
# plt.show(block=False)
# plt.waitforbuttonpress()
# plt.close()

vsp.DeleteAllResults()

degen_mgr = vsp.run_degen_geom(set_index=vsp_charm_set_index)

rotor_rw = []
with open(os.path.join(path, base_directory, template_directory, charm_prop_rw_template), "r") as f:
    rotor_rw = f.readlines()

rotor_settings = charm.build_default_rotor_settings(degen_mgr=degen_mgr, default_rpm=prop_rpm,
                                                    default_template=rotor_rw)

rotor_settings[vsp_prop_id].icoll = 1
rotor_settings[vsp_prop_id].initial_collective = prop_collective
rotor_settings[vsp_prop_id].rpm = prop_rpm
rotor_settings[vsp_prop_id].ct = prop_ct
rotor_settings.nchord = 4
rotor_settings.nspan_override = 40

u_speed = airspeed_fts * math.cos(alpha_deg * uu.deg2rad)
v = 0.0
w = -airspeed_fts * math.sin(alpha_deg * uu.deg2rad)

files_to_write = charm.build_charm_input_files(degen_mgr=degen_mgr, case_name=charm_casename,
                                               rotor_settings=rotor_settings,
                                            # You don't need wing_settings b/c you don't have a wing
                                            #    wing_settings=wing_settings,
                                               unit_factor=uu.in2ft,
                                               run_char_filename=os.path.join(path, base_directory,
                                                                              template_directory, charm_rc_template),
                                               velocity=[u_speed, v, w], isv66=True)
# per above, pitch_deg is no longer needed and so is not defined.
# run_dir_name = f"new_run_v{airspeed_fts:0.3f}_pitch{pitch_deg:3.2f}_alpha_{alpha_deg:3.2f}"
run_dir_name = f"test_run_v{airspeed_fts:0.3f}_alpha_{alpha_deg:3.2f}"

cwd = os.getcwd()
try:
    charm_exe_dir = os.path.join(path, base_directory, run_directory, run_dir_name)
    os.makedirs(charm_exe_dir, exist_ok=overwrite_files)
    os.chdir(charm_exe_dir)

    for filename, file_contents in files_to_write.items():
        with open(filename, "w") as f:
            f.write(file_contents)
finally:
    os.chdir(cwd)
