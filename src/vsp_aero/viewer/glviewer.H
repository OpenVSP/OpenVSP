#ifndef GL_VIEWER_H
#define GL_VIEWER_H

#include <stdlib.h>
#include <math.h>
#include <stdio.h>
//#include <config.h>
#include <sys/stat.h>
//#include <unistd.h>
#include <time.h>

#include <FL/Fl.H>
#include <stdlib.h>

#include <FL/Fl_Gl_Window.H>
#include <FL/gl.h>
#include <FL/fl_ask.H>

#include <FL/Fl_Browser.H>
#include <FL/Fl_Button.H>
#include <FL/forms.H>

#ifdef OSX

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
//#include "tiffio.h"
#include "TagList.H"
#include "TagListGroup.H"
#include "surfIDs.H"
#include "RotorDisk.H"
#include "PropElement.H"
#include "ControlSurface.H"
#include "VSP_DegenGeom.H"

//  Define marked tri types

#define SRF_TRI      5
#define GOMGEOM_TRI 10

#define TORAD 3.141592/180.

#define SYM_X 1
#define SYM_Y 2
#define SYM_Z 3

#define   VLM_MODEL 1
#define PANEL_MODEL 2

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

}; typedef struct NODE_STRUCT NODE;

struct EDGE_STRUCT
{

	int node1;
	int node2;

   int tri1;
   int tri2;
   
   int SurfaceID;
   
   int IsKuttaEdge;

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
    
    // Cpmin, and Cpmax
    
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

    float Sref;
    float Cref;
    float Bref;
    float Xcg;
    float Ycg;
    float Zcg;
    float ScaleFactor;
    
    int NumberOfWings_;
    int NumberOfBodies_;
    int NumberOfCart3dSurfaces_;
    
    char **BodyListName_;
    char **WingListName_;
    char **Cart3dListName_;

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

    float *Cp;
    
    float *TempTriangleArray;
    float *TempNodalArray;
    float *TempTotalArea;
    
    float *LastFunction;
    float NodalMin, NodalMax;

    int OctreeCutDirection;
    float OctreeCutValue;
    
    int NumberOfTrailingVortexEdges_;
    int NumberOfSubVortexNodes_;
    float **XWake_;
    float **YWake_;
    float **ZWake_;
    
    // Propulsion element data
    
    int NumberOfPropulsionElements;
    int MaxNumberOfPropulsionElements;
    PROPULSION_ELEMENT *PropulsionElement;
    
    // Control Surface data
    
    int NumberOfControlSurfaces;
    CONTROL_SURFACE *ControlSurface;

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
    char LegendTitle[80];
    float LegendMinClip, LegendMaxClip;
    float LegendMin, LegendMax;

    int UseEnglishUnits;

    // Data for tris and nodes

    int NumberOfTris;
    int NumberOfNodes;
    int NumberOfEdges;

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
    int DrawSmoothFunctionsIsOn;

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
    
    int DrawPropulsionElementsIsOn;    
        
    int DrawWakesToInfinityIsOn;
    
    int DrawControlSurfacesIsOn;
    
    // Panel Group

    int DrawSurfaceRegionsIsOn;
    int DrawSurfaceRegionsShadedIsOn;
    int DrawSurfaceRegionsBordersIsOn;

    int DrawComGeomTagsIsOn;
    int DrawComGeomTagsShadedIsOn;
    int DrawComGeomTagsBordersIsOn;

    int DrawCpIsOn;
    void DrawRotorSurfacesShaded(void);

    void percent_to_rgb(float per, float col[3], int colmap);
    void SetSurfaceRegionColor(int SurfaceRegionID);
    void SetTagSurfaceColor(int SurfaceID, int MaxVals);
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
    
    void DrawWakes(void);
    
    void DrawControlSurfaces(void);

    void DrawShadedSolution(float *Function, float FMin, float FMax);
    void DrawShadedSolution(int   *Function, float FMin, float FMax);

    void DrawShadedSolutionPerTri(float *Function, float FMin, float FMax);
    void DrawShadedSolutionPerTri(int   *Function, float FMin, float FMax);

    void DrawShadedSolutionPerNode(float *Function, float FMin, float FMax);

    void DrawCp(void);
 
    void Draw(void);
    void DrawLegend(void);
    void DrawLabel(void);
    void DrawCGLabel(void);
    void DrawCGMarker();
    void DrawAxes();

    
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
    
    void LoadCaseFile(char *FileName);
 
    // Allows byte swapping on read/writes of binary files
    // so we can deal with endian issues across platforms

    int ByteSwapForADB;
 
    // ADB file pointers

    fpos_t StartOfWallTemperatureData;

    // Write out a tiff file

    int WriteTiff(char *filename, char *description,
                  int x, int y, int width, int height, int compression);

public:

    // Constructor, Destructor, Copy

    GL_VIEWER(int x,int y,int w,int h,const char *l=0);
   ~GL_VIEWER(void);

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

    // Write out a tiff file

    void WriteTiffFile(char *FileName);

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
    void DrawSmoothFunctionsWasPicked(void)         { ToggleFlag(DrawSmoothFunctionsIsOn,                       "DrawSmoothFunctionsIs"); };
    void DrawReflectedGeometryWasPicked(void)       { ToggleFlag(DrawReflectedGeometryIsOn,                     "DrawReflectedGeometryIs"); };

    void DrawLegendWasPicked(void)                  { ToggleFlag(DrawLegendIsOn,                                "DrawLegendIs"); };
    void DrawLabelWasPicked(void)                   { ToggleFlag(DrawLabelIsOn,                                 "DrawLabelIs"); };
    void DrawCGLabelWasPicked(void)                 { ToggleFlag(DrawCGLabelIsOn,                               "DrawCGLabelIs"); };
    void DrawWhiteBackgroundWasPicked(void)         { ToggleFlag(DrawWithWhiteBackgroundIsOn,                   "DrawWithWhiteBackgroundIs");};
    void DrawFlatShadedWasPicked(void)              { ToggleFlag(DrawFlatShadedIsOn,                            "DrawFlatShadedIs"); };
    
    void DrawCGWasPicked(void)                      { ToggleFlag(DrawCGIsOn,                                    "DrawCGIs");     };
    void DrawAxesWasPicked(void)                    { ToggleFlag(DrawAxesIsOn,                                  "DrawAxesIs");     };

    void DrawWakesWasPicked(void)                   { ToggleFlag(DrawWakesIsOn,                                 "DrawWakesIs");     };
    void DrawRotorsWasPicked(void)                  { ToggleFlag(DrawPropulsionElementsIsOn,                    "DrawPropulsionElementsIs");     };
    
    void DraControlSurfacesWasPicked(void)          { ToggleFlag(DrawControlSurfacesIsOn,                       "DrawControlSurfacesIsOn");     };

    void DrawWakesToInfinityWasPicked(void)         { ToggleFlag(DrawWakesToInfinityIsOn,                       "DrawWakesToInfinityIsOn");     };
    
    // ADB Menu

    void ClearADBFile(void);

    void DrawWireFrameWasPicked(void)                { ToggleFlag(DrawWireFrameIsOn,                             "DrawWireFrameIs"); };

    void DrawOnlySelectedWasPicked(void)             { ToggleFlag(DrawOnlySelectedIsOn,                          "DrawOnlySelectedIs");};

    void DrawShadedWasPicked(void)                   { ToggleFlag(DrawShadedIsOn,                                "DrawShadedIs");
                                                       ToggleFlag(DrawCpIsOn,                                    "DrawCpIs");     };

    void DrawCpWasPicked(void)                       { ToggleFlag(DrawShadedIsOn,                                "DrawShadedIs");
                                                       ToggleFlag(DrawCpIsOn,                                    "DrawCpIs");     };

    void DrawCoarseMeshWasPicked(void)               { ToggleFlag(DrawCoarseMeshesIsOn,                          "DrawCoarseMeshesIs"); };

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

};

#endif

