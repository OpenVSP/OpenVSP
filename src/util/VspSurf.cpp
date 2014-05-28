//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// VspSurf.cpp:
// J.R Gloudemans
//
//////////////////////////////////////////////////////////////////////


#include <stdio.h>
#include <math.h>
#include <algorithm>
#include <set>

#include "VspSurf.h"
#include "StlHelper.h"

#include "eli/geom/curve/piecewise_creator.hpp"
#include "eli/geom/surface/piecewise_creator.hpp"
#include "eli/geom/intersect/minimum_distance_surface.hpp"

typedef piecewise_surface_type::index_type surface_index_type;
typedef piecewise_surface_type::point_type surface_point_type;
typedef piecewise_surface_type::rotation_matrix_type surface_rotation_matrix_type;
typedef piecewise_surface_type::bounding_box_type surface_bounding_box_type;
typedef piecewise_surface_type::tolerance_type surface_tolerance_type;

typedef eli::geom::curve::piecewise_cubic_spline_creator<double, 3, surface_tolerance_type> piecewise_cubic_spline_creator_type;
typedef eli::geom::curve::piecewise_linear_creator<double, 3, surface_tolerance_type> piecewise_linear_creator_type;

//=============================================================================//
//============================ VspJointInfo    ================================//
//=============================================================================//

VspJointInfo::VspJointInfo()
{
    m_State = 0;
    for ( int i = 0; i < 4; ++i )
    {
        m_LeftAngle[i] = 0;
        m_LeftStrength[i] = 0;
        m_LeftCurvature[i] = 0;
        m_RightAngle[i] = 0;
        m_RightStrength[i] = 0;
        m_RightCurvature[i] = 0;
    }
}

VspJointInfo::VspJointInfo( const VspJointInfo &ji )
{
    m_State = ji.m_State;
    for ( int i = 0; i < 4; ++i )
    {
        m_LeftAngle[i] = ji.m_LeftAngle[i];
        m_LeftStrength[i] = ji.m_LeftStrength[i];
        m_LeftCurvature[i] = ji.m_LeftCurvature[i];
        m_RightAngle[i] = ji.m_RightAngle[i];
        m_RightStrength[i] = ji.m_RightStrength[i];
        m_RightCurvature[i] = ji.m_RightCurvature[i];
    }
}

VspJointInfo::~VspJointInfo()
{
}

void VspJointInfo::SetLeftParams( int side, const double &angle, const double &strength, const double &curvature )
{
    if ( side < NUM_SIDES )
    {
        m_LeftAngle[side] = angle;
        m_LeftStrength[side] = strength;
        m_LeftCurvature[side] = curvature;
    }
}

double VspJointInfo::GetLeftAngle( int side ) const
{
    if ( side < NUM_SIDES )
    {
        return m_LeftAngle[side];
    }
    else
    {
        return -1;
    }
}

double VspJointInfo::GetLeftStrength( int side ) const
{
    if ( side < NUM_SIDES )
    {
        return m_LeftStrength[side];
    }
    else
    {
        return -1;
    }
}

double VspJointInfo::GetLeftCurvature( int side ) const
{
    if ( side < NUM_SIDES )
    {
        return m_LeftCurvature[side];
    }
    else
    {
        return -1;
    }
}

void VspJointInfo::SetRightParams( int side, const double &angle, const double &strength, const double &curvature )
{
    if ( side < NUM_SIDES )
    {
        m_RightAngle[side] = angle;
        m_RightStrength[side] = strength;
        m_RightCurvature[side] = curvature;
    }
}

double VspJointInfo::GetRightAngle( int side ) const
{
    if ( side < NUM_SIDES )
    {
        return m_RightAngle[side];
    }
    else
    {
        return -1;
    }
}

double VspJointInfo::GetRightStrength( int side ) const
{
    if ( side < NUM_SIDES )
    {
        return m_RightStrength[side];
    }
    else
    {
        return -1;
    }
}

double VspJointInfo::GetRightCurvature( int side ) const
{
    if ( side < NUM_SIDES )
    {
        return m_RightCurvature[side];
    }
    else
    {
        return -1;
    }
}

void VspJointInfo::SetState( int state )
{
    if ( ( state > -1 ) && ( state < NUM_CONTINUITY_TYPES ) )
    {
        m_State = state;
    }
    else
    {
        assert( false );
    }
}

int VspJointInfo::GetState() const
{
    return m_State;
}

bool VspJointInfo::C1Continuous() const
{
    switch ( m_State )
    {
    case( C1_AUTO ):
    case( C1 ):
    case( C2_AUTO ):
    case( C2 ):
    case( G2 ):
    {
        return true;
        break;
    }
    case( FULL ):
    case( C0 ):
    case( G1 ):
    {
        return false;
        break;
    }
    default:
    {
        assert( false );
        break;
    }
    }
    return false;
}

bool VspJointInfo::G1Continuous() const
{
    switch ( m_State )
    {
    case( C1_AUTO ):
    case( C1 ):
    case( G1 ):
    case( C2_AUTO ):
    case( C2 ):
    case( G2 ):
    {
        return true;
        break;
    }
    case( FULL ):
    case( C0 ):
    {
        return false;
        break;
    }
    default:
    {
        assert( false );
        break;
    }
    }
    return false;
}

bool VspJointInfo::C2Continuous() const
{
    switch ( m_State )
    {
    case( C2_AUTO ):
    case( C2 ):
    {
        return true;
        break;
    }
    case( FULL ):
    case( C0 ):
    case( C1_AUTO ):
    case( C1 ):
    case( G1 ):
    case( G2 ):
    {
        return false;
        break;
    }
    default:
    {
        assert( false );
        break;
    }
    }
    return false;
}

bool VspJointInfo::G2Continuous() const
{
    switch ( m_State )
    {
    case( C2_AUTO ):
    case( C2 ):
    case( G2 ):
    {
        return true;
        break;
    }
    case( FULL ):
    case( C0 ):
    case( C1_AUTO ):
    case( C1 ):
    case( G1 ):
    {
        return false;
        break;
    }
    default:
    {
        assert( false );
        break;
    }
    }
    return false;
}

int VspJointInfo::DegreesOfFreedom() const
{
    switch ( m_State )
    {
    case( FULL ):
    case( C2_AUTO ):
    case( C2 ):
    case( G2 ):
    {
        return 3;
        break;
    }
    case( C0 ):
    case( C1_AUTO ):
    case( C1 ):
    case( G1 ):
    {
        return 2;
        break;
    }
    default:
    {
        assert( false );
        break;
    }
    }
    return -1;
}

bool VspJointInfo::FiniteDifferenceFp() const
{
    return ( m_State == C1_AUTO );
}

bool VspJointInfo::FiniteDifferenceFpp() const
{
    return ( m_State == C2_AUTO );
}

//===== Constructor  =====//
VspSurf::VspSurf()
{
    m_FlipNormal = false;
}

//===== Destructor  =====//
VspSurf::~VspSurf()
{
}

int VspSurf::GetNumSectU() const
{
    return m_Surface.number_u_patches();
}

int VspSurf::GetNumSectW() const
{
    return m_Surface.number_v_patches();
}

void VspSurf::ReverseUDirection()
{
    m_Surface.reverse_u();
}

void VspSurf::ReverseWDirection()
{
    m_Surface.reverse_v();
}

//==== Flip U/W Directions =====//
void VspSurf::SwapUWDirections()
{
    m_Surface.swap_uv();
}

//==== Transform Control Points =====//
void VspSurf::Transform( Matrix4d & mat )
{
    surface_rotation_matrix_type rmat;
    double *mmat( mat.data() );
    surface_point_type trans;

    rmat << mmat[0], mmat[4], mmat[8],
         mmat[1], mmat[5], mmat[9],
         mmat[2], mmat[6], mmat[10];
    trans << mmat[12], mmat[13], mmat[14];

    m_Surface.rotate( rmat );
    m_Surface.translate( trans );
}

void VspSurf::GetBoundingBox( BndBox &bb ) const
{
    surface_bounding_box_type bbx;
    vec3d v3min, v3max;

    m_Surface.get_bounding_box( bbx );
    v3min.set_xyz( bbx.get_min().x(), bbx.get_min().y(), bbx.get_min().z() );
    v3max.set_xyz( bbx.get_max().x(), bbx.get_max().y(), bbx.get_max().z() );
    bb.Reset();
    bb.Update( v3min );
    bb.Update( v3max );
}

bool VspSurf::IsClosedU() const
{
    return m_Surface.closed_u();
}

bool VspSurf::IsClosedW() const
{
    return m_Surface.closed_v();
}

double VspSurf::FindNearest( double &u, double &w, const vec3d &pt ) const
{
    double dist;
    surface_point_type p;
    p << pt.x(), pt.y(), pt.z();

    dist = eli::geom::intersect::minimum_distance( u, w, m_Surface, p );

	return dist;
}

double VspSurf::FindNearest( double &u, double &w, const vec3d &pt, const double &u0, const double &w0 ) const
{
    double dist;
    surface_point_type p;
    p << pt.x(), pt.y(), pt.z();

    dist = eli::geom::intersect::minimum_distance( u, w, m_Surface, p, u0, w0 );

	return dist;
}

double VspSurf::FindNearest01( double &u, double &w, const vec3d &pt ) const
{
    double dist;
    int num_sectU, num_sectW;

    num_sectU = GetNumSectU();
    num_sectW = GetNumSectW();

    dist = FindNearest( u, w, pt );

    u = u / num_sectU;
    w = w / num_sectW;

	return dist;
}

double VspSurf::FindNearest01( double &u, double &w, const vec3d &pt, const double &u0, const double &w0 ) const
{
    double dist;
    int num_sectU, num_sectW;

    num_sectU = GetNumSectU();
    num_sectW = GetNumSectW();

    dist = FindNearest( u, w, pt, u0 * num_sectU, w0 * num_sectW );

    u = u / num_sectU;
    w = w / num_sectW;

	return dist;
}

void VspSurf::GetUConstCurve( VspCurve &c, const double &u ) const
{
    piecewise_curve_type pwc;
    m_Surface.get_uconst_curve(pwc, u);
    c.SetCurve(pwc);
}

void VspSurf::GetWConstCurve( VspCurve &c, const double &w ) const
{
    piecewise_curve_type pwc;
    m_Surface.get_vconst_curve(pwc, w);
    c.SetCurve(pwc);
}

//===== Compute a Relative Rotation Transformation Matrix from Component's
//      Coordinate System to a Surface Coordinate System ====//
Matrix4d VspSurf::CompRotCoordSys( const double &u, const double &w )
{
    Matrix4d retMat; // Return Matrix

    double tempMat[16];
    // Get du and norm, cross them to get the last orthonormal vector
    vec3d du = CompTanU01( u, w );
    du.normalize();
    vec3d norm = CompNorm01( u, w ); // use CompNorm01 since normals now face outward
    norm.normalize();
    vec3d dw = cross( norm, du );

    // Place axes in as cols of Rot mat
    retMat.getMat( tempMat );
    tempMat[0] = du.x();
    tempMat[4] = dw.x();
    tempMat[8]  = norm.x();
    tempMat[1] = du.y();
    tempMat[5] = dw.y();
    tempMat[9]  = norm.y();
    tempMat[2] = du.z();
    tempMat[6] = dw.z();
    tempMat[10] = norm.z();
    retMat.initMat( tempMat );
    return retMat;
}

//===== Compute a Relative Translation Transformation Matrix from Component's
//      Coordinate System to a Surface Coordinate System ====//
Matrix4d VspSurf::CompTransCoordSys( const double &u, const double &w )
{
    Matrix4d retMat; // Return Matrix

    // Get x,y,z location of u,w coordinate and place in translation matrix
    vec3d cartCoords = CompPnt01( u, w );
    retMat.translatef( cartCoords.x(), cartCoords.y(), cartCoords.z() );
    return retMat;
}

void VspSurf::CreateBodyRevolution( const VspCurve &input_crv )
{
    eli::geom::surface::create_body_of_revolution( m_Surface, input_crv.GetCurve(), 0, eli::geom::surface::OUTWARD_NORMAL );

    ResetFlipNormal();
}

void VspSurf::InterpolateManual( const std::vector<VspCurve> &input_crv_vec, const vector<VspJointInfo> &joint_info_vec, bool closed_flag )
{
    vector<piecewise_curve_type> cross_section;
    vector<VspJointInfo> joint_info( joint_info_vec );
    VspCurve spine;

    // NOTE: This should be passed in as a parameter
    {
        vector<vec3d> pt( 2 );
        vector<double> param( 2 );

        pt[0].set_xyz( 0, 0, 0 );
        pt[1].set_xyz( 1, 0, 0 );
        param[0] = 0;
        param[1] = 1;
        spine.InterpolateLinear( pt, param, false );
    }

    // error checking
    if ( input_crv_vec.size() < 3 )
    {
        std::cerr << "Too few cross sections to VspSurf::InterpolateManual()" << std::endl;
        return;
    }

    if ( input_crv_vec.size() != joint_info_vec.size() )
    {
        std::cerr << "Cross section vector not same length as joint info vector?!?!" << std::endl;
        return;
    }

    // get cross sections to be: (1) same number of sections, (2) same section parameterizations
    // and (3) same degree curves on start and end of each patch
    PrepairCrossSections( cross_section, input_crv_vec );

    // add last cross section that is same as first for closed surfaces
    if ( closed_flag )
    {
        VspJointInfo ji;

        for ( int idx = 0; idx < 4; ++idx )
        {
            ji.SetRightParams( idx, joint_info[0].GetLeftAngle( idx ),
                               joint_info[0].GetLeftStrength( idx ),
                               joint_info[0].GetLeftCurvature( idx ) );
            ji.SetLeftParams( idx, joint_info[joint_info.size() - 1].GetRightAngle( idx ),
                              joint_info[joint_info.size() - 1].GetRightStrength( idx ),
                              joint_info[joint_info.size() - 1].GetRightCurvature( idx ) );
        }

        joint_info.push_back( ji );
        cross_section.push_back( cross_section[0] );
    }

    surface_index_type i, j, nu( cross_section.size() - 1 ), nv( cross_section[0].number_segments() );

    m_Surface.init_uv( nu, nv );

    // TODO: At this point should set the u- and v-parameterization since know du=1 and dv's come from cross_section[0]

    bool print_left_side( false );
    int i_print = 0;

    int joint_v[4] = {VspJointInfo::TOP_SIDE, VspJointInfo::LEFT_SIDE, VspJointInfo::BOTTOM_SIDE, VspJointInfo::RIGHT_SIDE};
    vector<VspCurve> fp_left( nu + 1 ), fpp_left( nu + 1 ), fp_right( nu + 1 ), fpp_right( nu + 1 );

    // determine the top, bottom, left, & right 1st and 2nd derivatives for skinning
    for ( i = 0; i <= nu; ++i )
    {
        vector<vec3d> fp_left_pt( 4 ), fpp_left_pt( 4 ), fp_right_pt( 4 ), fpp_right_pt( 4 );
        vector<double> param( 5 );

        param[0] = 0;
        param[1] = 1;
        param[2] = 2;
        param[3] = 3;
        param[4] = 4;
        for ( j = 0; j < 4; ++j )
        {
            int side( joint_v[j] ), dof;
            surface_point_type xvec, yvec, zvec;

            dof = joint_info[i].DegreesOfFreedom();

            if ( dof > 1 )
            {
                double u( i / static_cast<double>( nu ) ), v( j );
                piecewise_curve_type sp( spine.GetCurve() );

                // determine the coordinate system for this side
                // in: cross_section, sp, u, v
                // out: xvec, yvec, zvec
                {
                    surface_point_type fv;

                    // TODO: xvec will be the tangent vector of the spine once generalized skinning is implemented
                    xvec = sp.fp( u );

                    fv = cross_section[i].fp( v );
                    if ( fv.norm() == 0 )
                    {
                        double ang( v * PI / 2 );
                        surface_rotation_matrix_type mat;

                        fv << 0, 1, 0;
                        mat << 1, 0,              0,
                            0, std::cos( ang ), -std::sin( ang ),
                            0, std::sin( ang ),  std::cos( ang );
                        fv = fv * mat;
                    }

                    yvec = fv;
                    yvec.normalize();
                    zvec = xvec.cross( yvec );
                    zvec.normalize();
                }

                // determine the left and right finite difference approximations to f_u and f_uu
                surface_point_type fu_fd, fuu_fd;
                if ( i == 0 )
                {
                    fu_fd = cross_section[1].f( v ) - cross_section[0].f( v );
                    fuu_fd = ( cross_section[2].f( v ) - 2 * cross_section[1].f( v ) + cross_section[0].f( v ) );
                }
                else if ( i == nu )
                {
                    fu_fd = cross_section[i].f( v ) - cross_section[i - 1].f( v );
                    fuu_fd = ( cross_section[i].f( v ) - 2 * cross_section[i - 1].f( v ) + cross_section[i - 2].f( v ) );
                }
                else
                {
                    fu_fd = 0.5 * ( cross_section[i + 1].f( v ) - cross_section[i - 1].f( v ) );
                    fuu_fd = ( cross_section[i + 1].f( v ) - 2 * cross_section[i].f( v ) + cross_section[i - 1].f( v ) );
                }

                // calculate the first derivative
                if ( joint_info[i].FiniteDifferenceFp() )
                {
                    fp_left_pt[j].set_xyz( fu_fd.x(), fu_fd.y(), fu_fd.z() );
                    fp_right_pt[j] = fp_left_pt[j];
                }
                else
                {
                    double left_angle, right_angle, fu_fd_norm( fu_fd.norm() );
                    surface_point_type tmp;

                    left_angle = joint_info[i].GetLeftAngle( side );
                    right_angle = joint_info[i].GetRightAngle( side );

                    tmp = std::cos( left_angle ) * xvec + std::sin( left_angle ) * zvec;
                    fp_left_pt[j].set_xyz( tmp.x(), tmp.y(), tmp.z() );
                    if ( joint_info[i].C1Continuous() )
                    {
                        fp_left_pt[j] = fp_left_pt[j] * fu_fd_norm * joint_info[i].GetLeftStrength( side );
                        fp_right_pt[j] = fp_left_pt[j];
                    }
                    else
                    {
                        if ( joint_info[i].G1Continuous() )
                        {
                            fp_right_pt[j] = fp_left_pt[j];
                        }
                        else
                        {
                            tmp = std::cos( right_angle ) * xvec + std::sin( right_angle ) * zvec;
                            fp_right_pt[j].set_xyz( tmp.x(), tmp.y(), tmp.z() );
                        }
                        fp_left_pt[j] = fp_left_pt[j] * fu_fd_norm * joint_info[i].GetLeftStrength( side );
                        fp_right_pt[j] = fp_right_pt[j] * fu_fd_norm * joint_info[i].GetRightStrength( side );
                    }
                }

                if ( dof > 2 )
                {
                    // calculate the second derivative
                    if ( joint_info[i].FiniteDifferenceFpp() )
                    {
                        fpp_left_pt[j].set_xyz( fuu_fd.x(), fuu_fd.y(), fuu_fd.z() );
                        fpp_right_pt[j] = fpp_left_pt[j];
                    }
                    else
                    {
                        int udim[2];
                        double alpha( 1.5 ), angle[2], curvature[2];
                        surface_point_type cp[2], cpp[2];
                        Eigen::Matrix<double, 3, 3> A;
                        Eigen::Matrix<double, 3, 1> rhs;

                        angle[0] = joint_info[i].GetLeftAngle( side );
                        angle[1] = joint_info[i].GetRightAngle( side );
                        curvature[0] = joint_info[i].GetLeftCurvature( side );
                        curvature[1] = joint_info[i].GetRightCurvature( side );
                        cp[0] << fp_left_pt[j].x(), fp_left_pt[j].y(), fp_left_pt[j].z();
                        cp[1] << fp_right_pt[j].x(), fp_right_pt[j].y(), fp_right_pt[j].z();
                        if ( i == 0 )
                        {
                            udim[0] = 3;
                            udim[1] = joint_info[i].DegreesOfFreedom() + joint_info[i + 1].DegreesOfFreedom() - 1;
                        }
                        else if ( i == nu )
                        {
                            udim[0] = joint_info[i - 1].DegreesOfFreedom() + joint_info[i].DegreesOfFreedom() - 1;
                            udim[1] = 3;
                        }
                        else
                        {
                            udim[0] = joint_info[i - 1].DegreesOfFreedom() + joint_info[i].DegreesOfFreedom() - 1;
                            udim[1] = joint_info[i].DegreesOfFreedom() + joint_info[i + 1].DegreesOfFreedom() - 1;
                        }

                        int nit;

                        if ( joint_info[i].C2Continuous() )
                        {
                            nit = 1;
                        }
                        else
                        {
                            nit = 2;
                        }
                        for ( int ii = 0; ii < nit; ++ii )
                        {
                            double tmp( cp[ii].norm() );

                            tmp *= tmp * tmp;

                            // this solves for the second derivative subject to the following conditions:
                            // * the xvec-coordinate of the resulting control point is 1.5 times
                            //   farther than the slope control point xvec-distance
                            // * the second derivative vector is in the xvec-zvec plane
                            // * the second derivative yields the specified (signed) curvature
                            if ( ( angle[ii] < 75 * DEG_2_RAD ) && ( angle[ii] > -60 * DEG_2_RAD ) )
                            {
                                A.row( 0 ) = xvec;
                                A.row( 1 ) = yvec.cross( cp[ii] );
                                A.row( 2 ) = zvec.cross( cp[ii] );
                                rhs( 0 ) = ( udim[ii] - 1 ) * ( alpha - 2 ) * xvec.dot( cp[ii] );
// FIX: MIGHT NEED THIS TO MATCH OLD SKINNING RESULTS SO CHECK
//                              if (i==0)
//                              {
//                                  rhs(0)*=-1;
//                              }
                                rhs( 1 ) = curvature[ii] * tmp;
                                rhs( 2 ) = 0;
                            }
                            // if x there is too little variation in the xvec direction of the
                            // slope, then need change set of equations to:
                            // * the zvec-coordinate of the resulting control point is 1.5 times
                            //   farther than the slope control point zvec-distance
                            // * the second derivative vector is in the xvec-zvec plane
                            // * the second derivative yields the specified (signed) curvature
                            else
                            {
                                A.row( 0 ) = yvec;
                                A.row( 1 ) = yvec.cross( cp[ii] );
                                A.row( 2 ) = zvec;
                                rhs( 0 ) = 0;
                                rhs( 1 ) = curvature[ii] * tmp;
                                rhs( 2 ) = ( udim[ii] - 1 ) * ( alpha - 2 ) * zvec.dot( cp[ii] );
// FIX: MIGHT NEED THIS TO MATCH OLD SKINNING RESULTS SO CHECK
//                              if (i==0)
//                              {
//                                  rhs(2)*=-1;
//                              }
                            }

                            cpp[ii] = A.lu().solve( rhs ).transpose();
                        }
                        fpp_left_pt[j].set_xyz( cpp[0].x(), cpp[0].y(), cpp[0].z() );
                        if ( nit == 2 )
                        {
                            fpp_right_pt[j].set_xyz( cpp[1].x(), cpp[1].y(), cpp[1].z() );
                        }
                        else
                        {
                            fpp_right_pt[j] = fpp_left_pt[j];
                        }
                    }
                }
            }
        }

        // build interpolating curve
        fp_left[i].InterpolateCSpline( fp_left_pt, param, true );
        fp_right[i].InterpolateCSpline( fp_right_pt, param, true );
        fpp_left[i].InterpolateCSpline( fpp_left_pt, param, true );
        fpp_right[i].InterpolateCSpline( fpp_right_pt, param, true );
    }

#if 0
    for ( i = 0; i <= nu; ++i )
    {
        std::cout << "joint #" << i << std::endl;
        for ( j = 0; j < 4; ++j )
        {
            std::cout << "    ";
            switch ( j )
            {
            case( 0 ):
                std::cout << "top side:    ";
                break;
            case( 1 ):
                std::cout << "left side:   ";
                break;
            case( 2 ):
                std::cout << "bottom side: ";
                break;
            case( 3 ):
                std::cout << "right side:  ";
                break;
            }
//          std::cout << "\tfp=[(" << fp_left[i].CompPnt(j) << "), (" << fp_right[i].CompPnt(j) << ")]";
//          std::cout << "\tfpp=[(" << fpp_left[i].CompPnt(j) << "), (" << fpp_right[i].CompPnt(j) << ")]";
            std::cout << "\tfp.degree()=[" << fp_left[i].GetNumSections() << ", " << fp_right[i].GetNumSections() << "]";
            std::cout << "\tfpp.degree()=[" << fpp_left[i].GetNumSections() << ", " << fpp_right[i].GetNumSections() << "]";
            std::cout << std::endl;
        }
    }
#endif

    // cycle through each panel along skinning direction to connect cross sections
    for ( i = 0; i < nu; ++i )
    {
        double v_track( m_Surface.get_v0() );

        // cycle through each panel around cross section to create surface
        for ( j = 0; j < nv; ++j )
        {
            surface_patch_type s;
            surface_index_type udim, vdim;
            curve_segment_type cs;
            vec3d fu_left_side, fu_right_side, fuu_left_side, fuu_right_side;
            double dv;

            // get sizes for current surface patch
            cross_section[i].get( cs, dv, j );
            vdim = cs.degree();
            udim = joint_info[i].DegreesOfFreedom() + joint_info[i + 1].DegreesOfFreedom() - 1;
            s.resize( udim, vdim );

            // set the control points on the surface patch
            for ( surface_index_type jj = 0; jj <= vdim; ++jj )
            {
                vec3d v3d_tmp;
                surface_point_type pt_tmp, c;
                double v;

                // interpolate f_u and f_uu
                v = v_track + dv * jj / static_cast<double>( vdim );
                fu_left_side = fp_right[i].CompPnt( v );
                assert( static_cast<size_t>( i + 1 ) < fp_left.size() );
                fu_right_side = fp_left[i + 1].CompPnt( v );
                fuu_left_side = fpp_right[i].CompPnt( v );
                assert( static_cast<size_t>( i + 1 ) < fpp_left.size() );
                fuu_right_side = fpp_left[i + 1].CompPnt( v );

                // calculate the control points for the left side of this strip of the surface patch
                if ( print_left_side && ( i == i_print ) )
                {
                    std::cout << "new joint " << jj;
                    std::cout << "\tv=" << v;
                    std::cout << "udim=" << udim;
                }
                c = cross_section[i].f( v );
                switch( joint_info[i].DegreesOfFreedom() )
                {
                case( 3 ):
                {
                    v3d_tmp = ( ( fuu_left_side / ( udim - 1 ) ) + fu_left_side * 2.0 ) / udim;
                    pt_tmp << v3d_tmp.x(), v3d_tmp.y(), v3d_tmp.z();
                    s.set_control_point( pt_tmp + c, 2, jj );
                    if ( print_left_side && ( i == i_print ) )
                    {
                        std::cout << "\tcp(2)=" << pt_tmp + c;
                    }
                }
                case( 2 ):
                {
                    v3d_tmp = fu_left_side / udim;
                    pt_tmp << v3d_tmp.x(), v3d_tmp.y(), v3d_tmp.z();
                    s.set_control_point( pt_tmp + c, 1, jj );
                    if ( print_left_side && ( i == i_print ) )
                    {
                        std::cout << "\tcp(1)=" << pt_tmp + c;
                    }
                }
                case( 1 ):
                {
                    s.set_control_point( c, 0, jj );
                    if ( print_left_side && ( i == i_print ) )
                    {
                        std::cout << "\tcp(0)=" << c << std::endl;
                    }
                    break;
                }
                }
                c = cross_section[i + 1].f( v );
                switch( joint_info[i + 1].DegreesOfFreedom() )
                {
                case( 3 ):
                {
                    v3d_tmp = ( ( fuu_right_side / ( udim - 1 ) ) - fu_right_side * 2.0 ) / udim;
                    pt_tmp << v3d_tmp.x(), v3d_tmp.y(), v3d_tmp.z();
                    s.set_control_point( pt_tmp + c, udim - 2, jj );
                }
                case( 2 ):
                {
                    v3d_tmp = fu_right_side / ( -udim );
                    pt_tmp << v3d_tmp.x(), v3d_tmp.y(), v3d_tmp.z();
                    s.set_control_point( pt_tmp + c, udim - 1, jj );
                }
                case( 1 ):
                {
                    s.set_control_point( c, udim, jj );
                    break;
                }
                }
            }

            // set the patch and increment the v-coordinate tracker
            m_Surface.set( s, i, j );
            v_track += dv;
        }
    }

#if 0
    for ( i = 0; i <= nu; ++i )
    {
        std::cout << "joint #" << i << std::endl;
        for ( j = 0; j < 4; ++j )
        {
            std::cout << "    ";
            switch ( j )
            {
            case( 0 ):
                std::cout << "top side:    ";
                break;
            case( 1 ):
                std::cout << "left side:   ";
                break;
            case( 2 ):
                std::cout << "bottom side: ";
                break;
            case( 3 ):
                std::cout << "right side:  ";
                break;
            }
            std::cout << "\tfp=[(" << m_Surface.f_u( std::max( 0.0, i - 1e-8 ), j ) << "), (" << m_Surface.f_u( std::min( static_cast<double>( nu ), i + 1e-8 ), j ) << ")]";
            std::cout << "\tfpp=[(" << m_Surface.f_uu( std::max( 0.0, i - 1e-8 ), j ) << "), (" << m_Surface.f_uu( std::min( static_cast<double>( nu ), i + 1e-8 ), j ) << ")]";
            std::cout << std::endl;
        }
    }
#endif

    // degree reduce patches that don't need to be so high
    DegreeReduceSections( input_crv_vec, closed_flag );

    ResetFlipNormal();
}


//==== Interpolate A Set Of Points =====//
void VspSurf::InterpolateLinear( const vector< VspCurve > &input_crv_vec, bool closed_flag )
{
    vector<piecewise_curve_type> cross_section;

    // error checking
    if ( input_crv_vec.size() < 2 )
    {
        std::cerr << "Too few cross sections to VspSurf::InterpolateLinear()" << std::endl;
        return;
    }

    // get cross sections to be: (1) same number of sections, (2) same (similar) section parameterizations
    // and (3) same degree curves on start and end of each patch
    PrepairCrossSections( cross_section, input_crv_vec );

    // add last cross section that is same as first for closed surfaces
    if ( closed_flag )
    {
        cross_section.push_back( cross_section[0] );
    }

    surface_index_type i, j, nu( cross_section.size() - 1 ), nv( cross_section[0].number_segments() );

    m_Surface.init_uv( nu, nv );

    // connect cross-sections
    for ( i = 0; i < nu; ++i )
    {
        for ( j = 0; j < nv; ++j )
        {
            surface_index_type ii, jj, udim( 1 ), vdim;
            surface_patch_type s;
            curve_segment_type cs[2];

            cross_section[i].get( cs[0], j );
            cross_section[i + 1].get( cs[1], j );

            vdim = cs[0].degree();
            s.resize( udim, vdim );

            for ( jj = 0; jj <= vdim; ++jj )
            {
                ii = 0;
                s.set_control_point( cs[ii].get_control_point( jj ), ii, jj );
                ii = 1;
                s.set_control_point( cs[ii].get_control_point( jj ), ii, jj );
            }

            m_Surface.set( s, i, j );
        }
    }

    // degree reduce patches that don't need to be so high
    DegreeReduceSections( input_crv_vec, closed_flag );

    ResetFlipNormal();
}

void VspSurf::InterpolatePCHIP( const vector< VspCurve > &input_crv_vec, bool closed_flag )
{
    vector<piecewise_curve_type> cross_section;

    // error checking
    if ( input_crv_vec.size() < 3 )
    {
        std::cerr << "Too few cross sections to VspSurf::InterpolatePCHIP()" << std::endl;
        InterpolateLinear( input_crv_vec, closed_flag );
        return;
    }

    if ( closed_flag )
    {
        std::cerr << "Cannot handle closed surfaces yet" << std::endl;
    }

    // get cross sections to be: (1) same number of sections, (2) same (similar) section parameterizations
    // and (3) same degree curves on start and end of each patch
    PrepairCrossSections( cross_section, input_crv_vec );

    surface_index_type i, j, nu( cross_section.size() - 1 ), nv( cross_section[0].number_segments() );
    piecewise_cubic_spline_creator_type pcsc( nu );

    m_Surface.init_uv( nu, nv );

    // connect cross-sections
    for ( j = 0; j < nv; ++j )
    {
        surface_index_type ii, jj, nv_ctrl_pts;
        vector<surface_point_type> pts;
        vector<surface_patch_type> sp( nu );
        curve_segment_type cs;
        piecewise_curve_type pc;

        // resize surface patches
        cross_section[0].get( cs, j );
        nv_ctrl_pts = cs.degree() + 1;
        for ( i = 0; i < nu; ++i )
        {
            sp[i].resize( 3, cs.degree() );
        }

        // fill jth row of patches
        for ( jj = 0; jj < nv_ctrl_pts; ++jj )
        {
            // extract points to fit
            pts.resize( nu + 1 );
            for ( ii = 0; ii <= nu; ++ii )
            {
                cross_section[ii].get( cs, j );
                pts[ii] = cs.get_control_point( jj );
            }

            // perform fit
            if ( closed_flag )
            {
                pcsc.set_chip( pts.begin(), eli::geom::general::C1 );
            }
            else
            {
                pcsc.set_chip( pts.begin(), eli::geom::general::NOT_CONNECTED );
            }
            if ( !pcsc.create( pc ) )
            {
                std::cerr << "Failed to create PCHIP surface. " << __LINE__ << std::endl;
            }

            // extract control points and fill patches
            for ( ii = 0; ii < nu; ++ii )
            {
                pc.get( cs, ii );
                sp[ii].set_control_point( cs.get_control_point( 0 ), 0, jj );
                sp[ii].set_control_point( cs.get_control_point( 1 ), 1, jj );
                sp[ii].set_control_point( cs.get_control_point( 2 ), 2, jj );
                sp[ii].set_control_point( cs.get_control_point( 3 ), 3, jj );
            }
        }

        // put patches into surface
        for ( ii = 0; ii < nu; ++ii )
        {
            m_Surface.set( sp[ii], ii, j );
        }
    }

    // degree reduce patches that don't need to be so high
    DegreeReduceSections( input_crv_vec, closed_flag );

    ResetFlipNormal();
}

void VspSurf::InterpolateCSpline( const vector< VspCurve > &input_crv_vec, bool closed_flag )
{
    vector<piecewise_curve_type> cross_section;

    // error checking
    if ( input_crv_vec.size() < 3 )
    {
        std::cerr << "Too few cross sections to VspSurf::InterpolateCSpline()" << std::endl;
        InterpolateLinear( input_crv_vec, closed_flag );
        return;
    }

    if ( closed_flag )
    {
        std::cerr << "Cannot handle closed surfaces yet" << std::endl;
    }

    // get cross sections to be: (1) same number of sections, (2) same (similar) section parameterizations
    // and (3) same degree curves on start and end of each patch
    PrepairCrossSections( cross_section, input_crv_vec );

    surface_index_type i, j, nu( cross_section.size() - 1 ), nv( cross_section[0].number_segments() );
    piecewise_cubic_spline_creator_type pcsc( nu );

    m_Surface.init_uv( nu, nv );

    // connect cross-sections
    for ( j = 0; j < nv; ++j )
    {
        surface_index_type ii, jj, nv_ctrl_pts;
        vector<surface_point_type> pts;
        vector<surface_patch_type> sp( nu );
        curve_segment_type cs;
        piecewise_curve_type pc;

        // resize surface patches
        cross_section[0].get( cs, j );
        nv_ctrl_pts = cs.degree() + 1;
        for ( i = 0; i < nu; ++i )
        {
            sp[i].resize( 3, cs.degree() );
        }

        // fill jth row of patches
        for ( jj = 0; jj < nv_ctrl_pts; ++jj )
        {
            // extract points to fit
            pts.resize( nu + 1 );
            for ( ii = 0; ii <= nu; ++ii )
            {
                cross_section[ii].get( cs, j );
                pts[ii] = cs.get_control_point( jj );
            }

            // perform fit
            if ( closed_flag )
            {
                pcsc.set_closed_cubic_spline( pts.begin() );
            }
            else
            {
                pcsc.set_cubic_spline( pts.begin() );
            }
            if ( !pcsc.create( pc ) )
            {
                std::cerr << "Failed to create CSpline surface. " << __LINE__ << std::endl;
            }

            // extract control points and fill patches
            for ( ii = 0; ii < nu; ++ii )
            {
                pc.get( cs, ii );
                sp[ii].set_control_point( cs.get_control_point( 0 ), 0, jj );
                sp[ii].set_control_point( cs.get_control_point( 1 ), 1, jj );
                sp[ii].set_control_point( cs.get_control_point( 2 ), 2, jj );
                sp[ii].set_control_point( cs.get_control_point( 3 ), 3, jj );
            }
        }

        // put patches into surface
        for ( ii = 0; ii < nu; ++ii )
        {
            m_Surface.set( sp[ii], ii, j );
        }
    }

    // degree reduce patches that don't need to be so high
    DegreeReduceSections( input_crv_vec, closed_flag );

    ResetFlipNormal();
}

void VspSurf::CompJointParams( int joint, VspJointInfo &jointInfo ) const
{
    int l_seg( joint - 1 ), r_seg( joint ), nsegu( GetNumSectU() );
    double small_ang( 1e-7 ), small_mag( 1e-5 );
    int v[4] = {VspJointInfo::TOP_SIDE, VspJointInfo::LEFT_SIDE, VspJointInfo::BOTTOM_SIDE, VspJointInfo::RIGHT_SIDE};
    bool closed( IsClosedU() );

    if ( joint > nsegu )
    {
        std::cerr << "invalid joint specified in VspSurf " << __LINE__ << std::endl;
        return;
    }

    // if want first joint then either get no data or wrap around
    if ( r_seg == 0 )
    {
        if ( closed )
        {
            l_seg = nsegu;
        }
        else
        {
            l_seg = -1;
            for ( int j = 0; j < 4; ++j )
            {
                jointInfo.SetLeftParams( v[j], 0, 0, 0 );
            }
        }
    }
    // else if want last joint then either get no data or wrap around
    else if ( r_seg == nsegu )
    {
        if ( closed )
        {
            r_seg = 0;
        }
        else
        {
            r_seg = -1;
            for ( int j = 0; j < 4; ++j )
            {
                jointInfo.SetRightParams( v[j], 0, 0, 0 );
            }
        }
    }

    // cycle through 4 sides and get values
    surface_patch_type surf;
    surface_point_type fu, fv, fuu, curv, xvec, yvec, zvec;
    surface_point_type pts[3], fu_approx;
    double du, dv, tmp, ta, ts, tc;
    for ( int j = 0; j < 4; ++j )
    {
        // store the three points needed for finite differences
        if ( l_seg >= 0 )
        {
            // central difference
            if ( r_seg >= 0 )
            {
                pts[0] = m_Surface.f( l_seg, j );
                pts[1] = m_Surface.f( r_seg, j );
                if ( nsegu > 2 )
                {
                    pts[2] = m_Surface.f( r_seg + 1, j );

                    fu_approx = 0.5 * ( pts[2] - pts[0] );
                }
                else
                {
                    fu_approx = pts[1] - pts[0];
                }
            }
            // backwards difference
            else
            {
                pts[1] = m_Surface.f( l_seg, j );
                pts[2] = m_Surface.f( l_seg + 1, j );

                fu_approx = pts[2] - pts[1];
            }
        }
        // forwards difference
        else
        {
            pts[0] = m_Surface.f( r_seg, j );
            pts[1] = m_Surface.f( r_seg + 1, j );

            fu_approx = -pts[0] + pts[1];
        }

        // if have valid left segment
        if ( l_seg >= 0 )
        {
//          std::cout << "side: " << j;

            m_Surface.get( surf, du, dv, l_seg, j );
            fu = surf.f_u( 1, 0 ) / du;
            fv = surf.f_v( 1, 0 ) / dv;
            fuu = surf.f_uu( 1, 0 ) / du / du;
            if ( fv.norm() < 1e-10 )
            {
                switch( v[j] )
                {
                    // top
                case( VspJointInfo::TOP_SIDE ):
                {
                    fv << 0, small_mag, 0;
                    break;
                }
                // right
                case( VspJointInfo::RIGHT_SIDE ):
                {
                    fv << 0, 0, small_mag;
                    break;
                }
                // bottom
                case( VspJointInfo::BOTTOM_SIDE ):
                {
                    fv << 0, -small_mag, 0;
                    break;
                }
                // left
                case( VspJointInfo::LEFT_SIDE ):
                {
                    fv << 0, 0, -small_mag;
                    break;
                }
                default:
                {
                    std::cerr << "should not get here in VspSurf " << __LINE__ << std::endl;
                    break;
                }
                }
            }
            if ( fuu.norm() == 0 )
            {
                switch( v[j] )
                {
                    // top
                case( VspJointInfo::TOP_SIDE ):
                {
                    fuu << 0, 0, -small_mag;
                    break;
                }
                // right
                case( VspJointInfo::RIGHT_SIDE ):
                {
                    fuu << 0, -small_mag, 0;
                    break;
                }
                // bottom
                case( VspJointInfo::BOTTOM_SIDE ):
                {
                    fuu << 0, 0, small_mag;
                    break;
                }
                // left
                case( VspJointInfo::LEFT_SIDE ):
                {
                    fuu << 0, small_mag, 0;
                    break;
                }
                default:
                {
                    std::cerr << "should not get here in VspSurf " << __LINE__ << std::endl;
                    break;
                }
                }
            }
//          std::cout << "\t fu=" << fu << "\tfv=" << fv << "\tfvv=" << fvv;

            // calculate angle
            xvec << 1, 0, 0;
            yvec = fv;
            yvec.normalize();
            zvec = xvec.cross( yvec );
            zvec.normalize();
            ta = std::atan2( zvec.dot( fu ), xvec.dot( fu ) );
//          std::cout << "\tdv=" << dv << "\tfv=" << surf.f_v(1, 0)/dv << "\tz=" << zvec << "\tfu=" << fu << "\tz*fu=" << zvec.dot(fu) << "\tx*fu=" << xvec.dot(fu) << std::endl;
//          std::cout << "\t left angle=" << ta;
            ta = ( std::abs( ta ) < small_ang ) ? 0 : ta;

            // calculate strength
            ts = fu.norm() / fu_approx.norm();
//          std::cout << "\tfu_approx=" << fu_approx << "\tfu=" << fu << "\t left strength=" << ts;

            // calculate curvature
            tmp = fu.norm();
            tmp *= tmp * tmp;
            tc = yvec.dot( fu.cross( fuu ) ) / tmp;
//          std::cout << "\t left curvature=" << tc << std::endl;

//          if (j==0)
//          {
//              std::cout << "GET:  ";
//              std::cout << "\txvec=" << xvec << "\tyvec=" << yvec << "\tzvec=" << zvec;
//              std::cout << "\tf=" << surf.f(0,1) << "\tfp=" << fu << "\tfpp=" << fuu;
//              std::cout << "\t|fp|^3=" << tmp << "\tfp x fpp=" << fu.cross(fuu);
//              std::cout << "\tfp_approx=" << fu_approx << "\tpt=" << pts[1] << "\tptm1=" << pts[0];
//              std::cout << "\tangle=" << ta << "\tstrength=" << ts << "\tcurvature=" << tc;
//              std::cout << std::endl;
//          }

            jointInfo.SetLeftParams( v[j], ta, ts, tc );
        }

        // if have valid right segment
        if ( r_seg >= 0 )
        {
//          std::cout << "side: " << v[j];

            m_Surface.get( surf, du, dv, r_seg, j );
            fu = surf.f_u( 0, 0 ) / du;
            fv = surf.f_v( 0, 0 ) / dv;
            fuu = surf.f_uu( 0, 0 ) / du / du;
            if ( fv.norm() < 1e-10 )
            {
                switch( v[j] )
                {
                    // top
                case( VspJointInfo::TOP_SIDE ):
                {
                    fv << 0, small_mag, 0;
                    break;
                }
                // right
                case( VspJointInfo::RIGHT_SIDE ):
                {
                    fv << 0, 0, small_mag;
                    break;
                }
                // bottom
                case( VspJointInfo::BOTTOM_SIDE ):
                {
                    fv << 0, -small_mag, 0;
                    break;
                }
                // left
                case( VspJointInfo::LEFT_SIDE ):
                {
                    fv << 0, 0, -small_mag;
                    break;
                }
                default:
                {
                    std::cerr << "should not get here in VspSurf " << __LINE__ << std::endl;
                    break;
                }
                }
            }
            if ( fuu.norm() == 0 )
            {
                switch( v[j] )
                {
                    // top
                case( VspJointInfo::TOP_SIDE ):
                {
                    fuu << 0, 0, -small_mag;
                    break;
                }
                // right
                case( VspJointInfo::RIGHT_SIDE ):
                {
                    fuu << 0, small_mag, 0;
                    break;
                }
                // bottom
                case( VspJointInfo::BOTTOM_SIDE ):
                {
                    fuu << 0, 0, small_mag;
                    break;
                }
                // left
                case( VspJointInfo::LEFT_SIDE ):
                {
                    fuu << 0, -small_mag, 0;
                    break;
                }
                default:
                {
                    std::cerr << "should not get here in VspSurf " << __LINE__ << std::endl;
                    break;
                }
                }
            }
//          std::cout << "\t fu=" << fu << "\tfv=" << fv << "\tfuu=" << fuu;

            // calculate angle
            xvec << 1, 0, 0;
            yvec = fv;
            yvec.normalize();
            zvec = xvec.cross( yvec );
            zvec.normalize();
            ta = std::atan2( zvec.dot( fu ), xvec.dot( fu ) );
//          std::cout << "\tright angle=" << ta;
            ta = ( std::abs( ta ) < small_ang ) ? 0 : ta;

            // calculate strength
            ts = fu.norm() / fu_approx.norm();
//          std::cout << "\tfu_approx=" << fu_approx << "\tfu=" << fu << "\tright strength=" << ts;

            // calculate curvature
            tmp = fu.norm();
            tmp *= tmp * tmp;
            tc = yvec.dot( fu.cross( fuu ) ) / tmp;
//          std::cout << "\tright curvature=" << tc << std::endl;

//          if (j==0)
//          {
//              std::cout << "GET:  ";
//              switch(v[j])
//              {
//                  case(0):
//                      std::cout << "Top Side:";
//                      break;
//                  case(1):
//                      std::cout << "Right Side:";
//                      break;
//                  case(2):
//                      std::cout << "Bottom Side:";
//                      break;
//                  case(3):
//                      std::cout << "Left Side:";
//                      break;
//              }
//              std::cout << "\txvec=" << xvec << "\tyvec=" << yvec << "\tzvec=" << zvec;
//              std::cout << "\tf=" << surf.f(0,0) << "\tfp=" << fu << "\tfpp=" << fuu;
//              std::cout << "\t|fp|^3=" << tmp << "\tfp x fpp=" << fv.cross(fuu);
//              std::cout << "\tfp_approx=" << fu_approx << "\tpt=" << pts[1] << "\tptm1=" << pts[0];
//              std::cout << "\tangle=" << ta << "\tstrength=" << ts << "\tcurvature=" << tc;
//              std::cout << std::endl;
//          }

            jointInfo.SetRightParams( v[j], ta, ts, tc );
        }
    }

//  std::cout << std::endl << std::endl;
}


//===== Compute Point On Surf Given  U V (Between 0 1 ) =====//
vec3d VspSurf::CompPnt01( double u, double v ) const
{
    return CompPnt( u * ( double )GetNumSectU(), v * ( double )GetNumSectW() );
}

//===== Compute Tangent In U Direction   =====//
vec3d VspSurf::CompTanU01( double u01, double v01 ) const
{
    return CompTanU( u01 * ( double )GetNumSectU(), v01 * ( double )GetNumSectW() );
}

//===== Compute Tangent In W Direction   =====//
vec3d VspSurf::CompTanW01( double u01, double v01 ) const
{
    return CompTanW( u01 * ( double )GetNumSectU(), v01 * ( double )GetNumSectW() );
}

//===== Compute Second Derivative U,U   =====//
vec3d VspSurf::CompTanUU01( double u01, double v01 ) const
{
    return CompTanUU( u01 * ( double )GetNumSectU(), v01 * ( double )GetNumSectW() );
}

//===== Compute Second Derivative W,W   =====//
vec3d VspSurf::CompTanWW01( double u01, double v01 ) const
{
    return CompTanWW( u01 * ( double )GetNumSectU(), v01 * ( double )GetNumSectW() );
}

//===== Compute Second Derivative U,W   =====//
vec3d VspSurf::CompTanUW01( double u01, double v01 ) const
{
    return CompTanUW( u01 * ( double )GetNumSectU(), v01 * ( double )GetNumSectW() );
}

//===== Compute Second Derivative U,U   =====//
vec3d VspSurf::CompTanUU( double u, double v ) const
{
    vec3d rtn;
    surface_point_type p( m_Surface.f_uu( u, v ) );

    rtn.set_xyz( p.x(), p.y(), p.z() );
    return rtn;
}

//===== Compute Second Derivative W,W   =====//
vec3d VspSurf::CompTanWW( double u, double v ) const
{
    vec3d rtn;
    surface_point_type p( m_Surface.f_vv( u, v ) );

    rtn.set_xyz( p.x(), p.y(), p.z() );
    return rtn;
}

//===== Compute Second Derivative U,W   =====//
vec3d VspSurf::CompTanUW( double u, double v ) const
{
    vec3d rtn;
    surface_point_type p( m_Surface.f_uv( u, v ) );

    rtn.set_xyz( p.x(), p.y(), p.z() );
    return rtn;
}

//===== Compute Tangent In U Direction   =====//
vec3d VspSurf::CompTanU( double u, double v ) const
{
    vec3d rtn;
    surface_point_type p( m_Surface.f_u( u, v ) );

    rtn.set_xyz( p.x(), p.y(), p.z() );
    return rtn;
}

//===== Compute Tangent In W Direction   =====//
vec3d VspSurf::CompTanW( double u, double v ) const
{
    vec3d rtn;
    surface_point_type p( m_Surface.f_v( u, v ) );

    rtn.set_xyz( p.x(), p.y(), p.z() );
    return rtn;
}

//===== Compute Point On Surf Given  U W =====//
vec3d VspSurf::CompPnt( double u, double v ) const
{
    vec3d rtn;
    surface_point_type p( m_Surface.f( u, v ) );

    rtn.set_xyz( p.x(), p.y(), p.z() );
    return rtn;
}

//===== Compute Normal  =====//
vec3d VspSurf::CompNorm( double u, double v ) const
{
    vec3d rtn;
    surface_point_type p( m_Surface.normal( u, v ) );

    if ( m_FlipNormal )
    {
        p = -p;
    }

    rtn.set_xyz( p.x(), p.y(), p.z() );
    return rtn;
}

//===== Compute Normal  0->1.0  =====//
vec3d VspSurf::CompNorm01( double u01, double v01 ) const
{
    return CompNorm( u01 * ( double )GetNumSectU(), v01 * ( double )GetNumSectW() );
}

//==== Tesselate Surface ====//
void VspSurf::Tesselate( int num_u, int num_v, vector< vector< vec3d > > & pnts, vector< vector< vec3d > > & norms ) const
{
    std::vector< vector< vec3d > > uw_pnts;
    Tesselate( num_u, num_v, pnts, norms, uw_pnts );
}

void VspSurf::Tesselate( int num_u, int num_v, vector< vector< vec3d > > & pnts, vector< vector< vec3d > > & norms, vector< vector< vec3d > > & uw_pnts ) const
{
    if( m_Surface.number_u_patches() == 0 || m_Surface.number_v_patches() == 0 )
    {
        return;
    }

    surface_index_type i, j, nu( num_u ), nv( num_v );
    double umin, umax, vmin, vmax;
    std::vector<double> u( nu ), v( nv );
    surface_point_type ptmp, ntmp;

    // resize pnts and norms
    pnts.resize( nu );
    norms.resize( nu );
    uw_pnts.resize( nu );
    for ( i = 0; i < nu; ++i )
    {
        pnts[i].resize( nv );
        norms[i].resize( nv );
        uw_pnts[i].resize( nv );
    }

    // calculate the u and v parameterizations
    m_Surface.get_parameter_min( umin, vmin );
    m_Surface.get_parameter_max( umax, vmax );
    for ( i = 0; i < nu; ++i )
    {
        u[i] = umin + ( umax - umin ) * static_cast<double>( i ) / ( nu - 1 );
    }
    for ( j = 0; j < nv; ++j )
    {
        v[j] = vmin + ( vmax - vmin ) * static_cast<double>( j ) / ( nv - 1 );
    }

    // calculate the coordinate and normal at each point
    for ( surface_index_type i = 0; i < nu; ++i )
    {
        for ( surface_index_type j = 0; j < nv; ++j )
        {
            ptmp = m_Surface.f( u[i], v[j] );
            ntmp = m_Surface.normal( u[i], v[j] );
            pnts[i][j].set_xyz( ptmp.x(), ptmp.y(), ptmp.z() );

            if ( m_FlipNormal )
            {
                ntmp = -ntmp;
            }
            norms[i][j].set_xyz( ntmp.x(), ntmp.y(), ntmp.z() );
            uw_pnts[i][j].set_xyz( u[i], v[j], 0.0 );
        }
    }
}

void VspSurf::Tesselate( const vector<int> &num_u, int num_v, std::vector< vector< vec3d > > & pnts,  std::vector< vector< vec3d > > & norms ) const
{
    std::vector< vector< vec3d > > uw_pnts;
    Tesselate( num_u, num_v, pnts, norms, uw_pnts );
}

void VspSurf::Tesselate( const vector<int> &num_u, int num_v, std::vector< vector< vec3d > > & pnts,  std::vector< vector< vec3d > > & norms,  std::vector< vector< vec3d > > & uw_pnts ) const
{
    surface_index_type i, j, nu, nv( num_v );
    double umin, umax, vmin, vmax;
    std::vector<double> u, v( nv );
    surface_point_type ptmp, ntmp;

    // calculate nu
    nu = 1;
    for ( int ii = 0; ii < GetNumSectU(); ++ii )
    {
        nu += num_u[ii] - 1;
    }

    // resize pnts and norms
    pnts.resize( nu );
    norms.resize( nu );
    uw_pnts.resize( nu );
    for ( i = 0; i < nu; ++i )
    {
        pnts[i].resize( nv );
        norms[i].resize( nv );
        uw_pnts[i].resize( nv );
    }

    // calculate the u and v parameterizations
    m_Surface.get_parameter_min( umin, vmin );
    m_Surface.get_parameter_max( umax, vmax );
    for ( j = 0; j < nv; ++j )
    {
        v[j] = vmin + ( vmax - vmin ) * static_cast<double>( j ) / ( nv - 1 );
    }

    u.resize( nu );
    double uumin( umin );
    size_t ii;
    for ( i = 0, ii = 0; ii < (size_t)GetNumSectU(); ++ii )
    {
        double du, dv;

        surface_patch_type surf;
        m_Surface.get( surf, du, dv, ii, 0 );
        for ( int iii = 0; iii < num_u[ii] - 1; ++iii, ++i )
        {
            u[i] = uumin + du * static_cast<double>( iii ) / ( num_u[ii] - 1 );
        }
        uumin += du;
    }
    u.back() = uumin;

    // calculate the coordinate and normal at each point
    for ( surface_index_type i = 0; i < nu; ++i )
    {
        for ( surface_index_type j = 0; j < nv; ++j )
        {
            ptmp = m_Surface.f( u[i], v[j] );
            ntmp = m_Surface.normal( u[i], v[j] );
            pnts[i][j].set_xyz( ptmp.x(), ptmp.y(), ptmp.z() );
            if ( m_FlipNormal )
            {
                ntmp = -ntmp;
            }
            norms[i][j].set_xyz( ntmp.x(), ntmp.y(), ntmp.z() );
            uw_pnts[i][j].set_xyz( u[i], v[j], 0.0 );
        }
    }
}

struct joint_pred
{
    double ref_val;
    double tol;
    bool operator()( double a ) const
    {
        return ( std::abs( a - ref_val ) < tol );
    }
};

void VspSurf::PrepairCrossSections( vector<piecewise_curve_type> &pc, const vector<VspCurve> &crv_in ) const
{
    size_t i, nc( crv_in.size() );
    surface_index_type j, ns, seg_cnt( 0 );
    vector<surface_index_type> deg_max;
    curve_segment_type cs;

    pc.resize( nc );

    // cycle through each curve to find all joints params
    typedef std::set<double> joint_param_collection;
    joint_param_collection joint_params;
    joint_param_collection::iterator it;
    joint_pred pred;
    double t, dt;

    pred.tol = 1e-4;

    // copy input curves to output vector and calculate the needed joint parameters
    joint_params.insert( 0 );
    for ( i = 0; i < nc; ++i )
    {
        curve_segment_type cst;
        pc[i] = crv_in[i].GetCurve();
        ns = pc[i].number_segments();
        for ( j = 0, t = 0; j < ns; ++j )
        {
            // calculate next parameter
            pc[i].get( cst, dt, j );
            t += dt;

            // catch case where segment is just small
            if ( dt < pred.tol )
            {
                joint_params.insert( t );
            }
            // otherwise see if it should be added
            else
            {
                pred.ref_val = t;
                it = std::find_if( joint_params.begin(), joint_params.end(), pred );
                if ( it == joint_params.end() )
                {
                    joint_params.insert( t );
                }
            }
        }
    }

    // debug printout
#if 0
    std::cout << "xsec joints:";
    for ( it = joint_params.begin(); it != joint_params.end(); ++it )
    {
        std::cout << "\t" << *it;
    }
    std::cout << std::endl;
#endif

    // cycle through each joint and make all curves have same number of segments and parameterization
    it = joint_params.begin();
    if ( *it != 0 )
    {
        std::cerr << "Invalid start parameter for joints " << __LINE__ << std::endl;
        assert( false );
        return;
    }
    for ( ++it, j = 0, t = 0; it != joint_params.end(); ++it, ++j )
    {
        curve_segment_type cst;
        piecewise_curve_type::error_code ec;

        for ( i = 0; i < nc; ++i )
        {
            pc[i].get( cs, dt, j );
            pred.ref_val = t + dt;
#if 0
            std::cout << "pred: " << pred.ref_val << "\t" << ( *it ) << std::endl;
#endif
            if ( !pred( *it ) )
            {
                ec = pc[i].split( ( *it ) );
                assert( ec == piecewise_curve_type::NO_ERRORS );
            }
        }
        t = ( *it );
    }

    // debug printout
#if 0
    for ( i = 0; i < nc; ++i )
    {
        curve_segment_type cst;
        std::cout << "curve #" << i << " joints:";

        ns = pc[i].number_segments();
        for ( j = 0, t = 0; j < ns; ++j )
        {
            pc[i].get( cs, dt, j );
            t += dt;
            std::cout << "\t" << t;
        }
        std::cout << std::endl;
    }
#endif

    // cycle through each curve to find maximum number of segments
    ns = joint_params.size() - 1;
    for ( i = 0; i < nc; ++i )
    {
        surface_index_type seg_cnt( pc[i].number_segments() );
        if ( ns != seg_cnt )
        {
            std::cerr << "XSec #" << i << " should have " << ns << " segments but have " << seg_cnt << " " << __LINE__ << std::endl;
            assert( false );
            return;
        }
    }
    deg_max.resize( ns, 0 );

    // cycle through curves to record max degree for each segment
    for ( i = 0; i < nc; ++i )
    {
        for ( j = 0; j < ns; ++j )
        {
            pc[i].get( cs, j );
            if ( deg_max[j] < cs.degree() )
            {
                deg_max[j] = cs.degree();
            }
        }
    }

    // make sure have same degree along strip
    for ( i = 0; i < pc.size(); ++i )
    {
        for ( j = 0; j < ns; ++j )
        {
            pc[i].get( cs, j );

            if ( cs.degree() < deg_max[j] )
            {
                for ( surface_index_type jj = cs.degree() + 1; jj <= deg_max[j]; ++jj )
                {
                    cs.degree_promote();
                }
                pc[i].replace( cs, j );
            }
        }
    }

    // debug printout
#if 0
    for ( i = 0; i < nc; ++i )
    {
        std::cout << "curve #" << i << " joints:";

        ns = pc[i].number_segments();
        for ( j = 0, t = 0; j < ns; ++j )
        {
            pc[i].get( cs, dt, j );
            t += dt;
            std::cout << "\t|" << t << ", " << cs.degree() << "|";
        }
        std::cout << std::endl;
    }
#endif
}

void VspSurf::DegreeReduceSections( const vector<VspCurve> &input_crv_vec, bool closed_flag )
{
//    std::cerr << "Need to re-enable DegreeReduceSections()" << std::endl;
#if 0
    surface_index_type i, j, nu( m_Surface.number_u_patches() ), nv( m_Surface.number_v_patches() );

    for ( i = 0; i < nu; ++i )
    {
        for ( j = 0; j < nv; ++j )
        {
            surface_index_type vdim;
            surface_patch_type sp;
            curve_segment_type clow, chigh;

            input_crv_vec[i].GetCurveSegment( clow, j );
            if ( closed_flag && ( i == nu - 1 ) )
            {
                input_crv_vec[0].GetCurveSegment( chigh, j );
            }
            else
            {
                input_crv_vec[i + 1].GetCurveSegment( chigh, j );
            }
            vdim = std::max( clow.degree(), chigh.degree() );

            m_Surface.get( sp, i, j );

            if ( sp.degree_v() > vdim )
            {
                for ( size_t jj = sp.degree_v() - 1; jj >= vdim; --jj )
                {
                    sp.demote_v( eli::geom::general::C0 );
                }
            }
        }
    }
#endif
}

void VspSurf::WriteBezFile( FILE* file_id, const std::string &geom_id, int surf_ind )
{
    // Make copy for local changes.
    piecewise_surface_type s( m_Surface );

    piecewise_surface_type::data_type ttol = 1e-6;
    s.to_cubic( ttol );

    vector<double> u_pmap;
    vector<double> w_pmap;
    s.get_pmap_uv( u_pmap, w_pmap );

    piecewise_surface_type::index_type ip, jp, nupatch, nvpatch;

    nupatch = s.number_u_patches();
    nvpatch = s.number_v_patches();

    int nupts = nupatch * 3 + 1;
    int nvpts = nvpatch * 3 + 1;

    vector< vector< surface_patch_type::point_type> > pts;
    pts.resize( nupts );
    for( int i = 0; i < nupts; ++i )
    {
        pts[i].resize( nvpts );
    }

    for( ip = 0; ip < nupatch; ++ip )
    {
        for( jp = 0; jp < nvpatch; ++jp )
        {
            surface_patch_type::index_type icp, jcp;

            surface_patch_type *patch = s.get_patch( ip, jp );

            for( icp = 0; icp <= 3; ++icp )
                for( jcp = 0; jcp <= 3; ++jcp )
                {
                    pts[ ip * 3 + icp ][ jp * 3 + jcp ] = patch->get_control_point( icp, jcp );
                }
        }
    }

    vector<int> split_u, split_w;
    split_u.push_back( 0 );
    split_w.push_back( 0 );

    split_w.push_back( ClosetPatchEnd( w_pmap, 0.25 * w_pmap[w_pmap.size() - 1] ) );
    split_w.push_back( ClosetPatchEnd( w_pmap, 0.5 * w_pmap[w_pmap.size() - 1] ) );
    split_w.push_back( ClosetPatchEnd( w_pmap, 0.75 * w_pmap[w_pmap.size() - 1] ) );

    split_u.push_back( nupts - 1 );
    split_w.push_back( nvpts - 1 );

    int num_sections = ( split_u.size() - 1 ) * ( split_w.size() - 1 );

    fprintf( file_id, "%s Component\n", geom_id.c_str() );
    fprintf( file_id, "%d  Num_Sections\n", num_sections );

    for ( int iu = 0 ; iu < ( int )split_u.size() - 1 ; iu++ )
    {
        for ( int iw = 0 ; iw < ( int )split_w.size() - 1 ; iw++ )
        {
            //==== Write Section ====//
            int num_u = split_u[iu + 1] - split_u[iu] + 1;
            int num_w = split_w[iw + 1] - split_w[iw] + 1;
            int num_u_map = split_u[iu + 1] / 3 - split_u[iu] / 3 + 1;
            int num_w_map = split_w[iw + 1] / 3 - split_w[iw] / 3 + 1;
            fprintf ( file_id, "%d %d  NumU, NumW\n", num_u, num_w );
            fprintf( file_id, "%d %d NumU_Map, NumW_Map\n", num_u_map, num_w_map );
            fprintf( file_id, "%d VspSurf_Index\n", surf_ind );
            //==== Write U,W Mapping ====//
            for ( int umi = split_u[iu] / 3; umi <= split_u[iu + 1] / 3; umi++ )
            {
                fprintf( file_id, "%20.20lf\n", u_pmap[umi] );
            }

            for ( int wmi = split_w[iw] / 3; wmi <= split_w[iw + 1] / 3; wmi++ )
            {
                fprintf( file_id, "%20.20lf\n", w_pmap[wmi] );
            }

            for ( int i = split_u[iu] ; i <= split_u[iu + 1] ; i++ )
                for ( int j = split_w[iw] ; j <= split_w[iw + 1] ; j++ )
                {
                    surface_patch_type::point_type p = pts[i][j];

                    fprintf( file_id, "%20.20lf %20.20lf %20.20lf\n", p.x(), p.y(), p.z() );
                }
        }
    }
}

int VspSurf::ClosetPatchEnd( const vector<double> & patch_endings, double end_val ) const
{
    // patch_endings should be a sorted vector
    int ind = ClosestElement( patch_endings, end_val );

    return ind * 3;

}
