import numpy as np
from vspaero.optimizer import pyVSPOptimizer
from vspaero.functions import CL

def main(fname_prefix):
    vspopt = pyVSPOptimizer()

    vspopt.set_unsteady_analysis(0)
    vspopt.set_number_of_optimization_functions(1)
    vspopt.set_optimization_functions(np.array([CL], dtype=np.int32))
    vspopt.setup(fname_prefix)
    vspopt.solve()

    with open(f"{fname_prefix}.pyvspaero.gradient", mode="w") as f:
        f.write(f"{vspopt.get_number_of_nodes()} \n")
        for p in range(vspopt.get_number_of_optimization_functions()):
            func = np.empty(1, dtype=np.float64)
            vspopt.get_function_value(p, func)
            f.write(f"Function: {p} value: {func[0]:f} \n")
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
