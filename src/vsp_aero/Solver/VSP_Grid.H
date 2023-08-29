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

    /** Initialize a new mesh **/
    
    void init(void);

    /** Size the node list **/

    void SizeNodeList(int NumNodes);
    
    /** Delete the node list **/
    
    void DeleteNodeList(void) { delete [] NodeList_; };

    /** Size the tri list **/

    void SizeTriList(int NumTris);
    
    /** Delete the tri list **/
    
    void DeleteTriList(void) { delete [] LoopList_; };

    /** Size a more general loop (vs tri) list **/
    
    void SizeLoopList(int NumLoops);
    
    /** Size the edge list **/

    void SizeEdgeList(int NumEdges);
    
    /** Delete the edge list **/
    
    void DeleteEdgeList(void) { delete [] EdgeList_; };

    /** Grid name **/

    char *GridName(void) { return GridName_; };

    /** Number of mesh nodes **/

    int NumberOfNodes(void) { return NumberOfNodes_; };
  
    /** Mesh node list **/
    
    inline VSP_NODE &NodeList(int i) { return NodeList_[i]; };

    /** Pointer to mesh node list **/
    
    inline VSP_NODE *NodeList(void) { return NodeList_; };

    /** Pass in a pointer to a node list and just use it ... **/
    
    void UseThisNodeList(int NumberOfNodes, VSP_NODE *NodeList) { NumberOfNodes_ = NumberOfNodes ; NodeList_ = NodeList; };

    /** Number of grid tris **/

    int NumberOfTris(void) { return NumberOfLoops_; };
    
    /** Mesh tri list **/
    
    inline VSP_LOOP &TriList(int i) { return LoopList_[i]; };

    /** Pointer to mesh tri list **/
    
    inline VSP_LOOP *TriList(void) { return LoopList_; };
    
    /** Pass in a pointer to a tri list and just use it ... **/

    void UseThisTriList(int NumberOfTris, VSP_LOOP *TriList) { NumberOfLoops_ = NumberOfTris ; LoopList_ = TriList; };

    /** Number of mesh loops **/
    
    int NumberOfLoops(void) { return NumberOfLoops_; };
    
    /** Mesh loop list **/
    
    VSP_LOOP &LoopList(int i) { return LoopList_[i]; };

    /** Pointer to mesh loop list **/
    
    VSP_LOOP *LoopList(void) { return LoopList_; };
    
    /** Pass in a pointer to a loop list and just use it ... **/

    void UseThisLoopList(int NumberOfLoops, VSP_LOOP *LoopList) { NumberOfLoops_ = NumberOfLoops ; LoopList_ = LoopList; };
    
    /** Number of mesh edges **/

    int NumberOfEdges(void) { return NumberOfEdges_; };

    /** Mesh edge list **/
    
    VSP_EDGE &EdgeList(int i) { return EdgeList_[i]; };

    /** Pointer to mesh edge list ***/
    
    VSP_EDGE *EdgeList(void) { return EdgeList_; };

    /** Pass in a pointer to an edge list and just use it ... **/
    
    void UseThisEdgeList(int NumberOfEdges, VSP_EDGE *EdgeList) { NumberOfEdges_ = NumberOfEdges ; EdgeList_ = EdgeList; };
    
    /** Size the kutta node list **/
    
    void SizeKuttaNodeList(int NumberOfKuttaNodes);
    
    /** Number of kutta nodes **/
    
    int NumberOfKuttaNodes(void) { return NumberOfKuttaNodes_; };
    
    /** Kutta node list **/
    
    int &KuttaNode(int i) { return KuttaNode_[i]; };
    
    /** Wing surface that the kutta node is on **/
    
    int &WingSurfaceForKuttaNode(int i) { return WingSurfaceForKuttaNode_[i]; };
    
    /** Component that kutta node is one **/
    
    int &ComponentIDForKuttaNode(int i) { return ComponentIDForKuttaNode_[i]; };
    
    /** Wing surface kutta node is on is periodic... ie, like a nacelle ***/
    
    int &WingSurfaceForKuttaNodeIsPeriodic(int i) { return WingSurfaceForKuttaNodeIsPeriodic_[i]; };
    
    /** Kutta node is on wing tip **/
    
    int &KuttaNodeIsOnWingTip(int i) { return KuttaNodeIsOnWingTip_[i]; };
    
    /** X coordinate of kutta node trailing edge point **/
    
    VSPAERO_DOUBLE &WakeTrailingEdgeX(int i) { return WakeTrailingEdgeX_[i]; };
    
    /** Y coordinate of kutta node trailing edge point **/
    
    VSPAERO_DOUBLE &WakeTrailingEdgeY(int i) { return WakeTrailingEdgeY_[i]; };
    
    /** Z coordinate of kutta node trailing edge point **/
    
    VSPAERO_DOUBLE &WakeTrailingEdgeZ(int i) { return WakeTrailingEdgeZ_[i]; };
    
    /** S over B (ie, non-dim span location) for kutta node **/
    
    VSPAERO_DOUBLE &KuttaNodeSoverB(int i) { return KuttaNodeSoverB_[i]; };
    
    /** Model type **/
    
    int &ModelType(void) { return ModelType_; };
    
    /** Surface type **/
    
    int &SurfaceType(void) { return SurfaceType_; };
    
    /** Component ID **/
    
    int &ComponentID(void) { return ComponentID_; };
   
    /** Create edge data from triangle list **/
    
    void CreateTriEdges(void);
    
    /** Check consistency of the grid... must be called after CreateTriEdges **/
    
    int CheckGridConsistency(void);
    
    /** Calculate normals and centroids for a triangular mesh **/
    
    void CalculateTriNormalsAndCentroids(void);
    
    /** Determine which which edges are updwind off the triangle centroid **/
    
    void CalculateUpwindEdges(void);
    
    /** Calculate upwind edge data **/
    
    void CreateUpwindEdgeData(void);
    
    /** Mark boundary edges **/
    
    void MarkBoundaries(void);
    
    /** Contraint on the minimum loop sized used during agglomeration **/
    
    VSPAERO_DOUBLE &MinLoopArea(void) { return MinLoopArea_; };

    /** Write out mesh data **/
    
    void WriteMesh(char *FileName);
    
    /** Update geometry based on supplied translation vector and quaternion rotation **/
    
    void UpdateGeometryLocation(VSPAERO_DOUBLE *TVec, VSPAERO_DOUBLE *OVec, QUAT &Quat, QUAT &InvQuat, int *ComponentInThisGroup);
    
    /** Set the edge Mach number **/
    
    void SetMachNumber(VSPAERO_DOUBLE Mach);

};

#include "END_NAME_SPACE.H"


#endif
