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
/*!
	\ingroup Enumerations
*/
/*! Enum that indicates if positions are relative or absolute. */
enum ABS_REL_FLAG { ABS,	/*!< Absolute position */
                    REL	/*!< Relative position */
};

/*!
	\ingroup Enumerations
*/
/*! Enum used to identify the format of exported airfoil files. */
enum AIRFOIL_EXPORT_TYPE { SELIG_AF_EXPORT,	/*!< Selig airfoil file format */
                           BEZIER_AF_EXPORT	/*!< Bezier airfoil file format */
};

/*!
	\ingroup Enumerations
*/
/*! Enum for specifying alignment. */
enum ALIGN_TYPE { ALIGN_LEFT,	/*!< Align to left */
                  ALIGN_CENTER,	/*!< Align to center */
                  ALIGN_RIGHT,	/*!< Align to right */
                  ALIGN_PIXEL,	/*!< Align to specified pixel */
                  ALIGN_TOP,	/*!< Align to top */
                  ALIGN_MIDDLE,	/*!< Align to middle */
                  ALIGN_BOTTOM,	/*!< Align to bottom */
                  NUM_ALIGN_TYPE,	/*!< Number of alignment types */
};

/*!
	\ingroup Enumerations
*/
/*! Enum for specifying angular units. */
enum ANG_UNITS { ANG_RAD,	/*!< Radians */
                 ANG_DEG	/*!< Degrees */
};

/*!
	\ingroup Enumerations
*/
/*! Enum for specifying image rotation angles (CW). */
enum ANGLE { ANG_0,	/*!< Zero deg */
             ANG_90,	/*!< 90 deg */
             ANG_180,	/*!< 180 deg */
             ANG_270,	/*!< 270 deg */
             NUM_ANG,	/*!< Number of angle choices */
};

/*!
	\ingroup Enumerations
*/
/*! Enum that identifies the atmospheric model used in the Parasite Drag tool. */
enum ATMOS_TYPE { ATMOS_TYPE_US_STANDARD_1976 = 0,	/*!< US Standard Atmosphere 1976 (default) */
                  ATMOS_TYPE_HERRINGTON_1966,	/*!< USAF 1966 */
                  ATMOS_TYPE_MANUAL_P_R,	/*!< Manual: pressure and density control */
                  ATMOS_TYPE_MANUAL_P_T,	/*!< Manual: pressure and temperature control */
                  ATMOS_TYPE_MANUAL_R_T,	/*!< Manual: density and temperature control */
                  ATMOS_TYPE_MANUAL_RE_L	/*!< Manual: Reynolds number and length control */
};

/*!
	\ingroup Enumerations
*/
/*! Enum that identifies the parent to child relative translation coordinate system. */
enum ATTACH_TRANS_TYPE { ATTACH_TRANS_NONE = 0,	/*!< No parent attachment for translations */
                         ATTACH_TRANS_COMP,	/*!< Translation relative to parent body axes */
                         ATTACH_TRANS_UV,	/*!< Translation relative to parent surface coordinate frame */
                         ATTACH_TRANS_RST,	/*!< Translation relative to parent per-section volume coordinate frame */
                         ATTACH_TRANS_LMN,	/*!< Translation relative to parent uniform volume coordinate frame */
                         ATTACH_TRANS_EtaMN,	/*!< Translation relative to wing parent uniform eta volume coordinate frame */
                         ATTACH_TRANS_NUM_TYPES	/*!< Number of translation attachment types */
};

/*!
	\ingroup Enumerations
*/
/*! Enum that determines parent to child relative rotation axes. */
enum ATTACH_ROT_TYPE { ATTACH_ROT_NONE = 0,	/*!< No parent attachment for rotations */
                       ATTACH_ROT_COMP,	/*!< Rotation relative to parent body axes */
                       ATTACH_ROT_UV,	/*!< Rotation relative to parent surface coordinate frame */
                       ATTACH_ROT_RST,	/*!< Rotation relative to parent per-section volume coordinate frame */
                       ATTACH_ROT_LMN,	/*!< Rotation relative to parent uniform volume coordinate frame */
                       ATTACH_ROT_EtaMN,	/*!< Rotation relative to wing parent eta volume coordinate frame */
                       ATTACH_ROT_NUM_TYPES	/*!< Number of rotation attachment types */
};

/*!
	\ingroup Enumerations
*/
/*! Enum that entity type. */
enum ATTRIBUTABLE_TYPE { ATTROBJ_PARM = 0,	/*!< Parm */
                         ATTROBJ_GEOM,	/*!< Geom */
                         ATTROBJ_VEH,	/*!< Vehicle */
                         ATTROBJ_SUBSURF,	/*!< SubSurf */
                         ATTROBJ_MEASURE,	/*!< Measure */
                         ATTROBJ_LINK,	/*!< Link */
                         ATTROBJ_ADVLINK,	/*!< Adv Link */
                         ATTROBJ_ATTR,	/*!< Attribute  */
                         ATTROBJ_COLLECTION,	/*!< Attribute Collection */
                         ATTROBJ_XSEC,	/*!< Cross Section */
                         ATTROBJ_SEC,	/*!< Wing Section */
                         ATTROBJ_MODE,	/*!< Mode */
                         ATTROBJ_SET,	/*!< Geom Set */
                         ATTROBJ_VARGROUP,	/*!< Var Preset Group */
                         ATTROBJ_VARSETTING,	/*!< Var Preset Setting*/
                         ATTROBJ_FREE,	/*!< Unattached attribute */
};

/*!
	\ingroup Enumerations
*/
/*! Enum for attribute event group. */
enum ATTRIBUTE_EVENT_GROUP { ATTR_GROUP_NONE = -1,	/*!< No event (user attributes) */
                             ATTR_GROUP_WATERMARK,	/*!< Watermark group */
                             NUM_ATTR_EVENT_GROUPS	/*!< Number attribute event groups */
};

/*!
	\ingroup Enumerations
*/
/*! Enum for bogie spacing control. */
enum BOGIE_SPACING_TYPE { BOGIE_CENTER_DIST,	/*!< Distance between centers */
                          BOGIE_CENTER_DIST_FRAC,	/*!< Fractional distance between centers */
                          BOGIE_GAP,	/*!< Gap */
                          BOGIE_GAP_FRAC,	/*!< Fractional gap */
                          NUM_BOGIE_SPACING_TYPE,	/*!< Number of bogie spacing control modes */
};

/*!
	\ingroup Enumerations
*/
/*! Enum for Body of Revolution mode control. */
enum BOR_MODE { BOR_FLOWTHROUGH,	/*!< Flowthrough mode (default) */
                BOR_UPPER,	/*!< Upper surface mode */
                BOR_LOWER,	/*!< Lower surface mode */
                BOR_NUM_MODES	/*!< Number of Body of Revolution modes */
};

/*!
	\ingroup Enumerations
*/
/*! Enum used to select between maximum camber or ideal lift coefficient inputs. */
enum CAMBER_INPUT_FLAG { MAX_CAMB,	/*!< Input maximum camber, calculate ideal lift coefficient */
                         DESIGN_CL	/*!< Input ideal lift coefficient, calculate maximum camber */
};

/*!
	\ingroup Enumerations
*/
/*! Enum used to set camera view. */
enum CAMERA_VIEW { CAM_TOP,	/*!< Camera top view */
                   CAM_FRONT,	/*!< Camera front view */
                   CAM_FRONT_YUP,	/*!< Camera front Y-up view */
                   CAM_LEFT,	/*!< Camera left view */
                   CAM_LEFT_ISO,	/*!< Camera left isometric view */
                   CAM_BOTTOM,	/*!< Camera bottom view */
                   CAM_REAR,	/*!< Camera rear view */
                   CAM_RIGHT,	/*!< Camera right view */
                   CAM_RIGHT_ISO,	/*!< Camera right isometric view */
                   CAM_CENTER	/*!< Camera center view */
};

/*!
	\ingroup Enumerations
*/
/*! Enum that identifies the end cap types for a geometry (i.e. wing root and tip). */
enum CAP_TYPE { NO_END_CAP,	/*!< No end cap */
                FLAT_END_CAP,	/*!< Flat end cap */
                ROUND_END_CAP,	/*!< Round end cap */
                EDGE_END_CAP,	/*!< Edge end cap */
                SHARP_END_CAP,	/*!< Sharp end cap */
                POINT_END_CAP,	/*!< Point end cap */
                ROUND_EXT_END_CAP_NONE,	/*!< Extended round end cap, but not extended */
                ROUND_EXT_END_CAP_LE,	/*!< Extended round end cap, extend LE */
                ROUND_EXT_END_CAP_TE,	/*!< Extended round end cap, extend TE */
                ROUND_EXT_END_CAP_BOTH,	/*!< Extended round end cap, extend both */
                NUM_END_CAP_OPTIONS	/*!< Number of end cap options */
};

/*!
	\ingroup Enumerations
*/
/*! Enum used to identify each CFD mesh control option (also applicable to FEA Mesh). \\sa SetCFDMeshVal(), SetFEAMeshVal() */
enum CFD_CONTROL_TYPE {     CFD_MIN_EDGE_LEN,	/*!< Minimum mesh edge length */
                            CFD_MAX_EDGE_LEN,	/*!< Maximum mesh edge length */
                            CFD_MAX_GAP,	/*!< Maximum mesh edge gap */
                            CFD_NUM_CIRCLE_SEGS,	/*!< Number of edge segments to resolve circle */
                            CFD_GROWTH_RATIO,	/*!< Maximum allowed edge growth ratio */
                            CFD_LIMIT_GROWTH_FLAG,	/*!< Rigorous 3D growth limiting flag */
                            CFD_INTERSECT_SUBSURFACE_FLAG,	/*!< Flag to intersect sub-surfaces */
                            CFD_HALF_MESH_FLAG,	/*!< Flag to generate a half mesh */
                            CFD_FAR_FIELD_FLAG,	/*!< Flag to generate a far field mesh */
                            CFD_FAR_MAX_EDGE_LEN,	/*!< Maximum far field mesh edge length */
                            CFD_FAR_MAX_GAP,	/*!< Maximum far field mesh edge gap */
                            CFD_FAR_NUM_CIRCLE_SEGS,	/*!< Number of far field edge segments to resolve circle */
                            CFD_FAR_SIZE_ABS_FLAG,	/*!< Relative or absolute size flag */
                            CFD_FAR_LENGTH,	/*!< Far field length */
                            CFD_FAR_WIDTH,	/*!< Far field width */
                            CFD_FAR_HEIGHT,	/*!< Far field height */
                            CFD_FAR_X_SCALE,	/*!<  Far field X scale */
                            CFD_FAR_Y_SCALE,	/*!<  Far field Y scale */
                            CFD_FAR_Z_SCALE,	/*!<  Far field Z scale */
                            CFD_FAR_LOC_MAN_FLAG,	/*!< Far field location flag: centered or manual */
                            CFD_FAR_LOC_X,	/*!< Far field X location */
                            CFD_FAR_LOC_Y,	/*!< Far field Y location */
                            CFD_FAR_LOC_Z,	/*!< Far field Z location */
                            CFD_SRF_XYZ_FLAG,	/*!< Flag to include X,Y,Z intersection curves in export files */
};

/*!
	\ingroup Enumerations
*/
/*! Enum used to describe various CFD Mesh export file options. \\sa SetComputationFileName(), ComputeCFDMesh() */
enum CFD_MESH_EXPORT_TYPE { CFD_STL_FILE_NAME,	/*!< STL export type */
                            CFD_POLY_FILE_NAME,	/*!< POLY export type */
                            CFD_TRI_FILE_NAME,	/*!< TRI export type */
                            CFD_OBJ_FILE_NAME,	/*!< OBJ export type */
                            CFD_DAT_FILE_NAME,	/*!< DAT export type */
                            CFD_KEY_FILE_NAME,	/*!< KEY export type */
                            CFD_GMSH_FILE_NAME,	/*!< GMSH export type */
                            CFD_TKEY_FILE_NAME,	/*!< TKEY export type */
                            CFD_FACET_FILE_NAME,	/*!< FACET export type */
                            CFD_VSPGEOM_FILE_NAME,	/*!< VSPGEOM export type */
                            CFD_NUM_FILE_NAMES,	/*!< Number of CFD Mesh export file types */
};

/*!
	\ingroup Enumerations
*/
/*! Enum that indicates the CFD Mesh source type. \\sa AddCFDSource() */
enum CFD_MESH_SOURCE_TYPE { POINT_SOURCE,	/*!< Point source */
                            LINE_SOURCE,	/*!< Line source */
                            BOX_SOURCE,	/*!< Box source */
                            ULINE_SOURCE,	/*!< Constant U Line source */
                            WLINE_SOURCE,	/*!< Constant W Line source */
                            NUM_SOURCE_TYPES,	/*!< Number of CFD Mesh source types */
};

/*!
	\ingroup Enumerations
*/
/*! Enum that identifies how CFD and FEA meshes are colored. */
enum CFD_VIS_TYPE { TAG,    /*!< Color mesh by tag value (component, subsurface, part, etc) */
                    REASON    /*!< Color mesh by local edge length reason */
};

/*!
	\ingroup Enumerations
*/
/*! Enum that identifies the Parasite Drag Tool laminar friction coefficient equation choice. */
enum CF_LAM_EQN { CF_LAM_BLASIUS = 0,	/*!< Blasius laminar Cf equation */
                  CF_LAM_BLASIUS_W_HEAT,	/*!< Blasius laminar Cf equation with heat (NOT IMPLEMENTED) */  // TODO: Remove or implement
};

/*!
	\ingroup Enumerations
*/
/*! Enum that identifies the Parasite Drag Tool turbulent friction coefficient equation choice. */
enum CF_TURB_EQN { CF_TURB_EXPLICIT_FIT_SPALDING = 0,	/*!< Explicit Fit of Spalding turbulent Cf equation */
                   CF_TURB_EXPLICIT_FIT_SPALDING_CHI,	/*!< Explicit Fit of Spalding and Chi turbulent Cf equation */
                   CF_TURB_EXPLICIT_FIT_SCHOENHERR,	/*!< Explicit Fit of Schoenherr turbulent Cf equation */
                   DO_NOT_USE_CF_TURB_IMPLICIT_KARMAN,	/*!< Implicit Karman turbulent Cf equation (DO NOT USE) */
                   CF_TURB_IMPLICIT_SCHOENHERR,	/*!< Implicit Schoenherr turbulent Cf equation */
                   CF_TURB_IMPLICIT_KARMAN_SCHOENHERR,	/*!< Implicit Karman-Schoenherr turbulent Cf equation */
                   CF_TURB_POWER_LAW_BLASIUS,	/*!< Power Law Blasius turbulent Cf equation */
                   CF_TURB_POWER_LAW_PRANDTL_LOW_RE,	/*!<Power Law Prandtl Low Re turbulent Cf equation */
                   CF_TURB_POWER_LAW_PRANDTL_MEDIUM_RE,	/*!< Power Law Prandtl Medium Re turbulent Cf equation */
                   CF_TURB_POWER_LAW_PRANDTL_HIGH_RE,	/*!< Power Law Prandtl High Re turbulent Cf equation */
                   CF_TURB_SCHLICHTING_COMPRESSIBLE,	/*!< Schlichting Compressible turbulent Cf equation */
                   DO_NOT_USE_CF_TURB_SCHLICHTING_INCOMPRESSIBLE,	/*!< Schlichting Incompressible turbulent Cf equation (DO NOT USE) */
                   DO_NOT_USE_CF_TURB_SCHLICHTING_PRANDTL,	/*!< Schlichting-Prandtl turbulent Cf equation (DO NOT USE) */
                   DO_NOT_USE_CF_TURB_SCHULTZ_GRUNOW_HIGH_RE,	/*!< Schultz-Grunow High Re turbulent Cf equation (DO NOT USE) */
                   CF_TURB_SCHULTZ_GRUNOW_SCHOENHERR,	/*!< Schultz-Grunow Estimate of Schoenherr turbulent Cf equation. */
                   DO_NOT_USE_CF_TURB_WHITE_CHRISTOPH_COMPRESSIBLE,	/*!< White-Christoph Compressible turbulent Cf equation (DO NOT USE) */
                   CF_TURB_ROUGHNESS_SCHLICHTING_AVG,	/*!< Roughness Schlichting Avg turbulent Cf equation. */
                   DO_NOT_USE_CF_TURB_ROUGHNESS_SCHLICHTING_LOCAL,	/*!< Roughness Schlichting Local turbulent Cf equation (DO NOT USE) */
                   DO_NOT_USE_CF_TURB_ROUGHNESS_WHITE,	/*!< Roughness White turbulent Cf equation (DO NOT USE) */
                   CF_TURB_ROUGHNESS_SCHLICHTING_AVG_FLOW_CORRECTION,	/*!< Roughness Schlichting Avg Compressible turbulent Cf equation. */
                   CF_TURB_HEATTRANSFER_WHITE_CHRISTOPH	/*!< Heat Transfer White-Christoph turbulent Cf equation. */
};

/*!
	\ingroup Enumerations
*/
/*! Enum for Chevron curve modification types. */
enum CHEVRON_TYPE { CHEVRON_NONE,	/*!< No chevron. */
                    CHEVRON_PARTIAL,	/*!< One or more chevrons of limited extent. */
                    CHEVRON_FULL,	/*!< Full period of chevrons. */
                    CHEVRON_NUM_TYPES	/*!< Number of chevron types. */
};

/*!
	\ingroup Enumerations
*/
/*! Enum for Chevron W parameter modes. */
enum CHEVRON_W01_MODES { CHEVRON_W01_SE, // Start and End	
                         CHEVRON_W01_CW, // Center and Width	
                         CHEVRON_W01_NUM_MODES	/*!< Number of chevron W parameter mode types. */
};

/*!
	\ingroup Enumerations
*/
/*! Enum for clearance geom modes. */
enum CLEARANCE_GEOM_MODE { CLEARANCE_ROTOR_TIP_PATH,	/*!< Rotor tip path */
                           CLEARANCE_ROTOR_BURST,	/*!< Rotor burst zone */
                           NUM_CLEARANCE_MODES	/*!< Number of clearance geom modes. */
};

/*!
	\ingroup Enumerations
*/
/*! Enum for Snap To collision error types. */
enum COLLISION_ERRORS { COLLISION_OK,	/*!< No Error. */
                        COLLISION_INTERSECT_NO_SOLUTION,	/*!< Touching, no solution */
                        COLLISION_CLEAR_NO_SOLUTION,	/*!< Not touching, no solution */
};

/*!
	\ingroup Enumerations
*/
/*! Enum used to identify various export file types. */
enum COMPUTATION_FILE_TYPE  {   NO_FILE_TYPE        = 0,	/*!< No export file type */
                                COMP_GEOM_TXT_TYPE  = 1,	
                                COMP_GEOM_CSV_TYPE  = 1<<1,	
                                DRAG_BUILD_TSV_TYPE_DEPRECATED = 1<<2,	
                                SLICE_TXT_TYPE      = 1<<3,	
                                MASS_PROP_TXT_TYPE  = 1<<4,	
                                DEGEN_GEOM_CSV_TYPE = 1<<5,	
                                DEGEN_GEOM_M_TYPE   = 1<<6,	
                                CFD_STL_TYPE        = 1<<7,	
                                CFD_POLY_TYPE       = 1<<8,	
                                CFD_TRI_TYPE        = 1<<9,	
                                CFD_OBJ_TYPE        = 1<<10,	
                                CFD_DAT_TYPE        = 1<<11,	
                                CFD_KEY_TYPE        = 1<<12,	
                                CFD_GMSH_TYPE       = 1<<13,	
                                CFD_SRF_TYPE_DEPRECATED        = 1<<14,	
                                CFD_TKEY_TYPE       = 1<<15,	
                                PROJ_AREA_CSV_TYPE  = 1<<16,	
                                WAVE_DRAG_TXT_TYPE  = 1<<17,	
                                VSPAERO_PANEL_TRI_TYPE = 1<<18,	
                                DRAG_BUILD_CSV_TYPE = 1<<19,	
                                CFD_FACET_TYPE      = 1<<20,	
                                CFD_CURV_TYPE_DEPRECATED       = 1<<21,	
                                CFD_PLOT3D_TYPE_DEPRECATED     = 1<<22,	
                                CFD_VSPGEOM_TYPE    = 1<<23,	
                                VSPAERO_VSPGEOM_TYPE = 1<<24,	
};

/*!
	\ingroup Enumerations
*/
/*! Enum used to identify conformal component trim type. */
enum CONFORMAL_TRIM_TYPE { U_TRIM,	/*!< Trim by U coordinate */
                           L_TRIM,	/*!< Trim by L coordinate */
                           ETA_TRIM,	/*!< Trim by Eta coordinate */
                           NUM_TRIM_TYPES	/*!< Number of conformal component trim types */
};

/*!
	\ingroup Enumerations
*/
/*! Enum used to identify delimiter type. */
enum DELIM_TYPE { DELIM_COMMA,	/*!< Comma delimiter */
                  DELIM_USCORE,	/*!< Underscore delimiter */
                  DELIM_SPACE,	/*!< Space delimiter */
                  DELIM_NONE,	/*!< No delimiter */
                  DELIM_NUM_TYPES	/*!< Number of delimiter types */
};

/*!
	\ingroup Enumerations
*/
/*! Enum for 3D background depth location settings. */
enum DEPTH_TYPE { DEPTH_FRONT,	/*!< Set 3D background before model */
                  DEPTH_REAR,	/*!< Set 3D background behind model */
                  DEPTH_FREE,	/*!< Set 3D background at specified location */
                  NUM_DEPTH_TYPE	/*!< Number of depth types */
};


/*!
	\ingroup Enumerations
*/
/*! Enum for 2D or 3D DXF export options. */
enum DIMENSION_SET { SET_3D,	/*!< 3D DXF export (default) */
                     SET_2D,	/*!< 2D DXF export */
};

/*!
	\ingroup Enumerations
*/
/*! Enum used to identify axis of rotation or translation. */
enum DIR_INDEX { X_DIR = 0,	/*!< X direction */
                 Y_DIR = 1,	
                 Z_DIR = 2,	
                 ALL_DIR = 3	
};

/*!
	\ingroup Enumerations
*/
/*! Enum for selecting the GUI display type for Geoms. */
enum DISPLAY_TYPE { DISPLAY_BEZIER,	/*!< Display the normal Bezier surface (default) */
                    DISPLAY_DEGEN_SURF,	/*!< Display as surface Degen Geom */
                    DISPLAY_DEGEN_PLATE,	/*!< Display as plate Degen Geom */
                    DISPLAY_DEGEN_CAMBER	/*!< Display as camber Degen Geom */
};

/*!
	\ingroup Enumerations
*/
/*! Enum for selecting the GUI draw type for Geoms. */
enum DRAW_TYPE { GEOM_DRAW_WIRE,	/*!< Draw the wireframe mesh (see through) */
                 GEOM_DRAW_HIDDEN,	/*!< Draw the hidden mesh */
                 GEOM_DRAW_SHADE,	/*!< Draw the shaded mesh */
                 GEOM_DRAW_TEXTURE,	/*!< Draw the textured mesh */
                 GEOM_DRAW_NONE	/*!< Do not draw anything */
};

/*!
	\ingroup Enumerations
*/
/*! Enum for identifying which kind of integrated flowpath modeling components is modeled. */
enum ENGINE_GEOM_IO_TYPE { ENGINE_GEOM_NONE,	/*!< Component is not an integrated flowpath component. */
                           ENGINE_GEOM_INLET,	/*!< Component represents integrated flowpath inlet. */
                           ENGINE_GEOM_INLET_OUTLET,	/*!< Component represents integrated flowpath inlet and outlet. */
                           ENGINE_GEOM_OUTLET,	/*!< Component represents integrated flowpath outlet. */
                           ENGINE_GEOM_IO_NUM_TYPES	/*!< Number of integrated flowpath component types. */
};

/*!
	\ingroup Enumerations
*/
/*! Enum for identifying how integrated flowpath modeling engine is represented. */
enum ENGINE_GEOM_TYPE { ENGINE_GEOM_FLOWTHROUGH,	/*!< Component is modeled as flowthrough engine. */
                        ENGINE_GEOM_TO_LIP,	/*!< Component is modeled to the lip. */
                        ENGINE_GEOM_FLOWPATH,	/*!< Component flowpath is modeled. */
                        ENGINE_GEOM_TO_FACE,	/*!< Component is modeled to face. */
                        ENGINE_GEOM_NUM_TYPES	/*!< Number of integrated flowpath modeling types. */
};

/*!
	\ingroup Enumerations
*/
/*! Enum for how integrated flowpath modeling key points are identified. */
enum ENGINE_LOC_MODE { ENGINE_LOC_INDEX,	/*!< Integrated flowpath key point identified by XSec index. */
                       ENGINE_LOC_U	/*!< Integrated flowpath key point identified by U parameter. */
};

/*!
	\ingroup Enumerations
*/
/*! Enum for integrated flowpath modeling key points. */
enum ENGINE_LOC_INDEX_TYPES { ENGINE_LOC_INLET_LIP,	/*!< Integrated flowpath key point is inlet lip. */
                              ENGINE_LOC_INLET_FACE,	/*!< Integrated flowpath key point is inlet face. */
                              ENGINE_LOC_OUTLET_LIP,	/*!< Integrated flowpath key point is outlet lip. */
                              ENGINE_LOC_OUTLET_FACE,	/*!< Integrated flowpath key point is outlet face. */
                              ENGINE_LOC_NUM	/*!< Number of integrated flowpath key point locations. */
};

/*!
	\ingroup Enumerations
*/
/*! Enum for identifying how integrated flowpath modeling components are modeled. */
enum ENGINE_MODE_TYPE { ENGINE_MODE_FLOWTHROUGH,	/*!< Represent integrated flowpath as flowthrough engine. */
                        ENGINE_MODE_FLOWTHROUGH_NEG,	/*!< Represent integrated flowpath as flowthrough engine with negative flowpath. */
                        ENGINE_MODE_TO_LIP,	/*!< Represent integrated flowpath to the lip. */
                        ENGINE_MODE_TO_FACE,	/*!< Represent integrated flowpath to the face. */
                        ENGINE_MODE_TO_FACE_NEG,	/*!< Represent integrated flowpath to the face with negative flowpath to the face. */
                        ENGINE_MODE_EXTEND,	/*!< Represent integrated flowpath with farfield extensions. */
                        ENGINE_MODE_NUM_TYPES	/*!< Number of integrated flowpath representations. */
};

/*!
	\ingroup Enumerations
*/
/*! Enum for OpenVSP API error codes. */
enum ERROR_CODE {   VSP_OK,	/*!< No error */
                    VSP_INVALID_PTR,	/*!< Invalid pointer error */
                    VSP_INVALID_TYPE,	/*!< Invalid type error */
                    VSP_CANT_FIND_TYPE,	/*!< Can't find type error */
                    VSP_CANT_FIND_PARM,	/*!< Can't find parm error */
                    VSP_CANT_FIND_NAME,	/*!< Can't find name error */
                    VSP_INVALID_GEOM_ID,	/*!< Invalid Geom ID error */
                    VSP_FILE_DOES_NOT_EXIST,	/*!< File does not exist error */
                    VSP_FILE_WRITE_FAILURE,	/*!< File write failure error */
                    VSP_FILE_READ_FAILURE,	/*!< File read failure error */
                    VSP_WRONG_GEOM_TYPE,	/*!< Wrong Geom type error */
                    VSP_WRONG_XSEC_TYPE,	/*!< Wrong XSec type error */
                    VSP_WRONG_FILE_TYPE,	/*!< Wrong file type error */
                    VSP_INDEX_OUT_RANGE,	/*!< Index out of range error */
                    VSP_INVALID_XSEC_ID,	/*!< Invalid XSec ID error */
                    VSP_INVALID_ID,	/*!< Invalid ID error */
                    VSP_CANT_SET_NOT_EQ_PARM,	/*!< Can't set NotEqParm error */
                    VSP_AMBIGUOUS_SUBSURF,	/*!< Ambiguous flow-through sub-surface error */
                    VSP_INVALID_VARPRESET_SETNAME,	/*!< Invalid Variable Preset set name error */
                    VSP_INVALID_VARPRESET_GROUPNAME,	/*!< Invalid Variable Preset group name error */
                    VSP_CONFORMAL_PARENT_UNSUPPORTED,	/*!< Unsupported Conformal Geom parent error */
                    VSP_UNEXPECTED_RESET_REMAP_ID,	/*!< Unexpected reset remap ID error */
                    VSP_INVALID_INPUT_VAL,	/*!< Invalid input value error */
                    VSP_INVALID_CF_EQN,	/*!< Invalid friction coefficient equation error */
                    VSP_INVALID_DRIVERS,	/*!< Invalid drivers for driver group */
                    VSP_ADV_LINK_BUILD_FAIL,	/*!< Advanced link build failure */
                    VSP_DEPRECATED,	/*!< This capability has been deprecated and is not longer supported */
                    VSP_LINK_LOOP_DETECTED,	/*!< A parameter link loop was detected and stopped */
                    VSP_DUPLICATE_NAME,	/*!< A duplicate name has been provided */
                    VSP_GUI_DEVICE_DEACTIVATED, /*!< A deactivated GUI device was touched */
                    VSP_COULD_NOT_CREATE_BACKGROUND3D,	/*!< Could not create and add Background3D */
                    VSP_NUM_ERROR_CODE	/*!< Total number of VSP error codes */
};

/*!
	\ingroup Enumerations
*/
/*! Enum used to indicate Parasite Drag Tool excressence type. */
enum EXCRES_TYPE { EXCRESCENCE_COUNT = 0,	/*!< Drag counts excressence type */
                   EXCRESCENCE_CD,	/*!< Drag coefficient excressence type */
                   EXCRESCENCE_PERCENT_GEOM,	/*!< Percent of parent Geom drag coefficient excressence type */
                   EXCRESCENCE_MARGIN,	/*!< Percent margin excressence type */
                   EXCRESCENCE_DRAGAREA,	/*!< Drag area (D/q) excressence type */
};

/*!
	\ingroup Enumerations
*/
/*! Enum for OpenVSP export types. */
enum EXPORT_TYPE {  EXPORT_FELISA,	/*!< FELISA export type (NOT IMPLEMENTED) */  // TODO: Remove or implement
                    EXPORT_XSEC,	/*!< XSec (*.hrm) export type */
                    EXPORT_STL,	/*!< Stereolith (*.stl) export type */
                    EXPORT_AWAVE,	/*!< AWAVE export type (NOT IMPLEMENTED) */  // TODO: Remove or implement
                    EXPORT_NASCART,	/*!< NASCART (*.dat) export type */
                    EXPORT_POVRAY,	/*!< POVRAY (*.pov) export type */
                    EXPORT_CART3D,	/*!< Cart3D (*.tri) export type */
                    EXPORT_VSPGEOM,	/*!< VSPGeom (*.vspgeom) export type */
                    EXPORT_VORXSEC,	/*!< VORXSEC  export type (NOT IMPLEMENTED) */  // TODO: Remove or implement
                    EXPORT_XSECGEOM,	/*!< XSECGEOM export type (NOT IMPLEMENTED) */  // TODO: Remove or implement
                    EXPORT_GMSH,	/*!< Gmsh (*.msh) export type */
                    EXPORT_X3D,	/*!< X3D (*.x3d) export type */
                    EXPORT_STEP,	/*!< STEP (*.stp) export type */
                    EXPORT_PLOT3D,	/*!< PLOT3D (*.p3d) export type */
                    EXPORT_IGES,	/*!< IGES (*.igs) export type */
                    EXPORT_BEM,	/*!< Blade Element (*.bem) export type */
                    EXPORT_DXF,	/*!< AutoCAD (*.dxf) export type */
                    EXPORT_FACET,	/*!< Xpatch (*.facet) export type */
                    EXPORT_SVG,	/*!< SVG (*.svg) export type */
                    EXPORT_PMARC,	/*!< PMARC 12 (*.pmin) export type */
                    EXPORT_OBJ,	/*!< OBJ (*.obj) export type */
                    EXPORT_SELIG_AIRFOIL,	/*!< Airfoil points (*.dat) export type */
                    EXPORT_BEZIER_AIRFOIL,	/*!< Airfoil curves (*.bz) export type */
                    EXPORT_IGES_STRUCTURE,	/*!< IGES structure (*.igs) export type */
                    EXPORT_STEP_STRUCTURE	/*!< STEP structure (*.stp) export type */
};

/*!
	\ingroup Enumerations
*/
/*! Enum used to indicate FEA boundary condition constraint type. */
enum FEA_BC_MODE { FEA_BCM_USER,	/*!< FEA boundary condition constraints user defined. */
                   FEA_BCM_ALL,	/*!< FEA boundary condition constrains all DOF. */
                   FEA_BCM_PIN,	/*!< FEA boundary condition pin constraints. */
                   FEA_BCM_SYMM,	/*!< FEA boundary condition symmetrical constraints. */
                   FEA_BCM_ASYMM,	/*!< FEA boundary condition antisymmetrical constraints. */
                   FEA_NUM_BCM_MODES	/*!< Number of FEA boundary condition constraint types. */
};

/*!
	\ingroup Enumerations
*/
/*! Enum used to indicate FEA boundary condition definition type. */
enum FEA_BC_TYPE { FEA_BC_STRUCTURE,	/*!< FEA boundary condition assigned to structure. */
                   FEA_BC_PART,	/*!< FEA boundary condition assigned to part. */
                   FEA_BC_SUBSURF,	/*!< FEA boundary condition assigned to subsurface. */
                   FEA_NUM_BC_TYPES	/*!< Number of FEA boundary condition definition types. */
};

/*!
	\ingroup Enumerations
*/
/*! Enum used to indicate FEA beam element cross-section type. */
enum FEA_CROSS_SECT_TYPE { FEA_XSEC_GENERAL = 0,	/*!< General XSec type */
                           FEA_XSEC_CIRC,	/*!< Circle XSec type */
                           FEA_XSEC_PIPE,	/*!< Pipe XSec type */
                           FEA_XSEC_I,	/*!< I XSec type */
                           FEA_XSEC_RECT,	/*!< Rectangle XSec type */
                           FEA_XSEC_BOX	/*!< Box XSec type */
};

/*!
	\ingroup Enumerations
*/
/*! Enum for the various FEA Mesh export types. */
enum FEA_EXPORT_TYPE { FEA_MASS_FILE_NAME,	/*!< FEA Mesh mass export type */
                       FEA_NASTRAN_FILE_NAME,	/*!< FEA Mesh NASTRAN export type */
                       FEA_NKEY_FILE_NAME,	/*!< FEA Mesh NKey export type */
                       FEA_CALCULIX_FILE_NAME,	/*!< FEA Mesh Calculix export type */
                       FEA_STL_FILE_NAME,	/*!< FEA Mesh STL export type */
                       FEA_GMSH_FILE_NAME,	/*!< FEA Mesh GMSH export type */
                       FEA_SRF_FILE_NAME,	/*!< FEA Mesh SRF export type */
                       FEA_CURV_FILE_NAME,	/*!< FEA Mesh CURV export type */
                       FEA_PLOT3D_FILE_NAME,	/*!< FEA Mesh PLOT3D export type */
                       FEA_IGES_FILE_NAME,	/*!< FEA Mesh trimmed IGES export type */
                       FEA_STEP_FILE_NAME,	/*!< FEA Mesh trimmed STEP export type */
                       FEA_NUM_FILE_NAMES	/*!< Number of FEA Mesh export type. */
};

/*!
	\ingroup Enumerations
*/
/*! Enum for FEA fixed point types. */
enum FEA_FIX_PT_TYPE { FEA_FIX_PT_ON_BODY,	/*!< On body fixed point type */
                       FEA_FIX_PT_GLOBAL_XYZ,	/*!< Global XYZ off body fixed point type */
                       FEA_FIX_PT_DELTA_XYZ,	/*!< Delta XYZ off body fixed point type */
                       FEA_FIX_PT_DELTA_UVN,	/*!< Delta UVN off body fixed point type */
                       FEA_FIX_PT_GEOM_ORIGIN,	/*!< Geom origin off body fixed point type */
                       FEA_FIX_PT_GEOM_CG,	/*!< Geom CG off body fixed point type */
                       FEA_NUM_FIX_PT_TYPES	/*!< Number of off body fixed point types */
};

/*!
	\ingroup Enumerations
*/
/*! Enum for FEA material types. */
enum FEA_MATERIAL_TYPE { FEA_ISOTROPIC,	/*!< Isotropic material */
                         FEA_ENG_ORTHO,	/*!< Orthotropic material in engineering parameters */
                         FEA_ENG_ORTHO_TRANS_ISO,	/*!< Orthotropic material with transverse isotropy assumed in engineering parameters */
                         FEA_LAMINATE,	/*!< Laminate buildup material */
                         FEA_NUM_MAT_TYPES	/*!< Number of FEA material types */
};

/*!
	\ingroup Enumerations
*/
/*! Enum for FEA material orientation types. */
enum FEA_ORIENTATION_TYPE { FEA_ORIENT_GLOBAL_X,	/*!< FEA Global X material orientation */
                            FEA_ORIENT_GLOBAL_Y,	/*!< FEA Global Y material orientation */
                            FEA_ORIENT_GLOBAL_Z,	/*!< FEA Global Z material orientation */
                            FEA_ORIENT_COMP_X,	/*!< FEA Comp X material orientation */
                            FEA_ORIENT_COMP_Y,	/*!< FEA Comp Y material orientation */
                            FEA_ORIENT_COMP_Z,	/*!< FEA Comp Z material orientation */
                            FEA_ORIENT_PART_U,	/*!< FEA Part U material orientation */
                            FEA_ORIENT_PART_V,	/*!< FEA Part V material orientation */
                            FEA_ORIENT_OML_U,	/*!< FEA OML U material orientation */
                            FEA_ORIENT_OML_V,	/*!< FEA OML V material orientation */
                            FEA_ORIENT_OML_R,	/*!< FEA OML R material orientation */
                            FEA_ORIENT_OML_S,	/*!< FEA OML S material orientation */
                            FEA_ORIENT_OML_T,	/*!< FEA OML T material orientation */
                            FEA_NUM_ORIENT_TYPES	/*!< Number of FEA material orientation types */
};

/*!
	\ingroup Enumerations
*/
/*! Enum for FEA Part element types. */
enum FEA_PART_ELEMENT_TYPE { FEA_DEPRECATED = -1,	/*!< Flag for deprecated element type option */
                             FEA_SHELL = 0,	/*!< Shell (tris) FEA element type */
                             FEA_BEAM,	/*!< Beam FEA element type */
                             FEA_SHELL_AND_BEAM,	/*!< Both Shell and Beam FEA element types */
                             FEA_NO_ELEMENTS,	/*!< FEA part with no elements */
                             FEA_NUM_ELEMENT_TYPES	/*!< Number of FEA element type choices */
};

/*!
	\ingroup Enumerations
*/
/*! Enum used to identify the available FEA Part types. */
enum FEA_PART_TYPE { FEA_SLICE = 0,	/*!< Slice FEA Part type */
                     FEA_RIB,	/*!< Rib FEA Part type */
                     FEA_SPAR,	/*!< Spar FEA Part type */
                     FEA_FIX_POINT,	/*!< Fixed Point FEA Part type */
                     FEA_DOME,	/*!< Dome FEA Part type */
                     FEA_RIB_ARRAY,	/*!< Rib array FEA Part type */
                     FEA_SLICE_ARRAY,	/*!< Slice array FEA Part type */
                     FEA_SKIN,	/*!< Skin FEA Part type */
                     FEA_TRIM,	/*!< Trim FEA Part type */
                     FEA_POLY_SPAR,	/*!< Poly Spar FEA Part type */
                     FEA_NUM_TYPES	/*!< Number of FEA Part types */
};

/*!
	\ingroup Enumerations
*/
/*! Enum that defines the type of edge to set the initial position of FEA Ribs and FEA Rib Arrays to. */
enum FEA_RIB_NORMAL { NO_NORMAL,	/*!< FEA Rib or Rib Array has no set perpendicular edge */
                      LE_NORMAL,	/*!< FEA Rib or Rib Array is set perpendicular to the leading edge */
                      TE_NORMAL,	/*!< FEA Rib or Rib Array is set perpendicular to the trailing edge */
                      SPAR_NORMAL	/*!< FEA Rib or Rib Array is set perpendicular to an FEA Spar */
};

/*!
	\ingroup Enumerations
*/
/*! Enum for FEA Shell treatment types. */
enum FEA_SHELL_TREATMENT_TYPE { FEA_KEEP = 0,	/*!< Keep shell elements */
                                FEA_DELETE,	/*!< Delete shell elements */
                                FEA_NUM_SHELL_TREATMENT_TYPES	/*!< Number of FEA subsurface treatment choices */
};

/*!
	\ingroup Enumerations
*/
/*! Enum for FEA Slice orientation. */
enum FEA_SLICE_TYPE { XY_BODY = 0,	/*!< Slice is parallel to parent Geom body XY plane */
                      YZ_BODY,	/*!< Slice is parallel to parent Geom body YZ plane */
                      XZ_BODY,	/*!< Slice is parallel to parent Geom body XZ plane */
                      XY_ABS,	/*!< Slice is parallel to absolute XY plane */
                      YZ_ABS,	/*!< Slice is parallel to absolute YZ plane */
                      XZ_ABS,	/*!< Slice is parallel to absolute XZ plane */
                      SPINE_NORMAL	/*!< Slice is perpendicular to thespine of the parent Geom */
};

/*!
	\ingroup Enumerations
*/
/*! Enum for FEA Poly spar point types. */
enum FEA_POLY_SPAR_POINT { POLY_SPAR_POINT_U01,	/*!< Spar points span specified in U01 */
                           POLY_SPAR_POINT_U0N,	/*!< Spar points span specified in U0N */
                           POLY_SPAR_POINT_ETA,	/*!< Spar points span specified in eta */
                           NUM_POLY_SPAR_POINT_TYPES	/*!< Number of poly spar point types */
};

/*!
	\ingroup Enumerations
*/
/*! Enum used to identify the FEA Mesh unit system (length, mass). */
enum FEA_UNIT_TYPE { SI_UNIT = 0, /*!< FEA Files output in (m, kg) */
                     CGS_UNIT, /*!< FEA Files output in (cm, g) */
                     MPA_UNIT, /*!< FEA Files output in (mm, tonne) */
                     BFT_UNIT, /*!< FEA Files output in (ft, slug) */
                     BIN_UNIT /*!< FEA Files output in (in, lbf*sec^2/in) */
};

/*!
	\ingroup Enumerations
*/
/*! Enum for Parasite Drag Tool form factor equations for body-type components. */
enum FF_B_EQN { FF_B_MANUAL = 0,	/*!< Manual FF equation */
                FF_B_SCHEMENSKY_FUSE,	/*!< Schemensky Fuselage FF equation */
                FF_B_SCHEMENSKY_NACELLE,	/*!< Schemensky Nacelle FF equation */
                FF_B_HOERNER_STREAMBODY,	/*!< Hoerner Streamlined Body FF equation */
                FF_B_TORENBEEK,	/*!< Torenbeek FF equation */
                FF_B_SHEVELL,	/*!< Shevell FF equation */
                FF_B_COVERT,	/*!< Covert FF equation */
                FF_B_JENKINSON_FUSE,	/*!< Jenkinson Fuselage FF equation */
                FF_B_JENKINSON_WING_NACELLE,	/*!< Jenkinson Wing Nacelle FF equation */
                FF_B_JENKINSON_AFT_FUSE_NACELLE,	/*!< Jenkinson Aft Fuselage Nacelle FF equation */
};

/*!
	\ingroup Enumerations
*/
/*! Enum for Parasite Drag Tool form factor equations for wing-type components. */
enum FF_W_EQN { FF_W_MANUAL = 0,	/*!< Manual FF equation */
                FF_W_EDET_CONV,	/*!< EDET Conventional Airfoil FF equation */
                FF_W_EDET_ADV,	/*!< EDET Advanced Airfoil FF equation */
                FF_W_HOERNER,	/*!< Hoerner FF equation */
                FF_W_COVERT,	/*!< Covert FF equation */
                FF_W_SHEVELL,	/*!< Shevell FF equation */
                FF_W_KROO,	/*!< Kroo FF equation */
                FF_W_TORENBEEK,	/*!< Torenbeek FF equation */
                FF_W_DATCOM,	/*!< DATCOM FF equation */
                FF_W_SCHEMENSKY_6_SERIES_AF,	/*!< Schemensky 6 Series Airfoil FF equation */
                FF_W_SCHEMENSKY_4_SERIES_AF,	/*!< Schemensky 4 Series Airfoil FF equation */
                FF_W_JENKINSON_WING,	/*!< Jenkinson Wing FF equation */
                FF_W_JENKINSON_TAIL,	/*!< Jenkinson Tail FF equation */
                FF_W_SCHEMENSKY_SUPERCRITICAL_AF,	/*!< Schemensky Supercritical Airfoil FF equation */
};

/*!
	\ingroup Enumerations
*/
/*! Enum for Parasite Drag Tool freestream unit system. */
enum FREESTREAM_PD_UNITS { PD_UNITS_IMPERIAL = 0,	/*!< Imperial unit system */
                           PD_UNITS_METRIC	/*!< Metric unit system */
};

/*!
	\ingroup Enumerations
*/
/*! Enum for setting the mode of the file chooser. */
enum FILE_CHOOSER_MODE { OPEN,	/*!< Browse files that already exist */
                         SAVE,	/*!< Browse file system and enter file name */
                         NUM_FILE_CHOOSER_MODES	/*!< Number of file chooser modes */
};

/*!
	\ingroup Enumerations
*/
/*! Enum for file chooser type. */
enum FILE_CHOOSER_TYPE { FC_OPENVSP,	/*!< OpenVSP's own file chooser with directory preferences. */
                         FC_NATIVE,	/*!< Operating system's native file chooser */
                         NUM_FILE_CHOOSER_TYPES	/*!< Number of file chooser types */
};

/*!
	\ingroup Enumerations
*/
/*! Enum used for custom GUI development. */
enum GDEV { GDEV_TAB,	/*!< Custom GUI Tab */
            GDEV_SCROLL_TAB,	/*!< Custom GUI Fl_Scroll and Tab */
            GDEV_GROUP,	/*!< Custom GUI Group */
            GDEV_PARM_BUTTON,	/*!< Custom GUI ParmButton */
            GDEV_INPUT,	/*!< Custom GUI Input */
            GDEV_OUTPUT,	/*!< Custom GUI Output */
            GDEV_SLIDER,	/*!< Custom GUI Slider */
            GDEV_SLIDER_ADJ_RANGE,	/*!< Custom GUI SliderAdjRangeInput */
            GDEV_CHECK_BUTTON,	/*!< Custom GUI CheckButton */
            GDEV_CHECK_BUTTON_BIT,	/*!< Custom GUI CheckButtonBit */
            GDEV_RADIO_BUTTON,	/*!< Custom GUI RadioButton */
            GDEV_TOGGLE_BUTTON,	/*!< Custom GUI ToggleButton */
            GDEV_TOGGLE_BUTTON_FREE,	/*!< Custom GUI ToggleButton without Parm */
            GDEV_TOGGLE_RADIO_GROUP,	/*!< Custom GUI ToggleRadioGroup (NOT IMPLEMENTED) */  // TODO: Implement or remove
            GDEV_TRIGGER_BUTTON,	/*!< Custom GUI TriggerButton */
            GDEV_COUNTER,	/*!< Custom GUI Counter */
            GDEV_CHOICE,	/*!< Custom GUI Choice */
            GDEV_ADD_CHOICE_ITEM,	/*!< Add item to custom GUI Choice */
            GDEV_SLIDER_INPUT,	/*!< Custom GUI SliderInput */
            GDEV_SLIDER_ADJ_RANGE_INPUT,	/*!< Custom GUI SliderAdjRangeInput */
            GDEV_SLIDER_ADJ_RANGE_TWO_INPUT,	/*!< Custom GUI SliderAdjRangeInput with two inputs (NOT IMPLEMENTED) */  // TODO: Implement or remove
            GDEV_FRACT_PARM_SLIDER,	/*!< Custom GUI FractParmSlider */
            GDEV_STRING_INPUT,	/*!< Custom GUI StringInput */
            GDEV_INDEX_SELECTOR,	/*!< Custom GUI IndexSelector */
            GDEV_COLOR_PICKER,	/*!< Custom GUI ColorPicker */
            GDEV_YGAP,	/*!< Custom GUI Y gap */
            GDEV_DIVIDER_BOX,	/*!< Custom GUI divider box */
            GDEV_BEGIN_SAME_LINE,	/*!< Set begin same line flag for custom GUI */
            GDEV_END_SAME_LINE,	/*!< Set end same line flag for custom GUI */
            GDEV_FORCE_WIDTH,	/*!< Set forced width for custom GUI */
            GDEV_SET_FORMAT,	/*!< Set format label for custom GUI */
            NUM_GDEV_TYPES,	/*!< Number of GDEV types */
            ALL_GDEV_TYPES, /*!< Flag for all GDEV types */
};

/*!
	\ingroup Enumerations
*/
/*! Enum for OpenVSP Human component gender types. */
enum GENDER { MALE,	/*!< Male Human component */
              FEMALE	/*!< Female Human component */
};

/*!
	\ingroup Enumerations
*/
/*! Enum for geom screen types. */
enum GUI_GEOM_SCREEN { POD_GEOM_SCREEN,	/*!< Pod geom screen */
                       FUSELAGE_GEOM_SCREEN,	/*!< Fuselage geom screen */
                       MS_WING_GEOM_SCREEN,	/*!< Wing geom screen */
                       BLANK_GEOM_SCREEN,	/*!< Blank geom screen */
                       MESH_GEOM_SCREEN,	/*!< Mesh geom screen */
                       STACK_GEOM_SCREEN,	/*!< Stack geom screen */
                       CUSTOM_GEOM_SCREEN,	/*!< Custom geom screen */
                       PT_CLOUD_GEOM_SCREEN,	/*!< Point cloud geom screen */
                       PROP_GEOM_SCREEN,	/*!< Propeller geom screen */
                       HINGE_GEOM_SCREEN,	/*!< Hinge geom screen */
                       MULT_GEOM_SCREEN,	/*!< Multiple geom screen */
                       CONFORMAL_SCREEN,	/*!< Conformal geom screen */
                       ELLIPSOID_GEOM_SCREEN,	/*!< Ellipsoid geom screen */
                       BOR_GEOM_SCREEN,	/*!< Body of revolution geom screen */
                       WIRE_FRAME_GEOM_SCREEN,	/*!< Wireframe geom screen */
                       HUMAN_GEOM_SCREEN,	/*!< Human geom screen */
                       ROUTING_GEOM_SCREEN,	/*!< Routing geom screen */
                       CLEARANCE_GEOM_SCREEN,	/*!< Clearance geom screen */
                       GEAR_GEOM_SCREEN,	/*!< Gear geom screen */
                       NUM_GEOM_SCREENS,	/*!< Number of geom screens */
                       ALL_GEOM_SCREENS	/*!< All geom screens */
};

/*!
	\ingroup Enumerations
*/
/*! Enum for OpenVSP screen types. */
enum GUI_VSP_SCREEN { VSP_ADV_LINK_SCREEN,	/*!< Advanced linking screen */
                      VSP_ADV_LINK_VAR_RENAME_SCREEN,	/*!< Advanced link variable rename screen */
                      VSP_AERO_STRUCT_SCREEN,	/*!< Aero / structural analysis screen */
                      VSP_AIRFOIL_CURVES_EXPORT_SCREEN,	/*!< Airfoil curves export screen */
                      VSP_AIRFOIL_POINTS_EXPORT_SCREEN,	/*!< Airfoil points screen */
                      VSP_ATTRIBUTE_EXPLORER_SCREEN,	/*!< Attribute details screen */
                      VSP_BACKGROUND_SCREEN,	/*!< Background control screen */
                      VSP_BACKGROUND3D_SCREEN,	/*!< Background3D control screen */
                      VSP_BACKGROUND3D_PREVIEW_SCREEN,	/*!< Background3D preview screen */
                      VSP_BEM_OPTIONS_SCREEN,	/*!< Blade element method options screen */
                      VSP_CFD_MESH_SCREEN,	/*!< CFD Mesh screen */
                      VSP_CLIPPING_SCREEN,	/*!< Clipping screen */
                      VSP_COMP_GEOM_SCREEN,	/*!< CompGeom screen */
                      VSP_COR_SCREEN,	/*!< Center of rotation screen */
                      VSP_CURVE_EDIT_SCREEN,	/*!< Curve edit screen */
                      VSP_DEGEN_GEOM_SCREEN,	/*!< Degen geom screen */
                      VSP_DESIGN_VAR_SCREEN,	/*!< Design variables screen */
                      VSP_DXF_OPTIONS_SCREEN,	/*!< DXF options screen */
                      VSP_EXPORT_SCREEN,	/*!< Export screen */
                      VSP_FEA_PART_EDIT_SCREEN,	/*!< FEA Part edit screen */
                      VSP_FEA_XSEC_SCREEN,	/*!< FEA XSec screen */
                      VSP_FIT_MODEL_SCREEN,	/*!< Fit model screen */
                      VSP_IGES_OPTIONS_SCREEN,	/*!< IGES options screen */
                      VSP_IGES_STRUCTURE_OPTIONS_SCREEN,	/*!< IGES structure options screen */
                      VSP_EXPORT_CUSTOM_SCRIPT,	/*!< Custom geom export screen */
                      VSP_IMPORT_SCREEN,	/*!< Import screen */
                      VSP_INTERFERENCE_SCREEN,	/*!< Interference screen */
                      VSP_LIGHTING_SCREEN,	/*!< Lighting screen */
                      VSP_MANAGE_GEOM_SCREEN,	/*!< Manage geom screen */
                      VSP_MANAGE_TEXTURE_SCREEN,	/*!< Texture mapping screen */
                      VSP_MASS_PROP_SCREEN,	/*!< Mass properties screen */
                      VSP_MATERIAL_EDIT_SCREEN,	/*!< Material edit screen */
                      VSP_MEASURE_SCREEN,	/*!< Measure screen */
                      VSP_MODE_EDITOR_SCREEN,	/*!< Mode editor screen */
                      VSP_NERF_MANAGE_GEOM_SCREEN,  /*!< NERF'ed (limited to make safe) Manage geom screen */
                      VSP_SNAP_TO_SCREEN,	/*!< Snap to screen */
                      VSP_PARASITE_DRAG_SCREEN,	/*!< Parasite drg screen */
                      VSP_PARM_DEBUG_SCREEN,	/*!< Parameter debug screen */
                      VSP_PARM_LINK_SCREEN,	/*!< Parameter linking screen */
                      VSP_PARM_SCREEN,	/*!< Parameter screen */
                      VSP_PICK_SET_SCREEN,	/*!< Pick set screen */
                      VSP_PREFERENCES_SCREEN,	/*!< Preferences screen */
                      VSP_PROJECTION_SCREEN,	/*!< Projected area screen */
                      VSP_PSLICE_SCREEN,	/*!< Planar slicing screen */
                      VSP_SCREENSHOT_SCREEN,	/*!< Screenshot screen */
                      VSP_SELECT_FILE_SCREEN,	/*!< Select file screen */
                      VSP_SET_EDITOR_SCREEN,	/*!< Set editor screen */
                      VSP_STEP_OPTIONS_SCREEN,	/*!< STEP options screen */
                      VSP_STEP_STRUCTURE_OPTIONS_SCREEN,	/*!< STEP structure options screen */
                      VSP_STL_OPTIONS_SCREEN,	/*!< STL options screen */
                      VSP_STRUCT_SCREEN,	/*!< Structure definition screen */
                      VSP_STRUCT_ASSEMBLY_SCREEN,	/*!< Structure assembly screen */
                      VSP_SURFACE_INTERSECTION_SCREEN,	/*!< Surface intersection screen */
                      VSP_SVG_OPTIONS_SCREEN,	/*!< SVG options screen */
                      VSP_USER_PARM_SCREEN,	/*!< User parameter screen */
                      VSP_VAR_PRESET_SCREEN,	/*!< Variable presets editor screen */
                      VSP_VEH_NOTES_SCREEN,	/*!< Vehicle notes screen */
                      VSP_VEH_SCREEN,	/*!< Veh geom screen */
                      VSP_VIEW_SCREEN,	/*!< Adjust viewpoint screen */
                      VSP_VSPAERO_PLOT_SCREEN,	/*!< VSPAERO results manager screen */
                      VSP_VSPAERO_SCREEN,	/*!< VSPAERO screen */
                      VSP_XSEC_SCREEN,	/*!< XSec screen */
                      VSP_WAVEDRAG_SCREEN,	/*!< Wave drag screen */
                      VSP_MAIN_SCREEN,   /*!< Main screen */ // Leave at end of list, helps draw after update.
                      VSP_NUM_SCREENS,	/*!< Number of screens */
                      VSP_ALL_SCREENS   /*!< Flag for all screens */
};

/*!
	\ingroup Enumerations
*/
/*! Initial shape enums for XS_EDIT_CURVE type XSecs. */
enum INIT_EDIT_XSEC_TYPE { EDIT_XSEC_CIRCLE,	/*!< Circle initialized as cubic Bezier type */
                           EDIT_XSEC_ELLIPSE,	/*!< Ellipse initialized as PCHIP type */
                           EDIT_XSEC_RECTANGLE	/*!< Rectangle initialized as linear type */
};

/*!
	\ingroup Enumerations
*/
/*! Enum for OpenVSP import types. */
enum IMPORT_TYPE {  IMPORT_STL,	/*!< Stereolith (*.stl) import */
                    IMPORT_NASCART,	/*!< NASCART (*.dat) import */
                    IMPORT_CART3D_TRI,	/*!< Cart3D (*.try) import */
                    IMPORT_XSEC_MESH,	/*!< XSec as Tri Mesh (*.hrm) import */
                    IMPORT_PTS,	/*!< Point Cloud (*.pts) import */
                    IMPORT_V2,	/*!< OpenVSP v2 (*.vsp) import */
                    IMPORT_BEM,	/*!< Blade Element (*.bem) import */
                    IMPORT_XSEC_WIRE,	/*!< XSec as Wireframe (*.hrm) import */
                    IMPORT_P3D_WIRE	/*!< Plot3D as Wireframe (*.p3d) import */
};

/*!
	\ingroup Enumerations
*/
/*! Enum for Surface Intersection export file types. */
enum INTERSECT_EXPORT_TYPE { INTERSECT_SRF_FILE_NAME,	/*!< SRF intersection file type */
                             INTERSECT_CURV_FILE_NAME,	/*!< CURV intersection file type */
                             INTERSECT_PLOT3D_FILE_NAME,	/*!< PLOT3D intersection file type */
                             INTERSECT_IGES_FILE_NAME,	/*!< IGES intersection file type */
                             INTERSECT_STEP_FILE_NAME,	/*!< STEP intersection file type */
                             INTERSECT_NUM_FILE_NAMES	/*!< Number of surface intersection file types */
};

/*!
	\ingroup Enumerations
*/
/*! Enum that describes units for length. */
enum LEN_UNITS { LEN_MM,	/*!< Millimeter  */
                 LEN_CM,	/*!< Centimeter */
                 LEN_M,	/*!< Meter */
                 LEN_IN,	/*!< Inch */
                 LEN_FT,	/*!< Feet */
                 LEN_YD,	/*!< Yard */
                 LEN_UNITLESS,	/*!< Unitless */
                 NUM_LEN_UNIT	/*!< Number of length unit types */
};

/*!
	\ingroup Enumerations
*/
/*! Enum that describes units for mass. */
enum MASS_UNIT { MASS_UNIT_G = 0,	/*!< Gram */
                 MASS_UNIT_KG,	/*!< Kilogram */
                 MASS_UNIT_TONNE,	/*!< Tonne */
                 MASS_UNIT_LBM,	/*!< Pound-mass */
                 MASS_UNIT_SLUG,	/*!< Slug */
                 MASS_LBFSEC2IN,    /*!< lbf*sec^2/in */
                 NUM_MASS_UNIT	/*!< Number of mass unit types */
};

/*!
	\ingroup Enumerations
*/
/*! Enum that describes the criteria that set the local minimum edge length. */
enum MESH_REASON { NO_REASON,	/*!< No reason determined. */
                   MAX_LEN_CONSTRAINT,	/*!< Maximum edge length. */
                   CURV_GAP,	/*!< Maximum gap curvature based criteria. */
                   CURV_NCIRCSEG,	/*!< Minimum number of segments to define a circle curvature based criteria. */
                   SOURCES,	/*!< Mesh sources. */
                   MIN_LEN_CONSTRAINT,  /*!< Minimum edge length. */                                                                 // MAX_LEN_CONSTRAINT + MIN_LEN_INCREMENT
                   MIN_LEN_CONSTRAINT_CURV_GAP, /*!< Maximum gap constrained by minimum length. */                                   // CURV_GAP + MIN_LEN_INCREMENT
                   MIN_LEN_CONSTRAINT_CURV_NCIRCSEG,    /*!< Number of segments to define a circle constrained by minimum length. */ // CURV_NCIRCSEG + MIN_LEN_INCREMENT
                   MIN_LEN_CONSTRAINT_SOURCES,  /*!< Mesh sources constrained by minimum length (not applied). */                    // Placeholder -- min len not applied to sources.
                   GROW_LIMIT_MAX_LEN_CONSTRAINT,	/*!< Maximum growth limit from maximum edge length (not used, growth limited small to large). */
                   GROW_LIMIT_CURV_GAP,	/*!< Maximum growth limit from maximum gap. */
                   GROW_LIMIT_CURV_NCIRCSEG,	/*!< Maximum growth limit from number of segments to define a circle. */
                   GROW_LIMIT_SOURCES,	/*!< Maximum growth limit from mesh sources. */
                   GROW_LIMIT_MIN_LEN_CONSTRAINT,	/*!< Maximum growth limit from minimum length constraint. */
                   GROW_LIMIT_MIN_LEN_CONSTRAINT_CURV_GAP,	/*!< Maximum growth limit from maximum gap constrained by minimum length. */
                   GROW_LIMIT_MIN_LEN_CONSTRAINT_CURV_NCIRCSEG,	/*!< Maximum growth limit from number of segments to define a circle constrained by minimum length. */
                   GROW_LIMIT_MIN_LEN_CONSTRAINT_SOURCES,	/*!< Maximum growth limit from sources constrained by minimum length. */
                   NUM_MESH_REASON,	/*!< Number of reasons that can set the mesh local minimum edge length. */
                   MIN_LEN_INCREMENT = MIN_LEN_CONSTRAINT - MAX_LEN_CONSTRAINT,	/*!< Reason increment when adding minimum length constraint. */
                   GROW_LIMIT_INCREMENT = GROW_LIMIT_CURV_GAP - CURV_GAP,	/*!< Reason increment when adding growth limit constraint. */
                   MIN_GROW_LIMIT = GROW_LIMIT_CURV_GAP	/*!< Reason marker for minimum reason to apply growth limit. */
};

/*!
	\ingroup Enumerations
*/
/*! Enum for ID length by vsp object type. */
enum OBJ_ID_LENGTH { ID_LENGTH_PRESET_GROUP = 5,	/*!< ID for Var Preset Groups are length 5 */
                     ID_LENGTH_PRESET_SETTING = 6,	/*!< ID for Var Preset Settings are length 6 */
                     ID_LENGTH_ATTR = 8,	/*!< ID for Attributes are length 8 */
                     ID_LENGTH_ATTRCOLL = 9,	/*!< ID for Attribute Collections are length 9 */
                     ID_LENGTH_PARMCONTAINER = 10,	/*!< ID for Parm Containers are length 10 */
                     ID_LENGTH_PARM = 11,	/*!< ID for Parms are length 11 */
};

/*!
	\ingroup Enumerations
*/
/*! Enum for OpenVSP's various Parm class types. */
enum PARM_TYPE { PARM_DOUBLE_TYPE,	/*!< Double Parm type (Parm) */
                 PARM_INT_TYPE,	/*!< Integer Parm type (IntParm) */
                 PARM_BOOL_TYPE,	/*!< Bool Parm type (BoolParm) */
                 PARM_FRACTION_TYPE,	/*!< Fraction Parm type (FractionParm) */
                 PARM_LIMITED_INT_TYPE,	/*!< Limited integer Parm type (LimIntParm) */
                 PARM_NOTEQ_TYPE,	/*!< Not equal Parm type (NotEqParm) */
                 PARM_POWER_INT_TYPE	/*!< Power integer Parm type (PowIntParm) */
};

/*!
	\ingroup Enumerations
*/
/*! Enum used to identify patch types for WireGeoms. */
enum PATCH_TYPE { PATCH_NONE,	/*!< No patch */
                  PATCH_POINT,	/*!< Point patch type */
                  PATCH_LINE,	/*!< Line patch type */
                  PATCH_COPY,	/*!< Copy patch type */
                  PATCH_HALFWAY,	/*!< Halfway patch type */
                  PATCH_NUM_TYPES	/*!< Number of patch types */
};

/*!
	\ingroup Enumerations
*/
/*! Enum for parametric curve types. */
enum PCURV_TYPE { LINEAR,	/*!< Linear curve type */
                  PCHIP,	/*!< Piecewise Cubic Hermite Interpolating Polynomial curve type */
                  CEDIT,	/*!< Cubic Bezier curve type */
                  APPROX_CEDIT,	/*!< Approximate curve as Cubic Bezier */
                  NUM_PCURV_TYPE	/*!< Number of curve types */
};

/*!
	\ingroup Enumerations
*/
/*! Enum that describes units for pressure. */
enum PRES_UNITS { PRES_UNIT_PSF = 0,	/*!< Pounds per square foot */
                  PRES_UNIT_PSI,	/*!< Pounds per square inch */
                  PRES_UNIT_BA,	/*!< Barye */
                  PRES_UNIT_PA,	/*!< Pascal */
                  PRES_UNIT_KPA,	/*!< Kilopascal */
                  PRES_UNIT_MPA,	/*!< Megapascal */
                  PRES_UNIT_INCHHG,	/*!< Inch of mercury */
                  PRES_UNIT_MMHG,	/*!< Millimeter of mercury */
                  PRES_UNIT_MMH20,	/*!< Millimeter of water */
                  PRES_UNIT_MB,	/*!< Millibar */
                  PRES_UNIT_ATM,	/*!< Atmosphere */
                  NUM_PRES_UNIT	/*!< Number of pressure unit choices */
};

/*!
	\ingroup Enumerations
*/
/*! Enum for Projected Area boundary option type. */
enum PROJ_BNDY_TYPE { NO_BOUNDARY,	/*!< No boundary */
                      SET_BOUNDARY,	/*!< Set boundary */
                      GEOM_BOUNDARY,	/*!< Geom boundary */
                      NUM_PROJ_BNDY_OPTIONS	/*!< Number of projected area boundary options */
};

/*!
	\ingroup Enumerations
*/
/*! Enum for Projected Area direction type. */
enum PROJ_DIR_TYPE { X_PROJ,	/*!< Project in X axis direction */
                     Y_PROJ,	/*!< Project in Y axis direction */
                     Z_PROJ,	/*!< Project in Z axis direction */
                     GEOM_PROJ,	/*!< Project toward a Geom */
                     VEC_PROJ,	/*!< Project along a 3D vector */
                     NUM_PROJ_DIR_OPTIONS	/*!< Number of Projected Area direction types */
};

/*!
	\ingroup Enumerations
*/
/*! Enum for Projected Area target type. */
enum PROJ_TGT_TYPE { SET_TARGET,	/*!< Set target type */
                     GEOM_TARGET,	/*!< Geom target type */
                     MODE_TARGET,	/*!< Mode target type */
                     NUM_PROJ_TGT_OPTIONS	/*!< Number of Projected Area target types */
};

/*!
	\ingroup Enumerations
*/
/*! Enum used to specify the mode of a propeller blade azimuth control. */
enum PROP_AZIMUTH_MODE { PROP_AZI_UNIFORM,	/*!< Propeller blades are uniformly spaced */
                         PROP_AZI_FREE,	/*!< Propeller blades are free to spaced arbitrarially */
                         PROP_AZI_BALANCED,	/*!< Propeller blade balance is enforced */
                         NUM_PROP_AZI,	/*!< Number of propeller blade azimuth modes */
};

/*!
	\ingroup Enumerations
*/
/*! Enum used to specify the mode of a propeller Geom. */
enum PROP_MODE { PROP_BLADES,	/*!< Propeller Geom is defined by individual propeller blades */
                 PROP_BOTH,	/*!< Propeller Geom is defined by blades and a disk together */
                 PROP_DISK	/*!< Propeller Geom is defined by a flat circular disk */
};

/*!
	\ingroup Enumerations
*/
/*! Enum for the various propeller blade curve parameterization options. */
enum PROP_PCURVE { PROP_CHORD,	/*!< Chord parameterization */
                   PROP_TWIST,	/*!< Twist parameterization */
                   PROP_RAKE,	/*!< Rake parameterization */
                   PROP_SKEW,	/*!< Skew parameterization */
                   PROP_SWEEP,	/*!< Sweep parameterization */
                   PROP_THICK,	/*!< Thickness parameterization */
                   PROP_CLI,	/*!< Induced lift coefficient parameterization */
                   PROP_AXIAL,	/*!< Axial parameterization */
                   PROP_TANGENTIAL,	/*!< Tangential parameterization */
                   NUM_PROP_PCURVE	/*!< Number of propeller blade curve parameterization options */
};

/*!
	\ingroup Enumerations
*/
/*! Enum used to identify array reordering instructions. */
enum REORDER_TYPE { REORDER_MOVE_UP,	/*!< Move up one position */
                    REORDER_MOVE_DOWN,	/*!< Move down one position */
                    REORDER_MOVE_TOP,	/*!< Move to top */
                    REORDER_MOVE_BOTTOM,	/*!< Move to bottom */
                    NUM_REORDER_TYPES	/*!< Number reordering instructions */
};

/*!
	\ingroup Enumerations
*/
/*! Enum used to indicate manual or component reference type. */
enum REF_WING_TYPE { MANUAL_REF = 0,    /*!< Manually specify the reference areas and lengths */
                     COMPONENT_REF, /*!< Use a particular wing to calculate the reference area and lengths */
                     NUM_REF_TYPES	/*!< Number of wing reference types */
};

/*!
	\ingroup Enumerations
*/
/*! Enum representing the possible data types returned from the ResultsMgr. Datatypes are shared with Attribute Definitions */
enum RES_DATA_TYPE {    INVALID_TYPE = -1,	/*!< Invalid data type */
                        BOOL_DATA = 0,	/*!< Bool data type */
                        INT_DATA = 1,	/*!< Integer data type */
                        DOUBLE_DATA = 2,	/*!< Double data type */
                        STRING_DATA = 3,	/*!< String data type */
                        VEC3D_DATA = 4,	/*!< Vec3d data type */
                        INT_MATRIX_DATA = 5,	/*!< Int matrix data type */
                        DOUBLE_MATRIX_DATA = 6,	/*!< Double matrix data type */
                        // DEPRECATED DATA TYPE = 7,   // Deprecated type removed, RES_DATA_TYPE must skip 7.
                        ATTR_COLLECTION_DATA = 8,	/*!< Attribute collection data type */
                        PARM_REFERENCE_DATA = 9,	/*!< Parm reference data type */
};

/*!
	\ingroup Enumerations
*/
/*! Enum representing the possible Geom types returned from the ResultsMgr. */
enum RES_GEOM_TYPE {    MESH_INDEXED_TRI,	/*!< Indexed triangulated mesh Geom type */
                        MESH_SLICE_TRI,	/*!< Sliced Triangulated mesh Geom type */
                        GEOM_XSECS,	/*!< GeomXSec Geom type */
                        MESH_INDEX_AND_SLICE_TRI,	/*!< Both indexed and sliced triangulated mesh Geom type */
};

/*!
	\ingroup Enumerations
*/
/*! Enum that describes units for density. */
enum RHO_UNITS { RHO_UNIT_SLUG_FT3 = 0,	/*!< Slug per cubic foot */
                 RHO_UNIT_G_CM3,	/*!< Gram per cubic centimeter */
                 RHO_UNIT_KG_M3,	/*!< Kilogram per cubic meter */
                 RHO_UNIT_TONNE_MM3,	/*!< Tonne per cubic millimeter */
                 RHO_UNIT_LBM_FT3,	/*!< Pound-mass per cubic foot */
                 RHO_UNIT_LBFSEC2_IN4,	/*!< Pound-force-second squared per inch to the fourth */
                 RHO_UNIT_LBM_IN3,	/*!< Pound-mass per cubic inch */
                 NUM_RHO_UNIT	/*!< Number of density unit options */
};

/*!
	\ingroup Enumerations
*/
/*! Enum that identifies the coordinate type used for a routing point. */
enum ROUTE_PT_TYPE { ROUTE_PT_COMP,	/*!< Routing point relative to parent body axes */
                     ROUTE_PT_UV,	/*!< Routing point relative to parent surface coordinate frame */
                     ROUTE_PT_RST,	/*!< Routing point relative to parent per-section volume coordinate frame */
                     ROUTE_PT_LMN,	/*!< Routing point relative to parent uniform volume coordinate frame */
                     ROUTE_PT_EtaMN,	/*!< Routing point relative to wing parent uniform eta volume coordinate frame */
                     ROUTE_PT_NUM_TYPES	/*!< Number of routing point coordinate types */
};

/*!
	\ingroup Enumerations
*/
/*! Enum that identifies the coordinate offset type used for a routing point. */
enum ROUTE_PT_DELTA_TYPE { ROUTE_PT_DELTA_XYZ,	/*!< Routing point ofset in global axes */
                           ROUTE_PT_DELTA_COMP,	/*!< Routing point offset in parent body frame */
                           ROUTE_PT_DELTA_UVN,	/*!< Routing point offset in parent surface coordinate frame */
                           ROUTE_PT_DELTA_NUM_TYPES	/*!< Number of routing point offset coordinate types */
};

/*!
	\ingroup Enumerations
*/
/*! Enum representing the possible ways to scale a 3D background image. */
enum SCALE_TYPE { SCALE_WIDTH,	/*!< Scale image to match desired width */
                  SCALE_HEIGHT,	/*!< Scale image to match desired height */
                  SCALE_WIDTH_HEIGHT,	/*!< Scale image to match desired width and height */
                  SCALE_RESOLUTION,	/*!< Scale image to specified resolution */
                  NUM_SCALE_TYPES,	/*!< Number of ways to scale 3D background image. */
};

/*!
	\ingroup Enumerations
*/
/*! Enum for specifying named set types. */
enum SET_TYPE { SET_NONE = -1,	/*!< None set */
                SET_ALL = 0,	/*!< All set */
                SET_SHOWN = 1,	/*!< Shown set */
                SET_NOT_SHOWN = 2,	/*!< Not shown set */
                SET_FIRST_USER = 3,	/*!< First user-defined set */
                MIN_NUM_USER = 20,  /*!< Minimum number of user sets */
                MAX_NUM_SETS = 1000	/*!< Maximum possible number of sets */
};

/*!
	\ingroup Enumerations
*/
/*! Enum that identifies the trimmed STEP export representation type. */
enum STEP_REPRESENTATION { STEP_SHELL,	/*!< Manifold shell surface STEP file representation */
                           STEP_BREP	/*!< Manifold solid BREP STEP file representation */
};

/*!
	\ingroup Enumerations
*/
/*! Enum used to identify Parasite Drag Tool sub-surface treatment. */
enum SUBSURF_INCLUDE { SS_INC_TREAT_AS_PARENT,	/*!< Treat the sub-surface the same as the parent */
                       SS_INC_SEPARATE_TREATMENT,	/*!< Treat the sub-surface separately from the parent */
                       SS_INC_ZERO_DRAG,	/*!< No drag contribution for the sub-surface */
};

/*!
	\ingroup Enumerations
*/
/*! Enum for indicating which part of the parent surface a sub-surfacce is dedfine. */
enum SUBSURF_INOUT { INSIDE,	/*!< The interior of the sub-surface is its surface */
                     OUTSIDE,	/*!< The exterior of the sub-surface is its surface */
                     NONE	/*!< No part of the parent surface belongs to the sub-surface */
};

/*!
	\ingroup Enumerations
*/
/*! Enum that identifies which surface coordinate is constant for a line sub-surface. */
enum SUBSURF_LINE_TYPE { CONST_U,	/*!< Constant U sub-surface */
                         CONST_W	/*!< Constant W sub-surface */
};

/*!
	\ingroup Enumerations
*/
/*! Enum for the various sub-surface types. */
enum SUBSURF_TYPE { SS_LINE,	/*!< Line sub-surface type */
                    SS_RECTANGLE,	/*!< Rectangle sub-surface type */
                    SS_ELLIPSE,	/*!< Ellipse sub-surface type */
                    SS_CONTROL,	/*!< Control sub-surface type */
                    SS_LINE_ARRAY,	/*!< Line array sub-surface type */
                    SS_FINITE_LINE,	/*!< Finite line sub-surface type */
                    SS_NUM_TYPES	/*!< Number of sub-surface types */
};

/*!
	\ingroup Enumerations
*/
/*! Enum that represents various symmetry types. */
enum SYM_FLAG {  SYM_XY = ( 1 << 0 ),    /*!< XY planar symmetry. */
                 SYM_XZ = ( 1 << 1 ),	/*!< XZ planar symmetry. */
                 SYM_YZ = ( 1 << 2 ),	/*!< YZ planar symmetry. */
                 SYM_ROT_X = ( 1 << 3 ),	/*!< X rotational symmetry. */
                 SYM_ROT_Y = ( 1 << 4 ),	/*!< Y rotational symmetry. */
                 SYM_ROT_Z = ( 1 << 5 ),	/*!< Z rotational symmetry. */
                 SYM_PLANAR_TYPES = 3,	/*!< Number of planar symmetry types. */
                 SYM_NUM_TYPES = 6	/*!< Number of symmetry types. */
};

/*!
	\ingroup Enumerations
*/
/*! Enum that represents various cross section symmetry types. */
enum SYM_XSEC_TYP { SYM_NONE,    /*!< No cross section symmetry. */
                    SYM_RL,    /*!< Right/left cross section symmetry. */
                    SYM_TB,    /*!< Top/bottom cross section symmetry. */
                    SYM_ALL    /*!< All cross section symmetry. */
};

/*!
	\ingroup Enumerations
*/
/*! Enum that describes units for temperature. */
enum TEMP_UNITS { TEMP_UNIT_K = 0,	/*!< Kelvin */
                  TEMP_UNIT_C,	/*!< Celsius */
                  TEMP_UNIT_F,	/*!< Fahrenheit  */
                  TEMP_UNIT_R,	/*!< Rankine  */
                  NUM_TEMP_UNIT	/*!< Number of temperature unit choices  */
};

/*!
	\ingroup Enumerations
*/
/*! Enum that describes units for velocity. */
enum VEL_UNITS { V_UNIT_FT_S = 0,	/*!< Feet per second */
                 V_UNIT_M_S,	/*!< Meter per second */
                 V_UNIT_MPH,	/*!< Mile per hour */
                 V_UNIT_KM_HR,	/*!< Kilometer per hour */
                 V_UNIT_KEAS,	/*!< Knots equivalent airspeed */
                 V_UNIT_KTAS,	/*!< Knots true airspeed */
                 V_UNIT_MACH,	/*!< Mach */
};

/*!
	\ingroup Enumerations
*/
/*! Enum for 2D drawing types (DXF & SVG). */
enum VIEW_NUM { VIEW_1,	/*!< One 2D view */
                VIEW_2HOR,	/*!< Two horizontal 2D views */
                VIEW_2VER,	/*!< Two vertical 2D views */
                VIEW_4,	/*!< Four 2D views */
};

/*!
	\ingroup Enumerations
*/
/*! Enum for describing 2D view rotations (DXF & SVG). */
enum VIEW_ROT { ROT_0,	/*!< No rotation */
                ROT_90,	/*!< 90 degree rotation */
                ROT_180,	/*!< 180 degree rotation */
                ROT_270,	/*!< 270 degree rotation */
};

/*!
	\ingroup Enumerations
*/
/*! Enum for describing 2D view types (DXF & SVG). */
enum VIEW_TYPE { VIEW_LEFT,	/*!< Left 2D view type */
                 VIEW_RIGHT,	/*!< Right 2D view type */
                 VIEW_TOP,	/*!< Top 2D view type */
                 VIEW_BOTTOM,	/*!< Bottom 2D view type */
                 VIEW_FRONT,	/*!< Front 2D view type */
                 VIEW_REAR,	/*!< Rear 2D view type */
                 VIEW_NONE,	/*!< No 2D view type */
                 VIEW_NUM_TYPES	/*!< Number of 2D view types */
};

/*!
	\ingroup Enumerations
*/
/*! Enum that definies the VSPAERO analysis method. */
enum VSPAERO_ANALYSIS_METHOD { VORTEX_LATTICE,	/*!< VSPAERO vortex lattice method */
                               PANEL	/*!< VSPAERO panel method */
};

/*!
	\ingroup Enumerations
*/
/*! Enums for VSPAERO unsteady noise calculation types. */
enum VSPAERO_NOISE_TYPE { NOISE_FLYBY,	/*!< Set up fly by noise analysis in VSPAERO for PSU-WOPWOP */
                          NOISE_FOOTPRINT,	/*!< Set up footprint noise analysis in VSPAERO for PSU-WOPWOP */
                          NOISE_STEADY,	/*!< Set up steady state noise analysis in VSPAERO for PSU-WOPWOP */
};

/*!
	\ingroup Enumerations
*/
/*! Enums for VSPAERO unsteady noise units. */
enum VSPAERO_NOISE_UNIT { NOISE_SI,	/*!< Assume geometry and VSPAERO inputs in SI (m N kg s) for PSU-WOPWOP  */
                          NOISE_ENGLISH	/*!< Assume geometry and VSPAERO inputs in english (ft lbf slug s) units, will convert to SI (m N kg s) for PSU-WOPWOP */
};

/*!
	\ingroup Enumerations
*/
/*! Enum for the types of preconditioner used in VSPAERO. */
enum VSPAERO_PRECONDITION { PRECON_MATRIX = 0,	/*!< Matrix preconditioner */
                            PRECON_JACOBI,	/*!< Jacobi preconditioner */
                            PRECON_SSOR,	/*!< Symmetric successive over-relaxation preconditioner */
};

/*!
	\ingroup Enumerations
*/
/*! Enum for the types of VSPAERO stability analyses. */
enum VSPAERO_STABILITY_TYPE { STABILITY_OFF = 0,	/*!< No stability analysis (off) */
                              STABILITY_DEFAULT = 1,	/*!< Steady 6DOF stability analysis */
                              STABILITY_P_ANALYSIS = 2,	/*!< Unsteady roll stability analysis */
                              STABILITY_Q_ANALYSIS = 3,	/*!< Unsteady pitch stability analysis */
                              STABILITY_R_ANALYSIS = 4,	/*!< Unsteady yaw stability analysis */
                              STABILITY_PITCH = 5,	/*!< Simplified pitch stability analysis */
                              STABILITY_NUM_TYPES = 6	/*!< Number of stability analysis types */
};

/*!
	\ingroup Enumerations
*/
/*! Enum for the VSPAERO stall modeling options (Cl Max VSPAERO input). */
enum VSPAERO_CLMAX_TYPE { CLMAX_OFF,	/*!< Stall modeling off (Cl Max = 0) */
                          CLMAX_2D,	/*!< 2D Cl Max stall modeling with user defined value */
                          CLMAX_CARLSON    /*!< Carlson's Pressure Correlation */
};

/*!
	\ingroup Enumerations
*/
/*! Enum that is used to describe surfaces in CFD Mesh. */
enum VSP_SURF_CFD_TYPE { CFD_NORMAL,	/*!< Normal CFD Mesh surface */
                         CFD_NEGATIVE,	/*!< Negative volume CFD Mesh surface */
                         CFD_TRANSPARENT,	/*!< Transparent CFD Mesh surface */
                         CFD_STRUCTURE,	/*!< FEA structure CFD Mesh surface */
                         CFD_STIFFENER,	/*!< FEA stiffener CFD Mesh surface */
                         CFD_MEASURE_DUCT,	/*!< Measure duct cross sectional area surface */
                         CFD_NUM_TYPES,	/*!< Number of CFD Mesh surface types */
};

/*!
	\ingroup Enumerations
*/
/*! Enum for the different surface types in OpenVSP. */
enum VSP_SURF_TYPE { NORMAL_SURF,	/*!< Normal VSP surface */
                     WING_SURF,	/*!< Wing VSP surface */
                     DISK_SURF,	/*!< Disk VSP surface */
                     NUM_SURF_TYPES,	/*!< Number of VSP surface types */
};

/*!
	\ingroup Enumerations
*/
/*! Enum for chevron W parameter start/end hints. */
enum W_HINT { W_RIGHT_0,    /*!< Chevron start/ends at right (W = 0) of cross section */
              W_BOTTOM,    /*!< Chevron start/ends at bottom of cross section */
              W_LEFT,	/*!< Chevron start/ends at left of cross section */
              W_TOP,	/*!< Chevron start/ends at top of cross section */
              W_RIGHT_1,	/*!< Chevron start/ends at right (W = 1) of cross section */
              W_FREE,	/*!< Chevron start/ends at user specified point on cross section */
};

/*!
	\ingroup Enumerations
*/
/*! Enum used to identify the type of wing blending between XSecs. */
enum WING_BLEND { BLEND_FREE,	/*!< Free blending */
                  BLEND_ANGLES,	/*!< Blend based on angles (sweep & dihedral) */
                  BLEND_MATCH_IN_LE_TRAP,	/*!< Match inboard leading edge trapezoid */
                  BLEND_MATCH_IN_TE_TRAP,	/*!< Match inboard trailing edge trapezoid */
                  BLEND_MATCH_OUT_LE_TRAP,	/*!< Match outboard leading edge trapezoid */
                  BLEND_MATCH_OUT_TE_TRAP,	/*!< Match outboard trailing edge trapezoid */
                  BLEND_MATCH_IN_ANGLES,	/*!< Match inboard angles */
                  BLEND_MATCH_LE_ANGLES,	/*!< Match leading edge angles */
                  BLEND_NUM_TYPES	/*!< Number of blending types */
};

/*!
	\ingroup Enumerations
*/
/*! Enum for controlling wing section planform parameter control and linking. */
enum WING_DRIVERS { AR_WSECT_DRIVER,	/*!< Aspect ratio driver */
                    SPAN_WSECT_DRIVER,	/*!< Span driver */
                    AREA_WSECT_DRIVER,	/*!< Area driver */
                    TAPER_WSECT_DRIVER,	/*!< Taper driver */
                    AVEC_WSECT_DRIVER,	/*!< Average chord driver */
                    ROOTC_WSECT_DRIVER,	/*!< Root chord driver */
                    TIPC_WSECT_DRIVER,	/*!< Tip chord driver */
                    SECSWEEP_WSECT_DRIVER,	/*!< Section sweep driver */
                    NUM_WSECT_DRIVER,	/*!< Number of wing section drivers */
                    SWEEP_WSECT_DRIVER = SECSWEEP_WSECT_DRIVER + 1,	
                    SWEEPLOC_WSECT_DRIVER = SECSWEEP_WSECT_DRIVER + 2,	
                    SECSWEEPLOC_WSECT_DRIVER = SECSWEEP_WSECT_DRIVER + 3,	
};

/*!
	\ingroup Enumerations
*/
/*! Enum that identifies the working XDDM type. */
enum XDDM_QUANTITY_TYPE { XDDM_VAR,	/*!< Variable XDDM type */
                          XDDM_CONST,	/*!< Constant XDDM type */
};

/*!
	\ingroup Enumerations
*/
/*! Enum for modifying XSec through closure types. */
enum XSEC_CLOSE_TYPE { CLOSE_NONE,	/*!< No closure */
                       CLOSE_SKEWLOW,	/*!< Skew lower closure */
                       CLOSE_SKEWUP,	/*!< Skew upper closure */
                       CLOSE_SKEWBOTH,	/*!< Skew both closure */
                       CLOSE_EXTRAP,	/*!< Extrapolate closure */
                       CLOSE_NUM_TYPES	/*!< Number of XSec closure types */
};

/*!
	\ingroup Enumerations
*/
/*! Enum that identifies the various OpenVSP XSecCurve types. */
enum XSEC_CRV_TYPE {XS_UNDEFINED = -1,	
                    XS_POINT,	/*!< Point XSec */
                    XS_CIRCLE,	/*!< Circle XSec */
                    XS_ELLIPSE,	/*!< Ellipse XSec */
                    XS_SUPER_ELLIPSE,	/*!< Super ellipse XSec */
                    XS_ROUNDED_RECTANGLE,	/*!< Rounded rectangle XSec */
                    XS_GENERAL_FUSE,	/*!< General fuselage XSec */
                    XS_FILE_FUSE,	/*!< Fuselage file XSec */
                    XS_FOUR_SERIES,	/*!< Four series XSec */
                    XS_SIX_SERIES,	/*!< Six series XSec */
                    XS_BICONVEX,	/*!< Biconvex XSec */
                    XS_WEDGE,	/*!< Wedge XSec */
                    XS_EDIT_CURVE,	/*!< Generic Edit Curve XSec */
                    XS_FILE_AIRFOIL,	/*!< Airfoil file XSec */
                    XS_CST_AIRFOIL,	/*!< CST airfoil XSec */
                    XS_VKT_AIRFOIL,	/*!< VKT airfoil XSec */
                    XS_FOUR_DIGIT_MOD,	/*!< Four digit modified XSec */
                    XS_FIVE_DIGIT,	/*!< Five digit XSec */
                    XS_FIVE_DIGIT_MOD,	/*!< Five digit modified XSec */
                    XS_ONE_SIX_SERIES,	/*!< One six series XSec */
                    XS_NUM_TYPES	/*!< Number of XSec types */
};

/*!
	\ingroup Enumerations
*/
/*! Enum for XSec drivers. */
enum XSEC_DRIVERS { WIDTH_XSEC_DRIVER,     // First two are used for Circle.  Others are used for general XSecCurves	
                    AREA_XSEC_DRIVER,      // Area must be second entry.	
                    HEIGHT_XSEC_DRIVER,	/*!< Height driver */
                    HWRATIO_XSEC_DRIVER,	/*!< Height/width ratio driver */
                    NUM_XSEC_DRIVER,	/*!< Number of XSec drivers */
                    CIRCLE_NUM_XSEC_DRIVER = 2	
};

/*!
	\ingroup Enumerations
*/
/*! Enum used to identify XSec flap type. */
enum XSEC_FLAP_TYPE { FLAP_NONE,	/*!< No flap */
                      FLAP_PLAIN,	/*!< Plain flap */
                      FLAP_NUM_TYPES	/*!< Number of flap types */
};

/*!
	\ingroup Enumerations
*/
/*! Enum for XSec side types. */
enum XSEC_SIDES_TYPE {  XSEC_BOTH_SIDES,	/*!< Both sides */
                        XSEC_LEFT_SIDE,	/*!< Left side */
                        XSEC_RIGHT_SIDE	/*!< Right side */
};

/*!
	\ingroup Enumerations
*/
/*! Enum used to identify XSec trim type. */
enum XSEC_TRIM_TYPE { TRIM_NONE,	/*!< No trimming */
                      TRIM_X,	/*!< Trim XSec by X */
                      TRIM_THICK,	/*!< Trim XSec by thickness */
                      TRIM_NUM_TYPES	/*!< Number of trimming types */
};

/*!
	\ingroup Enumerations
*/
/*! Enum for the various XSec types in OpenVSP. */
enum XSEC_TYPE { XSEC_FUSE,	/*!< Fuselage XSec Geom */
                 XSEC_STACK,	/*!< Stack XSec Geom */
                 XSEC_WING,	/*!< Wing XSec Geom */
                 XSEC_CUSTOM,	/*!< Custom XSec Geom */
                 XSEC_PROP,	/*!< Propeller XSec Geom */
                 XSEC_NUM_TYPES	/*!< Number of XSec types */
};

/*!
	\ingroup Enumerations
*/
/*! Enum for XSec width shift. */
enum XSEC_WIDTH_SHIFT { XS_SHIFT_LE = 0,	/*!< Shift leading edge */
                        XS_SHIFT_MID = 1,	
                        XS_SHIFT_TE = 2	
};

}

#endif
