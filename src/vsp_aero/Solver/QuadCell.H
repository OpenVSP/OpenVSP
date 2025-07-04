//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#ifndef QUAD_CELL_H
#define QUAD_CELL_H

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.H"
#include "QuadNode.H"
#include "QuadEdge.H"

#include "START_NAME_SPACE.H"

class QUAD_CELL {
   
private:

    int HasPoint_;
    int SurfaceEdge_;
    int Node_[4];    
    int Child_[4];
    int Edge_[8];
    int Parent_;
    int Level_;
    
    double InsertedPoint_[3];    

public:

    QUAD_CELL(void);
   ~QUAD_CELL(void);
    QUAD_CELL(const QUAD_CELL &QuadCell);
    
    /** Cell has a point in it **/
    
    int &HasPoint(void) { return HasPoint_; };
    
    /** Surface edge this point (may) be related to **/
    
    int &SurfaceEdge(void) { return SurfaceEdge_; };
    
    /** i'th node for this cell **/
    
    int &Node(int i) { return Node_[i]; };
    
    /** i'th child for this cell **/
    
    int &Child(int i) { return Child_[i]; };
    
    /** i'th edge for this cell **/
    
    int &Edge(int i) { return Edge_[i]; };
    
    /** Parent cell for this edge **/
    
    int &Parent(void) { return Parent_; };
    
    /** Quad tree level **/
    
    int &Level(void) { return Level_; };
    
    /** Neighbor cell... up to eight cells **/
    
    int NeighborCell(int i);

    /** Inserted point vector **/
    
    double *InsertedPoint(void) { return InsertedPoint_; };         
    
    /** Inserted point coordinates **/
    
    double &InsertedPoint(int i) { return InsertedPoint_[i]; }; 

};

#include "END_NAME_SPACE.H"

#endif
