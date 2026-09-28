//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// CloneDeleteDialog.h: asks what becomes of the Clones of Geoms being deleted or cut.
//
//////////////////////////////////////////////////////////////////////

#ifndef CLONEDELETEDIALOG_H
#define CLONEDELETEDIALOG_H

#include <string>
#include <vector>

class Vehicle;

// Deletes or cuts the selected Geoms.  If that would leave a Clone with nothing to copy, asks
// first what to do with it; the user may cancel.
void DeleteOrCutActiveGeomVec( Vehicle* veh, bool cut );

// Returns a vsp::CLONE_DELETE_TYPE for these Clones, or -1 for cancel.
int AskCloneDelete( Vehicle* veh, const std::vector< std::string > & clone_vec, bool cut );

#endif // CLONEDELETEDIALOG_H
