#ifndef GL_VIEWER_H
#define GL_VIEWER_H

#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#endif

#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <sys/stat.h>

#ifndef WIN32
#include <unistd.h>
#endif

#include <time.h>

#include <FL/Fl.H>
#include <stdlib.h>

#include <FL/Fl_Gl_Window.H>
#include <FL/gl.h>
#include <FL/fl_ask.H>

#include <FL/Fl_Browser.H>
#include <FL/Fl_Button.H>
#include <FL/forms.H>

#ifdef __APPLE__

//#include <OpenGL/glu.h>
//#include <GLUT/glut.h>

#else

//#include <GL/glu.h>
//#include <GL/glut.h>

#endif

#include "trackball.H"
#include "utils.H"
#include "glf.H"
#include "binaryio.H"
#include "viewerUI.h"
#include "TagList.H"
#include "TagListGroup.H"
#include "surfIDs.H"
#include "RotorDisk.H"
#include "PropElement.H"
#include "ControlSurface.H"
#include "VSP_DegenGeom.H"
#include "FEM_Node.H"
#include "Optimization_Node.H"

#include "stb_image_write.h"

//  Define marked tri types

#define SRF_TRI      5
#define GOMGEOM_TRI 10

#define TORAD 3.141592/180.

#define SYM_X 1
#define SYM_Y 2
#define SYM_Z 3

#define   VLM_MODEL 1
#define PANEL_MODEL 2

#define IMPULSE_ANALYSIS 1
#define HEAVE_ANALYSIS   2
#define P_ANALYSIS       3
#define Q_ANALYSIS       4
#define R_ANALYSIS       5
#define PATH_ANALYSIS    6

// Forward declarations

class viewerUI;

// Triangle Structure

struct TRI_STRUCT
{

	int node1;
	int node2;
	int node3;

	int edge1;
	int edge2;
	int edge3;

	int surface_id;
   int surface_type;
	float area;

}; typedef struct TRI_STRUCT TRI;

// Node Structure

struct NODE_STRUCT
{

	float x;
	float y;
	float z;
   
   int SurfID;

}; typedef struct NODE_STRUCT NODE;

struct EDGE_STRUCT
{

	int node1;
	int node2;

   int tri1;
   int tri2;
   
   int SurfaceID;
   
   int IsKuttaEdge;
   
   int IsBoundaryEdge;

}; typedef struct EDGE_STRUCT EDGE;

// Small class for building edge list

class GL_EDGE_ENTRY {

public:

    int node;
    int tri_1;
    int tri_2;
    int next;

};

// Small class for solution list

class SOLUTION_CASE {

public:

    float Mach;
    float Alpha;
    float Beta;
    char CommentLine[2000];

};

// The GL_VIEWER Class definition

class GL_VIEWER : public Fl_Gl_Window {

private:

    // File version

    int FILE_VERSION;
    
    // Model type
    
    int ModelType;
    
    // Symmetry Flag
    
    int SymmetryFlag;
    
    // Unsteady Analysis Type
    
    int TimeAccurate_;
    
    // Cpmin, and Cpmax
    
    int UserSetPlotLimits;
    
    float CpMinSoln;
    float CpMaxSoln;

    // Rotation and translation data

    float vAng;
    float hAng;
    float zAng;
    float xshift;
    float yshift;

    double size;
    float Brightness;

    int OldX, OldY;

    int NewX,  NewY;

    float OldScale, NewScale;

    float NewTransX, NewTransY;

    float OldTransX, OldTransY;

    float OldQuat[4], NewQuat[4];

    int FAST_DRAW_ON;
    
    // VSP Geometry
    
    VSP_DEGEN_GEOM VSPGeom_;
    
    VSP_DEGEN_GEOM &VSPGeom(void) { return VSPGeom_; };

    // Aerothermal database data
    
    int NumberOfADBCases_;
    int UserSelectedSolutionCase_;
    
    SOLUTION_CASE *ADBCaseList_;

    char file_name[2000];
    char file_name_no_path[2000];
    char path[200];

    float Sref;
    float Cref;
    float Bref;
    float Xcg;
    float Ycg;
    float Zcg;
    float ScaleFactor;
    
    int NumberOfSurfaces_;
    int NumberOfWings_;
    int NumberOfBodies_;
    int NumberOfCart3dSurfaces_;
    
    char **BodyListName_;
    char **WingListName_;
    char **Cart3dListName_;

    int *WingGroupID_;
    int *BodyGroupID_;
    
    int NumberOfMachs;
    int NumberOfBetas;
    int NumberOfAlphas;

    int UserChoiceMach;
    int UserChoiceBeta;
    int UserChoiceAlpha;

    int CurrentChoiceMach;
    int CurrentChoiceBeta;
    int CurrentChoiceAlpha;

    float CurrentEdgeMach;
    float CurrentBeta;
    float CurrentAlpha;

    float *MachList;
    float *BetaList;
    float *AlphaList;

    float *Cp, *CpSteady, *CpUnsteady, *Gamma;
    double *GammaN, *dCp_Unsteady, *Fx, *Fy, *Fz;
    double *U, *V, *W;
    
    int NumberOfLineContourLevels;
    
    float *TempTriangleArray;
    float *TempNodalArray;
    float *TempTotalArea;
    
    float *LastFunction;
    float NodalMin, NodalMax;

    int OctreeCutDirection;
    float OctreeCutValue;
    
    int NumberOfTrailingVortexEdges_;
    int NumberOfSubVortexNodes_;
    int DrawWakesColored_;
    int *WingWake_;
    int MaxWings_;
    float **XWake_;
    float **YWake_;
    float **ZWake_;
    float *SWake_;
    
    // Propulsion element data
    
    int NumberOfPropulsionElements;
    int MaxNumberOfPropulsionElements;
    PROPULSION_ELEMENT *PropulsionElement;
    
    // Control Surface data
    
    int NumberOfControlSurfaces;
    int *ControlSurfaceLoop;
    CONTROL_SURFACE *ControlSurface;
    
    void RotateControlSurfaceNode( float xyz[3], int ConSurf );

    // BEAM3D data
    
    void LoadBeam3DFEMData(void);
    void LoadFEMDeformationData(int i, char *FileName);
    void DrawBEAM3DFEM(void);

    FEM_NODE *FemData_;
    FEM_NODE &FemData(int i) { return FemData_[i]; };
    
    // Adjoint data
    
    OPT_NODE OptimizationData_;
    
    void LoadAdjointandGradients(void);

    // Adjust center of geometry for better rotations

    float GeometryXShift;
    float GeometryYShift;
    float GeometryZShift;

    float XMin, XMax;
    float YMin, YMax;
    float ZMin, ZMax;
    float ViewSize;
    float ZoomScale;

    // Surface ID colors

    float SurfaceID_R[20001];
    float SurfaceID_G[20001];
    float SurfaceID_B[20001];

    float CpMinActual, CpMaxActual;
    float CpMin, CpMax;

    float CpSteadyMinActual, CpSteadyMaxActual;
    float CpSteadyMin, CpSteadyMax;
    
    float CpUnsteadyMinActual, CpUnsteadyMaxActual;
    float CpUnsteadyMin, CpUnsteadyMax;
    
    float GammaMinActual, GammaMaxActual;
    float GammaMin, GammaMax;      
      
    char LegendTitle[80];
    float LegendMinClip, LegendMaxClip;
    float LegendMin, LegendMax;

    int UseEnglishUnits;

    // Data for tris and nodes

    int NumberOfVortexLoops;
    int NumberOfTris;
    int NumberOfNodes;
    int NumberOfEdges;
    int NumberOfSurfaceVortexEdges;

    TRI *TriList;
    NODE *NodeList;
    EDGE *EdgeList;
    
    int NumberOfMeshLevels;
    int *NumberOfCourseNodesForLevel;
    int *NumberOfCourseEdgesForLevel;
    NODE **CoarseNodeList;
    EDGE **CoarseEdgeList;
    
    int NumberOfKuttaEdges;
    int NumberOfKuttaNodes;

    // Surface Normals

    float *Nx;
    float *Ny;
    float *Nz;

    // Node, area averaged, surface Normals

    float *Nodal_Nx;
    float *Nodal_Ny;
    float *Nodal_Nz;
    
    void CalculateSurfaceNormals(int Case);

    void CreateTriEdges(void);
    
    // What to draw

    int DrawSmoothShadeIsOn;
    int DrawPerTriFunctionIsOn;
    int DrawSmoothFunctionsIsOn;
    int DrawLineContoursIsOn;

    int DrawReflectedGeometryIsOn;

    int DrawLegendIsOn;
    int DrawLabelIsOn;
    int DrawWithWhiteBackgroundIsOn;

    int DrawShadedIsOn;
    int DrawWireFrameIsOn;
    int DrawCoarseMeshesIsOn;
    int DrawOnlySelectedIsOn;
    
    int CoarseMeshLevelSelected;
    
    int DrawFlatShadedIsOn;
    
    int DrawCGLabelIsOn;

    int DrawCGIsOn;
    
    int DrawAxesIsOn;
    
    int DrawWakesIsOn;
    
    int DrawWakeLinesIsOn;
    
    int DrawWakePointsIsOn;

    int DrawPropulsionElementsIsOn;    
        
    int DrawWakesToInfinityIsOn;
    
    int DrawControlSurfacesIsOn;
    
    int DrawControlSurfacesDeflectedIsOn;
    
    int DrawXPlaneIsOn;
    
    int DrawYPlaneIsOn;
    
    int DrawZPlaneIsOn;
    
    int DrawBEAM3DFEMIsOn;

    // Panel Group

    int DrawSurfaceRegionsIsOn;
    int DrawSurfaceRegionsShadedIsOn;
    int DrawSurfaceRegionsBordersIsOn;

    int DrawComGeomTagsIsOn;
    int DrawComGeomTagsShadedIsOn;
    int DrawComGeomTagsBordersIsOn;

    int DrawCpIsOn;
    int DrawCpSteadyIsOn;
    int DrawCpUnsteadyIsOn;
    int DrawGammaIsOn;
    void DrawRotorSurfacesShaded(void);

    void percent_to_rgb(float per, float col[3], int colmap);
    void SetSurfaceRegionColor(int SurfaceRegionID);
    void SetTagSurfaceColor(int SurfaceID, int MaxVals);
    void SetTagSurfaceColor(int SurfaceID, int MaxVals, double Alpha);
    void SetControlSurfaceColor(int ControlSurfaceID);
    void SetMaterialTypeColor(int MaterialID);

    float BackgroundRGBA[4];
    float TextRGBA[4];

    // Font stuff

    void WriteFontFile(void);

    // Support code

    void LoadExistingSolutionData(int Case);
    void LoadExistingGradientSolutionData(void);

    // Pointer to the vui

    viewerUI *vui;

    Fl_Browser *MachCaseBrowser;
    Fl_Browser *DynamicPressureCaseBrowser;
    Fl_Browser *AngleOfAttackCaseBrowser;
    
    Fl_Browser *PanelComGeomTagsBrowser;

    // Comgeom II panel data

    TAG_LIST PanelTagList;

    int *ComGeom2PanelTag;

    // OpengGL code

    void DrawBBox(void);
    void DrawWireFrame(void);
    void DrawShadedSurface(void);
    void DrawCoarseMeshNodesForLevel(int Level);
    void DrawCoarseMeshEdgesForLevel(int Level);
    void DrawShadedLineContours(float *Function, float FMin, float FMax);
    
    void DrawWakes(void);
    
    void DrawControlSurfaces(void);

    void DrawShadedSolution(float *Function, float FMin, float FMax);

    void DrawShadedSolutionPerTri(float *Function, float FMin, float FMax);

    void DrawShadedSolutionPerNode(float *Function, float FMin, float FMax);

    void DrawCp(void);
    void DrawCpSteady(void);
    void DrawCpUnsteady(void);
    void DrawGamma(void);
 
    void Draw(void);
    void DrawLegend(void);
    void DrawLabel(void);
    void DrawCGLabel(void);
    void DrawCGMarker(void);
    void DrawAxes(void);
    void DrawSymmetryPlane(void);
    
    void DrawCurrentSelectedComGeomRegions(void);

    // EdgeMach, Q, Alpha cases inputted by user

    int UserTouchedEdgeMach;
    int UserTouchedBeta;
    int UserTouchedAlpha;

    float UserInputtedEdgeMach;
    float UserInputtedBeta;
    float UserInputtedAlpha;

    // I/O Code

    void LoadMeshData(void);
    void UpdateMeshData(FILE *adb_file);
    void LoadCaseFile(char *FileName);
 
    // Allows byte swapping on read/writes of binary files
    // so we can deal with endian issues across platforms

    int ByteSwapForADB;
 
    // ADB file pointers

    fpos_t StartOfWallTemperatureData;

    // Write out a png file

    int WritePNG(char *filename, char *description,
                  int x, int y, int width, int height, int compression);

public:

    // Constructor, Destructor, Copy

    GL_VIEWER(int x,int y,int w,int h,const char *l=0);
   ~GL_VIEWER(void);

// Compatibility for FLTK before 1.3.4
#if FL_API_VERSION < 10304
    int pixel_w() { return w(); }
    int pixel_h() { return h(); }
#endif

    // Current file name

    char *CurrentFileName(void) { return file_name; };

    // Load in the mesh data

    void LoadInitialData(char *name);
    void LoadSolutionCaseList(void);
    void LoadSolutionData(void);

    void draw(void) { Draw(); };

    // Specific view code

    void ResetView(void);
    void FitToWindow(void);
    void TopView(void);
    void BottomView(void);
    void LeftView(void);
    void RightView(void);
    void FrontView(void);
    void RearView(void);
    void IsoViewUp(void);
    void IsoViewDown(void);

    void ZeroAllViews(void);

    // Run the script

    void RunScript(char *ScriptName, viewerUI *vui);

    // Write out a png file

    void WritePNGFile(char *FileName);
    void WritePNGFile(char *Path, char *FileName);

    void WriteMovieFrame(char *FileName);

    // Event handler

    int handle(int event);

    // Access functions

    void SetSize(float InputSize) { size = InputSize; };

    void v_angle(float angle) { vAng = angle;  };
    float v_angle(){return vAng;};

    void h_angle(float angle) { hAng = angle; };
    float h_angle(){return hAng;};

    void panx(float x) {xshift = x; };
    void pany(float y) {yshift = y; };

    // Set up EdgeMach, Q, Alpha case

    void FindMeshMinMax(void);
    void FindSolutionMinMax(void);
    void FindSolutionMinMax(float *Function, float &FMinActual, float &FmaxActual, float &FMin, float &FMax);
    void FixViewingBox(float x1, float x2, float y1, float y2, float z1, float z2 );

    int ForceView_;
    float BoxSize_;

    int DrawTimeStampIsOn;
    char TimeLabel_[1000];

    void FixViewBox(float BoxSize) { ForceView_ = 1; BoxSize_ = BoxSize; };

    void SetTimeStampLabel(char *Label) { DrawTimeStampIsOn = 1 ; sprintf(TimeLabel_,"%s",Label); };

    // Script File I/O

    FILE *ScriptFile;

    int WriteScriptFile;

    void StartScript(void);

    void ToggleFlag(int &Flag, const char *Text);
    
    void Exit(int Flag) { exit(Flag); };

    // What to draw

    void DrawSmoothShadeWasPicked(void)             { ToggleFlag(DrawSmoothShadeIsOn,                           "DrawSmoothShadeIs"); };

    void DrawPerTriFunctionsWasPicked(void)         { ToggleFlag(DrawPerTriFunctionIsOn,                        "DrawPerTriFunctionIs");  if ( DrawPerTriFunctionIsOn  ) {DrawSmoothFunctionsIsOn =   DrawLineContoursIsOn = 0; } };
    void DrawSmoothFunctionsWasPicked(void)         { ToggleFlag(DrawSmoothFunctionsIsOn,                       "DrawSmoothFunctionsIs"); if ( DrawSmoothFunctionsIsOn ) { DrawPerTriFunctionIsOn =   DrawLineContoursIsOn = 0; } };
    void DrawLineContoursWasPicked(void)            { ToggleFlag(DrawLineContoursIsOn,                          "DrawLineContoursIs");    if ( DrawLineContoursIsOn    ) {DrawSmoothFunctionsIsOn = DrawPerTriFunctionIsOn = 0; } };
    
    void DrawReflectedGeometryWasPicked(void)       { ToggleFlag(DrawReflectedGeometryIsOn,                     "DrawReflectedGeometryIs"); };

    void DrawLegendWasPicked(void)                  { ToggleFlag(DrawLegendIsOn,                                "DrawLegendIs"); };
    void DrawLabelWasPicked(void)                   { ToggleFlag(DrawLabelIsOn,                                 "DrawLabelIs"); };
    void DrawCGLabelWasPicked(void)                 { ToggleFlag(DrawCGLabelIsOn,                               "DrawCGLabelIs"); };
    void DrawWhiteBackgroundWasPicked(void)         { ToggleFlag(DrawWithWhiteBackgroundIsOn,                   "DrawWithWhiteBackgroundIs");};
    void DrawFlatShadedWasPicked(void)              { ToggleFlag(DrawFlatShadedIsOn,                            "DrawFlatShadedIs"); };
    
    void DrawCGWasPicked(void)                      { ToggleFlag(DrawCGIsOn,                                    "DrawCGIs");     };
    void DrawAxesWasPicked(void)                    { ToggleFlag(DrawAxesIsOn,                                  "DrawAxesIs");     };

    void DrawWakesWasPicked(void)                   { ToggleFlag(DrawWakesIsOn,                                 "DrawWakesIs");     };
    void DrawWakeLinesWasPicked(void)               { ToggleFlag(DrawWakeLinesIsOn,                             "DrawWakeLinesIs");     };
    void DrawWakePointsWasPicked(void)              { ToggleFlag(DrawWakePointsIsOn,                            "DrawWakePointsIs");     };

    void DrawRotorsWasPicked(void)                  { ToggleFlag(DrawPropulsionElementsIsOn,                    "DrawPropulsionElementsIs");     };
        
    void DrawControlSurfacesWasPicked(void)          { ToggleFlag(DrawControlSurfacesIsOn,                       "DrawControlSurfacesIsOn");     };
    
    void DrawControlSurfacesDeflectedWasPicked(void) { ToggleFlag(DrawControlSurfacesDeflectedIsOn,              "DrawControlSurfacesDeflectedIsOn");     };
    
    void DrawWakesToInfinityWasPicked(void)          { ToggleFlag(DrawWakesToInfinityIsOn,                       "DrawWakesToInfinityIsOn");     };
    
    void DrawXPlaneWasPicked(void)                   { ToggleFlag(DrawXPlaneIsOn,                       "DrawXPlaneIsOn");  DrawYPlaneIsOn = DrawZPlaneIsOn = 0;  };
    
    void DrawYPlaneWasPicked(void)                   { ToggleFlag(DrawYPlaneIsOn,                       "DrawYPlaneIsOn");  DrawXPlaneIsOn = DrawZPlaneIsOn = 0;  };

    void DrawZPlaneWasPicked(void)                   { ToggleFlag(DrawZPlaneIsOn,                       "DrawZPlaneIsOn");  DrawXPlaneIsOn = DrawYPlaneIsOn = 0;  };
    
    // ADB Menu

    void ClearADBFile(void);

    void DrawWireFrameWasPicked(void)                { ToggleFlag(DrawWireFrameIsOn,                             "DrawWireFrameIs"); };

    void DrawOnlySelectedWasPicked(void)             { ToggleFlag(DrawOnlySelectedIsOn,                          "DrawOnlySelectedIs");};

    void DrawShadedWasPicked(void)                   { if ( DrawShadedIsOn     == 0 ) { DrawShadedIsOn = 1; DrawCpIsOn = 0; DrawCpSteadyIsOn = 0; DrawCpUnsteadyIsOn = 0; DrawGammaIsOn = 0; }; };
    void DrawCpWasPicked(void)                       { if ( DrawCpIsOn         == 0 ) { DrawShadedIsOn = 0; DrawCpIsOn = 1; DrawCpSteadyIsOn = 0; DrawCpUnsteadyIsOn = 0; DrawGammaIsOn = 0; }; };
    void DrawCpSteadyWasPicked(void)                 { if ( DrawCpSteadyIsOn   == 0 ) { DrawShadedIsOn = 0; DrawCpIsOn = 0; DrawCpSteadyIsOn = 1; DrawCpUnsteadyIsOn = 0; DrawGammaIsOn = 0; }; };
    void DrawUnsteadyCpWasPicked(void)               { if ( DrawCpUnsteadyIsOn == 0 ) { DrawShadedIsOn = 0; DrawCpIsOn = 0; DrawCpSteadyIsOn = 0; DrawCpUnsteadyIsOn = 1; DrawGammaIsOn = 0; }; };
    void DrawGammaWasPicked(void)                    { if ( DrawGammaIsOn      == 0 ) { DrawShadedIsOn = 0; DrawCpIsOn = 0; DrawCpSteadyIsOn = 0; DrawCpUnsteadyIsOn = 0; DrawGammaIsOn = 1; }; };

    void DrawCoarseMeshWasPicked(void)               { ToggleFlag(DrawCoarseMeshesIsOn,                          "DrawCoarseMeshesIs"); };
    
    void DrawBEAM3DFEMOff(void)      { DrawBEAM3DFEMIsOn = 0; };
    void DrawBEAM3DFEMOn(void)       { DrawBEAM3DFEMIsOn = 1; };
    void DrawBEAM3DFEMDeformed(void) { DrawBEAM3DFEMIsOn = 2; };

    void SwapSurfaceNormals(void);
    
    // Solution case slider
    
    void SolutionCaseSliderWasMoved(int Case) { UserSelectedSolutionCase_ = Case; LoadSolutionData(); };
    
    // Access the number of solutions in the adb file
    
    int GetNumberOfADBCases(void) { return NumberOfADBCases_; };
    
    void GetCommentTextString(char *c) { sprintf(c,"%s",ADBCaseList_[UserSelectedSolutionCase_].CommentLine); };
                                                       
    // Access size of data set

    int GetNumberOfMachs(void) { return NumberOfMachs; };
    int GetNumberOfBetas(void) { return NumberOfBetas; };
    int GetNumberOfAlphas(void) { return NumberOfAlphas; };

    // Update the value of the EdgeMach, Q, Alpha case the user wants to view

    void  MachNumberWasChangedTo(int  EdgeMach) { UserTouchedEdgeMach  = 0 ; UserChoiceMach  = EdgeMach;  };
    void  BetaNumberWasChangedTo(int  Beta) { UserTouchedBeta  = 0 ; UserChoiceBeta  = Beta; };
    void AlphaNumberWasChangedTo(int Alpha) { UserTouchedAlpha = 0 ; UserChoiceAlpha = Alpha; };

    // Access actual EdgeMach values for each case

    void GetMachText(char *c)  { sprintf(c,"%2.9f", MachList[UserChoiceMach])         ; };
    void GetBetaText(char *c)  { sprintf(c,"%2.9f", BetaList[UserChoiceBeta])         ; };
    void GetAlphaText(char *c) { sprintf(c,"%2.9f", AlphaList[UserChoiceAlpha])       ; };

    // EdgeMach, Q, Alpha cases inputted by user

    void MachWasTouched(void)   { UserTouchedEdgeMach  = 1; };
    void BetaWasTouched(void)   { UserTouchedBeta  = 1; };
    void AlphaWasTouched(void ) { UserTouchedAlpha = 1; };

    void  SetEdgeMachTo(float EdgeMach)  { UserInputtedEdgeMach  = EdgeMach;   };
    void  SetBetaTo(float Beta)  { UserInputtedBeta  = Beta;   };
    void SetAlphaTo(float Alpha) { UserInputtedAlpha = Alpha;  };

    // Return actual EdgeMach, Q, Alpha after a load data set or interpolation operation

    void GetCurrentMachText(char *c)  { sprintf(c,"%3.3f", CurrentEdgeMach)  ; };
    void GetCurrentBetaText(char *c) { sprintf(c,"%3.3f", CurrentBeta) ; };
    void GetCurrentAlphaText(char *c) { sprintf(c,"%3.3f", CurrentAlpha) ; };

    // Set min and maximum Contour levels
    
    void SetNumberOfLineContourLevels(int NumLevels) { NumberOfLineContourLevels = NumLevels; };
    int GetNumberOfLineContourLevels(void) { return NumberOfLineContourLevels; };

    void SetSolutionMin(float MinVal);
    void SetSolutionMax(float MaxVal);

    float GetSolutionMin(void) { return LegendMin; };
    float GetSolutionMax(void) { return LegendMax; };

    // Use English Units in legends, etc.

    void UseEnglishUnitsWasPicked(void) { ToggleFlag(UseEnglishUnits,"UseEnglishUnits"); };

    void DrawComGeomTagsWasPicked(void) { DrawComGeomTagsIsOn = 1 - DrawComGeomTagsIsOn; };
    void DrawComGeomTagsShadedWasPicked(int val) { DrawComGeomTagsShadedIsOn = val; };
    void DrawComGeomTagsBordersWasPicked(int val) { DrawComGeomTagsBordersIsOn = val; };

    // Pointer to vui

    void PassTheVui(viewerUI *VUI) { vui = VUI; };

    // Panel ComGeom Tags browser

    void PassThePanelComGeomTagsBrowser(Fl_Browser *Browser) { PanelComGeomTagsBrowser = Browser; };
    void PanelComGeomTagsBrowser_UnSelectAll(void);
    void PanelComGeomTagsBrowser_SelectAll(void);
	 void PanelComGeomTagsBrowser_Update(void);

    // Set octree cutting plane direction and value

    void SetCuttingDirectionTo(int CutDir) { OctreeCutDirection = CutDir; };
    int GetCuttingDirection(void) { return OctreeCutDirection; };

    void SetCuttingPlaneValueto(float CutVal) { OctreeCutValue = CutVal; };
    float GetCuttingPlaneValue(void) { return OctreeCutValue; };
    
    // Endian issues

    void TurnByteSwapForADBReadsOn(void) { ByteSwapForADB = 1; };
    void TurnByteSwapForADBReadsOff(void) { ByteSwapForADB = 0; };

    void TurnByteSwapForALLReadsOn(void) { ByteSwapForADB = 1; };
    void TurnByteSwapForALLReadsOff(void) { ByteSwapForADB = 0; };
    
    // Make a movie
    
    void MakeMovie(char *FileName);
    
    // Draw wakes colored
    
    int &DrawWakesColored(void) { return DrawWakesColored_; };

};

#endif

