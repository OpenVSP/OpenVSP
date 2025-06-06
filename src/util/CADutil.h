//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

//////////////////////////////////////////////////////////////////////
// CADUtil.h
//
// Helper functions to export OpenVSP geometry to CAD (STEP or IGES). The 
// export may be watertight trimmed surfaces or individual unintersected surfaces.
//
//////////////////////////////////////////////////////////////////////

#if !defined(CADUTIL__INCLUDED_)
#define CADUTIL__INCLUDED_

#pragma warning( disable : 4018 )
#pragma warning( disable : 4800 )

#include <STEPfile.h>
#include <sdai.h>
#include <STEPattribute.h>
#include <ExpDict.h>
#include <Registry.h>
#include <errordesc.h>
#include <STEPcomplex.h>
#include <SdaiHeaderSchema.h>
#include <schema.h>
#include "SdaiCONFIG_CONTROL_DESIGN.h"
#include <string>

#include <api/dll_iges.h>
#include <api/dll_entity110.h>
#include <api/dll_entity128.h>
#include <api/dll_entity144.h>
#include <api/dll_entity126.h>
#include <api/dll_entity406.h>
#include <api/dll_entity142.h>
#include <api/dll_entity102.h>
#include <api/dll_entity314.h>

#include "APIDefines.h"
#include "Vec3d.h"

#include "eli/geom/surface/bezier.hpp"
#include "eli/geom/surface/piecewise.hpp"

typedef eli::geom::surface::piecewise<eli::geom::surface::bezier, double, 3> piecewise_surface_type;

class VspSurf;

class STEPutil
{

public:
    STEPutil( const int & len, const double & tol );
    virtual ~STEPutil();

    void  WriteFile( const string &fname );
    SdaiCartesian_point * MakePoint( const double & x, const double & y, const double & z );

    Registry * registry;
    InstMgr * instance_list;

    // Functions for STEP file representation
    void RepresentBREPSolid( vector < vector < SdaiAdvanced_face* > > adv_vec, const string& label = "" );
    void RepresentManifoldShell( vector < vector < SdaiAdvanced_face* > > adv_vec, const string& label = "" );
    void RepresentUntrimmedSurfs( const vector < SdaiB_spline_surface_with_knots* > &surf_vec, const string& label = "" );

    // Create a STEP planar surface. The surface will extend infinitely if it is not bounded
    SdaiSurface* MakePlane( const vec3d &center, const vec3d &norm, const vec3d &tangent, const string& label );

    // Convert a piecewise Bezier surface to a NURBS surface and add it to the STEP file. Additional options
    // are included to use Nanoflann to merge points that are close together
    SdaiSurface* MakeSurf( piecewise_surface_type& s, const string& label = "", bool mergepts = false, double merge_tol = 1e-8 );

    // Create a STEP vertex and add it to the STEP file. This function is used to define the endpoints 
    // of STEP edge curves.
    SdaiVertex_point* MakeVertex( const vec3d &vertex );

    // Convert a set of input control points to a NURBS curve and write to the STEP file. Additional options
    // are included to use Nanoflann to merge points that are close together
    SdaiB_spline_curve_with_knots* MakeCurve( const vector < vec3d > &cp_vec, const int& deg, const string& label = "", bool closed_curve = false, bool mergepnts = false, double merge_tol = 1e-8 );

    // Write a curve defined from the given control points to the STEP file. This function is mainly
    // available for sub-surface lines and the intersection of FEA Parts with each other.
    void MakeSurfaceCurve( vector < vec3d > cp_vec, const int& deg, const string& label = "", bool mergepnts = false, double merge_tol = 1e-8 );

protected:

    STEPfile * sfile;
    InstMgr * header_instances;

    STEPcomplex * context;
    SdaiShape_representation * shape_rep;
    SdaiProduct_definition_shape* pshape;

    STEPcomplex * Geometric_Context( const vsp::LEN_UNITS & len, const vsp::ANG_UNITS & angle, const char * tolstr );


    SdaiDirection * MakeDirection( const double & x, const double & y, const double & z );

    SdaiAxis2_placement_3d * DefaultAxis( );

    SdaiDate_and_time * DateTime( );

    SdaiSecurity_classification * Classification( SdaiPerson_and_organization * per_org, SdaiDate_and_time * date_time, SdaiProduct_definition_formation_with_specified_source * prod_def_form );

    void  STEPBoilerplate( const vsp::LEN_UNITS & len, const char * tolstr );


};

class IGESutil
{

public:
    IGESutil( const int& len );
    virtual ~IGESutil();

    void WriteFile( const string &fname, bool overwrite = true );

    // Identify the NURBS knot vector for a curve or direction of a surface given its degree and number of patches
    void IGESKnots( int deg, int npatch, vector< double >& knot );

    // Write a Bezier surface to the IGES model by extracting the Bezier parameters and converting to a NURBS surface
    DLL_IGES_ENTITY_128 MakeSurf( piecewise_surface_type& s, const string& label );

    // Bound a parent NURBS surface (entity 128) with an input control point vector
    DLL_IGES_ENTITY_144 MakeLoop( DLL_IGES_ENTITY_128& parent_surf, const vector < DLL_IGES_ENTITY_126* > &nurbs_vec, const string& label );

    DLL_IGES_ENTITY_126 MakeCurve( const vector < vec3d > &cp_vec, int deg, const string& label );

    // Create a hole in a trimmed IGES surface (entity 144) at the given control point vector
    void MakeCutout( DLL_IGES_ENTITY_128& parent_surf, DLL_IGES_ENTITY_144& trimmed_surf, const vector < DLL_IGES_ENTITY_126* > &nurbs_vec, const string& label );

protected:

    // Add an IGES bounding curve (entity 142) to a parent NURBS surface at the given control point vector
    DLL_IGES_ENTITY_142 MakeBound( DLL_IGES_ENTITY_128& parent_surf, const vector < DLL_IGES_ENTITY_126* > &nurbs_vec, const string& label );

    // Add a label to any DLL_IGES_ENTITY
    void AddLabel( DLL_IGES_ENTITY& entity, const string& label );

    DLL_IGES model;

};

// Extract the control points and patch data from a surface so that an equivalent NURBS
// surface can be defined.
void ExtractCPts( piecewise_surface_type& s, vector< vector< int > >& ptindxs, vector< vec3d >& allPntVec, 
                  piecewise_surface_type::index_type& maxu, piecewise_surface_type::index_type& maxv,
                  piecewise_surface_type::index_type& nupatch, piecewise_surface_type::index_type& nvpatch,
                  piecewise_surface_type::index_type& nupts, piecewise_surface_type::index_type& nvpts );


#endif // !defined(CADUTIL__INCLUDED_)
