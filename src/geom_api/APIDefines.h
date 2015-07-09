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
                    VSP_INVALID_ID,
                    VSP_CANT_SET_NOT_EQ_PARM
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
                    IMPORT_XSEC_MESH,
                    IMPORT_PTS
                 };

enum EXPORT_TYPE {  EXPORT_FELISA,
                    EXPORT_XSEC,
                    EXPORT_STL,
                    EXPORT_AWAVE,
                    EXPORT_NASCART,
                    EXPORT_POVRAY,
                    EXPORT_CART3D,
                    EXPORT_VORXSEC,
                    EXPORT_XSECGEOM,
                    EXPORT_GMSH,
                    EXPORT_X3D,
                    EXPORT_STEP
                 };

enum COMPUTATION_FILE_TYPE  {   NO_FILE_TYPE        = 0,
                                COMP_GEOM_TXT_TYPE  = 1,
                                COMP_GEOM_CSV_TYPE  = 2,
                                DRAG_BUILD_TSV_TYPE = 4,
                                SLICE_TXT_TYPE      = 8,
                                MASS_PROP_TXT_TYPE  = 16,
                                DEGEN_GEOM_CSV_TYPE = 32,
                                DEGEN_GEOM_M_TYPE   = 64,
                                CFD_STL_TYPE        = 128,
                                CFD_POLY_TYPE       = 256,
                                CFD_TRI_TYPE        = 512,
                                CFD_OBJ_TYPE        = 1024,
                                CFD_DAT_TYPE        = 2048,
                                CFD_KEY_TYPE        = 4096,
                                CFD_GMSH_TYPE       = 8192,
                                CFD_SRF_TYPE        = 16384,
                                CFD_TKEY_TYPE       = 32768,
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

enum CFD_CONTROL_TYPE {     CFD_MIN_EDGE_LEN,
                            CFD_MAX_EDGE_LEN,
                            CFD_MAX_GAP,
                            CFD_NUM_CIRCLE_SEGS,
                            CFD_GROWTH_RATIO,
                            CFD_LIMIT_GROWTH_FLAG,
                            CFD_INTERSECT_SUBSURFACE_FLAG,
                            CFD_HALF_MESH_FLAG,
                            CFD_FAR_FIELD_FLAG,
                            CFD_FAR_MAX_EDGE_LEN,
                            CFD_FAR_MAX_GAP,
                            CFD_FAR_NUM_CIRCLE_SEGS,
                            CFD_FAR_SIZE_ABS_FLAG,
                            CFD_FAR_LENGTH,
                            CFD_FAR_WIDTH,
                            CFD_FAR_HEIGHT,
                            CFD_FAR_X_SCALE,
                            CFD_FAR_Y_SCALE,
                            CFD_FAR_Z_SCALE,
                            CFD_FAR_LOC_MAN_FLAG,
                            CFD_FAR_LOC_X,
                            CFD_FAR_LOC_Y,
                            CFD_FAR_LOC_Z,
                            CFD_WAKE_SCALE,
                            CFD_WAKE_ANGLE,
};

enum CFD_MESH_SOURCE_TYPE { POINT_SOURCE,
                            LINE_SOURCE,
                            BOX_SOURCE,
                            NUM_SOURCE_TYPES,
                          };

enum XDDM_QUANTITY_TYPE { XDDM_VAR,
                          XDDM_CONST,
};


enum VSP_SURF_TYPE { NORMAL_SURF,
                     WING_SURF,
                     DISK_SURF,
                     NUM_SURF_TYPES,
               };

enum VSP_SURF_CFD_TYPE { CFD_NORMAL,
                         CFD_NEGATIVE,
                         CFD_TRANSPARENT,
                         CFD_NUM_TYPES,
                };

enum SUBSURF_TYPE { SS_LINE,
                    SS_RECTANGLE,
                    SS_ELLIPSE,
                    SS_NUM_TYPES };

enum WING_DRIVERS { AR_WSECT_DRIVER,
                    SPAN_WSECT_DRIVER,
                    AREA_WSECT_DRIVER,
                    TAPER_WSECT_DRIVER,
                    AVEC_WSECT_DRIVER,
                    ROOTC_WSECT_DRIVER,
                    TIPC_WSECT_DRIVER,
                    SECSWEEP_WSECT_DRIVER,
                    NUM_WSECT_DRIVER,
                    SWEEP_WSECT_DRIVER = SECSWEEP_WSECT_DRIVER + 1,
                    SWEEPLOC_WSECT_DRIVER = SECSWEEP_WSECT_DRIVER + 2,
                    SECSWEEPLOC_WSECT_DRIVER = SECSWEEP_WSECT_DRIVER + 3,
                  };

}   // Namespace

#endif // !defined(VSPDEFINES__INCLUDED_)
