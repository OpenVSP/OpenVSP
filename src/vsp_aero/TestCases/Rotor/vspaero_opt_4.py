import numpy as np
from vspaero.optimizer import pyVSPOptimizer
from vspaero.functions import CD

def main(fname_prefix):
    vspopt = pyVSPOptimizer()

    # vspopt.nthreads = 4
    vspopt.set_debug_print(True)
    vspopt.set_unsteady_analysis(1)
    vspopt.set_number_of_optimization_functions(1)
    vspopt.set_optimization_functions(np.array([ROTOR_CT_TOTAL], dtype=np.int32))
    # vspopt.output_sets = np.array([1], dtype=np.int32)
    vspopt.setup(fname_prefix)
    # vec = np.ones(vspopt.get_number_of_optimization_functions(), dtype=np.float64)
    # vspopt.solve_with_vec(vec)
    vspopt.solve()

    with open(f"{fname_prefix}.pyvspaero.gradient", mode="w") as f:
        f.write(f"{vspopt.get_number_of_nodes()} \n")
        for p in range(vspopt.get_number_of_optimization_functions()):

            # Get the optimization function value:
            func = np.empty(1, dtype=np.float64)
            vspopt.get_function_value(p, func)
            f.write(f"Function: {p+1} value: {func[0]:f} \n")

            # Get the optimization function value derivatives wrt the nodes.
            f.write("   Node            X          Y          Z            dFdX       dFdY       dFdZ \n")
            for i in range(vspopt.get_number_of_nodes()):
                f.write(f"{i+1}  {vspopt.node_x(i):10.6e} {vspopt.node_y(i):10.6e} {vspopt.node_z(i):10.6e}   {vspopt.gradient_x(p, i):10.6e} {vspopt.gradient_y(p, i):10.6e} {vspopt.gradient_z(p, i):10.6e}\n")
    

if __name__ == "__main__":
    from sys import argv
    try:
        fname_prefix = argv[1]
    except IndexError:
        print("error: no filename prefix provided")
        print(f"usage: {argv[0]} <filename_prefix>")
        exit(1)

    main(fname_prefix)
