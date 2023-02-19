include ../config.mk

all: viewer

options:
	@echo VSPAERO build options:
	@echo "CXX = $(CXX)"
	@echo "VIEWER_CXXFLAGS = $(VIEWER_CXXFLAGS)"
	@echo "VIEWER_LDFLAGS = $(VIEWER_LDFLAGS)"
	@echo "FLTK_CXXFLAGS = $(FLTK_CXXFLAGS)"
	@echo "FLTK_LDFLAGS = $(FLTK_LDFLAGS)"
	@echo "VSPAERO_VIEWER_CXXFLAGS = $(VSPAERO_VIEWER_CXXFLAGS)"
	@echo "VSPAERO_VIEWER_LDFLAGS = $(VSPAERO_VIEWER_LDFLAGS)"

VSPAERO_VIEWER_SRCS	= vspaero_viewer.C    \
					  glf.C		    	\
					  glviewer.C		\
					  trackball.C		\
					  utils.C			\
					  viewerUI.C	    	\
					  ContourLevel.C		\
					  CuttingPlane.C		\
					  binaryio.C	        \
					  RotorDisk.C		\
					  VSP_DegenGeom.C		\
					  VSP_Body.C		\
					  VSP_Wing.C		\
					  VSP_Grid.C		\
					  VSP_Tris.C		\
					  VSP_Node.C		\
					  VSP_Edge.C		\
					  VSP_Agglom.C		\
					  FEM_Node.C		\
					  Optimization_Node.C \
					  stb.C			\
					  EngineFace.C		\
					  quat.C

VSPAERO_VIEWER_OBJS = $(VSPAERO_VIEWER_SRCS:.C=.o)
VSPAERO_VIEWER_DEFINES = -DNDEBUG -DDO_GRAPHICS -DOCTREE_DO_GRAPHICS

VSPAERO_VIEWER_CXXFLAGS = $(VIEWER_CXXFLAGS) $(FLTK_CXXFLAGS)
VSPAERO_VIEWER_LDFLAGS = $(VIEWER_LDFLAGS) $(FLTK_LDFLAGS)

%.o: %.C
	$(CXX) $(VSPAERO_VIEWER_CXXFLAGS) $(VSPAERO_VIEWER_DEFINES) -c $^ -o $@

viewer: $(VSPAERO_VIEWER_OBJS)
	$(CXX) $(VSPAERO_VIEWER_CXXFLAGS) $^ $(VSPAERO_VIEWER_LDFLAGS) -o $@

clean:
	rm -f $(VSPAERO_VIEWER_OBJS)
	rm -f viewer

# https://www.gnu.org/software/make/manual/html_node/Phony-Targets.html
.PHONY: all clean options
