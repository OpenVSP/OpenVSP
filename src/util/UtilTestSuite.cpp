//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "UtilTestSuite.h"

#include <float.h>
#include "StringUtil.h"
#include "StlHelper.h"


//==== Test vec2d ====//
void UtilTestSuite::Vec2dUtilTest()
{
    //==== Test Vec2 Construction/Copy ====//
    vec2d v1 = vec2d();
    vec2d v2 = vec2d( 1.0, 2.0 );
    vec2d v3( v2 );
    vec2d* v4 = new vec2d;
    ( *v4 ) = v1;

    vec2d total = v1 + v2 - v3 + ( *v4 );       // Arithmetic
    delete v4;
    double mag = total.mag();
    TEST_ASSERT_DELTA( mag, 0.0, DBL_EPSILON );

    v1 = vec2d( 1.0, 2.0 );
    v2 = vec2d( 4.0, 6.0 );
    TEST_ASSERT_DELTA( dist( v1, v2 ), 5.0, DBL_EPSILON );          // Dist Between Pnts
    TEST_ASSERT_DELTA( dot( v1, v2 ), 16.0, DBL_EPSILON );          // Dot Product

    v1 = vec2d( 1.0, 0.0 );
    v2 = vec2d( 1.0, 1.0 );
    TEST_ASSERT_DELTA( angle( v1, v2 ), M_PI / 4.0, DBL_EPSILON );          // Angle
    TEST_ASSERT_DELTA( cos_angle( v1, v2 ), cos( M_PI / 4.0 ), DBL_EPSILON ); // Cos Angle

    v2 = vec2d( -1.0, 1.0 );
    TEST_ASSERT_DELTA( angle( v1, v2 ), 3.0 * M_PI / 4.0, DBL_EPSILON );
    TEST_ASSERT_DELTA( cos_angle( v1, v2 ), cos( 3.0 * M_PI / 4.0 ), DBL_EPSILON );

    v2 = vec2d( -1.0, -1.0 );
    TEST_ASSERT_DELTA( angle( v1, v2 ), 3.0 * M_PI / 4.0, DBL_EPSILON );    // Not signed angle

    vec2d A = vec2d( 0, 0 );
    vec2d B = vec2d( 1, 1 );
    vec2d C = vec2d( 0, 1 );
    vec2d D = vec2d( 1, 0 );
    vec2d intPnt;
    int intFlag = seg_seg_intersect( A, B, C, D, intPnt );              // Segment Intersection
    TEST_ASSERT( intFlag );
    TEST_ASSERT_DELTA( dist( intPnt, vec2d( 0.5, 0.5 ) ), 0.0, DBL_EPSILON );

    intFlag = seg_seg_intersect( A, vec2d( 0.4999, 0.4999 ), C, D, intPnt );                    // Segment Intersection
    TEST_ASSERT( intFlag == 0 );

    vec2d proj = proj_pnt_on_line_seg( A, B, vec2d( 1, 0 ) );           // Project on Line Seg
    TEST_ASSERT_DELTA( dist( proj, vec2d( 0.5, 0.5 ) ), 0.0, DBL_EPSILON );
}

//==== Test MessageMgr ====//
void UtilTestSuite::MessageMgrTest()
{
    MessageBaseTest mbt1;
    mbt1.SetName( "Base1" );    // An Object to Receive Messages
    MessageBaseTest mbt2;
    mbt2.SetName( "Base2" );
    MessageBaseTest mbt3;
    mbt3.SetName( "Base3" );
    MessageBaseTest mbt3A;
    mbt3A.SetName( "Base3" );
    MessageBaseTest* mbt3New = new MessageBaseTest();
    mbt3New->SetName( "Base3" );

    MessageMgr::getInstance().Register( &mbt1 );
    MessageMgr::getInstance().Register( &mbt2 );
    MessageMgr::getInstance().Register( &mbt3 );
    MessageMgr::getInstance().Register( &mbt3A );
    MessageMgr::getInstance().Register( mbt3New );

    // Use convenience methods to set up listeners
    MessageBaseTest mbt4;
    mbt4.SetName( "Base4" );
    mbt4.Register();

    MessageBaseTest mbt5;
    mbt5.Register( "Base5" );

    //==== Send A String Message To Base1 ====//
    MessageMgr::getInstance().Send( "Base1", "Sent_Message1" );
    TEST_ASSERT( mbt1.m_Data.m_String.compare( "Sent_Message1" ) == 0 );

    //==== Send A Data Message To Base2 ====//
    MessageData data;
    data.m_IntVec.push_back( 23 );
    data.m_IntVec.push_back( 2323 );
    MessageMgr::getInstance().Send( "Base2", data );
    TEST_ASSERT( mbt2.m_Data.m_IntVec[0] == 23  );
    TEST_ASSERT( mbt2.m_Data.m_IntVec[1] == 2323  );

    //==== Send A String Message To All Bases ====//
    MessageMgr::getInstance().SendAll( "Sent_Message_All" );
    TEST_ASSERT( mbt1.m_Data.m_String.compare( "Sent_Message_All" ) == 0 );
    TEST_ASSERT( mbt2.m_Data.m_String.compare( "Sent_Message_All" ) == 0 );
    TEST_ASSERT( mbt3.m_Data.m_String.compare( "Sent_Message_All" ) == 0 );
    TEST_ASSERT( mbt3A.m_Data.m_String.compare( "Sent_Message_All" ) == 0 );
    TEST_ASSERT( mbt3New->m_Data.m_String.compare( "Sent_Message_All" ) == 0 );
    TEST_ASSERT( mbt4.m_Data.m_String.compare( "Sent_Message_All" ) == 0 );
    TEST_ASSERT( mbt5.m_Data.m_String.compare( "Sent_Message_All" ) == 0 );

    //==== Test Removal ====//
    delete mbt3New;                             // Should UnRegister From MessageMgr
    MessageMgr::getInstance().UnRegister( &mbt3A );
    mbt4.UnRegister();
    MessageMgr::getInstance().SendAll( "After_Removal" );
    TEST_ASSERT( mbt1.m_Data.m_String.compare( "After_Removal" ) == 0 );
    TEST_ASSERT( mbt3A.m_Data.m_String.compare( "Sent_Message_All" ) == 0 );
    TEST_ASSERT( mbt4.m_Data.m_String.compare( "Sent_Message_All" ) == 0 );

    //==== Re-register and Send A String Message To Base4 ====//
    mbt4.Register();
    MessageMgr::getInstance().Send( "Base4", "Sent_Message4" );
    TEST_ASSERT( mbt4.m_Data.m_String.compare( "Sent_Message4" ) == 0 );

    //==== Send A String Message To Base5.  Base5 was simultaneously named & registered ====//
    MessageMgr::getInstance().Send( "Base5", "Sent_Message5" );
    TEST_ASSERT( mbt5.m_Data.m_String.compare( "Sent_Message5" ) == 0 );
}

//==== Test String Utilities =====//
void UtilTestSuite::StringUtilTest()
{
    //==== Example of STL String Class ====//
//  http://www.cplusplus.com/reference/string/string/
    string A( "Abcdefg" );
    string B( A );
    string C( A, 2, 4 );

    string D = A + B;
    string E;
    E.assign( "c-string" );             // Set String
    E.clear();
    E.append( C );
    E.insert( 2, "Insert" );            // 2 - Pos
    E.replace( 4, 3, "XXXXXXX" );       // 4 - Pos 3 - Num char to replace

    char ch = E.at( 2 );                // Character access
    ch = E[3];

    char* cstr = new char [E.size() + 1];
    strcpy( cstr, E.c_str() );          // Get Pointer to Internal Array
    delete [] cstr;

//  char buffer[255];
//  int len = E.copy(buffer,6,5);                   // Copy 6 - num char to copy, 5 - pos

    size_t found = E.find( "XXX" );         // Also rfind, find_first_of, find_last_of...
    if ( found != E.npos )
    {
        E.replace( found, 3, "YYY" );
    }

    string sub = E.substr( 2, 6 );          // Extract Substring

    if ( E.compare( sub ) != 0 )            // Not Equal Strings
    {
        sub = E;
    }

    //==== String Utils ====//
    string str( "What_Up_This_Is_A_Test" );
    StringUtil::change_from_to( str, '_', ' ' );
    TEST_ASSERT( str.compare( "What Up This Is A Test" ) == 0 );
    StringUtil::chance_space_to_underscore( str );
    StringUtil::chance_underscore_to_space( str );
    TEST_ASSERT( str.compare( "What Up This Is A Test" ) == 0 );

    str.assign( "    Leading_Trailing_Spaces      " );
    StringUtil::remove_leading( str, ' ' );
    TEST_ASSERT( str.compare( "Leading_Trailing_Spaces      " ) == 0 );
    StringUtil::remove_trailing( str, ' ' );
    TEST_ASSERT( str.compare( "Leading_Trailing_Spaces" ) == 0 );

}

//==== Test StlHelper =====//
void UtilTestSuite::StlHelperTest()
{
    deque< string > str_deque;
    str_deque.push_back( "ABCD" );
    str_deque.push_back( "BCBC" );
    str_deque.push_back( "ABCE" );
    str_deque.push_back( "ABCD" );
    str_deque.push_back( "BBBB" );
    str_deque.push_back( "ABCE" );

    TEST_ASSERT( deque_contains_val < string >( str_deque, string( "ABCE" ) ) );

    deque_remove_val < string >( str_deque, string( "ABCE" ) );

    TEST_ASSERT( str_deque.size() == 4 );
    TEST_ASSERT( str_deque[0] == "ABCD" && str_deque[1] == "BCBC" );
    TEST_ASSERT( str_deque[2] == "ABCD" && str_deque[3] == "BBBB" );

    vector< string > str_vector;
    str_vector.push_back( "ABCD" );
    str_vector.push_back( "BCBC" );
    str_vector.push_back( "ABCE" );
    str_vector.push_back( "ABCD" );
    str_vector.push_back( "BBBB" );
    str_vector.push_back( "ABCE" );

    TEST_ASSERT( vector_contains_val < string >( str_vector, string( "ABCE" ) ) );

    vector_remove_val < string >( str_vector, string( "ABCE" ) );

    TEST_ASSERT( str_vector.size() == 4 );
    TEST_ASSERT( str_vector[0] == "ABCD" && str_vector[1] == "BCBC" );
    TEST_ASSERT( str_vector[2] == "ABCD" && str_vector[3] == "BBBB" );
}

//==== Test VspCurve =====//
void UtilTestSuite::VspCurveTest()
{
#if 0
    VspCurve crv;

    vector< vec3d > pnt_vec;
    pnt_vec.push_back( vec3d( 0, 0, 0 ) );
    pnt_vec.push_back( vec3d( 1, 0, 0 ) );
    pnt_vec.push_back( vec3d( 1, 1, 0 ) );
    pnt_vec.push_back( vec3d( 0, 1, 0 ) );
    pnt_vec.push_back( vec3d( 0, 0, 0 ) );
    crv.Interpolate( pnt_vec, true );
    WriteCurve( crv, "crv1.dat" );

    pnt_vec.resize( 8 );
    pnt_vec[0] = vec3d( 0, 0, 0 );
    pnt_vec[1] = vec3d( 1, 0, 0 );
    pnt_vec[2] = vec3d( 1, 1, 0 );
    pnt_vec[3] = vec3d( 1.5, 0, 0 );
    pnt_vec[4] = vec3d( 1.5, 1, 0 );
    pnt_vec[5] = vec3d( 1.8, 1.1, 0 );
    pnt_vec[6] = vec3d( 2.0, 0.5, 0 );
    pnt_vec[7] = vec3d( 3, 0, 0 );

    vector< VspPntData > data_vec( 8 );
    data_vec[0].SetType( VspPntData::NORMAL );
    data_vec[1].SetType( VspPntData::NORMAL );
    data_vec[2].SetType( VspPntData::NORMAL );
    data_vec[3].SetType( VspPntData::NORMAL );
    data_vec[4].SetType( VspPntData::NORMAL );
    data_vec[5].SetType( VspPntData::NORMAL );
    data_vec[6].SetType( VspPntData::NORMAL );
    data_vec[7].SetType( VspPntData::NORMAL );

    crv.Interpolate( pnt_vec, data_vec, false );
    WriteCurve( crv, "crv_normal.dat" );

    //==== Test Linear Interpolate And Comp Length ====//
    pnt_vec.resize( 3 );
    pnt_vec[0] = vec3d( 0, 0, 0 );
    pnt_vec[1] = vec3d( 1, 0, 0 );
    pnt_vec[2] = vec3d( 1, 1, 0 );
    crv.LinearInterpolate( pnt_vec );
    WriteCurve( crv, "linearcrv.dat" );
    double len = crv.CompLength( 100 );

    TEST_ASSERT_DELTA( len, 2.0, 0.00001 );


    //==== Test Super Ellipse and Uniform Interpolation ====//
    SuperEllipse se;

    vector< vec3d > se_sparse = se.Build( 41, 1.0, 1.0, 8, 1 );
    vector< vec3d > se_dense = se.Build( 201, 1.0, 1.0, 8, 1 );

    vector< vector< vec3d > > pnt_vecs;
    pnt_vecs.push_back( se_dense );

    VspCurve se_bez;
    se_bez.Interpolate( se_sparse, true );
    vector< vec3d > se_tess;
    se_bez.Tesselate( 3, se_tess );
    pnt_vecs.push_back( se_tess );

    se_bez.UniformInterpolate( 41, true );
    vector< vec3d > se_uniform_tess;
    se_bez.Tesselate( 3, se_uniform_tess );
    pnt_vecs.push_back( se_uniform_tess );

    WritePntVecs( pnt_vecs, "se_curves.dat" );

    //==== Test Copy, Split, Append and Insert Control Point ====//
    pnt_vecs.clear();
    vector< vec3d > tess_pnts;
    pnt_vec.resize( 3 );
    pnt_vec[0] = vec3d( 0, 0, 0 );
    pnt_vec[1] = vec3d( 0, 1, 0 );
    pnt_vec[2] = vec3d( 0, 1, 1 );
    crv.Interpolate( pnt_vec, false );

    crv.Tesselate( 11, tess_pnts );
    pnt_vecs.push_back( tess_pnts );

    crv.InsertControlPnt( 1.8 );
    crv.Tesselate( 11, tess_pnts );
    pnt_vecs.push_back( tess_pnts );

    VspCurve copy_crv;
    copy_crv.Copy( crv );
    VspCurve split_crv = copy_crv.Split( 2 );
    copy_crv.Tesselate( 11, tess_pnts );
    pnt_vecs.push_back( tess_pnts );
    split_crv.Tesselate( 11, tess_pnts );
    pnt_vecs.push_back( tess_pnts );
    WritePntVecs( pnt_vecs, "insert_cp_curves.dat" );

    copy_crv.Copy( crv );

    VspCurve append_crv;
    pnt_vec[0] = vec3d( 0, 1, 1 );
    pnt_vec[1] = vec3d( 0, 0, 1 );
    pnt_vec[2] = vec3d( 0, 0, 2 );
    append_crv.Interpolate( pnt_vec, false );
    crv.Append( append_crv, false );
    crv.Tesselate( 11, tess_pnts );

    pnt_vecs.clear();
    pnt_vecs.push_back( tess_pnts );

    copy_crv.Append( append_crv, true );
    copy_crv.Tesselate( 11, tess_pnts );

    pnt_vecs.push_back( tess_pnts );

    WritePntVecs( pnt_vecs, "append_cp_curves.dat" );


    vector< vec3d > se_hh = se.Build( 41, 2.0, 1.0, 2, 0.1 );
    vector< vec3d > se_11 = se.Build( 41, 2.0, 1.0, 1, 1 );
    vector< vec3d > se_22 = se.Build( 41, 2.0, 1.0, 2, 2 );
    vector< vec3d > se_44 = se.Build( 41, 2.0, 1.0, 4, 4 );
    vector< vec3d > se_88 = se.Build( 41, 2.0, 1.0, 8, 8 );
    vector< vec3d > se_81 = se.Build( 41, 2.0, 1.0, 8, 1 );
    vector< vec3d > se_18 = se.Build( 41, 2.0, 1.0, 1, 8 );

    pnt_vecs.clear();
    pnt_vecs.push_back( se_hh );
    pnt_vecs.push_back( se_11 );
    pnt_vecs.push_back( se_22 );
    pnt_vecs.push_back( se_44 );
    pnt_vecs.push_back( se_88 );
    pnt_vecs.push_back( se_81 );
    pnt_vecs.push_back( se_18 );
    WritePntVecs( pnt_vecs, "super_ellipse.dat" );
#endif
}
//==== Write Point Vecs =====//
void UtilTestSuite::WritePntVecs( vector< vector< vec3d > > & pnt_vecs,  string file_name )
{
    const char* colors[] = {"GREEN", "BLUE", "PURPLE", "WHITE",  };

    FILE* fp = fopen( file_name.c_str(), "w" );
    if ( fp )
    {

        for ( int i = 0 ; i < ( int )pnt_vecs.size() ; i++ )
        {
            fprintf( fp, "C %s\n", colors[i % 4] );
            for ( int j = 0 ; j < ( int )pnt_vecs[i].size() ; j++ )
            {
                fprintf( fp, "%f %f\n", pnt_vecs[i][j].y(), pnt_vecs[i][j].z() );
            }
        }

        fclose( fp );
    }
}



//==== WriteCurve =====//
void UtilTestSuite::WriteCurve( VspCurve& crv, string file_name )
{
#if 0
    FILE* fp = fopen( file_name.c_str(), "w" );
    if ( fp )
    {
        vector< vec3d > cntl_pnts = crv.GetControlPnts();
        for ( int i = 0 ; i < ( int )cntl_pnts.size() ; i++ )
        {
            fprintf( fp, "%f %f\n", cntl_pnts[i].x(), cntl_pnts[i].y() );
        }
        fprintf( fp, "C GREEN\n" );

        int num_sects = crv.GetNumSections();
        for ( int i = 0 ; i < num_sects * 20 ; i++ )
        {
            double u = ( double )i / 20.0;
            vec3d p = crv.CompPnt( u );
            fprintf( fp, "%f %f\n", p.x(), p.y() );
        }
        vec3d p = crv.CompPnt( num_sects );
        fprintf( fp, "%f %f\n", p.x(), p.y() );

        fclose( fp );
    }
#endif
}

//==== Test VspSurf =====//
void UtilTestSuite::VspSurfTest()
{
#if 0
    vector< VspCurve > crv_vec;
    vector< vec3d > pnt_vec;
    pnt_vec.push_back( vec3d( 0, 0, 0 ) );
    pnt_vec.push_back( vec3d( 0, 0, 0 ) );
    pnt_vec.push_back( vec3d( 0, 0, 0 ) );
    pnt_vec.push_back( vec3d( 0, 0, 0 ) );
    pnt_vec.push_back( vec3d( 0, 0, 0 ) );

    VspCurve crv1;
    crv1.Interpolate( pnt_vec, true );
    crv_vec.push_back( crv1 );

    pnt_vec.clear();
    pnt_vec.push_back( vec3d( -1, -1, 1 ) );
    pnt_vec.push_back( vec3d( 1, -1, 1 ) );
    pnt_vec.push_back( vec3d( 1, 1, 1 ) );
    pnt_vec.push_back( vec3d( -1, 1, 1 ) );
    pnt_vec.push_back( vec3d( -1, -1, 1 ) );

    VspCurve crv2;
    crv2.Interpolate( pnt_vec, true );
    crv_vec.push_back( crv2 );

    pnt_vec.clear();
    pnt_vec.push_back( vec3d( -1, -1, 2 ) );
    pnt_vec.push_back( vec3d( 2, -1, 2 ) );
    pnt_vec.push_back( vec3d( 2, 1, 2 ) );
    pnt_vec.push_back( vec3d( -1, 1, 2 ) );
    pnt_vec.push_back( vec3d( -1, -1, 2 ) );

    VspCurve crv3;
    crv3.Interpolate( pnt_vec, true );
    crv_vec.push_back( crv3 );

    pnt_vec.clear();
    pnt_vec.push_back( vec3d( -1, -1, 3 ) );
    pnt_vec.push_back( vec3d( 2, -1, 3 ) );
    pnt_vec.push_back( vec3d( 2, 1, 3 ) );
    pnt_vec.push_back( vec3d( -1, 1, 3 ) );
    pnt_vec.push_back( vec3d( -1, -1, 3 ) );

    VspCurve crv4;
    crv4.Interpolate( pnt_vec, true );
    crv_vec.push_back( crv4 );


    vector< VspPntData > data_vec( crv_vec.size() );
    data_vec[0].m_Type = VspPntData::PREDICT;
    data_vec[1].m_Type = VspPntData::ONLY_FORWARD;
    data_vec[2].m_Type = VspPntData::ONLY_BACK;
    data_vec[3].m_Type = VspPntData::PREDICT;

    VspSurf srf1;
    srf1.Interpolate( crv_vec, data_vec, false );

    vector< vector< PntNorm > > pnt_norm_vec;
    srf1.Tesselate( 21, 21, pnt_norm_vec );
    WriteSurf( pnt_norm_vec, "testsurf.stl" );
    WriteControlPnts( srf1.GetControlPnts(), "testcontrolpnts.stl" );
#endif
}


//==== WriteSurface =====//
#if 0
void UtilTestSuite::WriteSurf( vector< vector< PntNorm > > & pnt_norm_vec, string file_name )
{
    FILE* file_id = fopen( file_name.c_str(), "w" );
    if ( file_id )
    {
        fprintf( file_id, "solid\n" );
        vec3d norm;
        vec3d v0, v1, v2, v3;
        for ( int i = 0 ; i < ( int )pnt_norm_vec.size() - 1; i++ )
        {
            for ( int j = 0 ; j < ( int )pnt_norm_vec[i].size() - 1 ; j++ )
            {
                v0 = pnt_norm_vec[i][j].m_Pnt;
                v1 = pnt_norm_vec[i + 1][j].m_Pnt;
                v2 = pnt_norm_vec[i + 1][j + 1].m_Pnt;
                v3 = pnt_norm_vec[i][j + 1].m_Pnt;

                fprintf( file_id, " facet normal  %lf %lf %lf\n",  norm.x(), norm.y(), norm.z() );
                fprintf( file_id, "   outer loop\n" );
                fprintf( file_id, "     vertex %lf %lf %lf\n", v0.x(), v0.y(), v0.z() );
                fprintf( file_id, "     vertex %lf %lf %lf\n", v1.x(), v1.y(), v1.z() );
                fprintf( file_id, "     vertex %lf %lf %lf\n", v2.x(), v2.y(), v2.z() );
                fprintf( file_id, "   endloop\n" );
                fprintf( file_id, " endfacet\n" );

                fprintf( file_id, " facet normal  %lf %lf %lf\n",  norm.x(), norm.y(), norm.z() );
                fprintf( file_id, "   outer loop\n" );
                fprintf( file_id, "     vertex %lf %lf %lf\n", v0.x(), v0.y(), v0.z() );
                fprintf( file_id, "     vertex %lf %lf %lf\n", v2.x(), v2.y(), v2.z() );
                fprintf( file_id, "     vertex %lf %lf %lf\n", v3.x(), v3.y(), v3.z() );
                fprintf( file_id, "   endloop\n" );
                fprintf( file_id, " endfacet\n" );

            }
        }
        fprintf( file_id, "endsolid\n" );

        fclose( file_id );
    }
}
#endif

//==== WriteSurface =====//
void UtilTestSuite::WriteControlPnts( const vector< vector< vec3d > > & control_pnts, string file_name )
{
    FILE* file_id = fopen( file_name.c_str(), "w" );
    if ( file_id )
    {
        fprintf( file_id, "solid\n" );
        vec3d norm;
        vec3d v0, v1, v2, v3;
        for ( int i = 0 ; i < ( int )control_pnts.size() - 1; i++ )
        {
            for ( int j = 0 ; j < ( int )control_pnts[i].size() - 1 ; j++ )
            {
                v0 = control_pnts[i][j];
                v1 = control_pnts[i + 1][j];
                v2 = control_pnts[i + 1][j + 1];
                v3 = control_pnts[i][j + 1];

                fprintf( file_id, " facet normal  %lf %lf %lf\n",  norm.x(), norm.y(), norm.z() );
                fprintf( file_id, "   outer loop\n" );
                fprintf( file_id, "     vertex %lf %lf %lf\n", v0.x(), v0.y(), v0.z() );
                fprintf( file_id, "     vertex %lf %lf %lf\n", v1.x(), v1.y(), v1.z() );
                fprintf( file_id, "     vertex %lf %lf %lf\n", v2.x(), v2.y(), v2.z() );
                fprintf( file_id, "   endloop\n" );
                fprintf( file_id, " endfacet\n" );

                fprintf( file_id, " facet normal  %lf %lf %lf\n",  norm.x(), norm.y(), norm.z() );
                fprintf( file_id, "   outer loop\n" );
                fprintf( file_id, "     vertex %lf %lf %lf\n", v0.x(), v0.y(), v0.z() );
                fprintf( file_id, "     vertex %lf %lf %lf\n", v2.x(), v2.y(), v2.z() );
                fprintf( file_id, "     vertex %lf %lf %lf\n", v3.x(), v3.y(), v3.z() );
                fprintf( file_id, "   endloop\n" );
                fprintf( file_id, " endfacet\n" );
            }
        }
        fprintf( file_id, "endsolid\n" );

        fclose( file_id );
    }
}

//==== Shared Pointer Test =====//
TestA::TestA( int v )
{
    m_A = v;
    m_BPtr = shared_ptr< TestB >( new TestB( *this ) );
}

TestA::~TestA()
{
}

int TestA::GetBA()
{
    return m_BPtr->GetA();
}

TestB::TestB( TestA & a_ref ) : m_ARef( a_ref )
{
    m_B = a_ref.GetA();
}

TestB::~TestB()
{
}

void UtilTestSuite::SharedPtrTest()
{
    TestA test_a( 2323 );

    int a = test_a.GetA();
    int b = test_a.GetBA();

    TEST_ASSERT( a == b );

    TestA other_a( 1111 );

    std::swap( test_a, other_a );
}

void UtilTestSuite::PointInPolyTest()
{
    std::vector< vec2d > polygon;
    vec2d test_pnt;

    polygon.push_back( vec2d( 0, 0 ) );
    polygon.push_back( vec2d( 0, 3 ) );
    polygon.push_back( vec2d( -4, 3 ) );
    polygon.push_back( vec2d( -2, 2 ) );
    polygon.push_back( vec2d( -4, 0 ) );
    polygon.push_back( vec2d( 0, 0 ) );

    test_pnt = vec2d( -2.5, 1.45 );

    bool in_poly = PointInPolygon( test_pnt, polygon );

    TEST_ASSERT( in_poly );

    test_pnt = vec2d( -3, 2 );
    in_poly = PointInPolygon( test_pnt, polygon );

    TEST_ASSERT( !in_poly );
}

void UtilTestSuite::BilinearInterpTest()
{
    vec3d p0, p1, p;
    double z0, z1, z2, z3, interp_val;
    p0 = vec3d( 1.3, 1.5, 0 );
    p1 = vec3d( 1.8, 2.1, 0 );
    p = vec3d( 1.3, 1.8, 0 );
    z0 = 21;
    z1 = 1.5;
    z2 = 5;
    z3 = 3;

    vector<double> weights;

    BilinearWeights( p0, p1, p, weights );
    interp_val = z0 * weights[0] + z1 * weights[1] + z2 * weights[2] + z3 * weights[3];
    TEST_ASSERT_DELTA( interp_val, 12.0, DBL_EPSILON );

    p.set_xyz( 1.55, 1.8, 0 );
    BilinearWeights( p0, p1, p, weights );
    interp_val = z0 * weights[0] + z1 * weights[1] + z2 * weights[2] + z3 * weights[3];
    TEST_ASSERT_DELTA( interp_val, 7.625, DBL_EPSILON );

    p.set_xyz( 1.55, 2.1, 0 );
    BilinearWeights( p0, p1, p, weights );
    interp_val = z0 * weights[0] + z1 * weights[1] + z2 * weights[2] + z3 * weights[3];
    TEST_ASSERT_DELTA( interp_val, 4.0, DBL_EPSILON );

    p.set_xyz( 1.8, 1.8, 0 );
    BilinearWeights( p0, p1, p, weights );
    interp_val = z0 * weights[0] + z1 * weights[1] + z2 * weights[2] + z3 * weights[3];
    TEST_ASSERT_DELTA( interp_val, 3.25, DBL_EPSILON );

    p.set_xyz( 1.425, 1.8, 0 );
    BilinearWeights( p0, p1, p, weights );
    interp_val = z0 * weights[0] + z1 * weights[1] + z2 * weights[2] + z3 * weights[3];
    TEST_ASSERT_DELTA( interp_val, 9.8125, DBL_EPSILON );

}
