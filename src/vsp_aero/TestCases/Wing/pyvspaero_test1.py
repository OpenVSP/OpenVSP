import numpy as np
from vspaero.optimizer import PyVSPOptimizer
v = PyVSPOptimizer("hershey")
print(f"v.nthreads = {v.nthreads}")
v.nthreads = 3
print(f"v.nthreads = {v.nthreads}")
print(f"v.nnodes = {v.nnodes}")
print(f"v.nloops = {v.nloops}")
print(f"v.noutputs = {v.noutputs}")
v.noutputs = 1
print(f"v.noutputs = {v.noutputs}")

for i in range(v.nnodes):
    print(f"v.nodex({i})= {v.nodex(i)}")
    print(f"v.nodey({i})= {v.nodey(i)}")
    print(f"v.nodez({i})= {v.nodez(i)}")

nodes = np.zeros((v.nnodes, 3))
for i in range(v.nnodes):
    nodes[i, 0] = v.nodex(i)
    nodes[i, 1] = v.nodey(i)
    nodes[i, 2] = v.nodez(i)

nodes += 100.0
v.update_geometry(nodes)

# Undo the changes.
nodes -= 100.0
v.update_geometry(nodes)

# Let's see if we can actually do something.
# v._solve_forward()

# for i in range(v.nnodes):
#     print(f"v.nodex({i})= {v.nodex(i)}")
#     print(f"v.nodey({i})= {v.nodey(i)}")
#     print(f"v.nodez({i})= {v.nodez(i)}")
