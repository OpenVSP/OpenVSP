//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#ifndef QUAD_TREE_H
#define QUAD_TREE_H

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.H"
#include "QuadCell.H"

#include "START_NAME_SPACE.H"

class QUAD_TREE {

private:
    
    int Direction_;
    int DirMap_[3];
    
    int NumberOfNodes_;
    int NumberOfEdges_;    
    int NumberOfCells_;

    int MaxNumberOfNodes_;
    int MaxNumberOfEdges_;    
    int MaxNumberOfCells_;
        
    QUAD_NODE *NodeList_;
    QUAD_EDGE *EdgeList_;
    QUAD_CELL *CellList_;
        
    void ResizeNodeList(int NewSize);
    void ResizeEdgeList(int NewSize);
    void ResizeCellList(int NewSize);
    void SplitCell(int Cell);
    
    int InsideQuad(int Cell, VSPAERO_DOUBLE xyz[3]);
       
public:

    QUAD_TREE(void);
   ~QUAD_TREE(void);
    QUAD_TREE(const QUAD_TREE &QuadTree);


    void InitializeTree(VSPAERO_DOUBLE X_min, VSPAERO_DOUBLE X_max,
                        VSPAERO_DOUBLE Y_min, VSPAERO_DOUBLE Y_max,
                        VSPAERO_DOUBLE Z_min, VSPAERO_DOUBLE Z_max,
                        VSPAERO_DOUBLE Value);
                        
    void ClearTree(void);                        
    void BufferTree(int Level);
    void BufferTreeOld(int Level);
    
    void WriteQuadTreeToFile(FILE *QuadFile);
    
    int NumberOfNodes(void) { return NumberOfNodes_; };
    
    int &NodeInsideBody(int i) { return NodeList_[i].InsideBody(); };
    
    VSPAERO_DOUBLE *xy(int i) { return NodeList_[i].xyz(); };
    
    VSPAERO_DOUBLE &x(int i) { return NodeList_[i].xyz()[DirMap_[0]]; };
    VSPAERO_DOUBLE &y(int i) { return NodeList_[i].xyz()[DirMap_[1]]; };
    VSPAERO_DOUBLE &z(int i) { return NodeList_[i].xyz()[DirMap_[2]]; };

    VSPAERO_DOUBLE *velocity(int i) { return NodeList_[i].velocity(); };
    VSPAERO_DOUBLE &Cp(int i) { return NodeList_[i].Cp(); };
     
    int CellHasPoint(int i) { return CellList_[i].HasPoint(); };
    int SurfaceEdge(int i) { return CellList_[i].SurfaceEdge(); };

    int NumberOfCells(void) { return NumberOfCells_; };    
    int InsertPoint(VSPAERO_DOUBLE xyz[3], int SurfaceEdge);
    int &Direction(void) { return Direction_; };

    void SmoothPressure(void);

};

#include "END_NAME_SPACE.H"

#endif
