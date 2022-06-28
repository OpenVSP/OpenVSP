//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#ifndef QUAD_EDGE_H
#define QUAD_EDGE_H

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.H"

#include "START_NAME_SPACE.H"

class QUAD_EDGE {

private:

    int Cell_[2];

public:
    
    QUAD_EDGE(void);
   ~QUAD_EDGE(void);
    QUAD_EDGE(const QUAD_EDGE &QuadEdge);
    
    int &Cell(int i) { return Cell_[i]; };

};

#include "END_NAME_SPACE.H"

#endif
