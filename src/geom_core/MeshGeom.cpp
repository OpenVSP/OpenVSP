//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

//******************************************************************************
//
//    Mesh Geometry Class
//
//
//   J.R. Gloudemans - 11/12/03
//
//
//******************************************************************************

#define _USE_MATH_DEFINES
#include <cmath>

#include "MeshGeom.h"

#include <utility>
#include "PtCloudGeom.h"
#include "NGonMeshGeom.h"
#include "LinkMgr.h"
#include "Vehicle.h"
#include "PntNodeMerge.h"

#include "StringUtil.h"
#include "StlHelper.h"

#include "SubSurfaceMgr.h"
#include "VspUtil.h"
#include <cstdio>
#include <cstdlib>

//==== Constructor =====//
MeshGeom::MeshGeom( Vehicle* vehicle_ptr ) : Geom( vehicle_ptr )
{
    m_Name = "MeshGeom";
    m_Type.m_Name = "Mesh";
    m_Type.m_Type = MESH_GEOM_TYPE;

    m_TessU.Deactivate();
    m_TessW.Deactivate();
    m_Origin.Deactivate();

    m_BBox.Update( vec3d( 0, 0, 0 ) );

    //==== Test Big/Little Endian ====//
    m_BigEndianFlag = 0;
    int i = 0x1000000;
    unsigned char* byte = ( unsigned char* ) &i;
    if ( *byte )
    {
        m_BigEndianFlag = 1;
    }
    else
    {
        m_BigEndianFlag = 0;
    }

    m_ScaleMatrix.loadIdentity();
    m_ScaleFromOrig.Init( "Scale_From_Original", "XForm", this, 1, 1.0e-5, 1.0e12 );

    m_ViewMeshFlag.Init( "MeshFlag", "Draw", this, true, 0, 1 );
    m_ViewSliceFlag.Init( "SliceFlag", "Draw", this, true, 0, 1 );

    m_StartColorDegree.Init( "StartColorDegree", "Draw", this, 0, 0, 359 ); 
    m_StartColorDegree.SetDescript( "Color degree on color wheel for 1st mesh, where 0 degrees is red, 120 degrees is green, 240 degrees is blue" );

    // Debug
    m_DrawType.Init( "Draw_Type", "Draw", this, DRAW_XYZ, DRAW_XYZ, DRAW_TAGS );
    m_DrawSubSurfs.Init( "Draw_Sub_UV", "Debug", this, 0, 0, 1 );

    m_MainSurfVec.clear();
}

//==== Destructor =====//
MeshGeom::~MeshGeom()
{
    int i;
    for ( i = 0 ; i < ( int )m_TMeshVec.size() ; i++ )
    {
        delete m_TMeshVec[i];
    }

    for ( i = 0 ; i < ( int )m_SliceVec.size() ; i++ )
    {
        delete m_SliceVec[i];
    }

    for ( i = 0 ; i < ( int )m_SubSurfVec.size(); i++ )
    {
        delete m_SubSurfVec[i];
    }

}

//==== Encode XML ====//
// Make sure FlattenTMeshVec has been called
xmlNodePtr MeshGeom::EncodeXml( xmlNodePtr & node )
{
    Geom::EncodeXml( node );
    xmlNodePtr mesh_node = xmlNewChild( node, nullptr, BAD_CAST "MeshGeom", nullptr );
    XmlUtil::AddIntNode( mesh_node, "Num_Meshes", ( int )m_TMeshVec.size() );
    for ( int i = 0 ; i < ( int )m_TMeshVec.size() ; i++ )
    {
        m_TMeshVec[i]->EncodeXml( mesh_node );
    }

    return mesh_node;
}

//==== Decode XML ====//
xmlNodePtr MeshGeom::DecodeXml( xmlNodePtr & node )
{
    Geom::DecodeXml( node );

    xmlNodePtr mesh_node = XmlUtil::GetNode( node, "MeshGeom", 0 );
    if ( mesh_node )
    {
        // delete any existing TMeshes
        for ( int i = 0 ; i < ( int )m_TMeshVec.size() ; i++ )
        {
            delete m_TMeshVec[i];
        }
        m_TMeshVec.clear();

        xmlNodePtr numMeshes_node = XmlUtil::GetNode( mesh_node, "Num_Meshes", 0 );
        unsigned int numMeshes = 0;
        if ( numMeshes_node )
        {
            numMeshes = XmlUtil::ExtractInt( numMeshes_node );
        }
        else
        {
            numMeshes = XmlUtil::GetNumNames( mesh_node, "TMesh" );
        }

        m_TMeshVec.resize( numMeshes );


        for ( int i = 0 ; i < numMeshes ; i++ )
        {
            m_TMeshVec[i] = new TMesh();
            xmlNodePtr tmesh_node = XmlUtil::GetNode( mesh_node, "TMesh", i );
            if ( tmesh_node )
            {
                m_TMeshVec[i]->DecodeXml( tmesh_node );
            }

            // Load this geom properties into each TMesh
            m_TMeshVec[i]->LoadGeomAttributes( this );
        }
    }

    return mesh_node;
}

int MeshGeom::ReadXSec( const char* file_name )
{
    FILE *fp;
    char str[256] = {};

    //==== Make Sure File Exists ====//
    if ( ( fp = fopen( file_name, "r" ) ) == ( FILE * )nullptr )
    {
        return 0;
    }

    //==== Read first Line of file and compare against expected header ====//
    fscanf( fp, "%255s INPUT FILE\n\n", str );
    if ( strcmp( "HERMITE", str ) != 0 )
    {
        fclose( fp );
        return 0;
    }
    //==== Read in number of components ====//
    int num_comps;
    fscanf( fp, " NUMBER OF COMPONENTS = %d\n", &num_comps );

    if ( num_comps <= 0 )
    {
        fclose( fp );
        return 0;
    }

    TMesh*  tMesh = new TMesh();
    for ( int c = 0 ; c < num_comps ; c++ )
    {
        char name_str[256];
        float x, y, z;
        int group_num, type;
        unsigned int num_pnts, num_cross;
        vector < vector < vec3d > > crossVec;

        fgets( name_str, 256, fp );
        fscanf( fp, " GROUP NUMBER = %d\n", &group_num );
        fscanf( fp, " TYPE = %d\n", &type );
        fscanf( fp, " CROSS SECTIONS = %u\n", &( num_cross ) );
        fscanf( fp, " PTS/CROSS SECTION = %u\n", &( num_pnts ) );

        //===== Size Cross Vec ====//
        crossVec.resize( num_cross );
        for ( int i = 0 ; i < num_cross ; i++ )
        {
            crossVec[i].resize( num_pnts );
            for ( int j = 0 ; j < num_pnts ; j++ )
            {
                fscanf( fp, "%f %f %f\n", &x, &y, &z );
                crossVec[i][j] = vec3d( x, y, z );
            }
        }

        int iQuad = 0;
        //==== Convert CrossSections to Triangles ====//
        for ( int i = 1 ; i < ( int )crossVec.size() ; i++ )
            for ( int j = 1 ; j < ( int )crossVec[i].size() ; j++ )
            {
                tMesh->AddTri( crossVec[ i - 1 ][ j - 1 ], crossVec[ i ][ j - 1 ], crossVec[ i ][ j ], iQuad );

                tMesh->AddTri( crossVec[ i - 1 ][ j - 1 ], crossVec[ i ][ j ], crossVec[ i - 1 ][ j ], iQuad );
                iQuad++;
            }
    }
    fclose( fp );

    m_TMeshVec.push_back( tMesh );
    UpdateBBox();


    return 1;
}

int MeshGeom::ReadSTL( const char* file_name )
{
    FILE* file_id = fopen( file_name, "r" );

    char str[256];
    float nx, ny, nz;
    float v0[3];
    float v1[3];
    float v2[3];
    TTri*  tPtr;
    TMesh*  tMesh = new TMesh();

    int binaryFlag = 0;

    if ( file_id )
    {
        //==== Cheesy ASCII/Binary Test ====//
        while ( fgets( str, 255, file_id ) )
        {
            for ( int i = 0 ; i < ( int )strlen( str ) ; i++ )
            {
                if ( ( unsigned char )str[i] > 127 )
                {
                    binaryFlag = 1;
                    break;
                }
            }
        }
        fclose( file_id );
    }

    file_id = fopen( file_name, "r" );

    if ( file_id )
    {
        if ( !binaryFlag )
        {
            while ( true )
            {
                char buf[256];

                fgets( buf, 255, file_id );
                if ( feof( file_id ) )
                {
                    break;
                }
                sscanf( buf, "%s", str );

                if ( strcmp( str, "endsolid" ) == 0 )
                {
                    fgets( buf, 255, file_id );
                    if ( feof( file_id ) )
                    {
                        break;
                    }
                    sscanf( buf, "%s", str );
                }

                if ( strcmp( str, "solid" ) == 0 )
                {
                    fgets( buf, 255, file_id );
                    if ( feof( file_id ) )
                    {
                        break;
                    }
                }

                // facet normal
                sscanf( buf, "%*s %*s %f %f %f\n", &nx, &ny, &nz );

                fgets( buf, 255, file_id ); // outer loop
                if ( feof( file_id ) )
                {
                    break;
                }

                fgets( buf, 255, file_id );
                if ( feof( file_id ) )
                {
                    break;
                }
                sscanf( buf, "%*s %f %f %f\n", &v0[0], &v0[1], &v0[2] );

                fgets( buf, 255, file_id );
                if ( feof( file_id ) )
                {
                    break;
                }
                sscanf( buf, "%*s %f %f %f\n", &v1[0], &v1[1], &v1[2] );

                fgets( buf, 255, file_id );
                if ( feof( file_id ) )
                {
                    break;
                }
                sscanf( buf, "%*s %f %f %f\n", &v2[0], &v2[1], &v2[2] );

                fgets( buf, 255, file_id ); // endloop
                if ( feof( file_id ) )
                {
                    break;
                }

                fgets( buf, 255, file_id ); // endfacet
                if ( feof( file_id ) )
                {
                    break;
                }

                //==== Add Valid Facet ====//
                tPtr = new TTri( tMesh );
                tPtr->m_Norm = vec3d( nx, ny, nz );
                tMesh->m_TVec.push_back( tPtr );

                //==== Put Nodes Into Tri ====//
                tPtr->m_N0 = new TNode();
                tPtr->m_N1 = new TNode();
                tPtr->m_N2 = new TNode();
                tPtr->m_N0->m_Pnt = vec3d( v0[0], v0[1], v0[2] );
                tPtr->m_N1->m_Pnt = vec3d( v1[0], v1[1], v1[2] );
                tPtr->m_N2->m_Pnt = vec3d( v2[0], v2[1], v2[2] );
                tMesh->m_NVec.push_back( tPtr->m_N0 );
                tMesh->m_NVec.push_back( tPtr->m_N1 );
                tMesh->m_NVec.push_back( tPtr->m_N2 );
            }
        }
        else
        {
            fclose( file_id );
            file_id = fopen( file_name, "rb" );         // Reopen as Binary

            //==== Header ====//
            fread( str, sizeof( char ), 80, file_id );

            int numFacet = ReadBinInt( file_id );

            for ( int i = 0 ; i < numFacet ; i++ )
            {
                nx = ReadBinFloat( file_id );
                ny = ReadBinFloat( file_id );
                nz = ReadBinFloat( file_id );

                v0[0] = ReadBinFloat( file_id );
                v0[1] = ReadBinFloat( file_id );
                v0[2] = ReadBinFloat( file_id );
                v1[0] = ReadBinFloat( file_id );
                v1[1] = ReadBinFloat( file_id );
                v1[2] = ReadBinFloat( file_id );
                v2[0] = ReadBinFloat( file_id );
                v2[1] = ReadBinFloat( file_id );
                v2[2] = ReadBinFloat( file_id );

                //==== Padding ====//
                fgetc( file_id );
                fgetc( file_id );

                //==== Add Valid Facet ====//
                tPtr = new TTri( tMesh );
                tPtr->m_Norm = vec3d( nx, ny, nz );
                tMesh->m_TVec.push_back( tPtr );

                //==== Put Nodes Into Tri ====//
                tPtr->m_N0 = new TNode();
                tPtr->m_N1 = new TNode();
                tPtr->m_N2 = new TNode();
                tPtr->m_N0->m_Pnt = vec3d( v0[0], v0[1], v0[2] );
                tPtr->m_N1->m_Pnt = vec3d( v1[0], v1[1], v1[2] );
                tPtr->m_N2->m_Pnt = vec3d( v2[0], v2[1], v2[2] );
                tMesh->m_NVec.push_back( tPtr->m_N0 );
                tMesh->m_NVec.push_back( tPtr->m_N1 );
                tMesh->m_NVec.push_back( tPtr->m_N2 );
            }
        }
        fclose( file_id );
    }


    if ( tMesh->m_TVec.size() == 0 )
    {
        delete tMesh;
        return 0;
    }

    m_TMeshVec.push_back( tMesh );
    UpdateBBox();

    return 1;

}

//==== Write Fuse File ====//
float MeshGeom::ReadBinFloat( FILE* fptr )
{
    float fval = 0x00;
    fread( &fval, 4, 1, fptr );

    if ( m_BigEndianFlag )
    {
        char swap[4];
        unsigned char *cptr = ( unsigned char * )&fval;
        swap[0] = cptr[3];
        swap[1] = cptr[2];
        swap[2] = cptr[1];

        cptr[3] = cptr[0];
        cptr[2] = swap[2];
        cptr[1] = swap[1];
        cptr[0] = swap[0];
    }
    return fval;
}

int MeshGeom::ReadBinInt( FILE* fptr )
{
    int ival = 0x00;
    fread( &ival, 4, 1, fptr );

    if ( m_BigEndianFlag )
    {
        char swap[4];
        unsigned char *cptr = ( unsigned char * )&ival;
        swap[0] = cptr[3];
        swap[1] = cptr[2];
        swap[2] = cptr[1];

        cptr[3] = cptr[0];
        cptr[2] = swap[2];
        cptr[1] = swap[1];
        cptr[0] = swap[0];
    }
    return ival;
}

//==== Write STL File ====//
void MeshGeom::WriteStl( FILE* file_id )
{
    int m;

    if ( m_ViewMeshFlag() )
    {
        for (m = 0; m < (int) m_TMeshVec.size(); m++)
        {
            m_TMeshVec[m]->WriteSTLTris(file_id, GetTotalTransMat());
        }
    }

    if ( m_ViewSliceFlag() )
    {
        for (m = 0; m < (int) m_SliceVec.size(); m++)
        {
            m_SliceVec[m]->WriteSTLTris(file_id, GetTotalTransMat());
        }
    }
}


int MeshGeom::ReadNascart( const char* file_name )
{
    int i;
    FILE* file_id = fopen( file_name, "r" );

    if ( !file_id )
    {
        return 0;
    }

    TMesh*  tMesh = new TMesh();

    //==== Read Number Tris and Nodes ====//
    float x, y, z;
    int n0, n1, n2;
    float col;
    int num_tris, num_nodes;

    fscanf( file_id, "%d", &num_nodes );
    fscanf( file_id, "%d", &num_tris  );

    vec3d p;
    vector< vec3d > pVec;
    for ( i = 0 ; i < num_nodes ; i++ )
    {
        fscanf( file_id, "%f %f %f", &x, &y, &z );
        p.set_xyz( x, -z, y );
        pVec.push_back( p );
    }

    for ( i = 0 ; i < num_tris ; i++ )
    {
        fscanf( file_id, "%d %d %d %f", &n0, &n2, &n1, &col );

        //==== Compute Normal ====//
        vec3d p10 = pVec[n1 - 1] - pVec[n0 - 1];
        vec3d p20 = pVec[n2 - 1] - pVec[n0 - 1];
        vec3d norm = cross( p10, p20 );
        norm.normalize();

        //==== Add Valid Facet ====//
        tMesh->AddTri( pVec[ n0 - 1 ], pVec[ n1 - 1 ], pVec[ n2 - 1 ], norm, -1 );
    }

    fclose( file_id );

    if ( tMesh->m_TVec.size() == 0 )
    {
        delete tMesh;
        return 0;
    }

    m_TMeshVec.push_back( tMesh );

    UpdateBBox();


    return 1;

}

//==== Read Tri File ====//
int MeshGeom::ReadTriFile( const char * file_name )
{
    int i;
    FILE* file_id = fopen( file_name, "r" );

    if ( !file_id )
    {
        return 0;
    }

    TMesh*  tMesh = new TMesh();

    //==== Read Number Tris and Nodes ====//
    float x, y, z;
    int n0, n1, n2;
    unsigned int num_tris, num_nodes;

    fscanf( file_id, "%u", &num_nodes );
    fscanf( file_id, "%u", &num_tris  );

    vec3d p;
    vector< vec3d > pVec;
    pVec.resize( num_nodes );
    for ( i = 0 ; i < num_nodes ; i++ )
    {
        fscanf( file_id, "%f %f %f", &x, &y, &z );
        p.set_xyz( x, y, z );
        pVec[i] = p;
    }

    for ( i = 0 ; i < num_tris ; i++ )
    {
        fscanf( file_id, "%d %d %d", &n0, &n1, &n2 );

        //==== Compute Normal ====//
        vec3d p10 = pVec[n1 - 1] - pVec[n0 - 1];
        vec3d p20 = pVec[n2 - 1] - pVec[n0 - 1];
        vec3d norm = cross( p10, p20 );
        norm.normalize();

        //==== Add Valid Facet ====//
        tMesh->AddTri( pVec[ n0 - 1 ], pVec[ n1 - 1 ], pVec[ n2 - 1 ], norm, -1 );
    }

    fclose( file_id );

    if ( tMesh->m_TVec.size() == 0 )
    {
        delete tMesh;
        return 0;
    }

    m_TMeshVec.push_back( tMesh );

    UpdateBBox();


    return 1;
}


void MeshGeom::WriteVSPGeom( const string file_name )
{
    //==== Open file ====//
    FILE *file_id = fopen( file_name.c_str(), "w" );

    fprintf( file_id, "# vspgeom v3\n" );
    fprintf( file_id, "1\n" );  // Number of meshes.

    //==== Count Number of Points & Tris ====//
    int num_pnts = 0;
    int num_tris = 0;
    int num_parts = 0;
    int num_wakes = 0;
    int i;

    vector< TTri* > trivec;
    vector< TNode* > nodvec;
    BuildIndexedMesh( m_TMeshVec, m_SliceVec, m_ViewMeshFlag(), m_ViewSliceFlag(), trivec, nodvec );
    IdentifyWakes( trivec, m_Wakes, m_PolyVec );
    num_parts += GetNumIndexedParts();
    num_pnts += (int)nodvec.size();
    num_tris += (int)trivec.size();
    num_wakes += GetNumWakes();

    fprintf( file_id, "%d %d %d\n", num_pnts,
                                    num_tris,
                                    num_wakes );

    //==== Dump Points ====//
    WriteVSPGeomPnts( file_id, nodvec, GetTotalTransMat() );

    fprintf( file_id, "%d\n", num_tris );

    int offset = 0;
    //==== Dump Tris ====//
    offset = WriteVSPGeomTris( file_id, offset, trivec, nodvec );

    WriteVSPGeomParts( file_id, trivec );

    int tcount = 1;
    WriteVSPGeomParents( file_id, tcount, trivec );

    fprintf( file_id, "%d\n", num_wakes );

    offset = 0;
    // Wake line data.
    offset = WriteVSPGeomWakes( file_id, offset, m_Wakes, nodvec );

    m_SurfDirty = true;
    Update();

    offset = 0;
    tcount = 1;
    //==== Dump alternate Tris ====//
    offset = WriteVSPGeomAlternateTris( file_id, offset, tcount, trivec, nodvec );

    tcount = 1;
    WriteVSPGeomAlternateParts( file_id, tcount, trivec );
    fclose( file_id );
}

void MeshGeom::WritePovRay( FILE* fid, int comp_num )
{
    // Make Sure FlattenTMeshVec has been called first
    string name = GetName();
    StringUtil::change_space_to_underscore( name );
    Matrix4d transMat = GetTotalTransMat();

    fprintf( fid, "#declare %s_%d = mesh { \n", name.c_str(), comp_num );

    for ( int i = 0 ; i < ( int )m_TMeshVec.size() ; i++ )
    {
        for ( int j = 0 ; j < ( int )m_TMeshVec[i]->m_TVec.size() ; j++ )
        {
            vec3d v0, v1, v2, n, d21;

            TTri* tri = m_TMeshVec[i]->m_TVec[j];

            v0 = transMat.xform( tri->m_N0->m_Pnt );
            v1 = transMat.xform( tri->m_N1->m_Pnt );
            v2 = transMat.xform( tri->m_N2->m_Pnt );
            d21 = v2 - v1;

            if ( d21.mag() > 0.000001 )
            {
                n = cross( d21, v0 - v1 );
                fprintf( fid, "smooth_triangle { \n" );
                WritePovRayTri( fid, v0, n );
                WritePovRayTri( fid, v1, n );
                WritePovRayTri( fid, v2, n, false );
            }
        }
    }

    fprintf( fid, " }\n" );
}

void MeshGeom::WriteX3D( xmlNodePtr node )
{
    xmlNodePtr set_node = xmlNewChild( node, nullptr, BAD_CAST "IndexedFaceSet", nullptr );
    xmlSetProp( set_node, BAD_CAST "solid", BAD_CAST "true" );
    xmlSetProp( set_node, BAD_CAST "creaseAngle", BAD_CAST "0.5"  );
    Matrix4d transMat = GetTotalTransMat();

    string indstr, crdstr;
    int offset = 0;
    char numstr[255];

    for ( int i = 0 ; i < ( int )m_TMeshVec.size() ; i++ )
    {
        for ( int j = 0 ; j < ( int )m_TMeshVec[i]->m_TVec.size() ; j++ )
        {
            vec3d v0, v1, v2, d21;

            TTri* tri = m_TMeshVec[i]->m_TVec[j];

            v0 = transMat.xform( tri->m_N0->m_Pnt );
            v1 = transMat.xform( tri->m_N1->m_Pnt );
            v2 = transMat.xform( tri->m_N2->m_Pnt );
            d21 = v2 - v1;

            if ( d21.mag() > 0.000001 )
            {
                snprintf( numstr, sizeof( numstr ), "%lf %lf %lf %lf %lf %lf %lf %lf %lf ", v0.x(), v0.y(), v0.z(),
                         v1.x(), v1.y(), v1.z(), v2.x(), v2.y(), v2.z() );
                crdstr += numstr;
                snprintf( numstr, sizeof( numstr ), "%d %d %d -1 ", offset, offset + 1, offset + 2 );
                offset += 3;
                indstr += numstr;
            }
        }
    }

    xmlSetProp( set_node, BAD_CAST "coordIndex", BAD_CAST indstr.c_str() );

    xmlNodePtr coord_node = xmlNewChild( set_node, nullptr, BAD_CAST "Coordinate", BAD_CAST " " );
    xmlSetProp( coord_node, BAD_CAST "point", BAD_CAST crdstr.c_str() );
}

void MeshGeom::UpdateBBox()
{
    BndBox new_box;
    ::UpdateBBox( new_box, m_TMeshVec, GetTotalTransMat() );

    if ( new_box != m_BBox )
    {
        m_BbXLen = new_box.GetMax( 0 ) - new_box.GetMin( 0 );
        m_BbYLen = new_box.GetMax( 1 ) - new_box.GetMin( 1 );
        m_BbZLen = new_box.GetMax( 2 ) - new_box.GetMin( 2 );

        m_BbXMin = new_box.GetMin( 0 );
        m_BbYMin = new_box.GetMin( 1 );
        m_BbZMin = new_box.GetMin( 2 );

        m_BBox = new_box;
        m_ScaleIndependentBBox = m_BBox;
    }
}

void MeshGeom::UpdateDrawObj()
{
    // Add in SubSurfaces to TMeshVec if m_DrawSubSurfs is true
    unsigned int num_meshes = m_TMeshVec.size();

    unsigned int num_uniq_tags = SubSurfaceMgr.GetNumTags();

    // Update Draw type based on if the disp subsurface is true
    if ( m_GuiDraw.GetDispSubSurfFlag() )
    {
        m_DrawType = MeshGeom::DRAW_TAGS;
    }
    else
    {
        m_DrawType = MeshGeom::DRAW_XYZ;
    }

    if ( m_DrawSubSurfs() )
    {
        m_TMeshVec.insert( m_TMeshVec.end(), m_SubSurfVec.begin(), m_SubSurfVec.end() );
    }

    // Mesh Should Be Flat Before Calling this Method
    int add_ind = 0;

    if ( m_DrawType() == MeshGeom::DRAW_XYZ || m_DrawType() == MeshGeom::DRAW_UV )
    {
        m_WireShadeDrawObj_vec.resize( m_TMeshVec.size(), DrawObj() );
    }
    else if ( m_DrawType() == MeshGeom::DRAW_BOTH )
    {
        m_WireShadeDrawObj_vec.resize( 2 * m_TMeshVec.size(), DrawObj() );
        add_ind = m_TMeshVec.size();
    }
    else if ( m_DrawType() == MeshGeom::DRAW_TAGS )
    {
        m_WireShadeDrawObj_vec.resize( num_uniq_tags );
        DrawObj new_obj;
        for ( int i = 0; i < num_uniq_tags; i++ )
        {
            m_WireShadeDrawObj_vec[i] = new_obj;
        }
    }
    else
    {
        m_WireShadeDrawObj_vec.resize( 0, DrawObj() );
        return;
    }


    Matrix4d trans = GetTotalTransMat();
    vec3d zeroV = m_ModelMatrix.xform( vec3d( 0.0, 0.0, 0.0 ) );

    if ( m_ViewMeshFlag.Get() )
    {
        if ( m_DrawType() & MeshGeom::DRAW_XYZ )
        {
            for ( int m = 0 ; m < ( int )m_TMeshVec.size() ; m++ )
            {
                unsigned int num_tris = m_TMeshVec[m]->m_TVec.size();
                unsigned int pi = 0;
                vector<TTri*>& tris = m_TMeshVec[m]->m_TVec;
                m_WireShadeDrawObj_vec[m].m_PntVec.resize( num_tris * 3 );
                m_WireShadeDrawObj_vec[m].m_NormVec.resize( num_tris * 3 );
                for ( int t = 0 ; t < ( int ) num_tris ; t++ )
                {
                    m_WireShadeDrawObj_vec[m].m_PntVec[pi] = trans.xform( tris[t]->m_N0->m_Pnt );
                    m_WireShadeDrawObj_vec[m].m_PntVec[pi + 1] = trans.xform( tris[t]->m_N1->m_Pnt );
                    m_WireShadeDrawObj_vec[m].m_PntVec[pi + 2] = trans.xform( tris[t]->m_N2->m_Pnt );
                    vec3d norm =  m_ModelMatrix.xform( tris[t]->m_Norm ) - zeroV;
                    m_WireShadeDrawObj_vec[m].m_NormVec[pi] = norm;
                    m_WireShadeDrawObj_vec[m].m_NormVec[pi + 1] = norm;
                    m_WireShadeDrawObj_vec[m].m_NormVec[pi + 2] = norm;
                    pi += 3;
                }
            }
        }

        if ( m_DrawType() & MeshGeom::DRAW_UV )
        {
            for ( int m = 0 ; m < ( int )m_TMeshVec.size() ; m++ )
            {
                m_TMeshVec[m]->MakeNodePntUW();
                int num_tris = m_TMeshVec[m]->m_TVec.size();
                int pi = 0;
                vector<TTri*>& tris = m_TMeshVec[m]->m_TVec;
                m_WireShadeDrawObj_vec[m + add_ind].m_PntVec.resize( num_tris * 3 );
                m_WireShadeDrawObj_vec[m + add_ind].m_NormVec.resize( num_tris * 3 );
                for ( int t = 0 ; t < ( int ) num_tris ; t++ )
                {
                    m_WireShadeDrawObj_vec[m + add_ind].m_PntVec[pi] = trans.xform( tris[t]->m_N0->m_Pnt );
                    m_WireShadeDrawObj_vec[m + add_ind].m_PntVec[pi + 1] = trans.xform( tris[t]->m_N1->m_Pnt );
                    m_WireShadeDrawObj_vec[m + add_ind].m_PntVec[pi + 2] = trans.xform( tris[t]->m_N2->m_Pnt );
                    vec3d norm =  m_ModelMatrix.xform( tris[t]->m_Norm ) - zeroV;
                    m_WireShadeDrawObj_vec[m + add_ind].m_NormVec[pi] = norm;
                    m_WireShadeDrawObj_vec[m + add_ind].m_NormVec[pi + 1] = norm;
                    m_WireShadeDrawObj_vec[m + add_ind].m_NormVec[pi + 2] = norm;
                    pi += 3;
                }
                m_TMeshVec[m]->MakeNodePntXYZ();
            }
        }

        if ( m_DrawType() == MeshGeom::DRAW_TAGS && ! m_DrawSubSurfs() )
        {
            // make map from tag to wire draw obj

            unordered_map<int, DrawObj*> tag_dobj_map;
            map< std::vector<int>, int >::const_iterator mit;
            map< std::vector<int>, int > tagMap = SubSurfaceMgr.GetSingleTagMap();
            int cnt = 0;
            for ( mit = tagMap.begin(); mit != tagMap.end() ; ++mit )
            {
                tag_dobj_map[ mit->second ] = &m_WireShadeDrawObj_vec[cnt];
                cnt++;
            }

            for ( int m = 0 ; m < ( int )m_TMeshVec.size() ; m++ )
            {
                int num_tris = m_TMeshVec[m]->m_TVec.size();
                vector<TTri*>& tris = m_TMeshVec[m]->m_TVec;
                for ( int t = 0 ; t < ( int ) num_tris ; t++ )
                {
                    DrawObj* d_obj = tag_dobj_map[ SubSurfaceMgr.GetTag( tris[t]->m_Tags ) ];
                    d_obj->m_PntVec.push_back( trans.xform( tris[t]->m_N0->m_Pnt ) );
                    d_obj->m_PntVec.push_back( trans.xform( tris[t]->m_N1->m_Pnt ) );
                    d_obj->m_PntVec.push_back( trans.xform( tris[t]->m_N2->m_Pnt ) );
                    vec3d norm =  m_ModelMatrix.xform( tris[t]->m_Norm ) - zeroV;
                    d_obj->m_NormVec.push_back( norm );
                    d_obj->m_NormVec.push_back( norm );
                    d_obj->m_NormVec.push_back( norm );
                }
            }
        }
    }

    // Remove subsurfaces From TMeshVec
    if ( m_DrawSubSurfs() )
    {
        m_TMeshVec.erase( m_TMeshVec.begin() + num_meshes, m_TMeshVec.end() );
    }

    //==== Bounding Box ====//
    m_HighlightDrawObj.m_PntVec = m_BBox.GetBBoxDrawLines();
    m_HighlightDrawObj.m_GeomChanged = true;

    if ( m_ViewSliceFlag.Get() )
    {
        //==== Draw Slices ====//
        for ( int i = 0 ; i < ( int )m_SliceVec.size(); i++ )
        {
            int draw_ind = m_WireShadeDrawObj_vec.size();
            m_WireShadeDrawObj_vec.push_back( DrawObj() );
            int num_tris = m_SliceVec[i]->m_TVec.size();
            int pi = 0;
            vector<TTri*>& tris = m_SliceVec[i]->m_TVec;
            m_WireShadeDrawObj_vec[draw_ind].m_PntVec.resize( num_tris * 3 );
            m_WireShadeDrawObj_vec[draw_ind].m_NormVec.resize( num_tris * 3 );
            for ( int t = 0 ; t < ( int ) num_tris ; t++ )
            {
                m_WireShadeDrawObj_vec[draw_ind].m_PntVec[pi] = trans.xform( tris[t]->m_N0->m_Pnt );
                m_WireShadeDrawObj_vec[draw_ind].m_PntVec[pi + 1] = trans.xform( tris[t]->m_N1->m_Pnt );
                m_WireShadeDrawObj_vec[draw_ind].m_PntVec[pi + 2] = trans.xform( tris[t]->m_N2->m_Pnt );
                vec3d norm =  m_ModelMatrix.xform( tris[t]->m_Norm ) - zeroV;
                m_WireShadeDrawObj_vec[draw_ind].m_NormVec[pi] = norm;
                m_WireShadeDrawObj_vec[draw_ind].m_NormVec[pi + 1] = norm;
                m_WireShadeDrawObj_vec[draw_ind].m_NormVec[pi + 2] = norm;
                pi += 3;
            }
        }
    }

    // Flag the DrawObjects as changed
    for ( int i = 0 ; i < ( int )m_WireShadeDrawObj_vec.size(); i++ )
    {
        m_WireShadeDrawObj_vec[i].m_GeomChanged = true;
    }


    // Calculate constants for color sequence.
    const int ncgrp = m_PolyVec.size(); // Number of basic colors
    const int ncstep = 1;
    const double nctodeg = 360.0/(ncgrp*ncstep);

    m_FeatureDrawObj_vec.resize( m_PolyVec.size() );
    for ( int i = 0; i < m_PolyVec.size(); i++ )
    {

        // Color sequence -- go around color wheel ncstep times with slight
        // offset from ncgrp basic colors.
        // Note, (cnt/ncgrp) uses integer division resulting in floor.
        double deg = m_StartColorDegree() + ( ( i % ncgrp ) * ncstep + ( i / ncgrp ) ) * nctodeg;

        if ( deg > 360 )
        {
            deg = (int)deg % 360;
        }

        vec3d rgb = m_FeatureDrawObj_vec[i].ColorWheel( deg );
        rgb.normalize();

        m_FeatureDrawObj_vec[i].m_Type = DrawObj::VSP_LINE_STRIP;
        m_FeatureDrawObj_vec[i].m_LineWidth = 5;
        m_FeatureDrawObj_vec[i].m_LineColor = rgb;
        m_FeatureDrawObj_vec[i].m_Screen = DrawObj::VSP_MAIN_SCREEN;

        char str[255];
        snprintf( str, sizeof( str ),  "_%d", i );
        m_FeatureDrawObj_vec[i].m_GeomID = m_ID + "Feature_" + str;

        m_FeatureDrawObj_vec[i].m_GeomChanged = true;

        m_FeatureDrawObj_vec[i].m_PntVec.resize( m_PolyVec[i].size() );
        for ( int j = 0; j < m_PolyVec[i].size(); j++ )
        {
            m_FeatureDrawObj_vec[i].m_PntVec[j] = trans.xform( m_PolyVec[i][j] );
        }
    }
}

void MeshGeom::LoadDrawObjs( vector< DrawObj* > & draw_obj_vec )
{
    int num_uniq_tags = SubSurfaceMgr.GetNumTags();

    // Calculate constants for color sequence.
    const int ncgrp = 6; // Number of basic colors
    const int ncstep = (int)ceil((double)num_uniq_tags/(double)ncgrp);
    const double nctodeg = 360.0/(ncgrp*ncstep);

    Geom::LoadDrawObjs( draw_obj_vec );
    for ( int i = 0 ; i < ( int )m_WireShadeDrawObj_vec.size() ; i++ )
    {
        if ( m_DrawType() == MeshGeom::DRAW_TAGS && m_GuiDraw.GetDispSubSurfFlag() )
        {
            // Color sequence -- go around color wheel ncstep times with slight
            // offset from ncgrp basic colors.
            // Note, (cnt/ncgrp) uses integer division resulting in floor.
            double deg = m_StartColorDegree() + ( ( i % ncgrp ) * ncstep + ( i / ncgrp ) ) * nctodeg;

            if ( deg > 360 )
            {
                deg = (int)deg % 360;
            }

            vec3d rgb = m_WireShadeDrawObj_vec[i].ColorWheel( deg );
            rgb.normalize();
            m_WireShadeDrawObj_vec[i].m_MaterialInfo.Ambient[0] = (float)rgb.x()/5.0f;
            m_WireShadeDrawObj_vec[i].m_MaterialInfo.Ambient[1] = (float)rgb.y()/5.0f;
            m_WireShadeDrawObj_vec[i].m_MaterialInfo.Ambient[2] = (float)rgb.z()/5.0f;
            m_WireShadeDrawObj_vec[i].m_MaterialInfo.Ambient[3] = (float)1.0f;

            m_WireShadeDrawObj_vec[i].m_MaterialInfo.Diffuse[0] = 0.4f + (float)rgb.x()/10.0f;
            m_WireShadeDrawObj_vec[i].m_MaterialInfo.Diffuse[1] = 0.4f + (float)rgb.y()/10.0f;
            m_WireShadeDrawObj_vec[i].m_MaterialInfo.Diffuse[2] = 0.4f + (float)rgb.z()/10.0f;
            m_WireShadeDrawObj_vec[i].m_MaterialInfo.Diffuse[3] = 1.0f;

            m_WireShadeDrawObj_vec[i].m_MaterialInfo.Specular[0] = 0.04f + 0.7f * (float)rgb.x();
            m_WireShadeDrawObj_vec[i].m_MaterialInfo.Specular[1] = 0.04f + 0.7f * (float)rgb.y();
            m_WireShadeDrawObj_vec[i].m_MaterialInfo.Specular[2] = 0.04f + 0.7f * (float)rgb.z();
            m_WireShadeDrawObj_vec[i].m_MaterialInfo.Specular[3] = 1.0f;

            m_WireShadeDrawObj_vec[i].m_MaterialInfo.Emission[0] = (float)rgb.x()/20.0f;
            m_WireShadeDrawObj_vec[i].m_MaterialInfo.Emission[1] = (float)rgb.y()/20.0f;
            m_WireShadeDrawObj_vec[i].m_MaterialInfo.Emission[2] = (float)rgb.z()/20.0f;
            m_WireShadeDrawObj_vec[i].m_MaterialInfo.Emission[3] = 1.0f;

            m_WireShadeDrawObj_vec[i].m_MaterialInfo.Shininess = 32.0f;

            m_WireShadeDrawObj_vec[i].m_LineColor = rgb;
        }
        switch( m_GuiDraw.GetDrawType() )
        {
        case vsp::DRAW_TYPE::GEOM_DRAW_WIRE:
            m_WireShadeDrawObj_vec[i].m_Type = DrawObj::VSP_WIRE_TRIS;
            break;

        case vsp::DRAW_TYPE::GEOM_DRAW_HIDDEN:
            m_WireShadeDrawObj_vec[i].m_Type = DrawObj::VSP_WIRE_HIDDEN_TRIS;
            break;

        case vsp::DRAW_TYPE::GEOM_DRAW_SHADE:
            m_WireShadeDrawObj_vec[i].m_Type = DrawObj::VSP_SHADED_TRIS;
            break;

        case vsp::DRAW_TYPE::GEOM_DRAW_NONE:
            m_WireShadeDrawObj_vec[i].m_Type = DrawObj::VSP_SHADED_TRIS;
            m_WireShadeDrawObj_vec[i].m_Visible = false;
            break;

        // Does not support Texture Mapping.  Render Shaded instead.
        case vsp::DRAW_TYPE::GEOM_DRAW_TEXTURE:
            m_WireShadeDrawObj_vec[i].m_Type = DrawObj::VSP_SHADED_TRIS;
            break;
        }
    }
}

//==== Create And Load Tris into Results Data Structures ====//
void MeshGeom::CreateGeomResults( Results* res )
{
    int restype = -1;
    if ( m_TMeshVec.size() && m_SliceVec.size() )
    {
        restype = vsp::MESH_INDEX_AND_SLICE_TRI;
    }
    else if ( m_TMeshVec.size() )
    {
        restype = vsp::MESH_INDEXED_TRI;
    }
    else if ( m_SliceVec.size() )
    {
        restype = vsp::MESH_SLICE_TRI;
    }
    res->Add( new NameValData( "Type", restype, "Mesh results geom type flag." ) );

    //==== Add Index Tris =====//
    if ( m_TMeshVec.size() )
    {
        vector< TTri* > trivec;
        vector< TNode* > nodvec;
        BuildIndexedMesh( m_TMeshVec, m_SliceVec, m_ViewMeshFlag(), m_ViewSliceFlag(), trivec, nodvec );

        vector< vec3d > pvec;
        Matrix4d XFormMat = GetTotalTransMat();
        //==== Write Out Nodes ====//
        for ( int i = 0 ; i < ( int )nodvec.size() ; i++ )
        {
            TNode* tnode = nodvec[i];
            pvec.push_back( XFormMat.xform( tnode->m_Pnt ) );
        }
        res->Add( new NameValData( "Num_Pnts", ( int )nodvec.size(), "Number of indexed points." ) );
        res->Add( new NameValData( "Tri_Pnts", pvec, "Coordinates of indexed points." ) );

        //==== Write Out Tris ====//
        vector< int > id0_vec;
        vector< int > id1_vec;
        vector< int > id2_vec;
        for ( int t = 0 ; t < ( int )trivec.size() ; t++ )
        {
            TTri* ttri = trivec[t];

            id0_vec.push_back( ttri->m_N0->m_ID );
            id1_vec.push_back( ttri->m_N1->m_ID );
            id2_vec.push_back( ttri->m_N2->m_ID );
        }
        res->Add( new NameValData( "Num_Tris", ( int )trivec.size(), "Number of indexed tris." ) );
        res->Add( new NameValData( "Tri_Index0", id0_vec, "Index of triangle node zero." ) );
        res->Add( new NameValData( "Tri_Index1", id1_vec, "Index of triangle node one." ) );
        res->Add( new NameValData( "Tri_Index2", id2_vec, "Index of triangle node two." ) );
    }

    //==== Add Slices =====//
    if ( m_SliceVec.size() )
    {
        //==== Load m_SliceVec ====//
        res->Add( new NameValData( "Num_Slices", ( int )m_SliceVec.size(), "Number of slices." ) );
        for ( int i = 0; i < ( int )m_SliceVec.size(); i++ )
        {
            res->Add( new NameValData( "Num_Slice_Tris", ( int )( int )m_SliceVec[i]->m_TVec.size(), "Number of tris in this slice." ) );
            vector< vec3d > slice_tri_n0_vec;
            vector< vec3d > slice_tri_n1_vec;
            vector< vec3d > slice_tri_n2_vec;
            for ( int j = 0; j < ( int )m_SliceVec[i]->m_TVec.size(); j++ )
            {
                TTri * tri = m_SliceVec[i]->m_TVec[j];
                slice_tri_n0_vec.push_back( tri->m_N0->m_Pnt );
                slice_tri_n1_vec.push_back( tri->m_N1->m_Pnt );
                slice_tri_n2_vec.push_back( tri->m_N2->m_Pnt );
            }
            res->Add( new NameValData( "Slice_Tris_Pnt_0", slice_tri_n0_vec, "Coordinates of triangle in slice node zero." ) );
            res->Add( new NameValData( "Slice_Tris_Pnt_1", slice_tri_n1_vec, "Coordinates of triangle in slice node one." ) );
            res->Add( new NameValData( "Slice_Tris_Pnt_2", slice_tri_n2_vec, "Coordinates of triangle in slice node two." ) );
        }
    }
}

void MeshGeom::CreatePtCloudGeom()
{
    vector< TTri* > trivec;
    vector< TNode* > nvec;
    BuildIndexedMesh( m_TMeshVec, m_SliceVec, m_ViewMeshFlag(), m_ViewSliceFlag(), trivec, nvec );
    unsigned int npts = nvec.size();

    if ( npts > 0 )
    {
        GeomType type = GeomType( PT_CLOUD_GEOM_TYPE, "PTS", true );
        string id = m_Vehicle->AddGeom( type );
        if ( !id.compare( "NONE" ) )
        {
            return;
        }

        PtCloudGeom* new_geom = ( PtCloudGeom* ) m_Vehicle->FindGeom( id );
        if ( new_geom )
        {
            new_geom->SetName( GetName() + "_Pts" );

            new_geom->m_Pts.resize( npts );

            Matrix4d XFormMat = GetTotalTransMat();

            for ( int j = 0; j < npts; j++ )
            {
                new_geom->m_Pts[j] = XFormMat.xform( nvec[j]->m_Pnt );
            }

            new_geom->InitPts();

            new_geom->Update();
            m_Vehicle->UpdateGUI();

        }
    }
}

string MeshGeom::CreateNGonMeshGeom( bool cullfracflag, double cullfrac, int n_ref, bool FindBodyWakes )
{
    GeomType type = GeomType( NGON_GEOM_TYPE, "NGON", true );
    string id = m_Vehicle->AddGeom( type );
    if ( !id.compare( "NONE" ) )
    {
        return id;
    }

    NGonMeshGeom* new_geom = ( NGonMeshGeom* ) m_Vehicle->FindGeom( id );
    if ( new_geom )
    {
        new_geom->m_ActiveMesh.SetUpperLimit( n_ref );

        new_geom->SetName( GetName() + "_NGon" );

        Matrix4d XFormMat = GetTotalTransMat();

        PGMesh *pgm = new_geom->m_PGMulti.GetActiveMesh();

        pgm->BuildFromTMeshVec( m_TMeshVec );

        pgm->MergeCoincidentNodes();
        // pgm->MergeDuplicateEdges();

        pgm->PolygonizeMesh();
        pgm->CleanColinearVerts();
        pgm->RemoveDegenEdges();

        pgm->FindAllDoubleBackNodes();
        pgm->SealDoubleBackNodes();

        new_geom->SplitLEGeom();

        if ( cullfracflag )
        {
            pgm->MakeRegions();
            pgm->ClearTris();
            pgm->CullOrphanThinRegions( cullfrac );
        }

        pgm->RemoveDegenFaces();

        pgm->IdentifyWingWakes();
        if ( FindBodyWakes )
        {
            pgm->IdentifyBodyWakes();
            pgm->IdentifyBodyNodeWakes();
        }

        PGMesh *pgm_prev = pgm;
        for ( int iref = 0; iref < n_ref; iref++ )
        {
            PGMesh *pgmi = new_geom->m_PGMulti.AddMesh();;

            pgmi->BuildFromPGMesh( pgm_prev );

            pgmi->Coarsen1();
            pgmi->Coarsen2();

            pgmi->CleanUnused();

            pgmi->DumpGarbage();

            pgmi->IdentifyParents();

            pgmi->IdentifyWingWakes();
            if ( FindBodyWakes )
            {
                pgmi->IdentifyBodyWakes();
                pgmi->IdentifyBodyNodeWakes();
            }

            pgm_prev = pgmi;
        }

        new_geom->m_SurfDirty = true;

        new_geom->Update();
        Vehicle::UpdateGUI();

    }
    return id;
}

void MeshGeom::Scale()
{
    double currentScale = m_Scale() / m_LastScale();
    m_ScaleFromOrig *= currentScale;
    m_LastScale = m_Scale();
}

void MeshGeom::ApplyScale()
{
    if ( std::abs( m_LastScale() - m_Scale() ) < 0.0000001 )
    {
        return;
    }

    ::ApplyScale( m_Scale() / m_LastScale(), m_TMeshVec );

    m_LastScale = m_Scale();
}

void MeshGeom::IntersectTrim( vector< DegenGeom > &degenGeom, bool degen, int intSubsFlag, bool halfFlag, const vector < string > & sub_vec )
{
    bool deleteopen = false;

    //==== Scale To 10 Units ====//
    UpdateBBox();
    m_LastScale = 1.0;
    m_Scale = 1000.0 / m_BBox.GetLargestDist();
    ApplyScale();

    MeshInfo info;
    Results *res = nullptr;
    if ( !degen )
    {
        //==== Create Results ====//
        res = ResultsMgr.CreateResults( "Comp_Geom", "CompGeom CSG mesh generation results." );
        res->Add( new NameValData( "Mesh_GeomID", GetID(), "GeomID of MeshGeom created." ) );
    }

    ::IntersectTrim( m_TMeshVec, m_SubSurfVec, m_BBox, degen, intSubsFlag, halfFlag,
                     deleteopen, sub_vec, res, degenGeom );

    //===== Reset Scale =====//
    m_Scale = 1;
    ApplyScale();
    UpdateBBox();

    if ( !degen )
    {
        //==== Write TXT File ====//
        if ( m_Vehicle->getExportCompGeomTxtFile() )
        {
            string txtfn = m_Vehicle->getExportFileName( vsp::COMP_GEOM_TXT_TYPE );
            res->WriteCompGeomTxtFile( txtfn );
        }

        //==== Write CSV File ====//
        if ( m_Vehicle->getExportCompGeomCsvFile() )
        {
            string csvfn = m_Vehicle->getExportFileName( vsp::COMP_GEOM_CSV_TYPE );
            res->WriteCompGeomCsvFile( csvfn );
        }
    }
}


//==== Call After BndBoxes Have Been Create But Before Intersect ====//
void MeshGeom::AreaSlice( int numSlices, const vec3d &norm_axis,
                          bool autoBounds, double start, double end, bool measureduct )
{
    string filename = m_Vehicle->getExportFileName( vsp::SLICE_TXT_TYPE );

    //==== Create Results ====//
    Results* res = ResultsMgr.CreateResults( "Slice", "Planar slicing results." );

    ::AreaSlice( m_TMeshVec, m_SliceVec, m_BBox, numSlices, norm_axis, autoBounds, start, end, measureduct, res );

    res->Add( new NameValData( "Mesh_GeomID", GetID(), "GeomID of MeshGeom created." ) );

    res->WriteSliceFile( filename );
}

void MeshGeom::WaveStartEnd( const double &sliceAngle, const vec3d &center )
{
    int ntheta = WaveDragMgr.m_NTheta;

    double Mach = 1/sin( sliceAngle );
    double beta = sqrt( Mach*Mach - 1.0 );

    for ( int itheta = 0; itheta < ntheta; itheta++ )
    {
        WaveDragMgr.m_StartX[itheta] = DBL_MAX;
        WaveDragMgr.m_EndX[itheta] = -DBL_MAX;

        double theta = WaveDragMgr.m_ThetaRad[itheta];

        for ( int i = 0 ; i < ( int )m_TMeshVec.size() ; i++ )
        {
            for ( int j = 0 ; j < ( int )m_TMeshVec[i]->m_TVec.size() ; j++ )
            {
                for ( int k = 0; k < 3; k++ )
                {
                    vec3d pt = m_TMeshVec[i]->m_TVec[j]->GetTriNode(k)->m_Pnt;

                    double xcon = beta * cos( theta - 0.5 * M_PI ) * ( pt.y() - center.y() ) + beta * sin( theta + 0.5 * M_PI ) * ( pt.z() - center.z() );
                    double xwave = pt.x() - xcon;
                    if ( xwave < WaveDragMgr.m_StartX[itheta] )
                    {
                        WaveDragMgr.m_StartX[itheta] = xwave;
                    }
                    if ( xwave > WaveDragMgr.m_EndX[itheta] )
                    {
                        WaveDragMgr.m_EndX[itheta] = xwave;
                    }
                }
            }
        }

        // Offset slightly to ensure good intersection when aircraft face lies on cutting plane
        WaveDragMgr.m_StartX[itheta] += 1e-3;
        WaveDragMgr.m_EndX[itheta] -= 1e-3;
    }
}


//==== Call After BndBoxes Have Been Created But Before Intersect ====//
void MeshGeom::WaveDragSlice( int numSlices, double sliceAngle, int coneSections,
                           const vector <string> & Flow_vec, bool Symm )
{
    //==== Update Bnd Box ====//
    UpdateBBox();

    // Get y, z dimensions of bounding box
    vec3d center = m_BBox.GetCenter();
    // Create a length 2% longer than longest bounding box side
    double size = m_BBox.GetLargestDist() * 1.02;

    // Set up theta angles around the cone.
    WaveDragMgr.SetupTheta( coneSections );

    //==== "Smart" Start/End Slice and Tube Routine ====//
    // Uses Mach and theta angles with aircraft nodes to determine start/end locations for slicing and
    // flow-through tube extensions

    WaveStartEnd( sliceAngle, center );

    // Emphasize limited scope of intermediate calculations.
    double tubestart, tubeend;
    {
        // Set global start/end slice locations
        double startX_global = *(min_element( WaveDragMgr.m_StartX.begin(), WaveDragMgr.m_StartX.end() ));
        double endX_global = *(max_element( WaveDragMgr.m_EndX.begin(), WaveDragMgr.m_EndX.end() ));

        // Set flow-through extension start/end locations
        double tubedist = 0.5 * size * sin( 0.5*M_PI - sliceAngle) / sin( sliceAngle );
        tubestart = startX_global - tubedist * 1.1;
        tubeend = endX_global + tubedist * 1.1;
    }

    //==== Flow-Through Accommodation Routine ====//
    // Extends designated subsurfaces outside slicing range (flow-through stream tubes)

    // Make TMesh* vector for the tubes connecting translated subsurfaces to their parent components
    vector< TMesh* > ss_tube_meshes;

    // Indexes of m_TMeshVec that will be used in merging tube extensions
    vector< int > mergeindex;

    vector<string> InletSS_vec;
    vector<string> ExitSS_vec;

    int ambcount = 0;
    WaveDragMgr.m_AmbigSubSurf = false;

    int iQuad = 0;
    for ( int i = 0 ; i < ( int )m_TMeshVec.size() ; i++ )
    {
        // Get vector of all subsurface pointers in current TMesh
        vector< SubSurface* > sub_surf_vec = SubSurfaceMgr.GetSubSurfs( m_TMeshVec[i]->m_OriginGeomID, m_TMeshVec[i]->m_SurfNum );

        for ( int ssv = 0 ; ssv < ( int )sub_surf_vec.size(); ssv++ )
        {
            vector < double > uvec;
            vector < double > vvec;
            if ( m_TMeshVec[i]->m_UWPnts.size() > 0 && m_TMeshVec[i]->m_UWPnts[0].size() > 0 )
            {
                uvec.resize( m_TMeshVec[i]->m_UWPnts.size() );
                for ( int j = 0 ; j < ( int )m_TMeshVec[i]->m_UWPnts.size(); j++ )
                {
                    uvec[j] = m_TMeshVec[i]->m_UWPnts[j][0].x();
                }
                vvec.resize( m_TMeshVec[i]->m_UWPnts[0].size() );
                for ( int j = 0; j < ( int ) m_TMeshVec[ i ]->m_UWPnts[ 0 ].size(); j++ )
                {
                    vvec[j] = m_TMeshVec[ i ]->m_UWPnts[ 0 ][ j ].y();
                }
            }

            // Populate vector of TMesh* for current subsurface
            vector< TMesh* > sub_surf_meshes;
            string subsurf_id = sub_surf_vec[ssv]->GetID();
            if ( vector_contains_val( Flow_vec, subsurf_id ) )
            {
                vector< TMesh* > tmp_vec = sub_surf_vec[ssv]->CreateTMeshVec( uvec, vvec );
                sub_surf_meshes.insert( sub_surf_meshes.end(), tmp_vec.begin(), tmp_vec.end() );
            }

            if ( !sub_surf_meshes.size() )
            {
                continue;
            }

            // Create tube extension location
            double tloc = 0;
            bool ambflag = false;
            for ( int ssm = 0 ; ssm < ( int )sub_surf_meshes.size() ; ssm++ )
            {
                // Build merge maps
                m_TMeshVec[i]->BuildMergeMaps();

                // Load sub_surf_meshes[ss]'s bbox
                sub_surf_meshes[ssm]->LoadBndBox();

                // Swap the m_TMeshVec[i]'s nodes to be UW instead of xyz
                m_TMeshVec[i]->MakeNodePntUW();
                m_TMeshVec[i]->LoadBndBox();

                // Intersect TMesh with sub_surface_meshes[ss]
                m_TMeshVec[i]->Intersect( sub_surf_meshes[ssm], true );

                // Split the triangles
                m_TMeshVec[i]->Split();

                // Make current m_TMeshVec[i] XYZ again and reset its octtree
                m_TMeshVec[i]->MakeNodePntXYZ();
                m_TMeshVec[i]->m_TBox.Reset();

                // Get and count negative and positive norm x values for all tris in current subsurface
                int negnorm = 0;
                int posnorm = 0;
                for( int t = 0; t < m_TMeshVec[i]->m_TVec.size(); t++ )
                {
                    if ( sub_surf_vec[ssv]->Subtag( m_TMeshVec[i]->m_TVec[t] ) )
                    {
                        double normcheck = m_TMeshVec[i]->m_TVec[t]->m_Norm.x();
                        if ( normcheck < 0 )
                        {
                            negnorm += 1;
                        }
                        else if ( normcheck > 0 )
                        {
                            posnorm += 1;
                        }
                    }
                }

                // Check negative/positive x norm counts and set tube extension direction
                if ( negnorm > 0 && posnorm > 0 )
                {
                    // Subsurf mesh direction is ambiguous. Set flag to true, advance count.
                    ambflag = true;
                    ambcount += 1;
                }
                else if ( negnorm > 0 )
                {
                    tloc = tubestart;
                }
                else if ( posnorm > 0 )
                {
                    tloc = tubeend;
                }

                if ( !ambflag ) // If subsurf mesh is NOT ambiguous, build tube
                {
                    // Build extension tube
                    TMesh* tempmesh = new TMesh();
                    ss_tube_meshes.push_back( tempmesh );
                    for ( int n = 0; n < sub_surf_meshes[ssm]->m_TVec.size(); n++ )
                    {
                        // For all nodes along the subsurface intersection
                        TTri* currtri = sub_surf_meshes[ssm]->m_TVec[n];
                        for ( int nn = 0; nn < currtri->m_ISectEdgeVec.size(); nn++ )
                        {
                            // Build nodes and norm for first triangle at this intersection edge
                            vec3d tt0_N0( currtri->m_ISectEdgeVec[nn]->m_N0->GetXYZPnt() );
                            vec3d tt0_N1( currtri->m_ISectEdgeVec[nn]->m_N1->GetXYZPnt() );
                            vec3d tt0_N2( tt0_N0 );
                            tt0_N2.set_x( tloc );
                            vec3d tt0_norm = cross( tt0_N2-tt0_N1, tt0_N0-tt0_N1 );
                            tt0_norm.normalize();

                            // Build nodes and norm for second triangle at this intersection edge
                            vec3d tt1_N0( tt0_N2 );
                            vec3d tt1_N1( tt0_N1 );
                            tt1_N1.set_x( tloc );
                            vec3d tt1_N2( tt0_N1 );
                            vec3d tt1_norm = cross( tt1_N0-tt1_N1, tt1_N2-tt1_N1 );
                            tt1_norm.normalize();

                            // Add the triangles to the mesh
                            tempmesh->AddTri( tt0_N0, tt0_N1, tt0_N2, tt0_norm, iQuad );
                            tempmesh->AddTri( tt1_N0, tt1_N1, tt1_N2, tt1_norm, iQuad );
                            iQuad++;
                        }
                    }
                    mergeindex.push_back( i );
                }

                // Flatten mesh
                TMesh* f_tmesh = new TMesh();
                f_tmesh->CopyFlatten( m_TMeshVec[i] );
                delete m_TMeshVec[i];
                m_TMeshVec[i] = f_tmesh;
            }

            // Pushes current subsurface pointer to either inlet or exit string vector
            if ( tloc == tubestart && !ambflag )
            {
                InletSS_vec.push_back( sub_surf_vec[ssv]->GetID() );
            }
            else if ( tloc == tubeend && !ambflag )
            {
                ExitSS_vec.push_back( sub_surf_vec[ssv]->GetID() );
            }
        }
    }
    // Tag meshes before regular intersection
    SubTagTris( true, m_TMeshVec );

    if ( ambcount > 0 )
    {
        WaveDragMgr.m_AmbigSubSurf = true;
    }


    // Create vectors of triangle tags for inlets and exits
    vector<int> fwd_ss_tags;
    vector<int> bwd_ss_tags;
    vector< SubSurface* > subsurf_vec = SubSurfaceMgr.GetSubSurfs();
    for ( int i = 0; i < subsurf_vec.size(); i++ )
    {
        if ( vector_contains_val( InletSS_vec, subsurf_vec[i]->GetID() ) )
        {
            fwd_ss_tags.push_back( i + 1 + m_TMeshVec.size() );
        }
        else if ( vector_contains_val( ExitSS_vec, subsurf_vec[i]->GetID() ) )
        {
            bwd_ss_tags.push_back( i + 1 + m_TMeshVec.size() );
        }
    }

    int translated = 0;
    // Translate all flow-through triangles
    for ( int i = 0; i < m_TMeshVec.size(); i ++ )
    {
        for ( int j = 0; j < m_TMeshVec[i]->m_TVec.size(); j++ )
        {
            for ( int k = 0; k < m_TMeshVec[i]->m_TVec[j]->m_Tags.size(); k++ )
            {
                int tag_num = m_TMeshVec[i]->m_TVec[j]->m_Tags[k];
                bool fwd_test = vector_contains_val( fwd_ss_tags, tag_num );
                bool bwd_test = vector_contains_val( bwd_ss_tags, tag_num );
                if ( fwd_test )
                {
                    translated += 1;
                    // Translate the subsurface forward
                    m_TMeshVec[i]->m_TVec[j]->m_N0->m_Pnt.set_x( tubestart );
                    m_TMeshVec[i]->m_TVec[j]->m_N1->m_Pnt.set_x( tubestart );
                    m_TMeshVec[i]->m_TVec[j]->m_N2->m_Pnt.set_x( tubestart );
                }
                else if ( bwd_test )
                {
                    // Translate the subsurface backward
                    m_TMeshVec[i]->m_TVec[j]->m_N0->m_Pnt.set_x( tubeend );
                    m_TMeshVec[i]->m_TVec[j]->m_N1->m_Pnt.set_x( tubeend );
                    m_TMeshVec[i]->m_TVec[j]->m_N2->m_Pnt.set_x( tubeend );
                }
            }
        }
    }

    //==== Count Components ====//
    vector< string > compIdVec;
    for ( int i = 0 ; i < ( int )m_TMeshVec.size() ; i++ )
    {
        string id = m_TMeshVec[i]->m_OriginGeomID;
        vector<string>::iterator iter;

        iter = find( compIdVec.begin(), compIdVec.end(), id );

        if ( iter == compIdVec.end() )
        {
            compIdVec.push_back( id );
        }
    }

    WaveDragMgr.Setup( numSlices, compIdVec.size() );

    WaveDragMgr.m_CompIDVec = compIdVec;

    // Merge tubes with their corresponding components
    for ( int i = 0 ; i < mergeindex.size() ; i++ )
    {
        m_TMeshVec[ mergeindex[i] ]->MergeTMeshes( ss_tube_meshes[i] );
        delete ss_tube_meshes[i];
    }

    // Merge and remove any remaining open meshes
    MeshInfo info;
    MergeRemoveOpenMeshes( m_TMeshVec, &info, true );

    //==== Create Bnd Box for Mesh Geoms ====//
    for ( int i = 0 ; i < ( int )m_TMeshVec.size() ; i++ )
    {
        m_TMeshVec[i]->LoadBndBox();
    }

    //==== Update mesh Bnd Box to include streamtube extensions ====//
    m_BBox.Reset();
    for ( int i = 0 ; i < ( int )m_TMeshVec.size() ; i++ )
    {
        m_BBox.Update( m_TMeshVec[i]->m_TBox.m_Box );
    }

    //==== Intersect All Mesh Geoms (before slicing) ====//
    for ( int i = 0 ; i < ( int )m_TMeshVec.size() ; i++ )
    {
        for ( int j = i + 1 ; j < ( int )m_TMeshVec.size() ; j++ )
        {
            m_TMeshVec[i]->Intersect( m_TMeshVec[j] );
        }
    }

    //==== Split Intersected Tri in Mesh ====//
    for ( int i = 0 ; i < ( int )m_TMeshVec.size() ; i++ )
    {
        m_TMeshVec[i]->Split();
    }

    WaveDragMgr.m_XNorm.resize( numSlices );
    for ( int islice = 0 ; islice < numSlices ; islice++ )
    {
        WaveDragMgr.m_XNorm[islice] = ( ( double )islice / ( double )( numSlices - 1 ) );
    }

    //==== Build Slice Mesh Object =====//
    for ( int islice = 0 ; islice < numSlices ; islice++ )
    {
        // For number of rotation sections, rotate  slices about x-axis
        for ( int itheta = 0; itheta < coneSections; itheta++ )
        {
            TMesh* tm = new TMesh();
            m_SliceVec.push_back( tm );

            tm->m_ThickSurf = false;
            tm->m_SurfCfdType = vsp::CFD_STRUCTURE;

            // Location of theta slices on x-axis
            double xcenter = WaveDragMgr.m_StartX[itheta] + WaveDragMgr.m_XNorm[islice] * ( WaveDragMgr.m_EndX[itheta] - WaveDragMgr.m_StartX[itheta] );

            // Current theta (checks for XZ symmetry option)
            double theta = WaveDragMgr.m_ThetaRad[itheta];

            // Establish generic plane
            const unsigned int ngp = 4;
            vector< vec3d > gp(ngp);
            gp[0].set_xyz( 0,  2.5*size,  2.5*size );
            gp[1].set_xyz( 0, -2.5*size,  2.5*size );
            gp[2].set_xyz( 0, -2.5*size, -2.5*size );
            gp[3].set_xyz( 0,  2.5*size, -2.5*size );

            for ( int m = 0; m < ngp; m++ )
            {
                // Rotate plane to Mach angle
                gp[m].rotate_y( -( 0.5 * M_PI - sliceAngle ) );
                // Rotate plane to current theta
                gp[m].rotate_x( theta );
                // Translate plane to current x
                gp[m].offset_x( xcenter );
                // Relocate to center YZ
                gp[m].offset_y( center.y() );
                gp[m].offset_z( center.z() );
            }

            // Get plane normal
            vec3d gpnorm = cross( gp[3]-gp[2], gp[1]-gp[2] );
            gpnorm.normalize();

            // Build triangles
            tm->AddTri( gp[ 2 ], gp[ 3 ], gp[ 0 ], gpnorm, iQuad );
            tm->AddTri( gp[ 2 ], gp[ 0 ], gp[ 1 ], gpnorm, iQuad );
            iQuad++;
        }
    }

    // Tube Slice: Start/End Locations
    vector<double> tubeslicesX;
    if ( Flow_vec.size() > 0 ) // Tubes exist, slice just inside them
    {
        tubeslicesX.push_back( tubestart + 0.001 );
        tubeslicesX.push_back( tubeend - 0.001 );
    }
    else // Tubes don't exist, slice just outside the aircraft
    {
        tubeslicesX.push_back( tubestart - 0.001 );
        tubeslicesX.push_back( tubeend + 0.001 );
    }

    //==== Build Tube Slice Mesh Objects =====//
    for ( int itube = 0 ; itube < tubeslicesX.size() ; itube++ )
    {
        TMesh* tm = new TMesh();
        m_SliceVec.push_back( tm );

        tm->m_ThickSurf = false;
        tm->m_SurfCfdType = vsp::CFD_STRUCTURE;

        // Location of theta slices on x-axis
        double xcenter = (double)tubeslicesX[itube];

        // Establish generic plane
        const unsigned int ngp = 4;
        vector< vec3d > gp(ngp);
        gp[0].set_xyz( 0,  2.5*size,  2.5*size );
        gp[1].set_xyz( 0, -2.5*size,  2.5*size );
        gp[2].set_xyz( 0, -2.5*size, -2.5*size );
        gp[3].set_xyz( 0,  2.5*size, -2.5*size );

        // Get plane normal
        vec3d gpnorm = cross( gp[3]-gp[2], gp[1]-gp[2] );
        gpnorm.normalize();

        // Move plane to current slice section location
        for ( int m = 0; m < ngp; m++ )
        {
            // Translate plane to current x
            gp[m].offset_x( xcenter );
            // Relocate to center YZ
            gp[m].offset_y( center.y() );
            gp[m].offset_z( center.z() );
        }

        // Build triangles
        tm->AddTri( gp[ 2 ], gp[ 3 ], gp[ 0 ], gpnorm, iQuad );
        tm->AddTri( gp[ 2 ], gp[ 0 ], gp[ 1 ], gpnorm, iQuad );
        iQuad++;
    }

    // Fill vector of cfdtypes so we don't have to pass TMeshVec all the way down.
    vector < int > bTypes( m_TMeshVec.size() );
    vector < bool > thicksurf( m_TMeshVec.size() );
    for ( int i = 0 ; i < ( int )m_TMeshVec.size() ; i++ )
    {
        bTypes[i] = m_TMeshVec[i]->m_SurfCfdType;
        thicksurf[i] = m_TMeshVec[i]->m_ThickSurf;
    }

    //==== Load Bnding Box ====//
    for ( int islice = 0 ; islice < ( int )m_SliceVec.size() ; islice++ )
    {
        TMesh* tm = m_SliceVec[islice];
        tm->LoadBndBox();

        //==== Intersect All Mesh Geoms ====//
        for ( int i = 0 ; i < ( int )m_TMeshVec.size() ; i++ )
        {
            tm->Intersect( m_TMeshVec[i] );

            m_TMeshVec[i]->RemoveIsectEdges();
        }

        //==== Split Intersected Tri in Mesh ====//
        tm->Split();

        //==== Determine Which Triangles Are Interior/Exterior ====//
        tm->DeterIntExt( m_TMeshVec );

        //==== Mark which triangles to ignore ====//
        tm->SetIgnoreTriFlag( bTypes, thicksurf );
    }

    //==== Pushback slice and area results ====//
    // Make ID lookup map.
    std::unordered_map< string, int > compIdMap;
    for ( int icomp = 0; icomp < compIdVec.size(); icomp++ )
    {
        compIdMap[ compIdVec[icomp] ] = icomp;
    }

    m_SliceVec[numSlices*coneSections]->m_CompAreaVec.resize( compIdVec.size() );
    double inA = m_SliceVec[numSlices*coneSections]->ComputeWaveDragArea( compIdMap );
    WaveDragMgr.m_InletArea = inA;

    m_SliceVec[numSlices*coneSections+1]->m_CompAreaVec.resize( compIdVec.size() );
    double exA = m_SliceVec[numSlices*coneSections+1]->ComputeWaveDragArea( compIdMap );
    WaveDragMgr.m_ExitArea = exA;


    for ( int islice = 0 ; islice < numSlices ; islice++ )
    {
        for ( int itheta = 0; itheta < coneSections; itheta++ )
        {
            int sindex = ( int )( islice * coneSections + itheta );

            m_SliceVec[sindex]->m_CompAreaVec.resize( compIdVec.size() );
            m_SliceVec[sindex]->ComputeWaveDragArea( compIdMap );

            for ( int icomp = 0; icomp < compIdVec.size(); icomp++ )
            {
                WaveDragMgr.m_CompSliceAreaDist[itheta][icomp][islice]= m_SliceVec[sindex]->m_CompAreaVec[icomp];
            }
            WaveDragMgr.m_SliceAreaDist[itheta][islice] = m_SliceVec[sindex]->m_WetArea;
        }
    }
}

//==== Call After BndBoxes Have Been Create But Before Intersect ====//
void MeshGeom::MassSlice( vector < DegenGeom > &degenGeom, bool degen, int numSlices, int idir, bool writefile,
                          double &totalMass, vec3d &centerOfGrav, vec3d &IxxIyyIzz, vec3d &IxyIxzIyz )
{
    string filename;
    if ( writefile )
    {
        filename = m_Vehicle->getExportFileName( vsp::MASS_PROP_TXT_TYPE );
    }

    //==== Create Results ====//
    Results *res = nullptr;

    if ( !degen )
    {
        res = ResultsMgr.CreateResults( "Mass_Properties", "Mass properties results." );
        res->Add( new NameValData( "Mesh_GeomID", GetID(), "GeomID of MeshGeom created." ) );
    }

    ::MassSlice( m_TMeshVec, m_SliceVec, m_BBox, degenGeom, degen, numSlices, idir, totalMass,
                 centerOfGrav, IxxIyyIzz, IxyIxzIyz, m_PointMassVec, res );

    if ( !degen )
    {
        if ( writefile )
        {
            res->WriteMassProp( filename );
        }
    }
}


//==== Check Current Geom For Problems ====//
void MeshGeom::WaterTightCheck( FILE* fid )
{
    int i, t, m;

    if ( !m_TMeshVec.size() )
    {
        return;
    }

    //==== Compute Min Edge Size Before Split ====//
    double minEdgeLen =  1.0e06;
    double minTriAng  =  1.0e06;
    double maxTriAng  = -1.0e06;
    for ( m = 0 ; m < ( int )m_TMeshVec.size() ; m++ )
    {
        double minE, minA, maxA;
        m_TMeshVec[m]->MeshStats( &minE, &minA, &maxA );
        if ( minE < minEdgeLen )
        {
            minEdgeLen = minE;
        }
        if ( minA < minTriAng )
        {
            minTriAng = minA;
        }
        if ( maxA > maxTriAng )
        {
            maxTriAng = maxA;
        }
    }

    //==== Load All Meshes into One ====//
    TMesh* oneMesh = new TMesh();

    for ( m = 0 ; m < ( int )m_TMeshVec.size() ; m++ )
    {
        TMesh* mesh = m_TMeshVec[m];
        for ( t = 0 ; t < ( int )mesh->m_TVec.size() ; t++ )
        {
            if ( mesh->m_TVec[t]->m_SplitVec.size() )
            {
                for ( i = 0 ; i < ( int )mesh->m_TVec[t]->m_SplitVec.size() ; i++ )
                {
                    if ( !mesh->m_TVec[t]->m_SplitVec[i]->m_IgnoreTriFlag )
                    {
                        TTri* tri = mesh->m_TVec[t]->m_SplitVec[i];
                        oneMesh->AddTri( tri->m_N0, tri->m_N1, tri->m_N2, mesh->m_TVec[t]->m_Norm, mesh->m_TVec[t]->m_iQuad, mesh->m_TVec[t]->m_jref, mesh->m_TVec[t]->m_kref );
                    }
                }
            }
            else if ( !mesh->m_TVec[t]->m_IgnoreTriFlag )
            {
                TTri* tri = mesh->m_TVec[t];
                oneMesh->AddTri( tri->m_N0, tri->m_N1, tri->m_N2, tri->m_Norm, tri->m_iQuad, tri->m_jref, tri->m_kref );
            }
        }
    }

    //==== Bound Box with Oct Tree ====//
    oneMesh->LoadBndBox();

    oneMesh->WaterTightCheck( fid, m_TMeshVec );

    //==== Delete Old Meshes and Add One Mesh ====//
    for ( i = 0 ; i < ( int )m_TMeshVec.size() ; i++ )
    {
        delete m_TMeshVec[i];
    }

    m_TMeshVec.clear();
    m_TMeshVec.push_back( oneMesh );
}

void MeshGeom::CreateDegenGeom( vector<DegenGeom> &dgs, bool preview, const int & n_ref )
{
    unsigned int num_meshes = m_TMeshVec.size();

    dgs.resize( num_meshes );

    for ( int i = 0; i < num_meshes; i++ )
    {
        DegenGeom &degenGeom = dgs[i];

        degenGeom.setType( DegenGeom::MESH_TYPE );

        degenGeom.setParentGeom( this );
        degenGeom.setSurfNum( i );
        degenGeom.setFlipNormal( false );
        degenGeom.setMainSurfInd( 0 );
        degenGeom.setSymCopyInd( 0 );

        Matrix4d trans = GetTotalTransMat();

        vector < double > tmatvec( 16 );
        for ( int j = 0; j < 16; j++ )
        {
            tmatvec[ j ] = trans.data()[ j ];
        }
        degenGeom.setTransMat( tmatvec );

        degenGeom.setNumXSecs( 0 );
        degenGeom.setNumPnts( 0 );
        degenGeom.setName( GetName() );
    }
}

vector<TMesh*> MeshGeom::CreateTMeshVec( bool skipnegflipnormal, const int & n_ref ) const
{
    vector<TMesh*> retTMeshVec;
    retTMeshVec.resize( m_TMeshVec.size() );
    for ( int i = 0 ; i < ( int )m_TMeshVec.size() ; i++ )
    {
        retTMeshVec[i] = new TMesh();
        retTMeshVec[i]->copy( m_TMeshVec[i] );
        retTMeshVec[i]->LoadGeomAttributes( this );
    }

    // Apply Transformations
    Matrix4d TransMat = GetTotalTransMat();
    TransformMeshVec( retTMeshVec, TransMat );

    return retTMeshVec;
}

//==== Get Total Transformation Matrix from Original Points ====//
Matrix4d MeshGeom::GetTotalTransMat() const
{
    Matrix4d retMat;
    retMat.initMat( m_ScaleMatrix );
    retMat.postMult( m_ModelMatrix );

    return retMat;
}


