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
#include "interp.H"

//  Define marked tri types

#define SRF_TRI      5
#define GOMGEOM_TRI 10

#define TORAD 3.141592/180.

#define XCUT 1
#define YCUT 2
#define ZCUT 3

#define   VLM_MODEL 1
#define PANEL_MODEL 2

#define CALCULIX_STATIC 1
#define CALCULIX_BUCKLE 2

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

// Calculix stuff

class CALCULIX_NODE {

private:

public:

    float x;
    float y;
    float z;
    
    float dx;
    float dy;
    float dz;
    
    float S1;
    float S2;
    float S3;
    float SMag;
    
};

class CALCULIX_ELEMENT {

private:

public:

    int ElementID;
    int NumberOfNodes; 
    int Node[8];
    int StructuralElementSet;
    int Whacked;
    int SizingGroup;
    
    float nx[6];
    float ny[6];
    float nz[6];
    
    float Area;
    float Thickness;
    float Volume[2];
    float MaxStressRatio;
    
    char *ElementName; // Pointer to the name... this is shared across all elements in the group
        
};

class CALCULIX_GROUP {
   
    int NumberOfElements;
    int *ElementList;
    
    float MinThick;
    float MaxThick;
    
};

// The GL_VIEWER Class definition

class ADBSLICER {

private:

    int Verbose_;

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
    char CalculixFileName[2000];

    int NumberOfCalculixNodes;
    int NumberOfCalculixElements;
    
    float CalculixStressMagMin;
    float CalculixStressMagMax;                              
           
    float MaxSkinThickness_;
    float MinSkinThickness_;
    float MaxStress_;
    float OptFact_;
    
    char ElementLabel_[10000];
    
    CALCULIX_NODE *CalculixNode;    
    CALCULIX_ELEMENT *CalculixElement;    
    
    int *CalculixNodePerm_;
    int *CalculixElementPerm_;
    
    int NumberOfCalculixSizingGroups_;
    float CalculixMinSizingThickness_;
    float CalculixMaxSizingThickness_;
    float CalculixDThickness_;

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

    TRI *TriList_;
    NODE *NodeList_;
    EDGE *EdgeList_;
    
    int NumberOfMeshLevels;
    int *NumberOfCourseNodesForLevel;
    int *NumberOfCourseEdgesForLevel;
    NODE **CoarseNodeList_;
    EDGE **CoarseEdgeList_;
    
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
    
    // Mesh interpolation data
    
    int NodeOffSet_;
    int ElementOffSet_;
    int AddLabel_;
    int MaxCalculixNode_;
    int MaxCalculixElement_;
    int FindClosestNode_;
    
    float DynamicPressure_;
    float BoundaryTolerance_;
    
    float xyz_find_[3];
    
    char Label_[10000];
    
    INTERP_MESH VSP_Mesh;
    INTERP_MESH FEM_Mesh;

    void LoadCalculixINPFileSurfaceElements(char *name);
    void CreateVSPInterpMesh(void);
    void WriteOutCalculixStaticAnalysisFile(char *name);

    void WriteOutCalculixStaticAnalysisFile(char *name, int AnalysisType);
    void WriteOutCalculixBoundaryConditions(FILE *LoadFile); 
    void WriteOutCalculixElementLoads(FILE *LoadFile);

public:

    // Constructor, Destructor, Copy

    ADBSLICER(void);
   ~ADBSLICER(void);
   
    TRI &TriList(int i) { return TriList_[i]; };
    NODE &NodeList(int i) { return NodeList_[i]; };
    
    void LoadFile(char *name);
    void SliceGeometry(char *name);
    void ParseCalculixFile(char *name);
    void CalculateCalulixOffSets(char *name) { Verbose_ = 0 ; ParseCalculixFile(name); };
    void InterpolateSolutionToCalculix(char *name);
    void MergeCalculixFiles(char *filename1, char *filename2, char *newfilename);
    void FindNearestNodeInCalculixFile(char *name, float *xyz);
    void LoadCalculixData(char *filename);
    void CleanCalculixInpFile(char *filename1, char *newfilename);
    void SmoothSkinThickness(char *filename1);
    void ResizeCalculixInputFileSkins(char *filename1, char *newfilename);
    void ResizeCalculixInputFileSkinsOld(char *filename1, char *newfilename);
    
    void OptimizationCalculixInpFile(char *filename1, char *filename2, char *filename3);
    void ScaleCalulixInpPressureLoads(char *filename1, char *newfilename, float ScaleFactor);
    int *RenumberCalulixINPFile(char *name);
    void WriteOutRenumberedCalculixINPFile(char *name);
    
    float &MaxSkinThickness(void) { return MaxSkinThickness_; };
    float &MinSkinThickness(void) { return MinSkinThickness_; };
    float &MaxStress(void) { return MaxStress_; };
    float &OptFact(void) { return OptFact_; };
    void SetElementName(char *Name) { sprintf(ElementLabel_,"%s",Name); };
        
    int &NodeOffSet(void) { return NodeOffSet_; };
    
    int &ElementOffSet(void) { return ElementOffSet_; };
    
    int &FindClosestNode(void) { return FindClosestNode_; };
    
    int &AddLabel(void) { return AddLabel_; };
    
    float &DynamicPressure(void) { return DynamicPressure_; };
    
    float &BoundaryTolerance(void) { return BoundaryTolerance_; };
    
    char *Label(void) { return Label_; };
    
    int &GnuPlot(void) { return GnuPlot_; };

};

#endif

