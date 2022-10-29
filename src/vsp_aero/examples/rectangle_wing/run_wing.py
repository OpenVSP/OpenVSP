import os
from vspaero.pyvspaero import pyVSPAero
from vspaero import functions

# Setup VLM assembler
vsp_file = os.path.join(os.path.dirname(__file__), "./rect_wing.vsp3")
VLMAssembler = pyVSPAero(vsp_file)
# Add output functions
VLMAssembler.add_function("CL", functions.CL)
VLMAssembler.add_function("CD", functions.CD)

# Create a steady problem instance
prob1 = VLMAssembler.create_steady_problem("Cruise", aoa=5.0)
prob2 = VLMAssembler.create_steady_problem("Maneuver", aoa=6.0)
all_problems = [prob1, prob2]

# Forward solve
funcs = {}
for problem in all_problems:
    problem.solve()
    problem.eval_functions(funcs)
    # Write out adb file for viewing
    problem.write_solution()
print(funcs)

# Adjoint solve
funcs_sens = {}
for problem in all_problems:
    # Get function sensitivities
    problem.eval_functions_sens(funcs_sens)
print(funcs_sens)

# Update problem and re-solve
new_funcs = {}
for problem in all_problems:
    xyz = problem.get_geometry()
    xyz[1::3] *= 3.0
    problem.set_geometry(xyz)
    problem.set_flight_vars(aoa=6.0)
    problem.solve()
    problem.eval_functions(new_funcs)
    problem.write_solution()
print(new_funcs)