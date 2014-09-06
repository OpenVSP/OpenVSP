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
enum ERROR_CODE {   VSP_OK,
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
                    VSP_INVALID_XSEC_ID,
                    VSP_INVALID_ID
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

enum XSEC_TYPE { XSEC_FUSE,
                 XSEC_STACK,
                 XSEC_WING,
                 XSEC_CUSTOM,
                 XSEC_NUM_TYPES
               };

enum DIR_INDEX { X_DIR = 0,
                 Y_DIR = 1,
                 Z_DIR = 2
               };

enum XSEC_WIDTH_SHIFT { XS_SHIFT_LE = 0,
                        XS_SHIFT_MID = 1,
                        XS_SHIFT_TE = 2
                      };

enum XSEC_CRV_TYPE {XS_POINT,
                    XS_CIRCLE,
                    XS_ELLIPSE,
                    XS_SUPER_ELLIPSE,
                    XS_ROUNDED_RECTANGLE,
                    XS_GENERAL_FUSE,
                    XS_FILE_FUSE,
                    XS_FOUR_SERIES,
                    XS_SIX_SERIES,
                    XS_BICONVEX,
                    XS_WEDGE,
                    XS_BEZIER,
                    XS_FILE_AIRFOIL,
                    XS_NUM_TYPES
               };

enum XSEC_SIDES_TYPE {  XSEC_BOTH_SIDES,
                        XSEC_LEFT_SIDE,
                        XSEC_RIGHT_SIDE };

enum IMPORT_TYPE {  IMPORT_STL,
                    IMPORT_NASCART,
                    IMPORT_CART3D_TRI,
                    IMPORT_XSEC_SURF,
                    IMPORT_XSEC_MESH
                 };

enum EXPORT_TYPE {  EXPORT_FELISA,
                    EXPORT_XSEC,
                    EXPORT_STL,
                    EXPORT_RHINO3D,
                    EXPORT_AWAVE,
                    EXPORT_NASCART,
                    EXPORT_POVRAY,
                    EXPORT_CART3D,
                    EXPORT_VORXSEC,
                    EXPORT_XSECGEOM,
                    EXPORT_GMSH,
                    EXPORT_X3D,
                    EXPORT_STEP,
                    EXPORT_BEZ
                 };

enum COMPUTATION_FILE_TYPE  {   NO_FILE_TYPE        = 0,
                                COMP_GEOM_TXT_TYPE  = 1,
                                COMP_GEOM_CSV_TYPE  = 2,
                                DRAG_BUILD_TSV_TYPE = 4,
                                SLICE_TXT_TYPE      = 8,
                                MASS_PROP_TXT_TYPE  = 16,
                                DEGEN_GEOM_CSV_TYPE = 32,
                                DEGEN_GEOM_M_TYPE   = 64,
                            };

enum SLICE_STYLE {  SLICE_PLANAR,
                    SLICE_AWAVE,
                 };


enum SET_TYPE { SET_ALL = 0,
                SET_SHOWN = 1,
                SET_NOT_SHOWN = 2,
                SET_FIRST_USER = 3
              };

enum RES_DATA_TYPE {    INT_DATA,
                        DOUBLE_DATA,
                        STRING_DATA,
                        VEC3D_DATA
                   };

enum RES_GEOM_TYPE {    MESH_INDEXED_TRI,
                        MESH_SLICE_TRI,
                        GEOM_XSECS,
                   };

enum CFD_MESH_EXPORT_TYPE { CFD_STL_FILE_NAME,
                            CFD_POLY_FILE_NAME,
                            CFD_TRI_FILE_NAME,
                            CFD_OBJ_FILE_NAME,
                            CFD_DAT_FILE_NAME,
                            CFD_KEY_FILE_NAME,
                            CFD_GMSH_FILE_NAME,
                            CFD_SRF_FILE_NAME,
                            CFD_TKEY_FILE_NAME,
                            CFD_NUM_FILE_NAMES,
                        };
}   // Namespace

#endif // !defined(VSPDEFINES__INCLUDED_)
