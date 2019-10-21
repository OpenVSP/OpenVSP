#ifndef ADBSLICER_H
#define ADBSLICER_H

#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <sys/stat.h>

#ifndef WIN32
#include <unistd.h>
#endif


#include "utils.H"
#include "binaryio.H"
#include "surfIDs.H"
#include "RotorDisk.H"
#include "PropElement.H"
#include "ControlSurface.H"

//  Define marked tri types

#define SRF_TRI      5
#define GOMGEOM_TRI 10

#define TORAD 3.141592/180.

#define XCUT 1
#define YCUT 2
#define ZCUT 3

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

class ADBSLICER {

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

    // Aerothermal database data
    
    int NumberOfADBCases_;
    
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

    float *MachList;
    float *BetaList;
    float *AlphaList;

    float *Cp;
    float *CpUnsteady;
    float *Gamma;
    
    float *CpNode;
    float *TotalArea;
    
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
    int *ControlSurfaceLoop;
    CONTROL_SURFACE *ControlSurface;
    
    // Mesh limits

    float XMin, XMax;
    float YMin, YMax;
    float ZMin, ZMax;

    // Limits

    float CpMinActual, CpMaxActual;
    float CpMin, CpMax;
 
    // Data for tris and nodes

    int NumberOfTris;
    int NumberOfNodes;
    int NumberOfEdges;
    int NumberOfVortexLoops;
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

    void CalculateSurfaceNormals(int Case);

    void CreateTriEdges(void);
    
    // Cut plane information
    
    int NumberOfCutPlanes;
    int *CutPlaneType;
    float *CutPlaneValue;

    // I/O Code
    
    int RotateGeometry;
    float CosRot, SinRot;
    FILE *SliceFile;

    void LoadMeshData(void);
    void LoadSolutionData(int Case);
    void LoadSolutionCaseList(void);

    void FindMeshMinMax(void);
    void FindSolutionMinMax(void);
    
    void LoadCutsFile(void);
    
    void Slice(int Case);    

    // Allows byte swapping on read/writes of binary files
    // so we can deal with endian issues across platforms

    int ByteSwapForADB;
 
    // ADB file pointers

    fpos_t StartOfWallTemperatureData;
    
    // File format stuff
    
    int GnuPlot_;

public:

    // Constructor, Destructor, Copy

    ADBSLICER(void);
   ~ADBSLICER(void);

    void LoadFile(char *name);
    
    int &GnuPlot(void) { return GnuPlot_; };

};

#endif

