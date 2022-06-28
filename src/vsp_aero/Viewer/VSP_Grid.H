//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#ifndef VSP_GRID_H
#define VSP_GRID_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "utils.H"
#include "VSP_Node.H"
#include "VSP_Tris.H"
#include "VSP_Edge.H"
#include "CharSizes.H"

// Definition of the VSP_GRID class

class VSP_GRID {

private:

    int Verbose_;
    
    // Function data

    char GridName_[FILE_NAME_SIZE];

    int NumberOfNodes_;

    VSP_NODE *NodeList_;

    int NumberOfTris_;

    VSP_TRIS *TriList_;

    int NumberOfEdges_;

    VSP_EDGE *EdgeList_;

public:

    // Constructor, Destructor, Copy

    VSP_GRID(void);
   ~VSP_GRID(void);
    VSP_GRID(const VSP_GRID &VSPGrid);

    // Initialize

    void init(void);

    // Size the Mach, Q, Alpha arrays

    void SizeMachList(int NumMach);
    void SizeBarsList(int NumBars);
    void SizeAlphaList(int NumAlpha);

    // Size the node list

    void SizeNodeList(int NumNodes);

    // Size the tri lists

    void SizeTriList(int NumTris);

    // Size the edge lists

    void SizeEdgeList(int NumEdges);

    // Access to  name

    char *GridName(void) { return GridName_; };

    // Access to Nodal data

    int NumberOfNodes(void) { return NumberOfNodes_; };
  
    VSP_NODE &NodeList(int i) { return NodeList_[i]; };

    VSP_NODE *NodeList(void) { return NodeList_; };

    void UseThisNodeList(int NumberOfNodes, VSP_NODE *NodeList) { NumberOfNodes_ = NumberOfNodes ; NodeList_ = NodeList; };

    // Access to Tri data

    int NumberOfTris(void) { return NumberOfTris_; };

    VSP_TRIS &TriList(int i) { return TriList_[i]; };

    VSP_TRIS *TriList(void) { return TriList_; };

    void UseThisTriList(int NumberOfTris, VSP_TRIS *TriList) { NumberOfTris_ = NumberOfTris ; TriList_ = TriList; };

    // Access to Edge data

    int NumberOfEdges(void) { return NumberOfEdges_; };

    VSP_EDGE &EdgeList(int i) { return EdgeList_[i]; };

    VSP_EDGE *EdgeList(void) { return EdgeList_; };

    void UseThisEdgeList(int NumberOfEdges, VSP_EDGE *EdgeList) { NumberOfEdges_ = NumberOfEdges ; EdgeList_ = EdgeList; };
    
    // Create edge data
    
    void CreateTriEdges(void);
    
    // Calculate normals and centroids
    
    void CalculateNormalsAndCentroids(void);
    
    // Calculate which edges are updwind off the triangle centroid
    
    void CalculateUpwindEdges(void);
    
    // Write out mesh
    
    void WriteMesh(char *FileName);

};

#endif
