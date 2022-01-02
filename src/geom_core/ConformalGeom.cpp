//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "APIDefines.h"
#include "VspUtil.h"
#include "ConformalGeom.h"
#include "Vehicle.h"
#include "VSP_Geom_API.h"
#include <cfloat>  //For DBL_EPSILON

using namespace vsp;

//==== Constructor ====//
ConformalGeom::ConformalGeom( Vehicle* vehicle_ptr ) : Geom( vehicle_ptr )
{
    m_Name = "ConformalGeom";
    m_Type.m_Name = "Conformal";
    m_Type.m_Type = CONFORMAL_GEOM_TYPE;

    m_Offset.Init( "Offset", "Design", this, 0.1, -1e12, 1.0e12 );
    m_Offset.SetDescript( "Internal Offset Distance to Conformal Surface" );

    m_UTrimFlag.Init( "UTrimFlag", "Design", this, false, 0, 1 );
    m_UTrimFlag.SetDescript( "Trim in U Direction Flag" );
    m_UTrimMin.Init( "UTrimMin", "Design", this, 0.1, 0.0, 0.9999 );
    m_UTrimMin.SetDescript( "Min U Trim Value" );
    m_UTrimMax.Init( "UTrimMax", "Design", this, 0.9, 0.0, 0.9999 );
    m_UTrimMax.SetDescript( "Max U Trim Value" );

    m_V1TrimFlag.Init( "V1TrimFlag", "Design", this, false, 0, 1 );
    m_V1TrimFlag.SetDescript( "Trim in V Direction Flag" );
    m_V1TrimBegin.Init( "V1TrimBegin", "Design", this, 0.3, 0.0, 1.0 );
    m_V1TrimBegin.SetDescript( "Begin V Trim Value" );
    m_V1TrimEnd.Init( "V1TrimEnd", "Design", this, 1.0, 0.0, 1.0 );
    m_V1TrimEnd.SetDescript( "End V Trim Value" );

    m_V2TrimFlag.Init( "V2TrimFlag", "Design", this, false, 0, 1 );
    m_V2TrimFlag.SetDescript( "Trim in V Direction Flag" );
    m_V2TrimBegin.Init( "V2TrimBegin", "Design", this, 0.25, 0.0, 1.0 );
    m_V2TrimBegin.SetDescript( "Begin V Trim Value" );
    m_V2TrimEnd.Init( "V2TrimEnd", "Design", this, 0.75, 0.0, 1.0 );
    m_V2TrimEnd.SetDescript( "End V Trim Value" );

    m_ChordTrimFlag.Init( "ChordTrimFlag", "Design", this, false, 0, 1 );
    m_ChordTrimFlag.SetDescript( "Chord Span Flag" );
    m_ChordTrimMin.Init( "ChordTrimMin", "Design", this, 0.1, 0.0, 1.0 );
    m_ChordTrimMin.SetDescript( "Min Chord Trim Value" );
    m_ChordTrimMax.Init( "ChordTrimMax", "Design", this, 0.9, 0.0, 1.0 );
    m_ChordTrimMax.SetDescript( "Max Chord Trim Value" );

    m_WingParentFlag = false;
    m_TessU = 41;
    m_TessW = 41;

    m_MainSurfVec.clear();

}

//==== Destructor ====//
ConformalGeom::~ConformalGeom()
{

}

void ConformalGeom::Scale()
{
    double currentScale = m_Scale() / m_LastScale();
    m_Offset *= currentScale;
    m_LastScale = m_Scale();
}

void ConformalGeom::UpdateSurf()
{
    //===== Find Parent ====//
    Geom* parent_geom = m_Vehicle->FindGeom( m_ParentID );
    if ( !parent_geom )
    {
        return;
    }

    //==== Wing Specific Stuff ====//
    m_WingParentFlag = false;
    if ( parent_geom->GetType().m_Type == MS_WING_GEOM_TYPE )
    {
        m_WingParentFlag = true;
    }

    //==== Copy XForm/Tess Data From Parent ====//
    CopyDataFrom( parent_geom );

    //==== Parent Reference Surfaces  ====//
    vector< VspSurf > parent_surf_vec;
    parent_surf_vec = parent_geom->GetMainSurfVecConstRef();

    //===== Copy Parent ====//
    vector< string > parent_id_vec;
    parent_id_vec.push_back( parent_geom->GetID() );
    vector< string > copy_id_vec = m_Vehicle->CopyGeomVec( parent_id_vec );
    if ( copy_id_vec.size() != 1 )
    {
        return;
    }

    Geom* copy_geom = m_Vehicle->FindGeom( copy_id_vec[0] );
    if ( !copy_geom )
    {
        return;
    }

    copy_geom->Update();    // Make Sure Copy Is Current

    //==== Offset Cross Sections And Reskin ====//
    double offset = m_Offset();
    copy_geom->OffsetXSecs( offset );
    copy_geom->Update();
    copy_geom->GetMainSurfVec( m_MainSurfVec );

    for ( int i = 0 ; i < (int)m_MainSurfVec.size() ; i++ )
    {
        m_MainSurfVec[i].SetFoilSurf( NULL );

        if ( m_MainSurfVec[i].IsClone() )
        {
            int clone_index = m_MainSurfVec[i].GetCloneIndex();
            Matrix4d clone_mat;
            m_MainSurfVec[i].GetCloneMat( clone_mat );

            m_MainSurfVec[i] = m_MainSurfVec[ clone_index ];
            m_MainSurfVec[i].Transform( clone_mat );
        }
        else
        {
            int skin_type = m_MainSurfVec[i].GetSkinType();

            //==== Only Works for Skin Ribs for Now ====//
            if (skin_type == VspSurf::SKIN_RIBS)
            {
                //==== Check If Wing ====//
                if (m_WingParentFlag)
                {
                    //==== Set Wing Specific Trimming Parms ====//
                    SetWingTrimParms(m_MainSurfVec[i]);
                }

                //==== Make Sure Ribs Are Centered ====//
                CenterRibCurves(m_MainSurfVec[i], parent_surf_vec[i], offset);

                //==== Offset Ribs ====//
                OffsetEndRibs(m_MainSurfVec[i], offset);

                //==== Adjust Shape By Scaling Fp and Moving End Ribs ====//
                if (!m_WingParentFlag)
                {
// Measure Error - Expensive
//ComputeMaxOffsetError( m_MainSurfVec[i], parent_surf_vec[i], offset, 20, 8 );
                    AdjustShape(m_MainSurfVec[i], parent_surf_vec[i], offset);
//ComputeMaxOffsetError( m_MainSurfVec[i], parent_surf_vec[i], offset, 20, 8 );
                }

                //==== Trim U and V is Needed ====//
                TrimU(m_MainSurfVec[i]);
                TrimV(m_MainSurfVec[i]);
            }
                //==== To Do - Body of Rev (Pods) ====//
            else if (skin_type == VspSurf::SKIN_BODY_REV)
            {
                //==== Trim U and V is Needed ====//
                TrimU(m_MainSurfVec[i]);
                TrimV(m_MainSurfVec[i]);
            }
        }
    }

    //==== Delete Geom Copy ====//
    m_Vehicle->DeleteGeom( copy_geom->GetID() );

}


void ConformalGeom::CopyDataFrom( Geom* geom_ptr )
{
    //==== Force Attached So Conformal Moves With Parent =====//
    m_TransAttachFlag = vsp::ATTACH_TRANS_COMP;
    m_RotAttachFlag = vsp::ATTACH_ROT_COMP;

    m_TransAttachFlag.Deactivate();
    m_RotAttachFlag.Deactivate();

    m_ULoc.Deactivate();
    m_WLoc.Deactivate();

    //==== Copy Cap Options ====//
    m_CapUMinOption = geom_ptr->m_CapUMinOption();
    m_CapUMinTess   = geom_ptr->m_CapUMinTess();
    m_CapUMaxOption = geom_ptr->m_CapUMaxOption();

    //=== Let User Change Tess
    //m_TessU = geom_ptr->m_TessU();
    //m_TessW = geom_ptr->m_TessW();

    m_SymAncestor = geom_ptr->m_SymAncestor();
    if ( m_SymAncestor() != 0 ) // Not global ancestor.
    {
        m_SymAncestor = m_SymAncestor() + 1;  // + 1 increment for parent
    }
    m_SymAncestOriginFlag = geom_ptr->m_SymAncestOriginFlag();
    m_SymPlanFlag = geom_ptr->m_SymPlanFlag();
    m_SymAxFlag = geom_ptr->m_SymAxFlag();
    m_SymRotN = geom_ptr->m_SymRotN();

    m_SymAncestor.Deactivate();
    m_SymAncestOriginFlag.Deactivate();
    m_SymPlanFlag.Deactivate();
    m_SymAxFlag.Deactivate();
    m_SymRotN.Deactivate();
}

void ConformalGeom::UpdateDrawObj()
{
    Geom::UpdateDrawObj();
}


void ConformalGeom::LoadDrawObjs( vector< DrawObj* > & draw_obj_vec )
{
    Geom::LoadDrawObjs( draw_obj_vec );
}

//==== Approximate Center ====//
vec3d ConformalGeom::ComputeCenter( curve_type & crv )
{
    vec3d sum(0,0,0);
    double tmax = crv.get_tmax();
    for ( int i = 0 ; i < 4 ; i++ )
    {
        double w0 = (double)i/4.0;
        vec3d pnt = crv.f( w0*tmax );
        sum = sum + pnt;
    }
    sum = sum*(1.0/4.0);

    return sum;
}

//==== Translate Curve ====//
void ConformalGeom::TranslateCurve( curve_type & crv, const vec3d & offset )
{
    piecewise_surface_type::point_type pnt;
    pnt << offset.x(), offset.y(), offset.z();
    crv.translate( pnt );
}

//==== Approx Offset Curve - Not Used ====//
void ConformalGeom::OffsetCurve( curve_type & crv, double offset )
{
    curve_type offset_crv;
    piecewise_curve_type::error_code err;
    vec3d center_pnt = ComputeCenter( crv );

    int ns = crv.number_segments();

    for ( int pp = 0; pp < ns; ++pp )
    {
        piecewise_surface_type::curve_type bez;
        crv.get( bez, pp );
        for ( int i=0; i<=bez.degree(); ++i)
        {
            double x = bez.get_control_point(i).x();
            double y = bez.get_control_point(i).y();
            double z = bez.get_control_point(i).z();
            vec3d cp = vec3d( x, y, z );
            vec3d dir = cp - center_pnt;
            double len = dir.mag();
            dir.normalize();


            double off_len = len - offset;
            off_len = MAX( 0.0, off_len );
            vec3d ncp = center_pnt + dir*off_len;

            piecewise_surface_type::point_type offset_cp;
            offset_cp << ncp.x(), ncp.y(), ncp.z();
            bez.set_control_point(offset_cp, i );

        }
        err = offset_crv.push_back( bez );
    }

    crv = offset_crv;
}

//==== Force Reskin ====//
void ConformalGeom::ReSkin( VspSurf & surf, vector< rib_data_type > & rib_vec )
{
    //==== Reskin Surface ====//
    bool closed_flag = surf.GetSkinClosedFlag();
    vector< int > degree_vec;
    surf.GetSkinDegreeVec( degree_vec );
    vector< double > parm_vec;
    surf.GetSkinParmVec( parm_vec );
    surf.SkinRibs( rib_vec, degree_vec, parm_vec, closed_flag );

    if ( m_WingParentFlag )
    {
        surf.FlipNormal();
        surf.SetSurfType( vsp::WING_SURF );
        surf.SetMagicVParm( true );
    }
}

//==== Check For Point XSec ====//
bool ConformalGeom::CheckIfRibIsPoint( rib_data_type & rib )
{
    piecewise_curve_type crv = rib.get_f();

    double tmax = crv.get_tmax();
    vec3d first_pnt = crv.f( 0.0 );
    for ( int i = 1 ; i < 10 ; i++ )
    {
        double w0 = (double)i/10.0;
        vec3d pnt = crv.f( w0*tmax );

        if ( dist( first_pnt, pnt ) > 1.0e-12 )
        {
            return false;
        }
    }
    return true;
}

//==== Move Rib To Center s ====//
void ConformalGeom::CenterRibCurves( VspSurf & surf, const VspSurf & ref_surf, double offset )
{
    vector< rib_data_type > rib_vec;
    surf.GetSkinRibVec( rib_vec );

    vector< rib_data_type > ref_rib_vec;
    ref_surf.GetSkinRibVec( ref_rib_vec );

    for ( int i = 0 ; i < rib_vec.size() ; i++ )
    {
        piecewise_curve_type ref_crv = ref_rib_vec[i].get_f();
        vec3d ref_center = ComputeCenter( ref_crv );

        piecewise_curve_type crv = rib_vec[i].get_f();
        vec3d center = ComputeCenter( crv);

        vec3d del_center = ref_center - center;
        TranslateCurve( crv, del_center );

        rib_vec[i].set_f( crv );
    }

    ReSkin( surf, rib_vec );
}

//==== Adjust Tangent to Reshape To Reduce Error ====//
void ConformalGeom::AdjustShape( VspSurf & surf, const VspSurf & ref_surf, double offset )
{
    vector< rib_data_type > rib_vec;
    surf.GetSkinRibVec( rib_vec );

    if ( rib_vec.size() < 2 )
        return;

    double u_r = 1.0/3.0;
    double u_l = 2.0/3.0;
    double del_scale = 0.01;
    double fraction_target_scale = 0.9;

    unsigned int num_ribs = rib_vec.size();
    vector< double > orig_offset_r(num_ribs, 0.0);
    vector< double > change_offset_r(num_ribs, 0.0);
    vector< double > orig_offset_l(num_ribs, 0.0);
    vector< double > change_offset_l(num_ribs, 0.0);
    vector< rib_data_type > orig_rib_vec = rib_vec;

    bool reskin_flag = false;
    //==== Save Orig Offset and Adjust Rib Fp Scales ====//
    for ( int i = 0 ; i < num_ribs-1 ; i++ )
    {
        orig_offset_r[i] = ComputeAvgOffset( surf, ref_surf, double(i) + u_r );
        orig_offset_l[i] = ComputeAvgOffset( surf, ref_surf, double(i) + u_l );

        if ( rib_vec[i].use_right_fp() )
        {
            piecewise_curve_type fp_r = rib_vec[i].get_right_fp();
            fp_r.scale(1.0 + del_scale);
            rib_vec[i].set_right_fp(fp_r);
            reskin_flag = true;
        }

        if ( rib_vec[i+1].use_left_fp() )
        {
            piecewise_curve_type fp_l = rib_vec[i + 1].get_left_fp();
            fp_l.scale(1.0 + del_scale);
            rib_vec[i + 1].set_left_fp(fp_l);
            reskin_flag = true;
        }
    }

    // This object never uses right_fp or left_fp for skinning.  AdjustShape will
    // not do anything, so bail now.
    if ( !reskin_flag )
    {
        return;
    }

    //==== Reskin All - Changes Will Effect Other Ribs =====//
    ReSkin( surf, rib_vec );

    //==== Compute Changed Offset And Del Fp Scale RIGHT ====//
    for ( int i = 0 ; i < num_ribs-1 ; i++ )
    {
        change_offset_r[i] = ComputeAvgOffset(surf, ref_surf, double(i) + u_r);
        if (rib_vec[i].use_right_fp())
        {
            double del_offset = orig_offset_r[i] - change_offset_r[i];
            if (std::abs(del_offset) > DBL_EPSILON)
            {
                double target_del_offset = orig_offset_r[i] - offset;
                double target_del_scale = del_scale * (target_del_offset / del_offset);
                double target_scale = 1.0 + target_del_scale * fraction_target_scale;
                if (target_scale > 0.5 && target_scale < 2.0)
                {
                    piecewise_curve_type fp_r = orig_rib_vec[i].get_right_fp();
                    fp_r.scale(target_scale);
                    rib_vec[i].set_right_fp(fp_r);
                }
            }
        }
    }

    //==== Compute Changed Offset And Del Fp Scale LEFT ====//
    for ( int i = 0 ; i < num_ribs-1 ; i++ )
    {
        change_offset_l[i] = ComputeAvgOffset( surf, ref_surf, double(i) + u_l );
        if ( rib_vec[i+1].use_left_fp() )
        {
            double del_offset = orig_offset_l[i] - change_offset_l[i];
            if (std::abs(del_offset) > DBL_EPSILON)
            {
                double target_del_offset = orig_offset_l[i] - offset;
                double target_del_scale = del_scale * (target_del_offset / del_offset);
                double target_scale = 1.0 + target_del_scale * fraction_target_scale;
                if (target_scale > 0.5 && target_scale < 2.0)
                {
                    piecewise_curve_type fp_l = orig_rib_vec[i + 1].get_left_fp();
                    fp_l.scale(target_scale);
                    rib_vec[i + 1].set_left_fp(fp_l);
                }
            }
        }
    }

    ReSkin( surf, rib_vec );

    //==== Check For Error Increase ===//
    reskin_flag = false;
    for ( int i = 0 ; i < num_ribs-1 ; i++ )
    {
        double off_r = ComputeAvgOffset( surf, ref_surf, double(i) + u_r );
        double off_l = ComputeAvgOffset( surf, ref_surf, double(i) + u_l );
        double err_r = std::abs( off_r - offset);
        double err_l = std::abs( off_l - offset);
        double orig_err_r = std::abs( orig_offset_r[i] - offset );
        double orig_err_l = std::abs( orig_offset_l[i] - offset );

        if ( err_r > orig_err_r && rib_vec[i].use_right_fp() )
        {
            piecewise_curve_type fp_r = orig_rib_vec[i].get_right_fp();
            rib_vec[i].set_right_fp( fp_r );
            reskin_flag = true;
        }
        if ( err_l > orig_err_l && rib_vec[i+1].use_left_fp() )
        {
            piecewise_curve_type fp_l = orig_rib_vec[i+1].get_left_fp();
            rib_vec[i+1].set_left_fp( fp_l );
            reskin_flag = true;
        }
    }

    if ( reskin_flag )
    {
        ReSkin( surf, rib_vec );
    }

}

//==== Offset End Ribs If Capped or Point ====//
void ConformalGeom::OffsetEndRibs( VspSurf & surf, double offset )
{
    vector< rib_data_type > rib_vec;
    surf.GetSkinRibVec( rib_vec );
    vector < double > parmvec;
    surf.GetSkinParmVec( parmvec );

    int num_ribs = rib_vec.size();
    if ( num_ribs < 2 )
        return;

    bool reskin_flag = false;

    bool u_min_rib_is_point = CheckIfRibIsPoint( rib_vec[0] );
    if ( m_CapUMinOption() != vsp::NO_END_CAP ||  u_min_rib_is_point )
    {
        piecewise_curve_type crv;
        double uoffset = FindEndUOffsetCurve( surf, offset, true, crv );

        if ( u_min_rib_is_point )
        {
            vec3d crv_center = ComputeCenter( crv );

            piecewise_curve_type orig_crv = rib_vec[0].get_f();
            vec3d orig_center = ComputeCenter( orig_crv );
            TranslateCurve( orig_crv, crv_center - orig_center );
            crv = orig_crv;
        }
        rib_vec[0].set_f( crv );

        // Check for blended ribs.
        for ( int irib = 1; irib < num_ribs; irib++ )
        {
            double unext = 0.5; // Parameter to stop checking.
            // Check rib parameter to check for blending
            if ( parmvec[irib] < ( unext - 1e-4 )) // num_ribs >= 2, so [1] is OK.
            {
                double unew = unext - ( (unext - uoffset) * ( unext - parmvec[irib] ) ) * 2.0;

                VspCurve c;
                surf.GetUConstCurve( c, unew );
                rib_vec[ irib ].set_f( c.GetCurve() );
            }
            else // All ribs in order -- once one fails, don't check the rest.
            {
                break;
            }
        }
        reskin_flag = true;
    }

    bool u_max_rib_is_point = CheckIfRibIsPoint( rib_vec[num_ribs-1] );
    if ( m_CapUMaxOption() != vsp::NO_END_CAP || u_max_rib_is_point )
    {
        piecewise_curve_type crv;
        double uoffset = FindEndUOffsetCurve( surf, offset, false, crv );

        if ( u_max_rib_is_point )
        {
            vec3d crv_center = ComputeCenter( crv );

            piecewise_curve_type orig_crv = rib_vec[num_ribs-1].get_f();
            vec3d orig_center = ComputeCenter( orig_crv );
            TranslateCurve( orig_crv, crv_center - orig_center );
            crv = orig_crv;
        }

        rib_vec[num_ribs-1].set_f( crv );

        // Check for blended ribs.
        for ( int irib = num_ribs - 2; irib >= 0; irib-- )
        {
            double unext = parmvec[ num_ribs - 1 ] - 0.5; // Next non-blended rib.
            // Check rib parameter to check for blending
            if ( parmvec[irib] > ( unext + 1e-4 ) )
            {
                double unew = unext + ( ( uoffset - unext ) * ( parmvec[irib] - unext ) ) * 2.0;

                VspCurve c;
                surf.GetUConstCurve( c, unew );
                rib_vec[ irib ].set_f( c.GetCurve() );
            }
            else // All ribs in order -- once one fails, don't check the rest.
            {
                break;
            }
        }
        reskin_flag = true;
    }

    if ( reskin_flag )
    {
        ReSkin( surf, rib_vec );
    }
}

//==== Move Offset Curve To Get Specified Offset ====//
double ConformalGeom::FindEndUOffsetCurve( VspSurf & surf, double offset, bool start_flag, curve_type & crv )
{
    ConformalSpine cs;
    cs.Build( surf );
    double u = 0.0;
    VspCurve vsp_crv;

    if ( start_flag )
    {
        u = cs.FindUGivenDist( offset );
        u = AdjustForSurfaceDist( surf, cs, u, offset, false );
    }
    else
    {
        double max_d = cs.GetMaxDist();
        u = cs.FindUGivenDist( max_d - offset );
        u = AdjustForSurfaceDist( surf, cs, u, offset, true );
    }

    surf.GetUConstCurve( vsp_crv, u );
    crv = vsp_crv.GetCurve();
    return u;
}

//==== End Points Of Conformal Surf Are Moved Offset Dist But Still May Be Too Close To Ref Surf  ===//
//==== Adjust Location Until Closest Dist Matches Offset ====//
double ConformalGeom::AdjustForSurfaceDist( const VspSurf & surf, const ConformalSpine & spine, double u, double offset, bool backward_flag )
{
    double su, sw;
    double start_u = u;
    double stop_u = u;

    vec3d center = spine.FindCenterGivenU( u );
    double w = surf.GetWMax() / 2.0;
    double surf_d = surf.FindNearest( su, sw, center, u, w );

    if ( surf_d >= offset )
    {
        return u;
    }

    ///==== Find U Endpoint Where Suf Dist Is Greater Than Offset ====//
    double del_u = 0.001;
    while ( del_u < 0.5 )
    {
        stop_u = u + del_u;
        if ( backward_flag )
        {
            stop_u = u - del_u;
        }
        center = spine.FindCenterGivenU( stop_u );
        surf_d = surf.FindNearest( su, sw, center, stop_u, w );

        //==== Found U ====//
        if ( surf_d > offset )
        {
            break;
        }
        else
        {
            start_u = stop_u;
            del_u *= 2.0;
        }
    }

    if ( del_u > 0.5 )
    {
        return start_u;
    }

    //==== Have Starting and Stopping U's - Use Bisection ====//
    int num_steps = 6;
    for ( int i = 0 ; i < num_steps ; i++ )
    {
        double bi_u = 0.5 * ( start_u + stop_u );
        center = spine.FindCenterGivenU( bi_u );
        surf_d = surf.FindNearest( su, sw, center, bi_u, w );

        if ( surf_d > offset )
        {
            stop_u = bi_u;
        }
        else
        {
            start_u = bi_u;
        }

    }
    return start_u;
}

//==== Solve For V Trim Parm Given Chord Trim Values ====//
void ConformalGeom::SetWingTrimParms(  VspSurf & surf )
{
    piecewise_surface_type* bez_surface = surf.GetBezierSurface();

    double u_max = bez_surface->get_umax();
    double v_max = bez_surface->get_vmax();

    if ( m_ChordTrimFlag() )
    {
        m_V1TrimFlag = true;
        m_V2TrimFlag = true;

        vec3d p0 = surf.CompPnt01( 0, 0 );
        vec3d p1 = surf.CompPnt01( 0, 0.5 );

        vec3d pmin = p0 + (p1 - p0)*m_ChordTrimMin();
        vec3d pmax = p0 + (p1 - p0)*m_ChordTrimMax();

        double u, vmin, vmax;
        surf.FindNearest01( u, vmin, pmin, 0, m_ChordTrimMin()/2.0 );
        surf.FindNearest01( u, vmax, pmax, 0, m_ChordTrimMax()/2.0 );

        if ( vmin < 0.5 )
        {
            m_V1TrimBegin = 1.0 - vmin;
            m_V1TrimEnd = vmin;
        }
        else
        {
            m_V1TrimBegin = vmin;
            m_V1TrimEnd = 1.0 - vmin;
        }
        if ( vmax < 0.5 )
        {
            m_V2TrimBegin = vmax;
            m_V2TrimEnd = 1.0 - vmax;
        }
        else
        {
            m_V2TrimBegin = 1.0 - vmax;
            m_V2TrimEnd = vmax;
        }
    }

}

//==== Trim In U Direction - Use Flat Cap ====//
void ConformalGeom::TrimU( VspSurf & surf )
{
    if ( !m_UTrimFlag() )
    {
        return;
    }

    // Prevent UTrim crossover.
    m_UTrimMin.SetUpperLimit( m_UTrimMax() - 0.001 );
    m_UTrimMax.SetLowerLimit( m_UTrimMin() + 0.001 );

    if ( m_UTrimMin() >= m_UTrimMax() )
    {
        m_UTrimMin = MAX( m_UTrimMax() - 0.001, 0.0 );
    }

    piecewise_surface_type* bez_surface = surf.GetBezierSurface();
    piecewise_surface_type::index_type nupatch, nvpatch;
    piecewise_surface_type::error_code err;

    nupatch = bez_surface->number_u_patches();
    nvpatch = bez_surface->number_v_patches();

    piecewise_surface_type s1, s2, s3, s4, s5, s6;
    double u_min = bez_surface->get_u0();
    double u_max = bez_surface->get_umax();

    err = bez_surface->split_u( s1, s2, u_max*m_UTrimMax() );
    if ( err != piecewise_surface_type::NO_ERRORS )
    {
        s1 = *bez_surface;          // Problems Dont Use This Split Val
    }

    err = s1.split_u( s3, s4, u_max*m_UTrimMin() );
    if ( err != piecewise_surface_type::NO_ERRORS )
    {
        s4 = s1;                    // Problems Dont Use This Split Val
    }

    *bez_surface = s4;

    bez_surface->set_u0( u_min );

    m_CapUMinOption = vsp::FLAT_END_CAP;
    m_CapUMaxOption = vsp::FLAT_END_CAP;

}

//==== Trim In V Direction ====//
void ConformalGeom::TrimV( VspSurf & surf )
{
    double cap_offset = 0.001;

    if (!m_V1TrimFlag() && !m_V2TrimFlag() )
    {
        return;
    }

    piecewise_surface_type* bez_surface = surf.GetBezierSurface();
    piecewise_surface_type s1, s2, s3, s4, s5, s6, temp_surf;
    piecewise_surface_type::index_type ip, jp, nupatch, nvpatch;

    //==== Find Trim Values =====//
    double v_max = bez_surface->get_vmax();
    double tb1 = v_max*m_V1TrimBegin();
    double te1 = v_max*m_V1TrimEnd();
    double tb2 = v_max*m_V2TrimBegin();
    double te2 = v_max*m_V2TrimEnd();

    //==== Move A Bit To Allow For Cap ====//
    if ( tb1 < cap_offset )
    {
        tb1 = cap_offset;
    }
    if ( tb2 < cap_offset )
    {
        tb2 = cap_offset;
    }

    s1 = *bez_surface;
    if ( m_V1TrimFlag() )
    {
        s1.split_v( tb1 );
        s1.split_v( te1 );
        //==== Cap Split ====//
        s1.split_v( tb1 - cap_offset );
    }

    if (  m_V2TrimFlag() )
    {
        s1.split_v( tb2 );
        s1.split_v( te2 );

        //==== Cap Split ====//
        s1.split_v( tb2 - cap_offset );
    }

    //==== Find Patches ====//
    vector< double > pmap;
    s1.get_pmap_v( pmap );
    int b1, e1, b2, e2;
    b1 = e1 = b2 = e2 = -1;
    for ( int i = 0 ; i < pmap.size() ; i++ )
    {
        if ( m_V1TrimFlag() )
        {
            if ( std::abs( pmap[i] - tb1 )  < DBL_EPSILON )
            {
                b1 = i;
            }
            if ( std::abs( pmap[i] - te1 )  < DBL_EPSILON )
            {
                e1 = i;
            }
        }
        if ( m_V2TrimFlag() )
        {
            if ( std::abs( pmap[i] - tb2 )  < DBL_EPSILON )
            {
                b2 = i;
            }
            if ( std::abs( pmap[i] - te2 )  < DBL_EPSILON )
            {
                e2 = i;
            }
        }
    }

    //==== Total Number Of Patches ====//
    nupatch = s1.number_u_patches();
    nvpatch = s1.number_v_patches();

    //==== Build List Of Trim/No Trim Patches ====//
    int start_index = 0;
    vector< int > patch_types;
    patch_types.resize( nvpatch, 0 );   // 0 - Use, 1 - Trim
    for ( int i = 0 ; i < nvpatch ; i++ )
    {
        if ( b1 >= 0 && e1 >= 0 )
        {
            if ( b1 < e1 && ( i >=  b1 && i < e1 ) )
            {
                patch_types[i] = 1;
            }
            else if ( e1 < b1 && i < e1  )
            {
                patch_types[i] = 1;
            }
            else if ( e1 < b1 && i >= b1  )
            {
                patch_types[i] = 1;
            }
        }
        if ( b2 >= 0 && e2 >= 0 )
        {
            if ( b2 < e2 && ( i >=  b2 && i < e2 ) )
            {
                patch_types[i] = 1;
            }
            else if ( e2 < b2 && i < e2  )
            {
                patch_types[i] = 1;
            }
            else if ( e2 < b2 && i >= b2  )
            {
                patch_types[i] = 1;
            }
        }
        if ( patch_types[i] )
        {
            start_index = i;
        }
    }

    //==== Find Used Patches  ====//
    vector< int > used_patch_vec;
    for ( int i = 0 ; i < nvpatch ; i++ )
    {
        int index = (i + start_index)%nvpatch;
        if ( patch_types[index] == 0 )
        {
            used_patch_vec.push_back(index);
        }
    }

    //===== Build Capped Surface ====//
    piecewise_surface_type capped_surf;
    capped_surf.init_uv( nupatch, used_patch_vec.size() );

    for( ip = 0; ip < nupatch; ++ip )
    {
        for ( jp = 0 ; jp < used_patch_vec.size() ; ++jp )
        {
            int pid = used_patch_vec[jp];
            surface_patch_type *patch = s1.get_patch( ip, pid );
            capped_surf.set( *patch, ip, jp );
        }
    }

    //==== Cap Trimmed Part ====//
    for ( int i = 1 ; i < (int)used_patch_vec.size() ; i++ )
    {
        //==== Look For Gap
        if ( ( used_patch_vec[i] - used_patch_vec[i-1] ) > 1 )
        {
            CapTrimmedSurf( capped_surf, i, i-1 );
        }
    }

    //==== Cap Last Trim
    CapTrimmedSurf( capped_surf, 0, capped_surf.number_v_patches()-1 );
    *bez_surface = capped_surf;

}

//==== Cap Trimmed Surface In V Direction ====//
void ConformalGeom::CapTrimmedSurf( piecewise_surface_type & psurf, int match_index, int stretch_index )
{
    piecewise_surface_type::index_type nupatch;

    nupatch = psurf.number_u_patches();
    for ( int ip = 0 ; ip < nupatch ; ++ip )
    {
        surface_patch_type::index_type icp, nu, nv;

        surface_patch_type *match_v_patch = psurf.get_patch( ip, match_index );
        surface_patch_type *stretch_v_patch = psurf.get_patch( ip, stretch_index );

        nu = match_v_patch->degree_u();
        nv = match_v_patch->degree_v();

        for( icp = 0; icp <= nu; ++icp )
        {
            surface_patch_type::point_type p = match_v_patch->get_control_point( icp, 0 );
            stretch_v_patch->set_control_point( p, icp, nv );
        }
    }
}

//==== Find Distances At Point Along Curve To Surf =====//
void ConformalGeom::FindDists( const VspSurf & surf, piecewise_curve_type & curve, double u0, vector< double > & dist_vec )
{
    int num_samps = dist_vec.size();
    if ( num_samps <= 0 )
        return;

    double u, w;
    double tmax = curve.get_tmax();
    for ( int i = 0 ; i < num_samps ; i++ )
    {
        double w0 = (double)i/(double)num_samps;
        vec3d pnt = curve.f( w0*tmax );

        dist_vec[i] = surf.FindNearest( u, w, pnt, u0, w0*tmax );
    }
}

double ConformalGeom::ComputeAvgOffset( VspSurf & surf, const VspSurf & ref_surf, double u )
{
    VspCurve crv;
    surf.GetUConstCurve( crv, u );
    piecewise_curve_type samp_crv = crv.GetCurve();

    vector< double > dist_vec;
    const unsigned int num_samps = 4;
    dist_vec.resize( num_samps, 0.0 );

    FindDists( ref_surf, samp_crv, u, dist_vec );

    double avg_offset = 0.0;
    for ( int i = 0 ; i < num_samps ; i++ )
    {
        avg_offset += dist_vec[i];
    }

    avg_offset /= (double)num_samps;

    return avg_offset;
}

//==== Compute Avg And Max Offset Errors and Print - Expensive Use for Debugging ====//
double ConformalGeom::ComputeMaxOffsetError( VspSurf & surf, VspSurf &  ref_surf, double offset, int num_u, int num_w )
{
    VspCurve crv;
    double max_err = 0.0;
    int sum_cnt = 0;
    double sum_err = 0.0;
    vector< double > dist_vec;

    for ( int i = 1 ; i < num_u-2 ; i++ )
    {
        double u = (surf.GetUMax()*(double)i)/(double)(num_u-1);

        surf.GetUConstCurve( crv, u );
        piecewise_curve_type samp_crv = crv.GetCurve();

        dist_vec.resize( num_w, 0.0 );
        FindDists( ref_surf, samp_crv, u, dist_vec );

        for ( int j = 0 ; j < (int)dist_vec.size() ; j++ )
        {
            double err = std::abs( dist_vec[j] - offset );
            if ( err > max_err )
                max_err = err;
            sum_err += err;
            sum_cnt++;

        }
    }
    double avg_err = sum_err/(double)sum_cnt;

printf( "Avg_Err = %f   Max_Err = %f\n", avg_err, max_err );

    return max_err;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//============================================================================================//
//================================  Spine  ===================================================//
////////////////////////////////////////////////////////////////////////////////////////////////

//==== Build Spine - Vector of U, Center Points, Distance Along Spine ====//
void ConformalSpine::Build( const VspSurf & surf )
{
    //===== Clear Data ====//
    m_UVec.clear();
    m_CenterVec.clear();
    m_DistVec.clear();
    m_MaxDist = 0.0;

    const unsigned int num_samps = 100;
    m_UVec.reserve( num_samps );
    m_CenterVec.reserve( num_samps );
    m_DistVec.reserve( num_samps );

    m_DistVec.push_back(0.0);

    for ( int i = 0 ; i < num_samps ; i++ )
    {
        double u = (double)( i * surf.GetUMax() ) / (double)(num_samps - 1);
        vec3d sum;

        for ( int j = 0 ; j < 4 ; j++ )
        {
            double w = (double)( j * surf.GetWMax() ) / 4.0;
            vec3d pnt = surf.CompPnt( u, w );

            sum = sum + pnt;
        }
        vec3d center = sum*0.25;

        m_UVec.push_back( u );
        m_CenterVec.push_back( center );

        if ( m_CenterVec.size() >= 2 )
        {
            m_DistVec.push_back(m_DistVec[i - 1] + dist(m_CenterVec[i], m_CenterVec[i - 1]));
        }
    }
    m_MaxDist = m_DistVec.back();
}

//==== Given Dist Along Spine - Find U ====//
double ConformalSpine::FindUGivenDist( double d ) const
{
    double ret_u = 0.0;

    if ( m_DistVec.size() < 2 )
    {
        return ret_u;
    }

    if ( d > m_DistVec.back() )
    {
        return m_UVec.back();
    }

    for ( int i = 1 ; i < m_DistVec.size() ; i++ )
    {
        if ( d >= m_DistVec[i-1] && d <= m_DistVec[i] )
        {
            double del_d = m_DistVec[i] - m_DistVec[i-1];
            if ( del_d < DBL_EPSILON )
            {
                return m_UVec[i - 1];
            }

            double fract = ( d - m_DistVec[i-1] ) / del_d;
            ret_u = m_UVec[i-1] + fract * ( m_UVec[i] - m_UVec[i-1] );

            return ret_u;
        }
    }

    return ret_u;
}

//==== Find Center Of Spine Given U ====//
vec3d ConformalSpine::FindCenterGivenU( double u ) const
{
    vec3d center;

    if ( m_CenterVec.size() < 2 )
    {
        return center;
    }

    center = m_CenterVec[0];
    for ( int i = 1 ; i < m_UVec.size() ; i++ )
    {
        if ( u >= m_UVec[i-1] && u <= m_UVec[i] )
        {
            double del_u = m_UVec[i] - m_UVec[i-1];
            if ( del_u < DBL_EPSILON )
            {
                return m_CenterVec[i - 1];
            }

            double fract = ( u - m_UVec[i-1] ) / del_u;
            center = m_CenterVec[i-1] + fract * ( m_CenterVec[i] - m_CenterVec[i-1] );
            return center;
        }
    }
    return center;
}

double ConformalSpine::GetSpineLength() const
{
    double length = 0;

    if ( m_CenterVec.size() < 2 )
    {
        return length;
    }

    for ( size_t i = 1; i < m_CenterVec.size(); i++ )
    {
        length += dist( m_CenterVec[i - 1], m_CenterVec[i] );
    }

    return length;
}

//==== Find U Given Length AlongSpine ====//
double ConformalSpine::FindUGivenLengthAlongSpine( double length ) const
{
    double u = 0;

    if ( m_UVec.size() < 2 || length < 0 || length > GetSpineLength() )
    {
        return u;
    }

    // Init values
    double length_0 = 0;
    double length_1 = 0;

    for ( size_t j = 1; j < m_CenterVec.size(); j++ )
    {
        length_1 = length_0 + dist( m_CenterVec[j - 1], m_CenterVec[j] );
        double u_0 = m_UVec[j - 1];
        double u_1 = m_UVec[j];

        if ( length >= length_0 && length <= length_1 )
        {
            if ( std::abs( length_0 - length ) <= FLT_EPSILON )
            {
                u = u_0;
            }
            else if ( std::abs( length_1 - length ) <= FLT_EPSILON )
            {
                u = u_1;
            }
            else
            {
                // Linear interpolation
                u = u_0 + ( length - length_0 ) * ( ( u_1 - u_0 ) / ( length_1 - length_0 ) );
            }
            return u;
        }
        else
        {
            length_0 = length_1;
        }
    }
    return u;
}
