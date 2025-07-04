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
    
    int InsideQuad(int Cell, double xyz[3]);

    void BufferTreeOld(int Level);
       
public:

    QUAD_TREE(void);
   ~QUAD_TREE(void);
    QUAD_TREE(const QUAD_TREE &QuadTree);

    /** Initialized thhe tree with thhe outer bounding box limits, Value is the cutting plane x,y,or z location **/
    
    void InitializeTree(double X_min, double X_max,
                        double Y_min, double Y_max,
                        double Z_min, double Z_max,
                        double Value);
                    
    /** Clear the tree **/
                        
    void ClearTree(void);                        
    
    /** Buffer the tree out **/
    
    void BufferTree(int Level);
    
    /** Write out the quad tree structure to file **/
    
    void WriteQuadTreeToFile(FILE *QuadFile);
    
    /** Number of nodes in this quad tree **/
    
    int NumberOfNodes(void) { return NumberOfNodes_; };
    
    /** Flag if node is inside a body **/
    
    int &NodeInsideBody(int i) { return NodeList_[i].InsideBody(); };
    
    /** Vector of xyz coordinates for node i of the quad tree **/
    
    double *xyz(int i) { return NodeList_[i].xyz(); };
    
    /** X coordinate of i'th node of the quad tree **/
    
    double &x(int i) { return NodeList_[i].xyz()[DirMap_[0]]; };
    
    /** Y coordinate of i'th node of the quad tree **/
    
    double &y(int i) { return NodeList_[i].xyz()[DirMap_[1]]; };

    /** Z coordinate of i'th node of the quad tree **/

    double &z(int i) { return NodeList_[i].xyz()[DirMap_[2]]; };
    
    /** Velocity vector at i'th node of the quad tree **/

    double *velocity(int i) { return NodeList_[i].velocity(); };
    
    /** Pressure coefficient at i'th node of the quad tree **/
    
    double &Cp(int i) { return NodeList_[i].Cp(); };
    
    /** Flag if cell has a point in it **/
     
    int CellHasPoint(int i) { return CellList_[i].HasPoint(); };
    
    /** Possible surface edge associated with inserted node **/
    
    int SurfaceEdge(int i) { return CellList_[i].SurfaceEdge(); };
    
    /** Number of quad cells **/

    int NumberOfCells(void) { return NumberOfCells_; };    
    
    /** Insert a point into the quad tree mesh **/
    
    int InsertPoint(double xyz[3], int SurfaceEdge);
    
    /** Quad tree direction ... x, y, z ... ie, 1, 2, 3 **/
    
    int &Direction(void) { return Direction_; };
    
    /** Smooth the pressure field **/

    void SmoothPressure(void);

};

#include "END_NAME_SPACE.H"

#endif
