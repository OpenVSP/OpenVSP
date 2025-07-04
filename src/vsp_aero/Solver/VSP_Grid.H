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

    char GridName_[MAX_CHAR_SIZE];

    int NumberOfNodes_;
    
    int NumberOfSurfaceNodes_;

    VSP_NODE *NodeList_;

    int NumberOfLoops_;

    int NumberOfSurfaceLoops_;

    VSP_LOOP *LoopList_;

    int NumberOfEdges_;

    int NumberOfSurfaceEdges_;

    VSP_EDGE *EdgeList_;
    
    int ModelType_;

    int SurfaceType_;

    double MinLoopArea_;
        
    // Wake data

    int NumberOfKuttaNodes_;

    int *KuttaNode_;
    int *WingSurfaceForKuttaNode_;
    int *WingSurfaceForKuttaNodeIsPeriodic_;
    int *ComponentIDForKuttaNode_;
    int *KuttaNodeIsOnWingTip_;
        
    double *WakeTrailingEdgeX_;
    double *WakeTrailingEdgeY_;
    double *WakeTrailingEdgeZ_;    
    
    double *KuttaNodeSoverB_;

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
    
    /** Number of mesh nodes on the surface of the body... this excludes all the wake nodes **/
    
    int NumberOfSurfaceNodes(void) { return NumberOfSurfaceNodes_; };
  
    /** Mesh node list **/
    
    inline VSP_NODE &NodeList(int i) { return NodeList_[i]; };

    /** Pointer to mesh node list **/
    
    inline VSP_NODE *NodeList(void) { return NodeList_; };

    /** Pass in a pointer to a node list and just use it ... **/
    
    void UseThisNodeList(int NumberOfNodes, VSP_NODE *NodeList) { NumberOfNodes_ = NumberOfNodes ; NodeList_ = NodeList; };

    /** Number of grid tris **/

    int NumberOfTris(void) { return NumberOfLoops_; };

    /** Number of mesh tris on the surface of the body... this excludes all the wake tris **/
    
    int NumberOfSurfaceTris(void) { return NumberOfSurfaceLoops_; };

    /** Mesh tri list **/
    
    inline VSP_LOOP &TriList(int i) { return LoopList_[i]; };

    /** Pointer to mesh tri list **/
    
    inline VSP_LOOP *TriList(void) { return LoopList_; };
    
    /** Pass in a pointer to a tri list and just use it ... **/

    void UseThisTriList(int NumberOfTris, VSP_LOOP *TriList) { NumberOfLoops_ = NumberOfTris ; LoopList_ = TriList; };

    /** Number of mesh loops **/
    
    int NumberOfLoops(void) { return NumberOfLoops_; };

    /** Number of mesh loops on the surface of the body... this excludes all the wake loops **/
    
    int NumberOfSurfaceLoops(void) { return NumberOfSurfaceLoops_; };
          
    /** Mesh loop list **/
    
    VSP_LOOP &LoopList(int i) { return LoopList_[i]; };

    /** Pointer to mesh loop list **/
    
    VSP_LOOP *LoopList(void) { return LoopList_; };
    
    /** Pass in a pointer to a loop list and just use it ... **/

    void UseThisLoopList(int NumberOfLoops, VSP_LOOP *LoopList) { NumberOfLoops_ = NumberOfLoops ; LoopList_ = LoopList; };
    
    /** Number of mesh edges **/

    int NumberOfEdges(void) { return NumberOfEdges_; };

    /** Number of mesh edges on the surface of the body... this excludes all the wake edges **/

    int &NumberOfSurfaceEdges(void) { return NumberOfSurfaceEdges_; };

    /** Mesh edge list **/
    
    VSP_EDGE &EdgeList(int i) { return EdgeList_[i]; };

    /** Pointer to mesh edge list ***/
    
    VSP_EDGE *EdgeList(void) { return EdgeList_; };

    /** Pass in a pointer to an edge list and just use it ... **/
    
    void UseThisEdgeList(int NumberOfEdges, VSP_EDGE *EdgeList) { NumberOfEdges_ = NumberOfEdges ; EdgeList_ = EdgeList; };
    
    /** Size the kutta node list **/
    
    void SizeKuttaNodeList(int NumberOfKuttaNodes);
    
    /** Copy the kutta list data **/
    
    void CopyKuttaList(VSP_GRID *Grid);
    
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
    
    double &WakeTrailingEdgeX(int i) { return WakeTrailingEdgeX_[i]; };
    
    /** Y coordinate of kutta node trailing edge point **/
    
    double &WakeTrailingEdgeY(int i) { return WakeTrailingEdgeY_[i]; };
    
    /** Z coordinate of kutta node trailing edge point **/
    
    double &WakeTrailingEdgeZ(int i) { return WakeTrailingEdgeZ_[i]; };
    
    /** S over B (ie, non-dim span location) for kutta node **/
    
    double &KuttaNodeSoverB(int i) { return KuttaNodeSoverB_[i]; };
    
    /** Model type **/
    
    int &ModelType(void) { return ModelType_; };
    
    /** Surface type **/
    
    int &SurfaceType(void) { return SurfaceType_; };

    /** Create edge data from triangle list **/
    
    void CreateTriEdges(void);
    
    /** Check consistency of the grid... must be called after CreateTriEdges **/
    
    int CheckGridConsistency(void);
    
    /** Calculate normals and centroids for a triangular mesh **/
    
    void CalculateTriNormalsAndCentroids(void);

    /** Calculate circumference of loops **/
        
    void CalculateLoopCircumferences(void);
    
    /** Determine which which edges are updwind off the triangle centroid **/
    
    void CalculateUpwindEdges(void);
    
    /** Calculate upwind edge data **/
    
    void CreateUpwindEdgeData(void);
    
    /** Mark boundary edges **/
    
    void MarkBoundaries(int DoSymmetryPlaneSolve);
    
    /** Determine how many loops, edges, and nodes lie on the bodies ... vs bodies + wakes **/
    
    void DetermineSurfaceMeshSize(void);
    
    /** Contraint on the minimum loop sized used during agglomeration **/
    
    double &MinLoopArea(void) { return MinLoopArea_; };

    /** Write out mesh data **/
    
    void WriteMesh(char *FileName);
    
    /** Update geometry based on supplied translation vector and quaternion rotation **/
    
    void UpdateGeometryLocation(double *TVec, double *OVec, QUAT &Quat, QUAT &InvQuat, int *ComponentInThisGroup);
    
    /** Set the edge Mach number **/
    
    void SetMachNumber(double Mach);

};

#include "END_NAME_SPACE.H"


#endif
