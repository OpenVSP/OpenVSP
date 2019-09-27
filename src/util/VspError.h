//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

#if !defined(VSP_VSP_ERROR__INCLUDED_)
#define VSP_VSP_ERROR__INCLUDED_


#include <string>

#include "MessageMgr.h"
#include "StackTrace.h"

#define VSPERROR( msg ) ( MessageMgr::getInstance().SendError( ( msg ), ( __func__ ), ( __FILE__ ), ( __LINE__ ), GetStackTrace( 0 ) ) )

#endif
