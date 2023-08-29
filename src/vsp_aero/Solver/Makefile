include ../config.mk

options:
	@echo VSPAERO build options:
	@echo "CXX = $(CXX)"
	@echo "SOLVER_CXXFLAGS = $(SOLVER_CXXFLAGS)"
	@echo "SOLVER_LDFLAGS = $(SOLVER_LDFLAGS)"
	@echo "AR = $(AR)"
	@echo "ARFLAGS = $(ARFLAGS)"
	@echo "OPENMP_CXXFLAGS = $(OPENMP_CXXFLAGS)"
	@echo "OPENMP_LDFLAGS = $(OPENMP_LDFLAGS)"
	@echo "ADEPT_CXXFLAGS = $(ADEPT_CXXFLAGS)"
	@echo "ADEPT_LDFLAGS = $(ADEPT_LDFLAGS)"

all: vspaero vspaero_adjoint vspaero_complex vspaero_opt

VSPAERO_SRCS = matrix.C	        \
               utils.C			     \
               quat.C			     \
               Vortex_Trail.C		\
               Vortex_Bound.C		\
               Vortex_Sheet.C		\
               VSP_Geom.C		\
               VSP_Edge.C		      \
               VSP_Grid.C	    	   \
               VSP_Node.C		       \
               VSP_Loop.C          \
               VSP_Solver.C		   \
               RotorDisk.C		    \
               VSP_Agglom.C		   \
               time.C 			\
               FastMatrix.C 			\
               FEM_Node.C    \
               ControlSurface.C    \
               ControlSurfaceGroup.C    \
               MatPrecon.C			\
               Gradient.C			\
               InteractionLoop.C   \
               VortexSheetInteractionLoop.C   \
               VortexSheetVortex_To_VortexInteractionSet.C \
               MergeSort.C			\
               SpanLoadData.C		       	\
               SpanLoadRotorData.C       	\
               ComponentGroup.C		\
               SearchLeaf.C			\
               Search.C			\
               WOPWOP.C			\
               VSPAERO_TYPES.C  		\
               BoundaryConditionData.C 	\
		       QuadNode.C			\
		       QuadEdge.C			\
		       QuadCell.C			\
		       QuadTree.C			\
		       EngineFace.C			\
               OptimizationFunction.C \
               AdjointGradient.C \
               vspaero.C

VSPAERO_OPTIMIZER_SRCS = VSP_Optimizer.C vspaero_opt.C

# Inspired by https://stackoverflow.com/questions/42271096/makefile-multiple-targets-from-same-source-file-with-different-flags
VSPAERO_SOLVER_OBJS = $(VSPAERO_SRCS:.C=.vspaero.o)
VSPAERO_ADJOINT_OBJS = $(VSPAERO_SRCS:.C=.adjoint.o)
VSPAERO_COMPLEX_OBJS = $(VSPAERO_SRCS:.C=.complex.o)
VSPAERO_OPTIMIZER_OBJS = $(VSPAERO_OPTIMIZER_SRCS:.C=.optimizer.o)

VSPAERO_SOLVER_DEFINES = -DMYTIME
VSPAERO_ADJOINT_DEFINES = -DMYTIME -DAUTODIFF
VSPAERO_COMPLEX_DEFINES = -DMYTIME -DCOMPLEXDIFF
VSPAERO_OPTIMIZER_DEFINES = -DMYTIME

VSPAERO_SOLVER_CXXFLAGS = $(SOLVER_CXXFLAGS) $(OPENMP_CXXFLAGS)
VSPAERO_ADJOINT_CXXFLAGS = $(SOLVER_CXXFLAGS) $(ADEPT_CXXFLAGS)
VSPAERO_COMPLEX_CXXFLAGS = $(SOLVER_CXXFLAGS) $(OPENMP_CXXFLAGS)
VSPAERO_OPTIMIZER_CXXFLAGS = $(SOLVER_CXXFLAGS) $(OPENMP_CXXFLAGS) $(ADEPT_CXXFLAGS)

VSPAERO_SOLVER_LDFLAGS = $(SOLVER_LDFLAGS) $(OPENMP_LDFLAGS)
VSPAERO_ADJOINT_LDFLAGS = $(SOLVER_LDFLAGS) $(ADEPT_LDFLAGS)
VSPAERO_COMPLEX_LDFLAGS = $(SOLVER_LDFLAGS) $(OPENMP_LDFLAGS)
VSPAERO_OPTIMIZER_LDFLAGS = $(SOLVER_LDFLAGS) $(OPENMP_LDFLAGS) $(ADEPT_LDFLAGS)

# TODO: it's apparently possible to include header files in the rule dependencies: https://stackoverflow.com/questions/2394609/makefile-header-dependencies
%.vspaero.o: %.C
	$(CXX) $(VSPAERO_SOLVER_CXXFLAGS) $(VSPAERO_SOLVER_DEFINES) -c $^ -o $@

%.adjoint.o: %.C
	$(CXX) $(VSPAERO_ADJOINT_CXXFLAGS) $(VSPAERO_ADJOINT_DEFINES) -c $^ -o $@

%.complex.o: %.C
	$(CXX) $(VSPAERO_COMPLEX_CXXFLAGS) $(VSPAERO_COMPLEX_DEFINES) -c $^ -o $@

%.optimizer.o: %.C
	$(CXX) $(VSPAERO_OPTIMIZER_CXXFLAGS) $(VSPAERO_OPTIMIZER_DEFINES) -c $^ -o $@

vspaero: $(VSPAERO_SOLVER_OBJS)
	$(CXX) $(VSPAERO_SOLVER_CXXFLAGS) $^ $(VSPAERO_SOLVER_LDFLAGS) -o $@

vspaero_adjoint: $(VSPAERO_ADJOINT_OBJS)
	$(CXX) $(VSPAERO_ADJOINT_CXXFLAGS) $^ $(VSPAERO_ADJOINT_LDFLAGS) -o $@

vspaero_complex: $(VSPAERO_COMPLEX_OBJS)
	$(CXX) $(VSPAERO_COMPLEX_CXXFLAGS) $^ $(VSPAERO_COMPLEX_LDFLAGS) -o $@

solverlib.a: $(VSPAERO_SOLVER_OBJS)
	$(AR) $(ARFLAGS) $@ $^

adjointlib.a: $(VSPAERO_ADJOINT_OBJS)
	$(AR) $(ARFLAGS) $@ $^

libvspaero.so: $(VSPAERO_SOLVER_OBJS)
	$(CXX) $(VSPAERO_SOLVER_CXXFLAGS) $(VSPAERO_SOLVER_LDFLAGS) -shared $^ -o $@

libvspaero_adjoint.so: $(VSPAERO_ADJOINT_OBJS)
	$(CXX) $(VSPAERO_ADJOINT_CXXFLAGS) $(VSPAERO_ADJOINT_LDFLAGS) -shared $^ -o $@

vspaero_opt: $(VSPAERO_OPTIMIZER_OBJS) solverlib.a adjointlib.a
	$(CXX) $(VSPAERO_OPTIMIZER_CXXFLAGS) $^ $(VSPAERO_OPTIMIZER_LDFLAGS) -o $@

clean:
	rm -f $(VSPAERO_SOLVER_OBJS) $(VSPAERO_ADJOINT_OBJS) $(VSPAERO_COMPLEX_OBJS) $(VSPAERO_OPTIMIZER_OBJS)
	rm -f vspaero vspaero_adjoint vspaero_complex solverlib.a adjointlib.a vspaero_opt

# https://www.gnu.org/software/make/manual/html_node/Phony-Targets.html
.PHONY: all clean options
