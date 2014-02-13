//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// API.h: interface for the Vehicle Class and Vehicle Mgr Singleton.
// J.R Gloudemans
//
//////////////////////////////////////////////////////////////////////

#if !defined(VSPDEFINES__INCLUDED_)
#define VSPDEFINES__INCLUDED_

namespace vsp
{
//======================== Error Codes ================================//
enum ErrorCode {    VSP_OK,
                    VSP_INVALID_PTR,
                    VSP_CANT_FIND_TYPE,
                    VSP_CANT_FIND_PARM,
                    VSP_CANT_FIND_NAME,
                    VSP_INVALID_GEOM_ID,
                    VSP_FILE_DOES_NOT_EXIST,
                    VSP_FILE_WRITE_FAILURE,
                    VSP_WRONG_XSEC_TYPE,
                    VSP_WRONG_FILE_TYPE,
                    VSP_INDEX_OUT_RANGE,
                    VSP_INVALID_XSEC_ID
               };

enum {  SYM_XY = ( 1 << 0 ),
        SYM_XZ = ( 1 << 1 ),
        SYM_YZ = ( 1 << 2 ),
        SYM_ROT_X = ( 1 << 3 ),
        SYM_ROT_Y = ( 1 << 4 ),
        SYM_ROT_Z = ( 1 << 5 ),
        SYM_PLANAR_TYPES = 3,
        SYM_NUM_TYPES = 6
     }; // Symmetry Flags

enum {  POINT,
        CIRCLE,
        SUPER_ELLIPSE,
        ROUNDED_RECTANGLE,
        GENERAL_FUSE,
        FILE_FUSE,
        FOUR_SERIES,
        SIX_SERIES,
        BICONVEX,
        WEDGE,
        BEZIER,
        FILE_AIRFOIL,
        NUM_TYPES
     };

enum {   COMP_GEOM_TXT_TYPE,
         COMP_GEOM_CSV_TYPE,
         SLICE_TXT_TYPE,
         MASS_PROP_TXT_TYPE,
         DRAG_BUILD_TSV_TYPE
     };

enum {   SET_ALL = 0,
         SET_SHOWN = 1,
         SET_NOT_SHOWN = 2,
         SET_FIRST_USER = 3
     };

}   // Namespace

#endif // !defined(VSPDEFINES__INCLUDED_)
