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
#include "VSP_Loop.H"
#include "VSP_Edge.H"
#include "CharSizes.H"

#include "START_NAME_SPACE.H"

// Definition of the VSP_GRID class

class VSP_GRID {

private:

    int Verbose_;  
    
    // Grid data

    char GridName_[FILE_NAME_SIZE];

    int NumberOfNodes_;

    VSP_NODE *NodeList_;

    int NumberOfLoops_;

    VSP_LOOP *LoopList_;

    int NumberOfEdges_;

    VSP_EDGE *EdgeList_;
    
    int ModelType_;

    int SurfaceType_;
    
    int ComponentID_;

    VSPAERO_DOUBLE MinLoopArea_;
        
    // Wake data

    int NumberOfKuttaNodes_;

    int *KuttaNode_;
    int *WingSurfaceForKuttaNode_;
    int *WingSurfaceForKuttaNodeIsPeriodic_;
    int *ComponentIDForKuttaNode_;
    int *KuttaNodeIsOnWingTip_;
        
    VSPAERO_DOUBLE *WakeTrailingEdgeX_;
    VSPAERO_DOUBLE *WakeTrailingEdgeY_;
    VSPAERO_DOUBLE *WakeTrailingEdgeZ_;    
    
    VSPAERO_DOUBLE *KuttaNodeSoverB_;

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
    void DeleteNodeList(void) { delete [] NodeList_; };

    // Size the tri lists

    void SizeTriList(int NumTris);
    void DeleteTriList(void) { delete [] LoopList_; };

    // Size the more general loop list
    
    void SizeLoopList(int NumLoops);
    
    // Size the edge lists

    void SizeEdgeList(int NumEdges);
    void DeleteEdgeList(void) { delete [] EdgeList_; };

    // Access to  name

    char *GridName(void) { return GridName_; };

    // Access to Nodal data

    int NumberOfNodes(void) { return NumberOfNodes_; };
  
    VSP_NODE &NodeList(int i) { return NodeList_[i]; };

    VSP_NODE *NodeList(void) { return NodeList_; };

    void UseThisNodeList(int NumberOfNodes, VSP_NODE *NodeList) { NumberOfNodes_ = NumberOfNodes ; NodeList_ = NodeList; };

    // Access to Tri data

    int NumberOfTris(void) { return NumberOfLoops_; };
    
    VSP_LOOP &TriList(int i) { return LoopList_[i]; };

    VSP_LOOP *TriList(void) { return LoopList_; };

    void UseThisTriList(int NumberOfTris, VSP_LOOP *TriList) { NumberOfLoops_ = NumberOfTris ; LoopList_ = TriList; };

    // Access to Loop data
    
    int NumberOfLoops(void) { return NumberOfLoops_; };
    
    VSP_LOOP &LoopList(int i) { return LoopList_[i]; };

    VSP_LOOP *LoopList(void) { return LoopList_; };

    void UseThisLoopList(int NumberOfLoops, VSP_LOOP *LoopList) { NumberOfLoops_ = NumberOfLoops ; LoopList_ = LoopList; };
    
    // Access to Edge data

    int NumberOfEdges(void) { return NumberOfEdges_; };

    VSP_EDGE &EdgeList(int i) { return EdgeList_[i]; };

    VSP_EDGE *EdgeList(void) { return EdgeList_; };

    void UseThisEdgeList(int NumberOfEdges, VSP_EDGE *EdgeList) { NumberOfEdges_ = NumberOfEdges ; EdgeList_ = EdgeList; };
    
    // Access to kutta node list
    
    void SizeKuttaNodeList(int NumberOfKuttaNodes);
    
    int NumberOfKuttaNodes(void) { return NumberOfKuttaNodes_; };
    int &KuttaNode(int i) { return KuttaNode_[i]; };
    int &WingSurfaceForKuttaNode(int i) { return WingSurfaceForKuttaNode_[i]; };
    int &ComponentIDForKuttaNode(int i) { return ComponentIDForKuttaNode_[i]; };
    int &WingSurfaceForKuttaNodeIsPeriodic(int i) { return WingSurfaceForKuttaNodeIsPeriodic_[i]; };
    int &KuttaNodeIsOnWingTip(int i) { return KuttaNodeIsOnWingTip_[i]; };
    
    VSPAERO_DOUBLE &WakeTrailingEdgeX(int i) { return WakeTrailingEdgeX_[i]; };
    VSPAERO_DOUBLE &WakeTrailingEdgeY(int i) { return WakeTrailingEdgeY_[i]; };
    VSPAERO_DOUBLE &WakeTrailingEdgeZ(int i) { return WakeTrailingEdgeZ_[i]; };
    
    VSPAERO_DOUBLE &KuttaNodeSoverB(int i) { return KuttaNodeSoverB_[i]; };
    
    // Model type
    
    int &ModelType(void) { return ModelType_; };
    
    // Surface type
    
    int &SurfaceType(void) { return SurfaceType_; };
    
    // Component ID
    
    int &ComponentID(void) { return ComponentID_; };
   
    // Create edge data
    
    void CreateTriEdges(void);
    
    // Calculate normals and centroids for a triangular mesh
    
    void CalculateTriNormalsAndCentroids(void);
    
    // Calculate which edges are updwind off the triangle centroid
    
    void CalculateUpwindEdges(void);
    void CreateUpwindEdgeData(void);
    
    VSPAERO_DOUBLE &MinLoopArea(void) { return MinLoopArea_; };

    // Write out mesh
    
    void WriteMesh(char *FileName);
    
    // Geometry update
    
    void UpdateGeometryLocation(VSPAERO_DOUBLE *TVec, VSPAERO_DOUBLE *OVec, QUAT &Quat, QUAT &InvQuat, int *ComponentInThisGroup);

};

#include "END_NAME_SPACE.H"


#endif
