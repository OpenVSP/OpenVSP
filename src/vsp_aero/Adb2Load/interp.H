#include "search.H"

#ifndef INTERP_H
#define INTERP_H

#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "binaryio.H"
#include "utils.H"

// Triangle Structure

class INTERP_TRI
{

   public:
   
    int NumberOfNodes;
    
	 int node1;  // Corner nodes
	 int node2;
	 int node3;
    
    int node4;  // Mid edge nodes
    int node5;
    int node6;  
    
    int element_id;
    
	 int surface_id;

    float nx;
    float ny;
    float nz;
	 float area;
    
	 int material_id;
	 float emissivity;
    
	 float x;
	 float y;
	 float z;

    float Cp;
    float Cp_Unsteady;
    float Gamma;

    int InterpNode[3];
    float InterpWeight[3];
    
    char *ElementName; // Pointer to element group name, this is shared across all elements in the gorup

}; 
// Node Structure

struct INTERP_NODE
{

   public:
   
    int node_id;

	 float x;
	 float y;
	 float z;

    float Cp;
    float Cp_Unsteady;
    float Gamma;

};

// Mesh Struct

class INTERP_MESH
{

   public:

    int number_of_nodes;
    int number_of_tris;

    INTERP_TRI *TriList;
    INTERP_NODE *NodeList;

    float *MachList;
    float *BarsList;
    float *AlphaList;
    float *BetaList;

    float Sref;
    float Cref;
    float Bref;
    float Xcg;
    float Ycg;
    float Zcg;

    float MinX, MaxX;
    float MinZ, MaxZ;
    float MinY, MaxY;

    float DynamicPressure;
    float FreeStreamPressure;

    float ScaleFactor;

    int num_Mach;
    int num_Bars;
    int num_Alpha;
    int num_Beta;

    int NumberOfControlSurfaces;
    int *NumberOfControlSurfaceDeflections;
    float **ControlSurfaceDeflection;
    
    int StagnationTri;

    int Planet;

    int WriteOutHalfGeometry;

};

// Mesh Struct

class INTERP
{
   
    private:

       int Symmetry;
       int CmToMeters;
       int SwapNormals;
       int AnchorFile;
       int IgnoreBox;
       int StrictInterpolation;
       
       void CalculateBoundingBox(INTERP_MESH *Mesh);
       void CalculateCentroids(INTERP_MESH *Mesh);
       void CalculateNormals(INTERP_MESH *Mesh);
       void CalculateNodalValues(INTERP_MESH *Mesh);
       void CalculateCentroidValues(INTERP_MESH *Mesh);
       void SwapSurfaceNormals(INTERP_MESH *Mesh);
       
       void InterpolateSolution(INTERP_MESH *Mesh1, INTERP_MESH *Mesh2);
       
       float Limiter2D(float Value, float Val1, float Val2, float Val3);
       
       void WriteADBFile(INTERP_MESH *Mesh, char *Name);
    
    public:
    
       INTERP(void);
       INTERP(const INTERP &Interp);
        
       void Interpolate(INTERP_MESH *Mesh1, INTERP_MESH *Mesh2);
       
       void IngoreBoundingBox(void) { IgnoreBox = 1; };
       void ForceStrictInterpolation(void) { StrictInterpolation = 1; };

};

#endif
