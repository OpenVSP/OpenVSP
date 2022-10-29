import os
from vspaero.pyvspaero import pyVSPAero
from vspaero import functions

vsp_file = os.path.join(os.path.dirname(__file__), "./Boeing_777-9x_ref.vsp3")
VLMAssembler = pyVSPAero(vsp_file, comps=["Wing"])
VLMAssembler.add_function("CL", functions.CL)
VLMAssembler.add_function("CD", functions.CD)
prob = VLMAssembler.create_steady_problem("Cruise", aoa=5.0)
prob.solve()
funcs = {}
prob.eval_functions(funcs)
print(funcs)
prob.write_solution()
funcs_sens = {}
prob.eval_functions_sens(funcs_sens)
xyz = prob.get_geometry()
xyz[1::3] *= 3.0
prob.set_geometry(xyz)
prob.set_flight_vars(aoa=6.0)
prob.solve()
prob.write_solution()