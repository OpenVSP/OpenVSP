include config.mk

# We want to pass these variables down to sub-makes (`$(MAKE) -C Solver` etc.), so export them:
# https://www.gnu.org/software/make/manual/html_node/Variables_002fRecursion.html
# Maybe it would be smarter to pass them as arguments to the sub-make (`$(MAKE) -C Solver CXXFLAGS=$(SOLVER_CXXFLAGS)`)?
# But then I wouldn't be able to build just, say, the stuff in `Solver/` or whatever with the flags set in `config.mk`.
export ARFLAGS OPENMP_CXXFLAGS OPENMP_LDFLAGS ADEPT_CXXFLAGS ADEPT_LDFLAGS FLTK_CXXFLAGS FLTK_LDFLAGS
export SOLVER_CXXFLAGS SOLVER_LDFLAGS VIEWER_CXXFLAGS VIEWER_LDFLAGS ADB2LOADS_CXXFLAGS ADB2LOADS_LDFLAGS

all: options
	$(MAKE) -C Solver all
	$(MAKE) -C Viewer all
	$(MAKE) -C Adb2Load all

config.mk:
	@echo "WARNING: no config.mk file found."
	@echo "Using an empty config.mk, which will likely fail."
	@echo ""
	@echo "Before compiling, copy an existing config file from the "; \
	echo "Config/Defaults/ directory to the vspaero root directory and  "; \
	echo "rename to config.mk. For example:"; \
	echo " ";\
	echo "  cp Config/Defaults/config.LINUX_GNU.mk ./config.mk"; \
	echo " ";\
	echo "The modify this config file as required. With the config file specified, rerun "; \
	echo "'make' and the build will start"; \
	touch $@

options:
	@echo VSPAERO build options:
	@echo "CXX = $(CXX)"
	@echo "SOLVER_CXXFLAGS = $(SOLVER_CXXFLAGS)"
	@echo "SOLVER_LDFLAGS = $(SOLVER_LDFLAGS)"
	@echo "VIEWER_CXXFLAGS = $(VIEWER_CXXFLAGS)"
	@echo "VIEWER_LDFLAGS = $(VIEWER_LDFLAGS)"
	@echo "ADB2LOADS_CXXFLAGS = $(ADB2LOADS_CXXFLAGS)"
	@echo "ADB2LOADS_LDFLAGS = $(ADB2LOADS_LDFLAGS)"
	@echo "AR = $(AR)"
	@echo "ARFLAGS = $(ARFLAGS)"
	@echo "OPENMP_CXXFLAGS = $(OPENMP_CXXFLAGS)"
	@echo "OPENMP_LDFLAGS = $(OPENMP_LDFLAGS)"
	@echo "ADEPT_CXXFLAGS = $(ADEPT_CXXFLAGS)"
	@echo "ADEPT_LDFLAGS = $(ADEPT_LDFLAGS)"
	@echo "FLTK_CXXFLAGS = $(FLTK_CXXFLAGS)"
	@echo "FLTK_LDFLAGS = $(FLTK_LDFLAGS)"

vspaero: options
	$(MAKE) -C Solver vspaero

vspaero_adjoint: options
	$(MAKE) -C Solver vspaero_adjoint

vspaero_complex: options
	$(MAKE) -C Solver vspaero_complex

vspaero_opt: options
	$(MAKE) -C Solver vspaero_opt

viewer: options
	$(MAKE) -C Viewer viewer

adb2loads: options
	$(MAKE) -C Adb2Load adb2loads

docs:
	cd Documentation; $(DOXYGEN) DoxConfigFile

install: all
	mkdir -p $(DESTDIR)$(BINDIR)
	cp -f Solver/vspaero $(DESTDIR)$(BINDIR)
	cp -f Solver/vspaero_adjoint $(DESTDIR)$(BINDIR)
	cp -f Solver/vspaero_complex $(DESTDIR)$(BINDIR)
	cp -f Solver/vspaero_opt $(DESTDIR)$(BINDIR)
	cp -f Viewer/viewer $(DESTDIR)$(BINDIR)
	cp -f Adb2Load/adb2loads $(DESTDIR)$(BINDIR)
	mkdir -p $(DESTDIR)$(LIBDIR)
	cp -f Solver/solverlib.a $(DESTDIR)$(LIBDIR)
	cp -f Solver/adjointlib.a $(DESTDIR)$(LIBDIR)

uninstall:
	rm -f $(DESTDIR)$(BINDIR)/vspaero
	rm -f $(DESTDIR)$(BINDIR)/vspaero_adjoint
	rm -f $(DESTDIR)$(BINDIR)/vspaero_complex
	rm -f $(DESTDIR)$(BINDIR)/vspaero_opt
	rm -f $(DESTDIR)$(BINDIR)/viewer
	- rmdir $(DESTDIR)$(BINDIR)
	rm -f $(DESTDIR)$(LIBDIR)/solverlib.a
	rm -f $(DESTDIR)$(LIBDIR)/adjointlib.a
	- rmdir $(DESTDIR)$(LIBDIR)

clean:
	$(MAKE) -C Solver clean
	$(MAKE) -C Viewer clean
	$(MAKE) -C Adb2Load clean

# test_wing: vspaero vspaero_adjoint
# 	@cd TestCases/Wing && \
# 	../../Solver/vspaero -opt -omp 4 -optfunction 1 hershey > vspaero.stdout && \
# 	../../Solver/vspaero_adjoint -adjoint -opt -optfunction 1 hershey > vspaero_adjoint.stdout && \
# 	echo "Wing test passed!"

# test_rotor: vspaero vspaero_adjoint
# 	@cd TestCases/Rotor && \
# 	../../Solver/vspaero -unsteady -opt -omp 4 -optfunction 2 prop > vspaero.stdout && \
# 	../../Solver/vspaero_adjoint -unsteady -opt -adjoint -optfunction 2 prop > vspaero_adjoint.stdout && \
# 	echo "Rotor test passed!"

# test: test_wing test_rotor

.PHONY: options docs all vspaero vspaero_adjoint vspaero_complex vspaero_opt viewer install uninstall clean test_wing test_rotor test
