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

#include "MeshGeom.h"
#include "ParmMgr.h"
#include "LinkMgr.h"
#include "VspSurf.h"
#include "Vehicle.h"
#include "PntNodeMerge.h"
#include "APIDefines.h"

#include "Defines.h"
#include "Tritri.h"
#include "BndBox.h"
#include "StringUtil.h"

#include "SubSurfaceMgr.h"
#include <set>
#include <map>

//==== Constructor =====//
MeshGeom::MeshGeom( Vehicle* vehicle_ptr ) : Geom( vehicle_ptr )
{
    m_Name = "MeshGeom";
    m_Type.m_Name = "Mesh";
    m_Type.m_Type = MESH_GEOM_TYPE;
    m_MeshType = INTERSECTION_MESH;
    m_FileName = "";

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

    m_TotalTheoArea = m_TotalWetArea = 0.0;
    m_TotalTheoVol  = m_TotalWetVol  = 0.0;

    m_MassPropFlag = 0;
    m_CenterOfGrav = vec3d( 0, 0, 0 );
    m_TotalMass = 0.0;
    m_TotalIxx = 0.0;
    m_TotalIyy = 0.0;
    m_TotalIzz = 0.0;
    m_TotalIxy = 0.0;
    m_TotalIxz = 0.0;
    m_TotalIyz = 0.0;

    m_MinTriDen = 0.0;
    m_MaxTriDen = 1.0;

    m_MeshFlag = 0;
    m_OneMesh  = 0;

    m_ScaleMatrix.loadIdentity();
    m_ScaleFromOrig.Init( "Scale_From_Original", "XForm", this, 1, 1.0e-5, 1.0e12, false );
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
    xmlNodePtr mesh_node = xmlNewChild( node, NULL, BAD_CAST "MeshGeom", NULL );
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
        int numMeshes = 0;
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
    int i, j, c;
    FILE *fp;
    char str[256];

    //==== Make Sure File Exists ====//
    if ( ( fp = fopen( file_name, "r" ) ) == ( FILE * )NULL )
    {
        return 0;
    }

    //==== Read first Line of file and compare against expected header ====//
    fscanf( fp, "%s INPUT FILE\n\n", str );
    if ( strcmp( "HERMITE", str ) != 0 )
    {
        return 0;
    }
    //==== Read in number of components ====//
    int num_comps;
    fscanf( fp, " NUMBER OF COMPONENTS = %d\n", &num_comps );

    if ( num_comps <= 0 )
    {
        return 0;
    }

    TMesh*  tMesh = new TMesh();
    for ( c = 0 ; c < num_comps ; c++ )
    {
        char name_str[256];
        float x, y, z;
        int group_num, type;
        int num_pnts, num_cross;
        vector < vector < vec3d > > crossVec;

        fgets( name_str, 256, fp );
        fscanf( fp, " GROUP NUMBER = %d\n", &group_num );
        fscanf( fp, " TYPE = %d\n", &type );
        fscanf( fp, " CROSS SECTIONS = %d\n", &( num_cross ) );
        fscanf( fp, " PTS/CROSS SECTION = %d\n", &( num_pnts ) );

        //===== Size Cross Vec ====//
        crossVec.resize( num_cross );
        for ( i = 0 ; i < num_cross ; i++ )
        {
            crossVec[i].resize( num_pnts );
            for ( j = 0 ; j < num_pnts ; j++ )
            {
                fscanf( fp, "%f %f %f\n", &x, &y, &z );
                crossVec[i][j] = vec3d( x, y, z );
            }
        }

        //==== Convert CrossSections to Triangles ====//
        for ( int i = 1 ; i < ( int )crossVec.size() ; i++ )
            for ( int j = 1 ; j < ( int )crossVec[i].size() ; j++ )
            {
                AddTri( tMesh, crossVec[i - 1][j - 1], crossVec[i][j - 1], crossVec[i][j] );

                AddTri( tMesh, crossVec[i - 1][j - 1], crossVec[i][j], crossVec[i - 1][j] );
            }
    }
    fclose( fp );

    m_TMeshVec.push_back( tMesh );
    UpdateBBox();


    return 1;
}

void MeshGeom::AddTri( TMesh* tMesh, vec3d & p0, vec3d & p1, vec3d & p2 )
{
    double dist_tol = 1.0e-12;

    vec3d v01 = p1 - p0;
    vec3d v02 = p2 - p0;
    vec3d v12 = p2 - p1;

    if ( v01.mag() < dist_tol || v02.mag() < dist_tol || v12.mag() < dist_tol )
    {
        return;
    }

    vec3d norm = cross( v01, v02 );
    norm.normalize();

    tMesh->AddTri( p0, p1, p2, norm );
}



int MeshGeom::ReadSTL( const char* file_name )
{
    m_FileName = file_name;

    FILE* file_id = fopen( file_name, "r" );

    char str[256];
    float nx, ny, nz;
    float v0[3];
    float v1[3];
    float v2[3];
    TTri*  tPtr;
    TMesh*  tMesh = new TMesh();

    if ( file_id )
    {
        //==== Cheesy ASCII/Binary Test ====//
        int binaryFlag = 0;
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

        rewind( file_id );

        if ( !binaryFlag )
        {
            fgets( str, 255, file_id );

            int stopFlag = 0;

            while ( !stopFlag )
            {
                if ( EOF == fscanf( file_id, "%*s %*s %f %f %f\n", &nx, &ny, &nz ) )
                {
                    break;
                }

                if ( EOF == fscanf( file_id, "%*s %*s" ) )
                {
                    break;
                }

                if ( EOF == fscanf( file_id, "%*s %f %f %f\n", &v0[0], &v0[1], &v0[2] ) )
                {
                    break;
                }
                if ( EOF == fscanf( file_id, "%*s %f %f %f\n", &v1[0], &v1[1], &v1[2] ) )
                {
                    break;
                }
                if ( EOF == fscanf( file_id, "%*s %f %f %f\n", &v2[0], &v2[1], &v2[2] ) )
                {
                    break;
                }
                if ( EOF == fscanf( file_id, "%*s" ) )
                {
                    break;
                }
                if ( EOF == fscanf( file_id, "%*s" ) )
                {
                    break;
                }

                //==== Add Valid Facet ====//
                tPtr = new TTri();
                tPtr->m_InteriorFlag = 0;
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
                tPtr = new TTri();
                tPtr->m_InteriorFlag = 0;
                tPtr->m_Norm = vec3d( nx, ny, nz );
                tMesh->m_TVec.push_back( tPtr );

                //==== Put Nodes Into Tri ====//
                tPtr->m_N0 = new TNode();
                tPtr->m_N1 = new TNode();;
                tPtr->m_N2 = new TNode();
                tPtr->m_N0->m_Pnt = vec3d( v0[0], v0[1], v0[2] );
                tPtr->m_N1->m_Pnt = vec3d( v1[0], v1[1], v1[2] );
                tPtr->m_N2->m_Pnt = vec3d( v2[0], v2[1], v2[2] );

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

    for ( m = 0 ; m < ( int )m_TMeshVec.size() ; m++ )
    {
        m_TMeshVec[m]->WriteSTLTris( file_id, GetTotalTransMat() );
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
        tMesh->AddTri( pVec[n0 - 1], pVec[n1 - 1], pVec[n2 - 1], norm );
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
    int num_tris, num_nodes;

    fscanf( file_id, "%d", &num_nodes );
    fscanf( file_id, "%d", &num_tris  );

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
        tMesh->AddTri( pVec[n0 - 1], pVec[n1 - 1], pVec[n2 - 1], norm );
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

//==== Build NASCART Mesh and Save ====//
void MeshGeom::BuildNascartMesh( int partOffset )
{
    int m, s, t;

    m_NascartTriVec.clear();
    m_NascartNodeVec.clear();

    //==== Find All Exterior and Split Tris =====//
    for ( m = 0 ; m < ( int )m_TMeshVec.size() ; m++ )
    {
        for ( t = 0 ; t < ( int )m_TMeshVec[m]->m_TVec.size() ; t++ )
        {
            TTri* tri = m_TMeshVec[m]->m_TVec[t];
            if ( tri->m_SplitVec.size() )
            {
                for ( s = 0 ; s < ( int )tri->m_SplitVec.size() ; s++ )
                {
                    if ( !tri->m_SplitVec[s]->m_InteriorFlag )
                    {
                        char str[80];
                        sprintf( str, "%d", partOffset + m + 1 );
                        tri->m_SplitVec[s]->m_ID = str;
                        m_NascartTriVec.push_back( tri->m_SplitVec[s] );
                    }
                }
            }
            else if ( !tri->m_InteriorFlag )
            {
                char str[80];
                sprintf( str, "%d", partOffset + m + 1 );
                tri->m_ID = str;
                m_NascartTriVec.push_back( tri );
            }
        }
    }

    //==== Collect All Points ====//
    vector< TNode* > allNodeVec;
    for ( t = 0 ; t < ( int )m_NascartTriVec.size() ; t++ )
    {
        m_NascartTriVec[t]->m_N0->m_ID = ( int )allNodeVec.size();
        allNodeVec.push_back( m_NascartTriVec[t]->m_N0 );
        m_NascartTriVec[t]->m_N1->m_ID = ( int )allNodeVec.size();
        allNodeVec.push_back( m_NascartTriVec[t]->m_N1 );
        m_NascartTriVec[t]->m_N2->m_ID = ( int )allNodeVec.size();
        allNodeVec.push_back( m_NascartTriVec[t]->m_N2 );
    }
    vector< vec3d > allPntVec;
    for ( int i = 0 ; i < ( int )allNodeVec.size() ; i++ )
    {
        allPntVec.push_back( allNodeVec[i]->m_Pnt );
    }

    if ( allPntVec.size() == 0 )
    {
        return;
    }

    //==== Build Map ====//
    PntNodeCloud pnCloud;
    pnCloud.AddPntNodes( allPntVec );

    //==== Compute Tol ====//
    BndBox bb = m_Vehicle->GetBndBox();
    double tol = bb.GetLargestDist() * 1.0e-10;

    //==== Use NanoFlann to Find Close Points and Group ====//
    IndexPntNodes( pnCloud, tol );

    //==== Load Used Nodes ====//
    for ( int i = 0 ; i < ( int )allNodeVec.size() ; i++ )
    {
        if ( pnCloud.UsedNode( i ) )
        {
            m_NascartNodeVec.push_back( allNodeVec[i] );
        }
    }

    //==== Set Adjusted Node IDs ====//
    for ( int i = 0 ; i < ( int )allNodeVec.size() ; i++ )
    {
        allNodeVec[i]->m_ID = pnCloud.GetNodeUsedIndex( i );
    }

    //==== Remove Any Bogus Tris ====//
    vector< TTri* > goodTriVec;

    //==== Write Out Tris ====//
    for ( t = 0 ; t < ( int )m_NascartTriVec.size() ; t++ )
    {
        TTri* ttri = m_NascartTriVec[t];
        if ( ttri->m_N0->m_ID != ttri->m_N1->m_ID &&
                ttri->m_N0->m_ID != ttri->m_N2->m_ID &&
                ttri->m_N1->m_ID != ttri->m_N2->m_ID )
        {
            goodTriVec.push_back( ttri );
        }
    }
    m_NascartTriVec = goodTriVec;

}

void MeshGeom::WriteNascartPnts( FILE* fp )
{
    vec3d v;
    Matrix4d XFormMat = GetTotalTransMat();
    //==== Write Out Nodes ====//
    for ( int i = 0 ; i < ( int )m_NascartNodeVec.size() ; i++ )
    {
        TNode* tnode = m_NascartNodeVec[i];
        // Apply Transformations
        v = XFormMat.xform( tnode->m_Pnt );
        fprintf( fp, "%16.10g %16.10g %16.10g\n", v.x(), v.z(), -v.y() );
    }
}

void MeshGeom::WriteCart3DPnts( FILE* fp )
{
    //==== Write Out Nodes ====//
    vec3d v;
    Matrix4d XFormMat = GetTotalTransMat();
    for ( int i = 0 ; i < ( int )m_NascartNodeVec.size() ; i++ )
    {
        TNode* tnode = m_NascartNodeVec[i];
        // Apply Transformations
        v = XFormMat.xform( tnode->m_Pnt );
        fprintf( fp, "%16.10g %16.10g %16.10g\n", v.x(), v.y(),  v.z() );
    }
}

int MeshGeom::WriteGMshNodes( FILE* fp, int node_offset )
{
    vec3d v;
    Matrix4d XFormMat = GetTotalTransMat();
    for ( int i = 0 ; i < ( int )m_NascartNodeVec.size() ; i++ )
    {
        TNode* tnode = m_NascartNodeVec[i];
        // Apply Transformations
        v = XFormMat.xform( tnode->m_Pnt );
        fprintf( fp, "%d %16.10f %16.10f %16.10f\n", i + node_offset + 1,
                 v.x(), v.y(), v.z() );
    }
    return node_offset + ( int )m_NascartNodeVec.size();
}

int MeshGeom::WriteNascartTris( FILE* fp, int off )
{
    //==== Write Out Tris ====//
    for ( int t = 0 ; t < ( int )m_NascartTriVec.size() ; t++ )
    {
        TTri* ttri = m_NascartTriVec[t];
        fprintf( fp, "%d %d %d %s.0\n", ttri->m_N0->m_ID + 1 + off,  ttri->m_N2->m_ID + 1 + off, ttri->m_N1->m_ID + 1 + off, ttri->m_ID.c_str() );
    }

    return ( off + m_NascartNodeVec.size() );
}

int MeshGeom::WriteCart3DTris( FILE* fp, int off )
{
    //==== Write Out Tris ====//
    for ( int t = 0 ; t < ( int )m_NascartTriVec.size() ; t++ )
    {
        TTri* ttri = m_NascartTriVec[t];
        fprintf( fp, "%d %d %d\n", ttri->m_N0->m_ID + 1 + off,  ttri->m_N1->m_ID + 1 + off, ttri->m_N2->m_ID + 1 + off );
    }

    return ( off + m_NascartNodeVec.size() );
}

int MeshGeom::WriteGMshTris( FILE* fp, int node_offset, int tri_offset )
{
    //==== Write Out Tris ====//
    for ( int t = 0 ; t < ( int )m_NascartTriVec.size() ; t++ )
    {
        TTri* ttri = m_NascartTriVec[t];
        fprintf( fp, "%d 2 0 %d %d %d\n", t + tri_offset + 1,
                 ttri->m_N0->m_ID + 1 + node_offset,  ttri->m_N2->m_ID + 1 + node_offset, ttri->m_N1->m_ID + 1 + node_offset );
    }
    return ( tri_offset + m_NascartTriVec.size() );
}

int MeshGeom::WriteNascartParts( FILE* fp, int off )
{
    //==== Find All Exterior and Split Tris =====//
    for ( int m = 0 ; m < ( int )m_TMeshVec.size() ; m++ )
    {
        fprintf( fp, "%d.0  %s  0\n", off + m, m_TMeshVec[m]->m_NameStr.c_str() );
    }
    return off + m_TMeshVec.size();
}

int MeshGeom::WriteCart3DParts( FILE* fp, map< vector<int>, int > & tagMap )
{
    //==== Write Component IDs for each Tri =====//
    map< vector<int>, int >::iterator mi;
    for ( int t = 0 ; t < ( int )m_NascartTriVec.size() ; t++ )
    {
        mi = tagMap.find( m_NascartTriVec[t]->m_Tags );
        int tag;

        if ( mi != tagMap.end() )
        {
            tag = mi->second;
        }
        else
        {
            tag = -1;
        }

        fprintf( fp, "%d \n",  tag );
    }
    return 0;
}

void MeshGeom::WritePovRay( FILE* fid, int comp_num )
{
    // Make Sure FlattenTMeshVec has been called first
    string name = GetName();
    StringUtil::chance_space_to_underscore( name );
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
    xmlNodePtr set_node = xmlNewChild( node, NULL, BAD_CAST "IndexedFaceSet", NULL );
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
            vec3d v0, v1, v2, n, d21;

            TTri* tri = m_TMeshVec[i]->m_TVec[j];

            v0 = transMat.xform( tri->m_N0->m_Pnt );
            v1 = transMat.xform( tri->m_N1->m_Pnt );
            v2 = transMat.xform( tri->m_N2->m_Pnt );
            d21 = v2 - v1;

            if ( d21.mag() > 0.000001 )
            {
                sprintf( numstr, "%lf %lf %lf %lf %lf %lf %lf %lf %lf ", v0.x(), v0.y(), v0.z(),
                         v1.x(), v1.y(), v1.z(), v2.x(), v2.y(), v2.z() );
                crdstr += numstr;
                sprintf( numstr, "%d %d %d -1 ", offset, offset + 1, offset + 2 );
                offset += 3;
                indstr += numstr;
            }
        }
    }

    xmlSetProp( set_node, BAD_CAST "coordIndex", BAD_CAST indstr.c_str() );

    xmlNodePtr coord_node = xmlNewChild( set_node, NULL, BAD_CAST "Coordinate", BAD_CAST " " );
    xmlSetProp( coord_node, BAD_CAST "point", BAD_CAST crdstr.c_str() );
}

//==== Check if Dupicate Node - if Not Add ====//
void MeshGeom::CheckDupOrAdd( TNode* node, vector< TNode* > & nodeVec )
{
    double tol = 0.00000001;
    int dupFlag = 0;
    for ( int i = 0 ; i < ( int )nodeVec.size() ; i++ )
    {
        TNode* n = nodeVec[i];
        if ( fabs( n->m_Pnt.x() - node->m_Pnt.x() ) < tol )
            if ( fabs( n->m_Pnt.y() - node->m_Pnt.y() ) < tol )
                if ( fabs( n->m_Pnt.z() - node->m_Pnt.z() ) < tol )
                {
                    dupFlag = 1;
                    node->m_ID = i;
                }
        if ( dupFlag )
        {
            break;
        }
    }

    //==== Add To List ====//
    if ( !dupFlag )
    {
        node->m_ID = nodeVec.size();
        nodeVec.push_back( node );
    }
}

//==== Generate Cross Sections =====//
void MeshGeom::UpdateBBox()
{
    int i, j;
    m_BBox.Reset();
    Matrix4d transMat = GetTotalTransMat();
    for ( i = 0 ; i < ( int )m_TMeshVec.size() ; i++ )
    {
        for ( j = 0 ; j < ( int )m_TMeshVec[i]->m_TVec.size() ; j++ )
        {
            m_BBox.Update( transMat.xform( m_TMeshVec[i]->m_TVec[j]->m_N0->m_Pnt ) );
            m_BBox.Update( transMat.xform( m_TMeshVec[i]->m_TVec[j]->m_N1->m_Pnt ) );
            m_BBox.Update( transMat.xform( m_TMeshVec[i]->m_TVec[j]->m_N2->m_Pnt ) );
        }
    }
}

//==== Set up Draw Objects ====//
void MeshGeom::UpdateDrawObj()
{
    m_WireShadeDrawObj_vec.resize( m_TMeshVec.size() + m_SliceVec.size(), DrawObj() );
    Matrix4d transMat = GetTotalTransMat();

    //==== Load m_TMeshVec ====//
    for ( int i = 0; i < ( int )m_TMeshVec.size(); i++ )
    {
        m_WireShadeDrawObj_vec[i].m_PntVec.resize( m_TMeshVec[i]->m_TVec.size() * 3 );
        m_WireShadeDrawObj_vec[i].m_NormVec.resize( m_TMeshVec[i]->m_TVec.size() * 3 );
        int wi = 0;
        for ( int j = 0; j < ( int )m_TMeshVec[i]->m_TVec.size(); j++ )
        {
            TTri * tri = m_TMeshVec[i]->m_TVec[j];
            m_WireShadeDrawObj_vec[i].m_PntVec[wi] = transMat.xform( tri->m_N0->m_Pnt );
            m_WireShadeDrawObj_vec[i].m_PntVec[wi + 1] = transMat.xform( tri->m_N1->m_Pnt );
            m_WireShadeDrawObj_vec[i].m_PntVec[wi + 2] = transMat.xform( tri->m_N2->m_Pnt );
            m_WireShadeDrawObj_vec[i].m_NormVec[wi] = m_WireShadeDrawObj_vec[i].m_NormVec[wi + 1] =
                        m_WireShadeDrawObj_vec[i].m_NormVec[wi + 2] = m_ModelMatrix.xform( tri->m_Norm );

            wi += 3;
        }
        m_WireShadeDrawObj_vec[i].m_GeomChanged = true;
    }

    //==== Load m_SliceVec ====//
    int st = m_TMeshVec.size();
    for ( int i = 0; i < ( int )m_SliceVec.size(); i++ )
    {
        m_WireShadeDrawObj_vec[i + st].m_PntVec.resize( m_SliceVec[i]->m_TVec.size() * 3 );
        m_WireShadeDrawObj_vec[i + st].m_NormVec.resize( m_SliceVec[i]->m_TVec.size() * 3 );
        int wi = 0;
        for ( int j = 0; j < ( int )m_SliceVec[i]->m_TVec.size(); j++ )
        {
            TTri * tri = m_SliceVec[i]->m_TVec[j];
            m_WireShadeDrawObj_vec[i + st].m_PntVec[wi] = transMat.xform( tri->m_N0->m_Pnt );
            m_WireShadeDrawObj_vec[i + st].m_PntVec[wi + 1] = transMat.xform( tri->m_N1->m_Pnt );
            m_WireShadeDrawObj_vec[i + st].m_PntVec[wi + 2] = transMat.xform( tri->m_N2->m_Pnt );
            m_WireShadeDrawObj_vec[i + st].m_NormVec[wi] = m_WireShadeDrawObj_vec[i + st].m_NormVec[wi + 1] =
                        m_WireShadeDrawObj_vec[i + st].m_NormVec[wi + 2] = m_ModelMatrix.xform( tri->m_Norm );

            wi += 3;
        }
        m_WireShadeDrawObj_vec[i + st].m_GeomChanged = true;
    }

    //==== Bounding Box ====//
    m_HighlightDrawObj.m_PntVec = GetBBoxDrawLines();
}

void MeshGeom::LoadDrawObjs( vector< DrawObj* > & draw_obj_vec )
{
    char str[256];

    for ( int i = 0 ; i < ( int )m_WireShadeDrawObj_vec.size() ; i++ )
    {
        // Symmetry drawObjs have same m_ID. Make them unique by adding index
        // at the end of m_ID.
        sprintf( str, "%d", i );
        m_WireShadeDrawObj_vec[i].m_GeomID = m_ID + str;
        m_WireShadeDrawObj_vec[i].m_Visible = !m_GuiDraw.GetNoShowFlag();

        // Set Render Destination to Main VSP Window.
        m_WireShadeDrawObj_vec[i].m_Screen = DrawObj::VSP_MAIN_SCREEN;

        switch( m_GuiDraw.GetDrawType() )
        {
        case GeomGuiDraw::GEOM_DRAW_WIRE:
            m_WireShadeDrawObj_vec[i].m_LineWidth = 1.0;
            m_WireShadeDrawObj_vec[i].m_LineColor = m_GuiDraw.GetWireColor();
            m_WireShadeDrawObj_vec[i].m_Type = DrawObj::VSP_WIRE_TRIS;
            draw_obj_vec.push_back( &m_WireShadeDrawObj_vec[i] );
            break;

        case GeomGuiDraw::GEOM_DRAW_HIDDEN:
            m_WireShadeDrawObj_vec[i].m_Type = DrawObj::VSP_HIDDEN_TRIS;
            draw_obj_vec.push_back( &m_WireShadeDrawObj_vec[i] );
            break;

        case GeomGuiDraw::GEOM_DRAW_SHADE:
            m_WireShadeDrawObj_vec[i].m_Type = DrawObj::VSP_SHADED_TRIS;
            draw_obj_vec.push_back( &m_WireShadeDrawObj_vec[i] );
            break;

            // Does not support Texture Mapping.  Render Shaded instead.
        case GeomGuiDraw::GEOM_DRAW_TEXTURE:
            m_WireShadeDrawObj_vec[i].m_Type = DrawObj::VSP_SHADED_TRIS;
            draw_obj_vec.push_back( &m_WireShadeDrawObj_vec[i] );
            break;
        }
    }
    if ( m_Vehicle->IsGeomActive( m_ID ) )
    {
        m_HighlightDrawObj.m_Screen = DrawObj::VSP_MAIN_SCREEN;
        m_HighlightDrawObj.m_GeomID = "BBOX" + m_ID;
        m_HighlightDrawObj.m_LineWidth = 2.0;
        m_HighlightDrawObj.m_LineColor = vec3d( 1.0, 0., 0.0 );
        m_HighlightDrawObj.m_Type = DrawObj::VSP_LINES;
        draw_obj_vec.push_back( &m_HighlightDrawObj );
    }
}

//==== Create And Load Tris into Results Data Structures ====//
void MeshGeom::CreateGeomResults( Results* res )
{
    //==== Add Index Tris =====//
    if ( m_TMeshVec.size() )
    {
        res->Add( ResData( "Type", vsp::MESH_INDEXED_TRI ) );

        BuildNascartMesh( 0 );

        vector< vec3d > pvec;
        Matrix4d XFormMat = GetTotalTransMat();
        //==== Write Out Nodes ====//
        for ( int i = 0 ; i < ( int )m_NascartNodeVec.size() ; i++ )
        {
            TNode* tnode = m_NascartNodeVec[i];
            pvec.push_back( XFormMat.xform( tnode->m_Pnt ) );
        }
        res->Add( ResData( "Num_Pnts", ( int )m_NascartNodeVec.size() ) );
        res->Add( ResData( "Tri_Pnts", pvec ) );

        //==== Write Out Tris ====//
        vector< int > id0_vec;
        vector< int > id1_vec;
        vector< int > id2_vec;
        for ( int t = 0 ; t < ( int )m_NascartTriVec.size() ; t++ )
        {
            TTri* ttri = m_NascartTriVec[t];

            id0_vec.push_back( ttri->m_N0->m_ID );
            id1_vec.push_back( ttri->m_N1->m_ID );
            id2_vec.push_back( ttri->m_N2->m_ID );
        }
        res->Add( ResData( "Num_Tris", ( int )m_NascartTriVec.size() ) );
        res->Add( ResData( "Tri_Index0", id0_vec ) );
        res->Add( ResData( "Tri_Index1", id1_vec ) );
        res->Add( ResData( "Tri_Index2", id2_vec ) );
    }
    //==== Add Slices =====//
    else if ( m_SliceVec.size() )
    {
        res->Add( ResData( "Type", vsp::MESH_SLICE_TRI ) );

        Matrix4d transMat = GetTotalTransMat();

        //==== Load m_SliceVec ====//
        res->Add( ResData( "Num_Slices", ( int )m_SliceVec.size() ) );
        int st = m_TMeshVec.size();
        for ( int i = 0; i < ( int )m_SliceVec.size(); i++ )
        {
            res->Add( ResData( "Num_Slice_Tris", ( int )( int )m_SliceVec[i]->m_TVec.size() ) );
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
            res->Add( ResData( "Slice_Tris_Pnt_0", slice_tri_n0_vec ) );
            res->Add( ResData( "Slice_Tris_Pnt_1", slice_tri_n1_vec ) );
            res->Add( ResData( "Slice_Tris_Pnt_2", slice_tri_n2_vec ) );
        }
    }


}

//==== Compute And Load Normals ====//
void MeshGeom::load_normals()
{
}

//==== Draw Other Pnts XSecs====//
void MeshGeom::load_hidden_surf()
{
}


void MeshGeom::Scale()
{
    double currentScale = m_Scale() / m_LastScale;
    m_ScaleFromOrig *= currentScale;
    m_ScaleMatrix.loadIdentity();
    m_ScaleMatrix.scale( m_ScaleFromOrig() );
    m_LastScale = m_Scale();
}

void MeshGeom::ApplyScale()
{
    if ( fabs( m_LastScale - m_Scale() ) < 0.0000001 )
    {
        return;
    }

    map< TNode*, int > nodeMap;
    for ( int i = 0 ; i < ( int )m_TMeshVec.size() ; i++ )
    {
        for ( int j = 0 ; j < ( int )m_TMeshVec[i]->m_NVec.size() ; j++ )
        {
            TNode* n = m_TMeshVec[i]->m_NVec[j];
            nodeMap[n] = 1;
//          n->pnt = n->pnt*( scaleFactor()/m_lastScale );
        }
        //==== Split Tris ====//
        for ( int j = 0 ; j < ( int )m_TMeshVec[i]->m_TVec.size() ; j++ )
        {
            TTri* t = m_TMeshVec[i]->m_TVec[j];
            for ( int k = 0 ; k < ( int )t->m_NVec.size() ; k++ )
            {
                TNode* n = t->m_NVec[k];
                nodeMap[n] = 1;
//              n->pnt = n->pnt*( scaleFactor()/m_lastScale );
            }
        }

        for ( int j = 0 ; j < (int)m_TMeshVec[i]->m_XYZPnts.size() ; j++ )
        {
            for ( int k = 0 ; k < (int)m_TMeshVec[i]->m_XYZPnts[j].size() ; k++ )
            {
                m_TMeshVec[i]->m_XYZPnts[j][k] = m_TMeshVec[i]->m_XYZPnts[j][k] * ( m_Scale() / m_LastScale );
            }
        }
    }
    map<TNode*, int >::const_iterator iter;
    for ( iter = nodeMap.begin() ; iter != nodeMap.end() ; iter++ )
    {
        TNode* n = iter->first;
        n->m_Pnt = n->m_Pnt * ( m_Scale() / m_LastScale );
    }

    m_LastScale = m_Scale();
}

void MeshGeom::TransformMeshVec( vector<TMesh*> & meshVec, Matrix4d & TransMat )
{
    // Build Map of nodes
    map< TNode*, int > nodeMap;
    for ( int i = 0 ; i < ( int )meshVec.size() ; i++ )
    {
        for ( int j = 0 ; j < ( int )meshVec[i]->m_NVec.size() ; j++ )
        {
            TNode* n = meshVec[i]->m_NVec[j];
            nodeMap[n] = 1;
        }
        //==== Split Tris ====//
        for ( int j = 0 ; j < ( int )meshVec[i]->m_TVec.size() ; j++ )
        {
            TTri* t = meshVec[i]->m_TVec[j];
            for ( int k = 0 ; k < ( int )t->m_NVec.size() ; k++ )
            {
                TNode* n = t->m_NVec[k];
                nodeMap[n] = 1;
            }
        }
    }

    // Apply Transformation to Nodes
    map<TNode*, int >::const_iterator iter;
    for ( iter = nodeMap.begin() ; iter != nodeMap.end() ; iter++ )
    {
        TNode* n = iter->first;
        n->m_Pnt = TransMat.xform( n->m_Pnt );
    }

    // Apply Transformation to each triangle's normal vector
    for ( int i = 0 ; i < ( int )meshVec.size() ; i ++ )
    {
        for ( int j = 0 ; j < ( int )meshVec[i]->m_TVec.size() ; j++ )
        {
            if ( meshVec[i]->m_TVec[j]->m_SplitVec.size() )
            {
                for ( int t = 0 ; t < ( int ) meshVec[i]->m_TVec[j]->m_SplitVec.size() ; t++ )
                {
                    TTri* tri = meshVec[i]->m_TVec[j]->m_SplitVec[t];
                    tri->m_Norm = TransMat.xform( tri->m_Norm );
                }
            }
            else
            {
                TTri* tri = meshVec[i]->m_TVec[j];
                tri->m_Norm = TransMat.xform( tri->m_Norm );
            }
        }
    }
}

void MeshGeom::IntersectTrim( int meshf, int halfFlag )
{
    int i, j;

    m_MeshFlag = meshf;

    //FILE* fid = fopen(txtfn.c_str(), "w");

    //==== Check For Open Meshes and Merge or Delete Them ====//
    MeshInfo info;


    //==== Count Tris ====//
    int numTris = 0;
    for ( i = 0 ; i < ( int )m_TMeshVec.size() ; i++ )
    {
        if ( !m_TMeshVec[i]->m_HalfBoxFlag )
        {
            numTris += m_TMeshVec[i]->m_TVec.size();
        }
    }

    //==== Count Components ====//
    vector< string > compIdVec;
    for ( i = 0 ; i < ( int )m_TMeshVec.size() ; i++ )
    {
        if ( !m_TMeshVec[i]->m_HalfBoxFlag )
        {
            string id = m_TMeshVec[i]->m_PtrID;
            vector<string>::iterator iter;

            iter = find( compIdVec.begin(), compIdVec.end(), id );

            if ( iter == compIdVec.end() )
            {
                compIdVec.push_back( id );
            }
        }
    }

    //fprintf( fid, "...Comp Geom...\n" );
    //fprintf( fid, "%d Num Comps\n", (int)compIdVec.size() );
    //fprintf( fid, "%d Total Num Meshes\n", (int)m_TMeshVec.size() );
    //fprintf( fid, "%d Total Num Tris\n", numTris );

    //==== Create Results ====//
    Results* res = ResultsMgr.CreateResults( "Comp_Geom" );
    res->Add( ResData( "Num_Comps", ( int )compIdVec.size() ) );
    res->Add( ResData( "Total_Num_Meshes", ( int )m_TMeshVec.size() ) );
    res->Add( ResData( "Total_Num_Tris", numTris ) );


    //==== Scale To 10 Units ====//
    UpdateBBox();
    m_LastScale = 1.0;
    m_Scale = 1000.0 / m_BBox.GetLargestDist();
    ApplyScale();

    //==== Intersect Subsurfaces to make clean lines ====//
    for ( i = 0 ; i < ( int )m_TMeshVec.size() ; i++ )
    {
        vector< TMesh* > sub_surf_meshes;
        vector< SubSurface* > sub_surf_vec = SubSurfaceMgr.GetSubSurfs( m_TMeshVec[i]->m_PtrID );
        int ss;
        for ( ss = 0 ; ss < ( int )sub_surf_vec.size() ; ss++ )
        {
            vector< TMesh* > tmp_vec = sub_surf_vec[ss]->CreateTMeshVec();
            sub_surf_meshes.insert( sub_surf_meshes.end(), tmp_vec.begin(), tmp_vec.end() );
        }
        m_SubSurfVec.insert( m_SubSurfVec.end(), sub_surf_meshes.begin(), sub_surf_meshes.end() );

        if ( !sub_surf_meshes.size() )
        {
            continue;    // Skip if no sub surface meshes
        }

        // Load All surf_mesh_bboxes
        for ( ss = 0 ; ss < ( int )sub_surf_meshes.size() ; ss++ )
        {
            // Build merge maps
            m_TMeshVec[i]->BuildMergeMaps();

            sub_surf_meshes[ss]->LoadBndBox();
            // Swap the m_TMeshVec[i]'s nodes to be UW instead of xyz
            m_TMeshVec[i]->MakeNodePntUW();
            m_TMeshVec[i]->LoadBndBox();

            // Intersect TMesh with sub_surface_meshes
            m_TMeshVec[i]->Intersect( sub_surf_meshes[ss], true );

            // Split the triangles
            m_TMeshVec[i]->Split();

            // Make current TMesh XYZ again and reset its octtree
            m_TMeshVec[i]->MakeNodePntXYZ();
            m_TMeshVec[i]->m_TBox.Reset();

            // Flatten Mesh
            TMesh* f_tmesh = new TMesh();
            f_tmesh->CopyFlatten( m_TMeshVec[i] );
            delete m_TMeshVec[i];
            m_TMeshVec[i] = f_tmesh;
        }

        sub_surf_meshes.clear();
    }

    // Tag meshes before regular intersection
    SubTagTris();

    MergeRemoveOpenMeshes( &info );


    //==== Create Bnd Box for  Mesh Geoms ====//
    for ( i = 0 ; i < ( int )m_TMeshVec.size() ; i++ )
    {
        m_TMeshVec[i]->LoadBndBox();
    }

    //==== Update Bnd Box for  Combined ====//
    BndBox b;
    for ( i = 0 ; i < ( int )m_TMeshVec.size() ; i++ )
    {
        b.Update( m_TMeshVec[i]->m_TBox.m_Box );
    }
    m_BBox = b;
    //update_xformed_bbox();            // Load Xform BBox

    //==== Intersect All Mesh Geoms ====//
    for ( i = 0 ; i < ( int )m_TMeshVec.size() ; i++ )
    {
        for ( j = i + 1 ; j < ( int )m_TMeshVec.size() ; j++ )
        {
            m_TMeshVec[i]->Intersect( m_TMeshVec[j] );
        }
    }

    //==== Split Intersected Tri in Mesh ====//
    for ( i = 0 ; i < ( int )m_TMeshVec.size() ; i++ )
    {
        m_TMeshVec[i]->Split( m_MeshFlag );
    }

    //==== Determine Which Triangle Are Interior/Exterior ====//
    for ( i = 0 ; i < ( int )m_TMeshVec.size() ; i++ )
    {
        m_TMeshVec[i]->DeterIntExt( m_TMeshVec );
    }

    if ( halfFlag )
    {
        //==== Remove Half Mesh Box ===//
        vector< TMesh* > tempVec;
        for ( i = 0 ; i < ( int )m_TMeshVec.size() ; i++ )
        {
            if ( !m_TMeshVec[i]->m_HalfBoxFlag )
            {
                tempVec.push_back( m_TMeshVec[i] );
            }
            else
            {
                delete m_TMeshVec[i];
            }
        }
        m_TMeshVec = tempVec;
    }


    //===== Reset Scale =====//
    m_Scale = 1;
    ApplyScale();
    UpdateBBox();

    //==== Compute Areas ====//
    m_TotalTheoArea = m_TotalWetArea = 0.0;
    for ( i = 0 ; i < ( int )m_TMeshVec.size() ; i++ )
    {
        if ( !m_TMeshVec[i]->m_HalfBoxFlag )
        {
            m_TotalTheoArea += m_TMeshVec[i]->ComputeTheoArea();
            m_TotalWetArea  += m_TMeshVec[i]->ComputeWetArea();
        }
    }

    //==== Compute Theo Vols ====//
    m_TotalTheoVol = 0;
    for ( i = 0 ; i < ( int )m_TMeshVec.size() ; i++ )
    {
        if ( !m_TMeshVec[i]->m_HalfBoxFlag )
        {
            m_TotalTheoVol += m_TMeshVec[i]->ComputeTheoVol();
        }
    }

    //==== Compute Total Volume ====//
    m_TotalWetVol = 0.0;
    for ( i = 0 ; i < ( int )m_TMeshVec.size() ; i++ )
    {
        if ( !m_TMeshVec[i]->m_HalfBoxFlag )
        {
            m_TotalWetVol += m_TMeshVec[i]->ComputeTrimVol();
        }
    }

    //==== Load TMeshs Into Component Based Groups ====//
    vector < vector< TMesh* > > tMeshCompVec;
    for ( i = 0 ; i < ( int )compIdVec.size() ; i++ )
    {
        vector<TMesh*> cidVec;
        for ( j = 0 ; j < ( int )m_TMeshVec.size() ; j++ )
        {
            if ( compIdVec[i] == m_TMeshVec[j]->m_PtrID )
            {
                cidVec.push_back( m_TMeshVec[j] );
            }
        }
        if ( cidVec.size() > 0 )
        {
            tMeshCompVec.push_back( cidVec );
        }
    }

    //==== Sum Area/Vol Data and Place in First TMesh Data ====//
    double guessTotalWetVol = 0;
    for ( i = 0 ; i < ( int )tMeshCompVec.size() ; i++ )
    {
        double ta, wa, tv, wv;          // Theo Area, Wet Area, Theo Vol, Wet Vol
        ta = wa = tv = wv = 0.0;

        for ( j = 0 ; j < ( int )tMeshCompVec[i].size() ; j++ )
        {
            ta += tMeshCompVec[i][j]->m_TheoArea;
            wa += tMeshCompVec[i][j]->m_WetArea;
            tv += tMeshCompVec[i][j]->m_TheoVol;
        }

        tMeshCompVec[i][0]->m_TheoArea = ta;
        tMeshCompVec[i][0]->m_WetArea  = wa;
        tMeshCompVec[i][0]->m_TheoVol  = tv;
        tMeshCompVec[i][0]->m_GuessVol = tv * wa / ta;      // Guess
        tMeshCompVec[i][0]->m_WetVol   = 0.0;

        guessTotalWetVol += tMeshCompVec[i][0]->m_GuessVol;

    }

    double leftOver = m_TotalWetVol;
    int leftOverCnt = 20;
    while ( leftOverCnt > 0 )
    {
        leftOverCnt--;

        double sumWetVol = 0.0;
        for ( i = 0 ; i < ( int )tMeshCompVec.size() ; i++ )
        {
            double perWetVol = tMeshCompVec[i][0]->m_GuessVol / guessTotalWetVol;
            tMeshCompVec[i][0]->m_WetVol += perWetVol * ( leftOver );

            if ( tMeshCompVec[i][0]->m_WetVol > tMeshCompVec[i][0]->m_TheoVol )
            {
                tMeshCompVec[i][0]->m_WetVol = tMeshCompVec[i][0]->m_TheoVol;
            }
            sumWetVol += tMeshCompVec[i][0]->m_WetVol;
        }

        if ( sumWetVol < m_TotalWetVol )
        {
            leftOver = m_TotalWetVol - sumWetVol;
        }
        else
        {
            leftOver = 0.0;
        }

        if ( leftOver < 0.00001 )
        {
            leftOverCnt = 0;
        }
    }

    //==== Add Results ====//
    vector< string > name_vec;
    vector< double > theo_area_vec;
    vector< double > wet_area_vec;
    vector< double > theo_vol_vec;
    vector< double > wet_vol_vec;
    vector< double > min_chord;
    vector< double > avg_chord;
    vector< double > max_chord;
    vector< double > min_tc;
    vector< double > avg_tc;
    vector< double > max_tc;
    vector< double > avg_sweep;
    vector< double > length;
    vector< double > max_area;
    vector< double > length_dia;

    res->Add( ResData( "Num_Meshes", ( int )tMeshCompVec.size() ) );
    for ( i = 0 ; i < ( int )tMeshCompVec.size() ; i++ )
    {
        TMesh* tmsh = tMeshCompVec[i][0];
        name_vec.push_back( tmsh->m_NameStr );
        theo_area_vec.push_back( tmsh->m_TheoArea );
        wet_area_vec.push_back( tmsh->m_WetArea );
        theo_vol_vec.push_back( tmsh->m_TheoVol );
        wet_vol_vec.push_back( tmsh->m_WetVol );
        min_chord.push_back( tmsh->m_DragFactors.m_MinChord );
        avg_chord.push_back( tmsh->m_DragFactors.m_AvgChord );
        max_chord.push_back( tmsh->m_DragFactors.m_MaxChord );
        min_tc.push_back( tmsh->m_DragFactors.m_MinThickToChord );
        avg_tc.push_back( tmsh->m_DragFactors.m_AvgThickToChord );
        max_tc.push_back( tmsh->m_DragFactors.m_MaxThickToChord );
        avg_sweep.push_back( tmsh->m_DragFactors.m_AvgSweep );
        length.push_back( tmsh->m_DragFactors.m_Length );
        max_area.push_back( tmsh->m_DragFactors.m_MaxXSecArea );
        length_dia.push_back( tmsh->m_DragFactors.m_LengthToDia );
    }

    res->Add( ResData( "Comp_Name", name_vec ) );
    res->Add( ResData( "Theo_Area", theo_area_vec ) );
    res->Add( ResData( "Wet_Area", wet_area_vec ) );
    res->Add( ResData( "Theo_Vol", theo_vol_vec ) );
    res->Add( ResData( "Wet_Vol", wet_vol_vec ) );

    res->Add( ResData( "Total_Theo_Area", m_TotalTheoArea ) );
    res->Add( ResData( "Total_Wet_Area", m_TotalWetArea ) );
    res->Add( ResData( "Total_Theo_Vol", m_TotalTheoVol ) );
    res->Add( ResData( "Total_Wet_Vol", m_TotalWetVol ) );

    res->Add( ResData( "Min_Chord", min_chord ) );
    res->Add( ResData( "Avg_Chord", avg_chord ) );
    res->Add( ResData( "Max_Chord", max_chord ) );

    res->Add( ResData( "Min_TC", min_tc ) );
    res->Add( ResData( "Avg_TC", avg_tc ) );
    res->Add( ResData( "Max_TC", max_tc ) );

    res->Add( ResData( "Avg_Sweep", avg_sweep ) );
    res->Add( ResData( "Length", length ) );
    res->Add( ResData( "Max_Area", max_area ) );
    res->Add( ResData( "Length_Dia", length_dia ) );

    res->Add( ResData( "Num_Degen_Tris_Removed", info.m_NumDegenerateTriDeleted ) );
    res->Add( ResData( "Num_Open_Meshes_Removed", info.m_NumOpenMeshedDeleted ) );
    res->Add( ResData( "Num_Open_Meshes_Merged", info.m_NumOpenMeshesMerged ) );


    if ( m_MeshFlag && !halfFlag )
    {
//      WaterTightCheck(fid);
    }

    string txtfn = m_Vehicle->getExportFileName( vsp::COMP_GEOM_TXT_TYPE );
    res->WriteCompGeomTxtFile( txtfn );

    //==== Write CSV File ====//
    if ( !m_MeshFlag && m_Vehicle->getExportCompGeomCsvFile() )
    {
        string csvfn = m_Vehicle->getExportFileName( vsp::COMP_GEOM_CSV_TYPE );
        res->WriteCompGeomCsvFile( csvfn );
    }

    //==== Write Drag BuildUp File ====//
    if ( !m_MeshFlag && m_Vehicle->getExportDragBuildTsvFile() )
    {
        string tsvfn = m_Vehicle->getExportFileName( vsp::DRAG_BUILD_TSV_TYPE );
        res->WriteDragBuildFile( tsvfn );
    }
}

//==== Call After BndBoxes Have Been Create But Before Intersect ====//
void MeshGeom::SliceX( int numSlices )
{
    int i, j, s;

    FILE* fid = fopen( "comp_geom.txt", "w" );

    //==== Check For Open Meshes and Merge or Delete Them ====//
    MeshInfo info;
    MergeRemoveOpenMeshes( &info );

    //==== Count Tris ====//
    int numTris = 0;
    for ( i = 0 ; i < ( int )m_TMeshVec.size() ; i++ )
    {
        numTris += m_TMeshVec[i]->m_TVec.size();
    }

    //==== Count Components ====//
    vector< string > compIdVec;
    for ( i = 0 ; i < ( int )m_TMeshVec.size() ; i++ )
    {
        string id = m_TMeshVec[i]->m_PtrID;
        vector<string>::iterator iter;

        iter = find( compIdVec.begin(), compIdVec.end(), id );

        if ( iter == compIdVec.end() )
        {
            compIdVec.push_back( id );
        }
    }

    fprintf( fid, "...Slice...\n" );
    fprintf( fid, "%d Num Comps\n", ( int )compIdVec.size() );
    fprintf( fid, "%d Total Num Meshes\n", ( int )m_TMeshVec.size() );
    fprintf( fid, "%d Total Num Tris\n", numTris );

    //==== Create Bnd Box for  Mesh Geoms ====//
    for ( i = 0 ; i < ( int )m_TMeshVec.size() ; i++ )
    {
        m_TMeshVec[i]->LoadBndBox();
    }

    //==== Update Bnd Box for  Combined ====//
    BndBox b;
    for ( i = 0 ; i < ( int )m_TMeshVec.size() ; i++ )
    {
        b.Update( m_TMeshVec[i]->m_TBox.m_Box );
    }
    m_BBox = b;
    //update_xformed_bbox();            // Load Xform BBox

    double xMin = m_BBox.GetMin( 0 ) - 0.0001;
    double xMax = m_BBox.GetMax( 0 ) + 0.0001;

    //==== Build Slice Mesh Object =====//
    if ( numSlices < 3 )
    {
        numSlices = 3;
    }

    for ( s = 0 ; s < numSlices ; s++ )
    {
        TMesh* tm = new TMesh();
        m_SliceVec.push_back( tm );

        double x = xMin + ( ( double )s / ( double )( numSlices - 1 ) ) * ( xMax - xMin );

        double ydel = 1.02 * ( m_BBox.GetMax( 1 ) - m_BBox.GetMin( 1 ) );
        double ys   = m_BBox.GetMin( 1 ) - 0.01 * ydel;
        double zdel = 1.02 * ( m_BBox.GetMax( 2 ) - m_BBox.GetMin( 2 ) );
        double zs   = m_BBox.GetMin( 2 ) - 0.01 * zdel;

        for ( i = 0 ; i < 10 ; i++ )
        {
            double y0 = ys + ydel * 0.1 * ( double )i;
            double y1 = ys + ydel * 0.1 * ( double )( i + 1 );

            for ( j = 0 ; j < 10 ; j++ )
            {
                double z0 = zs + zdel * 0.1 * ( double )j;
                double z1 = zs + zdel * 0.1 * ( double )( j + 1 );

                tm->AddTri( vec3d( x, y0, z0 ), vec3d( x, y1, z0 ), vec3d( x, y1, z1 ), vec3d( 1, 0, 0 ) );
                tm->AddTri( vec3d( x, y0, z0 ), vec3d( x, y1, z1 ), vec3d( x, y0, z1 ), vec3d( 1, 0, 0 ) );
            }
        }
    }

    //==== Load Bnding Box ====//
    for ( s = 0 ; s < ( int )m_SliceVec.size() ; s++ )
    {
        TMesh* tm = m_SliceVec[s];
        tm->LoadBndBox();

        //==== Intersect All Mesh Geoms ====//
        for ( i = 0 ; i < ( int )m_TMeshVec.size() ; i++ )
        {
            tm->Intersect( m_TMeshVec[i] );

            for ( j = 0 ; j < ( int )m_TMeshVec[i]->m_TVec.size() ; j++ )
            {
                TTri* tri = m_TMeshVec[i]->m_TVec[j];
                for ( int e = 0 ; e < ( int )tri->m_ISectEdgeVec.size() ; e++ )
                {
                    delete tri->m_ISectEdgeVec[e]->m_N0;
                    delete tri->m_ISectEdgeVec[e]->m_N1;
                    delete tri->m_ISectEdgeVec[e];
                }
                tri->m_ISectEdgeVec.erase( tri->m_ISectEdgeVec.begin(), tri->m_ISectEdgeVec.end() );
            }
        }

        //==== Split Intersected Tri in Mesh ====//
        tm->Split();

        //==== Determine Which Triangle Are Interior/Exterior ====//
        tm->DeterIntExt( m_TMeshVec );

        //==== Flip Int/Ext Flags ====//
        for ( i = 0 ; i < ( int )tm->m_TVec.size() ; i++ )
        {
            TTri* tri = tm->m_TVec[i];
            if ( tri->m_SplitVec.size() )
            {
                for ( j = 0 ; j < ( int )tri->m_SplitVec.size() ; j++ )
                {
                    tri->m_SplitVec[j]->m_InteriorFlag = !( tri->m_SplitVec[j]->m_InteriorFlag );
                }
            }
            else
            {
                tri->m_InteriorFlag = !( tri->m_InteriorFlag );
            }
        }
    }

    //==== Delete Mesh Geometry ====//
    for ( i = 0 ; i < ( int )m_TMeshVec.size() ; i++ )
    {
        delete m_TMeshVec[i];
    }

    m_TMeshVec.erase( m_TMeshVec.begin(), m_TMeshVec.end() );

    fprintf( fid, "\n" );
    fprintf( fid, "X       Area\n" );
    for ( s = 0 ; s < ( int )m_SliceVec.size() ; s++ )
    {
        double x = xMin + ( ( double )s / ( double )( numSlices - 1 ) ) * ( xMax - xMin );
        m_SliceVec[s]->ComputeWetArea();

        fprintf( fid, "%9.3f  %9.3f\n", x, m_SliceVec[s]->m_WetArea );
    }

    if ( info.m_NumDegenerateTriDeleted )
    {
        fprintf( fid, "WARNING: %d degenerate triangle removed\n", info.m_NumDegenerateTriDeleted );
    }
    if ( info.m_NumOpenMeshedDeleted )
    {
        fprintf( fid, "WARNING: %d open meshes removed\n", info.m_NumOpenMeshedDeleted );
    }
    if ( info.m_NumOpenMeshesMerged )
    {
        fprintf( fid, "WARNING: %d open meshes merged\n", info.m_NumOpenMeshesMerged );
    }


    fclose( fid );

}


//==== Call After BndBoxes Have Been Create But Before Intersect ====//
void MeshGeom::AreaSlice( int style, int numSlices, double sliceAngle, double coneSections, vec3d norm_axis,
                          bool autoBounds, double start, double end )
{
    int tesselate = 0;
    int i, j, s;

    //==== Transform mesh geoms to align with cutting plane normal vector ====//
    // Make sure the norm is not (0,0,0)
    if ( norm_axis.mag() == 0 )
    {
        return;
    }
    // Make sure norm is normalized
    norm_axis.normalize();

    // Find a point on the plane containing (0,0,0) with the norm normal vector
    vec3d pnt;
    if ( norm_axis.x() != 0 )
    {
        pnt[1] = 1;
        pnt[2] = 1;
        pnt[0] = -1 * ( norm_axis.y() * pnt.y() + norm_axis.z() * pnt.z() ) / norm_axis.x();
    }
    else if ( norm_axis.y() != 0 )
    {
        pnt[0] = 1;
        pnt[2] = 1;
        pnt[1] = -1 * ( norm_axis.x() * pnt.x() + norm_axis.z() * pnt.z() ) / norm_axis.y();
    }
    else if ( norm_axis.z() != 0 )
    {
        pnt[0] = 1;
        pnt[1] = 1;
        pnt[2] = -1 * ( norm_axis.x() * pnt.x() + norm_axis.y() * pnt.y() ) / norm_axis.z();
    }
    else
    {
        return;
    }

    pnt.normalize();
    vec3d tvec = cross( norm_axis, pnt );
    tvec.normalize();
    Matrix4d transMat;
    double tempMat[16];
    tempMat[0] = norm_axis.x();
    tempMat[4] = norm_axis.y();
    tempMat[8] = norm_axis.z();
    tempMat[12] = 0;
    tempMat[1] = pnt.x();
    tempMat[5] = pnt.y();
    tempMat[9] = pnt.z();
    tempMat[13] = 0;
    tempMat[2] = tvec.x();
    tempMat[6] = tvec.y();
    tempMat[10] = tvec.z();
    tempMat[14] = 0;
    tempMat[3] = 0;
    tempMat[7] = 0;
    tempMat[11] = 0;
    tempMat[15] = 1;

    Matrix4d TransMat;
    TransMat.initMat( tempMat );

    TransformMeshVec( m_TMeshVec, TransMat );

    //==== Check For Open Meshes and Merge or Delete Them ====//
    MeshInfo info;
    MergeRemoveOpenMeshes( &info );

    //==== Create Results ====//
    Results* res = ResultsMgr.CreateResults( "Slice" );
    res->Add( ResData( "Num_Degen_Triangles_Removed", info.m_NumDegenerateTriDeleted ) );
    res->Add( ResData( "Num_Open_Meshes_Removed", info.m_NumOpenMeshedDeleted ) );
    res->Add( ResData( "Num_Open_Meshes_Merged", info.m_NumOpenMeshesMerged ) );


    //==== Count Tris ====//
    int numTris = 0;
    for ( i = 0 ; i < ( int )m_TMeshVec.size() ; i++ )
    {
        numTris += m_TMeshVec[i]->m_TVec.size();
    }

    //==== Count Components ====//
    vector< string > compIdVec;
    for ( i = 0 ; i < ( int )m_TMeshVec.size() ; i++ )
    {
        string id = m_TMeshVec[i]->m_PtrID;
        vector<string>::iterator iter;

        iter = find( compIdVec.begin(), compIdVec.end(), id );

        if ( iter == compIdVec.end() )
        {
            compIdVec.push_back( id );
        }
    }

    res->Add( ResData( "Style", style ) );
    res->Add( ResData( "Num_Comps", ( int )compIdVec.size() ) );
    res->Add( ResData( "Num_Meshes", ( int )m_TMeshVec.size() ) );
    res->Add( ResData( "Num_Tris", numTris ) );
    res->Add( ResData( "Axis_Vector", norm_axis ) );

    //==== Create Bnd Box for  Mesh Geoms ====//
    for ( i = 0 ; i < ( int )m_TMeshVec.size() ; i++ )
    {
        m_TMeshVec[i]->LoadBndBox();
    }

    //==== Update Bnd Box for  Combined ====//
    BndBox b;
    for ( i = 0 ; i < ( int )m_TMeshVec.size() ; i++ )
    {
        b.Update( m_TMeshVec[i]->m_TBox.m_Box );
    }
    m_BBox = b;
    //update_xformed_bbox();            // Load Xform BBox

    double xMin;
    double xMax;
    if ( autoBounds )
    {
        xMin = m_BBox.GetMin( 0 ) - 0.0001;
        xMax = m_BBox.GetMax( 0 ) + 0.0001;
    }
    else
    {
        xMin = start - 0.0001;
        xMax = end + 0.0001;
    }
    //==== Build Slice Mesh Object =====//
    if ( numSlices < 3 )
    {
        numSlices = 3;
    }

    if ( style == vsp::SLICE_PLANAR )
    {
        vec3d norm( 1, 0, 0 );

        for ( s = 0 ; s < numSlices ; s++ )
        {
            TMesh* tm = new TMesh();
            m_SliceVec.push_back( tm );

            double x = xMin + ( ( double )s / ( double )( numSlices - 1 ) ) * ( xMax - xMin );

            double ydel = 1.02 * ( m_BBox.GetMax( 1 ) - m_BBox.GetMin( 1 ) );
            double ys   = m_BBox.GetMin( 1 ) - 0.01 * ydel;
            double zdel = 1.02 * ( m_BBox.GetMax( 2 ) - m_BBox.GetMin( 2 ) );
            double zs   = m_BBox.GetMin( 2 ) - 0.01 * zdel;

            if ( tesselate )
            {
                for ( i = 0 ; i < 10 ; i++ )
                {
                    double y0 = ys + ydel * 0.1 * ( double )i;
                    double y1 = ys + ydel * 0.1 * ( double )( i + 1 );

                    for ( j = 0 ; j < 10 ; j++ )
                    {
                        double z0 = zs + zdel * 0.1 * ( double )j;
                        double z1 = zs + zdel * 0.1 * ( double )( j + 1 );

                        tm->AddTri( vec3d( x, y0, z0 ), vec3d( x, y1, z0 ), vec3d( x, y1, z1 ), norm );
                        tm->AddTri( vec3d( x, y0, z0 ), vec3d( x, y1, z1 ), vec3d( x, y0, z1 ), norm );
                    }
                }
            }
            else
            {
                tm->AddTri( vec3d( x, ys, zs ), vec3d( x, ys + ydel, zs ), vec3d( x, ys + ydel, zs + zdel ), norm );
                tm->AddTri( vec3d( x, ys, zs ), vec3d( x, ys + ydel, zs + zdel ), vec3d( x, ys, zs + zdel ), norm );
            }
        }

    }
    else if ( style == vsp::SLICE_AWAVE )
    {
        for ( s = 0 ; s < numSlices ; s++ )
        {

            double xcenter = xMin + ( ( double )s / ( double )( numSlices - 1 ) ) * ( xMax - xMin );

            double ydel = m_BBox.GetMax( 1 ) - m_BBox.GetMin( 1 );
            double zdel = m_BBox.GetMax( 2 ) - m_BBox.GetMin( 2 );
            double ycenter = m_BBox.GetMin( 1 ) + ydel * 0.5;
            double zcenter = m_BBox.GetMin( 2 ) + zdel * 0.5;

            double size = max( ydel, zdel ) * 1.02;
            if ( sliceAngle == 0 )
            {
                sliceAngle = 1;
            }
            double xdel = 2 * size / tan( DEG2RAD( sliceAngle ) );

            double radius = size / 2.0;

            // for num sections, rotate about x-axis
            for ( double a = 0; a < coneSections; a++ )
            {
                TMesh* tm = new TMesh();
                m_SliceVec.push_back( tm );

                double theta = ( 2.0 * PI * a ) / ( ( double ) coneSections );
                double rsintheta = radius * sin( theta );
                double rcostheta = radius * cos( theta );

                vec3d offset( xcenter, ycenter, zcenter );
                vec3d tr( -xdel / 2, rcostheta + rsintheta, rcostheta - rsintheta );
                vec3d tl( -xdel / 2, rsintheta - rcostheta, rcostheta + rsintheta );
                vec3d br = vec3d( 0, 0, 0 ) - tl;
                vec3d bl = vec3d( 0, 0, 0 ) - tr;
                vec3d norm = cross( br - bl, tl - bl );

                if ( tesselate )
                {
                    vec3d bl0, br0, tl0, tr0;
                    vec3d bl1, br1, tl1, tr1;
                    double u, v, u1, v1;
                    double increment = 0.1;
                    for ( u = 0; u < 1.0; u += increment )
                    {
                        // tesselate
                        u1 = u + increment;
                        bl0 = ( bl * u  + br * ( 1.0 - u ) );
                        br0 = ( bl * u1 + br * ( 1.0 - u1 ) );
                        tl0 = ( tl * u  + tr * ( 1.0 - u ) );
                        tr0 = ( tl * u1 + tr * ( 1.0 - u1 ) );
                        for ( v = 0; v < 1.0; v += increment )
                        {
                            // tesselate
                            v1 = v + increment;
                            bl1 = ( bl0 * v  + tl0 * ( 1.0 - v ) );
                            tl1 = ( bl0 * v1 + tl0 * ( 1.0 - v1 ) );
                            br1 = ( br0 * v  + tr0 * ( 1.0 - v ) );
                            tr1 = ( br0 * v1 + tr0 * ( 1.0 - v1 ) );

                            tm->AddTri( bl1 + offset, br1 + offset, tr1 + offset, norm );
                            tm->AddTri( bl1 + offset, tr1 + offset, tl1 + offset, norm );
                        }
                    }
                }
                else
                {
                    tm->AddTri( bl + offset, br + offset, tr + offset, norm );
                    tm->AddTri( bl + offset, tr + offset, tl + offset, norm );
                }
            }
        }

    }
    else
    {
        return;
    }


    //==== Load Bnding Box ====//
    for ( s = 0 ; s < ( int )m_SliceVec.size() ; s++ )
    {
        TMesh* tm = m_SliceVec[s];
        tm->LoadBndBox();

        //==== Intersect All Mesh Geoms ====//
        for ( i = 0 ; i < ( int )m_TMeshVec.size() ; i++ )
        {
            tm->Intersect( m_TMeshVec[i] );

            for ( j = 0 ; j < ( int )m_TMeshVec[i]->m_TVec.size() ; j++ )
            {
                TTri* tri = m_TMeshVec[i]->m_TVec[j];
                for ( int e = 0 ; e < ( int )tri->m_ISectEdgeVec.size() ; e++ )
                {
                    delete tri->m_ISectEdgeVec[e]->m_N0;
                    delete tri->m_ISectEdgeVec[e]->m_N1;
                    delete tri->m_ISectEdgeVec[e];
                }
                tri->m_ISectEdgeVec.erase( tri->m_ISectEdgeVec.begin(), tri->m_ISectEdgeVec.end() );
            }
        }

        //==== Split Intersected Tri in Mesh ====//
        tm->Split();

        //==== Determine Which Triangle Are Interior/Exterior ====//
        tm->DeterIntExt( m_TMeshVec );

        //==== Flip Int/Ext Flags ====//
        for ( i = 0 ; i < ( int )tm->m_TVec.size() ; i++ )
        {
            TTri* tri = tm->m_TVec[i];
            if ( tri->m_SplitVec.size() )
            {
                for ( j = 0 ; j < ( int )tri->m_SplitVec.size() ; j++ )
                {
                    tri->m_SplitVec[j]->m_InteriorFlag = !( tri->m_SplitVec[j]->m_InteriorFlag );
                }
            }
            else
            {
                tri->m_InteriorFlag = !( tri->m_InteriorFlag );
            }
        }
    }
    //==== Delete Mesh Geometry ====//
    for ( i = 0 ; i < ( int )m_TMeshVec.size() ; i++ )
    {
        delete m_TMeshVec[i];
    }

    m_TMeshVec.erase( m_TMeshVec.begin(), m_TMeshVec.end() );

    if ( style == vsp::SLICE_PLANAR )
    {
        vector< double > loc_vec;
        vector< double > area_vec;
        for ( s = 0 ; s < ( int )m_SliceVec.size() ; s++ )
        {
            double x = xMin + ( ( double )s / ( double )( numSlices - 1 ) ) * ( xMax - xMin );
            m_SliceVec[s]->ComputeWetArea();
            loc_vec.push_back( x );
            area_vec.push_back( m_SliceVec[s]->m_WetArea );
        }
        res->Add( ResData( "Num_Slices", ( int )m_SliceVec.size() ) );
        res->Add( ResData( "Slice_Loc", loc_vec ) );
        res->Add( ResData( "Slice_Area", area_vec ) );
    }
    else if ( style == vsp::SLICE_AWAVE )
    {
        vector< double > loc_vec;
        for ( i = 0; i < coneSections; i++ )
        {
            loc_vec.push_back( 360.0 * i / coneSections );
        }
        res->Add( ResData( "Num_Slices", ( int )numSlices ) );
        res->Add( ResData( "Slice_Loc", loc_vec ) );
        res->Add( ResData( "Num_Cone_Sections", ( int )coneSections ) );

        vector< double > x_vec;
        for ( s = 0 ; s < numSlices ; s++ )
        {
            double sum = 0;
            double x = xMin + ( ( double )s / ( double )( numSlices - 1 ) ) * ( xMax - xMin );
            x_vec.push_back( x );

            vector< double > wet_vec;
            for ( int r = 0; r < coneSections; r++ )
            {
                int sindex = ( int )( s * coneSections + r );
                m_SliceVec[sindex]->ComputeAwaveArea();
                sum += m_SliceVec[sindex]->m_WetArea;
                wet_vec.push_back( m_SliceVec[sindex]->m_WetArea );
            }
            res->Add( ResData( "Slice_Wet_Area", wet_vec ) );
            res->Add( ResData( "Slice_Sum_Area", sum ) );
            res->Add( ResData( "Slice_Avg_Area", sum / coneSections ) );
        }
        res->Add( ResData( "X_Loc", x_vec ) );
    }

    string filename = m_Vehicle->getExportFileName( vsp::SLICE_TXT_TYPE );
    res->WriteSliceFile( filename, style );

    //==== TransForm Slices to Match Orignal Coord Sys ====//
    TransMat.affineInverse();
    TransformMeshVec( m_SliceVec, TransMat );
}


vector<vec3d> MeshGeom::TessTri( vec3d t1, vec3d t2, vec3d t3, int iterations )
{
    vector<vec3d> triangles;
    triangles.push_back( t1 );
    triangles.push_back( t2 );
    triangles.push_back( t3 );

    for ( int i = 0; i < iterations; i++ )
    {
        triangles = TessTriangles( triangles );
    }
    return triangles;
}

vector<vec3d> MeshGeom::TessTriangles( vector<vec3d> &tri )
{
    assert( ( tri.size() ) % 3 == 0 );
    vector< vec3d > triangles;
    for ( int i = 0; i < ( int )tri.size(); i += 3 )
    {
        vec3d t1 = tri[ i ];
        vec3d t2 = tri[i + 1];
        vec3d t3 = tri[i + 2];

        vec3d c12 = ( t1 + t2 ) * 0.5;
        vec3d c13 = ( t1 + t3 ) * 0.5;
        vec3d c23 = ( t2 + t3 ) * 0.5;

        triangles.push_back( t1 );
        triangles.push_back( c12 );
        triangles.push_back( c13 );

        triangles.push_back( t2 );
        triangles.push_back( c23 );
        triangles.push_back( c12 );

        triangles.push_back( t3 );
        triangles.push_back( c13 );
        triangles.push_back( c23 );

        triangles.push_back( c23 );
        triangles.push_back( c13 );
        triangles.push_back( c12 );
    }
    return triangles;
}

vec3d MeshGeom::GetVertex3d( int surf, double x, double p, int r )
{
    if ( x < 0 )
    {
        x = 0;
    }
    if ( p < 0 )
    {
        p = 0;
    }
    if ( x > 1 )
    {
        x = 1;
    }
    if ( p > 1 )
    {
        p = 1;
    }


    if ( surf == 0 ) // tmesh
    {
        int t = ( int )( x * ( m_TMeshVec.size() - 1 ) + 0.5 );
        int v = ( int )( p * ( m_TMeshVec[t]->NumVerts() - 1 ) + 0.5 );
        return m_TMeshVec[t]->GetVertex( v );
    }
    else if ( surf == 1 ) //slice
    {
        int s = ( int )( x * ( m_SliceVec.size() - 1 ) + 0.5 );
        int v = ( int )( p * ( m_SliceVec[s]->NumVerts() - 1 ) + 0.5 );
        return m_SliceVec[s]->GetVertex( v );
    }
    return vec3d();
}

//void  MeshGeom::getVertexVec(vector< VertexID > *vertVec)
//{
//  buildVertexVec(&tMeshVec, 0, vertVec);
//  buildVertexVec(&sliceVec, 1, vertVec);
//}

//==== Call After BndBoxes Have Been Create But Before Intersect ====//
void MeshGeom::MassSliceX( int numSlices )
{
    int i, j, s;

    //==== Check For Open Meshes and Merge or Delete Them ====//
    MeshInfo info;
    MergeRemoveOpenMeshes( &info );

    //==== Create Results ====//
    Results* res = ResultsMgr.CreateResults( "Mass_Properties" );
    res->Add( ResData( "Num_Degen_Triangles_Removed", info.m_NumDegenerateTriDeleted ) );
    res->Add( ResData( "Num_Open_Meshes_Removed", info.m_NumOpenMeshedDeleted ) );
    res->Add( ResData( "Num_Open_Meshes_Merged", info.m_NumOpenMeshesMerged ) );

    //==== Count Tris ====//
    int numTris = 0;
    for ( i = 0 ; i < ( int )m_TMeshVec.size() ; i++ )
    {
        numTris += m_TMeshVec[i]->m_TVec.size();
    }

    //==== Count Components ====//
    vector< string > compIdVec;
    for ( i = 0 ; i < ( int )m_TMeshVec.size() ; i++ )
    {
        string id = m_TMeshVec[i]->m_PtrID;
        vector<string>::iterator iter;

        iter = find( compIdVec.begin(), compIdVec.end(), id );

        if ( iter == compIdVec.end() )
        {
            compIdVec.push_back( id );
        }
    }

    res->Add( ResData( "Num_Total_Meshes", ( int )m_TMeshVec.size() ) );
    res->Add( ResData( "Num_Total_Tris", numTris ) );

    //==== Create Bnd Box for  Mesh Geoms ====//
    for ( i = 0 ; i < ( int )m_TMeshVec.size() ; i++ )
    {
        m_TMeshVec[i]->LoadBndBox();
    }

    //==== Update Bnd Box for  Combined ====//
    BndBox b;
    for ( i = 0 ; i < ( int )m_TMeshVec.size() ; i++ )
    {
        b.Update( m_TMeshVec[i]->m_TBox.m_Box );
    }
    m_BBox = b;
    //update_xformed_bbox();            // Load Xform BBox

    double xMin = m_BBox.GetMin( 0 );
    double xMax = m_BBox.GetMax( 0 );

    double sliceW = ( xMax - xMin ) / ( double )( numSlices );

    //==== Build Slice Mesh Object =====//
    if ( numSlices < 3 )
    {
        numSlices = 3;
    }

    for ( s = 0 ; s < numSlices ; s++ )
    {
        TMesh* tm = new TMesh();
        m_SliceVec.push_back( tm );

        double x = xMin + ( double )s * sliceW + 0.5 * sliceW;

        double ydel = 1.02 * ( m_BBox.GetMax( 1 ) - m_BBox.GetMin( 1 ) );
        double ys   = m_BBox.GetMin( 1 ) - 0.01 * ydel;
        double zdel = 1.02 * ( m_BBox.GetMax( 2 ) - m_BBox.GetMin( 2 ) );
        double zs   = m_BBox.GetMin( 2 ) - 0.01 * zdel;

        for ( i = 0 ; i < 10 ; i++ )
        {
            double y0 = ys + ydel * 0.1 * ( double )i;
            double y1 = ys + ydel * 0.1 * ( double )( i + 1 );

            for ( j = 0 ; j < 10 ; j++ )
            {
                double z0 = zs + zdel * 0.1 * ( double )j;
                double z1 = zs + zdel * 0.1 * ( double )( j + 1 );

                tm->AddTri( vec3d( x, y0, z0 ), vec3d( x, y1, z0 ), vec3d( x, y1, z1 ), vec3d( 1, 0, 0 ) );
                tm->AddTri( vec3d( x, y0, z0 ), vec3d( x, y1, z1 ), vec3d( x, y0, z1 ), vec3d( 1, 0, 0 ) );
            }
        }
    }

    //==== Load Bnding Box ====//
    for ( s = 0 ; s < ( int )m_SliceVec.size() ; s++ )
    {
        TMesh* tm = m_SliceVec[s];
        tm->LoadBndBox();

        //==== Intersect All Mesh Geoms ====//
        for ( i = 0 ; i < ( int )m_TMeshVec.size() ; i++ )
        {
            tm->Intersect( m_TMeshVec[i] );

            for ( j = 0 ; j < ( int )m_TMeshVec[i]->m_TVec.size() ; j++ )
            {
                TTri* tri = m_TMeshVec[i]->m_TVec[j];
                for ( int e = 0 ; e < ( int )tri->m_ISectEdgeVec.size() ; e++ )
                {
                    delete tri->m_ISectEdgeVec[e]->m_N0;
                    delete tri->m_ISectEdgeVec[e]->m_N1;
                    delete tri->m_ISectEdgeVec[e];
                }
                tri->m_ISectEdgeVec.erase( tri->m_ISectEdgeVec.begin(), tri->m_ISectEdgeVec.end() );
            }
        }

        //==== Split Intersected Tri in Mesh ====//
        tm->Split();

        //==== Determine Which Triangle Are Interior/Exterior ====//
        tm->MassDeterIntExt( m_TMeshVec );

    }
    /**********
        //==== Delete Mesh Geometry ====//
        for ( i = 0 ; i < (int)tMeshVec.size() ; i++ )
        {
            delete tMeshVec[i];
        }
        tMeshVec.erase( tMeshVec.begin(), tMeshVec.end() );
    *********/
    //==== Intersect All Mesh Geoms ====//
    for ( i = 0 ; i < ( int )m_TMeshVec.size() ; i++ )
    {
        for ( j = i + 1 ; j < ( int )m_TMeshVec.size() ; j++ )
        {
            m_TMeshVec[i]->Intersect( m_TMeshVec[j] );
        }
    }

    //==== Split Intersected Tri in Mesh ====//
    for ( i = 0 ; i < ( int )m_TMeshVec.size() ; i++ )
    {
        m_TMeshVec[i]->Split();
    }

    //==== Determine Which Triangle Are Interior/Exterior ====//
    for ( i = 0 ; i < ( int )m_TMeshVec.size() ; i++ )
    {
        m_TMeshVec[i]->DeterIntExt( m_TMeshVec );
    }

    //==== Do Shell Calcs ====//
    vector< TriShellMassProp* > triShellVec;
    for ( s = 0 ; s < ( int )m_TMeshVec.size() ; s++ )
    {
        TMesh* tm = m_TMeshVec[s];
        if ( tm->m_ShellFlag )
        {
            for ( i = 0 ; i < ( int )tm->m_TVec.size() ; i++ )
            {
                TTri* tri = tm->m_TVec[i];
                if ( tri->m_SplitVec.size() )
                {
                    for ( j = 0 ; j < ( int )tri->m_SplitVec.size() ; j++ )
                    {
                        if ( tri->m_SplitVec[j]->m_InteriorFlag == 0 )
                        {
                            TriShellMassProp* tsmp = new TriShellMassProp( tm->m_PtrID, tm->m_ShellMassArea,
                                    tri->m_SplitVec[j]->m_N0->m_Pnt,
                                    tri->m_SplitVec[j]->m_N1->m_Pnt,
                                    tri->m_SplitVec[j]->m_N2->m_Pnt );
                            triShellVec.push_back( tsmp );
                        }
                    }
                }
                else if ( tri->m_InteriorFlag == 0 )
                {
                    TriShellMassProp* tsmp = new TriShellMassProp( tm->m_PtrID, tm->m_ShellMassArea,
                            tri->m_N0->m_Pnt, tri->m_N1->m_Pnt, tri->m_N2->m_Pnt );
                    triShellVec.push_back( tsmp );
                }
            }
        }
    }

    //==== Build Tetrahedrons ====//
    double prismLength = sliceW;
    vector< TetraMassProp* > tetraVec;
    m_MinTriDen = 1.0e06;
    m_MaxTriDen = 0.0;
    m_MpTriVec.clear();

    for ( s = 0 ; s < ( int )m_SliceVec.size() ; s++ )
    {
        TMesh* tm = m_SliceVec[s];
        for ( i = 0 ; i < ( int )tm->m_TVec.size() ; i++ )
        {
            TTri* tri = tm->m_TVec[i];

            if ( tri->m_SplitVec.size() )
            {
                for ( j = 0 ; j < ( int )tri->m_SplitVec.size() ; j++ )
                {
                    if ( tri->m_SplitVec[j]->m_InteriorFlag == 0 )
                    {
                        CreatePrism( tetraVec, tri->m_SplitVec[j], prismLength );
                        m_MpTriVec.push_back( tri->m_SplitVec[j] );
                    }
                }
            }
            else if ( tri->m_InteriorFlag == 0 )
            {
                CreatePrism( tetraVec, tri, prismLength );
                m_MpTriVec.push_back( tri );
            }
        }
    }

    //==== Add in Point Masses ====//
    for ( i = 0 ; i < ( int )m_PointMassVec.size() ; i++ )
    {
        tetraVec.push_back( m_PointMassVec[i] );
    }

    double totalVol = 0.0;
    for ( i = 0 ; i < ( int )tetraVec.size() ; i++ )
    {
        totalVol += fabs( tetraVec[i]->m_Vol );
    }

    vec3d cg( 0, 0, 0 );
    m_TotalMass = 0.0;
    for ( i = 0 ; i < ( int )tetraVec.size() ; i++ )
    {
        m_TotalMass += tetraVec[i]->m_Mass;
        cg = cg + tetraVec[i]->m_CG * tetraVec[i]->m_Mass;
    }
    for ( i = 0 ; i < ( int )triShellVec.size() ; i++ )
    {
        m_TotalMass += triShellVec[i]->m_Mass;
        cg = cg + triShellVec[i]->m_CG * triShellVec[i]->m_Mass;
    }

    if ( m_TotalMass )
    {
        cg = cg * ( 1.0 / m_TotalMass );
    }

    m_MassPropFlag = 1;
    m_CenterOfGrav = cg;

    m_TotalIxx = m_TotalIyy = m_TotalIzz = 0.0;
    m_TotalIxy = m_TotalIxz = m_TotalIyz = 0.0;
    for ( i = 0 ; i < ( int )tetraVec.size() ; i++ )
    {
        TetraMassProp* tet = tetraVec[i];
        m_TotalIxx += tet->m_Ixx +
                      tet->m_Mass * ( ( cg.y() - tet->m_CG.y() ) * ( cg.y() - tet->m_CG.y() ) + ( cg.z() - tet->m_CG.z() ) * ( cg.z() - tet->m_CG.z() ) );
        m_TotalIyy += tet->m_Iyy +
                      tet->m_Mass * ( ( cg.x() - tet->m_CG.x() ) * ( cg.x() - tet->m_CG.x() ) + ( cg.z() - tet->m_CG.z() ) * ( cg.z() - tet->m_CG.z() ) );
        m_TotalIzz += tet->m_Izz +
                      tet->m_Mass * ( ( cg.x() - tet->m_CG.x() ) * ( cg.x() - tet->m_CG.x() ) + ( cg.y() - tet->m_CG.y() ) * ( cg.y() - tet->m_CG.y() ) );

        m_TotalIxy += tet->m_Ixy +
                      tet->m_Mass * ( ( cg.x() - tet->m_CG.x() ) * ( cg.y() - tet->m_CG.y() ) );
        m_TotalIxz += tet->m_Ixz +
                      tet->m_Mass * ( ( cg.x() - tet->m_CG.x() ) * ( cg.z() - tet->m_CG.z() ) );
        m_TotalIyz += tet->m_Iyz +
                      tet->m_Mass * ( ( cg.y() - tet->m_CG.y() ) * ( cg.z() - tet->m_CG.z() ) );
    }
    for ( i = 0 ; i < ( int )triShellVec.size() ; i++ )
    {
        TriShellMassProp* trs = triShellVec[i];
        m_TotalIxx += trs->m_Ixx +
                      trs->m_Mass * ( ( cg.y() - trs->m_CG.y() ) * ( cg.y() - trs->m_CG.y() ) + ( cg.z() - trs->m_CG.z() ) * ( cg.z() - trs->m_CG.z() ) );
        m_TotalIyy += trs->m_Iyy +
                      trs->m_Mass * ( ( cg.x() - trs->m_CG.x() ) * ( cg.x() - trs->m_CG.x() ) + ( cg.z() - trs->m_CG.z() ) * ( cg.z() - trs->m_CG.z() ) );
        m_TotalIzz += trs->m_Izz +
                      trs->m_Mass * ( ( cg.x() - trs->m_CG.x() ) * ( cg.x() - trs->m_CG.x() ) + ( cg.y() - trs->m_CG.y() ) * ( cg.y() - trs->m_CG.y() ) );

        m_TotalIxy += trs->m_Ixy +
                      trs->m_Mass * ( ( cg.x() - trs->m_CG.x() ) * ( cg.y() - trs->m_CG.y() ) );
        m_TotalIxz += trs->m_Ixz +
                      trs->m_Mass * ( ( cg.x() - trs->m_CG.x() ) * ( cg.z() - trs->m_CG.z() ) );
        m_TotalIyz += trs->m_Iyz +
                      trs->m_Mass * ( ( cg.y() - trs->m_CG.y() ) * ( cg.z() - trs->m_CG.z() ) );
    }

    vector< string > name_vec;
    vector< string > id_vec;
    vector< double > mass_vec;
    vector< vec3d > cg_vec;
    vector< double > ixx_vec;
    vector< double > iyy_vec;
    vector< double > izz_vec;
    vector< double > ixy_vec;
    vector< double > ixz_vec;
    vector< double > iyz_vec;
    vector< double > vol_vec;

    for ( s = 0 ; s < ( int )m_TMeshVec.size() ; s++ )
    {
        TMesh* tm = m_TMeshVec[s];
        string id = tm->m_PtrID;
        id_vec.push_back( id );

        double compVol = 0.0;
        for ( i = 0 ; i < ( int )tetraVec.size() ; i++ )
        {
            if ( !tetraVec[i]->m_CompId.compare( id ) )
            {
                compVol += fabs( tetraVec[i]->m_Vol );
            }
        }

        vec3d cg( 0, 0, 0 );
        double compMass = 0.0;
        for ( i = 0 ; i < ( int )tetraVec.size() ; i++ )
        {
            if ( !tetraVec[i]->m_CompId.compare( id ) )
            {
                compMass += tetraVec[i]->m_Mass;
                cg = cg + tetraVec[i]->m_CG * tetraVec[i]->m_Mass;
            }
        }
        for ( i = 0 ; i < ( int )triShellVec.size() ; i++ )
        {
            if ( !triShellVec[i]->m_CompId.compare( id ) )
            {
                compMass += triShellVec[i]->m_Mass;
                cg = cg + triShellVec[i]->m_CG * triShellVec[i]->m_Mass;
            }
        }

        if ( compMass )
        {
            cg = cg * ( 1.0 / compMass );
        }

        double compIxx = 0.0;
        double compIyy = 0.0;
        double compIzz = 0.0;
        double compIxy = 0.0;
        double compIxz = 0.0;
        double compIyz = 0.0;
        for ( i = 0 ; i < ( int )tetraVec.size() ; i++ )
        {
            TetraMassProp* tet = tetraVec[i];
            if ( tet->m_CompId == id )
            {
                compIxx += tet->m_Ixx +
                           tet->m_Mass * ( ( cg.y() - tet->m_CG.y() ) * ( cg.y() - tet->m_CG.y() ) + ( cg.z() - tet->m_CG.z() ) * ( cg.z() - tet->m_CG.z() ) );
                compIyy += tet->m_Iyy +
                           tet->m_Mass * ( ( cg.x() - tet->m_CG.x() ) * ( cg.x() - tet->m_CG.x() ) + ( cg.z() - tet->m_CG.z() ) * ( cg.z() - tet->m_CG.z() ) );
                compIzz += tet->m_Izz +
                           tet->m_Mass * ( ( cg.x() - tet->m_CG.x() ) * ( cg.x() - tet->m_CG.x() ) + ( cg.y() - tet->m_CG.y() ) * ( cg.y() - tet->m_CG.y() ) );

                compIxy += tet->m_Ixy +
                           tet->m_Mass * ( ( cg.x() - tet->m_CG.x() ) * ( cg.y() - tet->m_CG.y() ) );
                compIxz += tet->m_Ixz +
                           tet->m_Mass * ( ( cg.x() - tet->m_CG.x() ) * ( cg.z() - tet->m_CG.z() ) );
                compIyz += tet->m_Iyz +
                           tet->m_Mass * ( ( cg.y() - tet->m_CG.y() ) * ( cg.z() - tet->m_CG.z() ) );
            }
        }
        for ( i = 0 ; i < ( int )triShellVec.size() ; i++ )
        {
            TriShellMassProp* trs = triShellVec[i];
            if ( trs->m_CompId == id )
            {
                compIxx += trs->m_Ixx +
                           trs->m_Mass * ( ( cg.y() - trs->m_CG.y() ) * ( cg.y() - trs->m_CG.y() ) + ( cg.z() - trs->m_CG.z() ) * ( cg.z() - trs->m_CG.z() ) );
                compIyy += trs->m_Iyy +
                           trs->m_Mass * ( ( cg.x() - trs->m_CG.x() ) * ( cg.x() - trs->m_CG.x() ) + ( cg.z() - trs->m_CG.z() ) * ( cg.z() - trs->m_CG.z() ) );
                compIzz += trs->m_Izz +
                           trs->m_Mass * ( ( cg.x() - trs->m_CG.x() ) * ( cg.x() - trs->m_CG.x() ) + ( cg.y() - trs->m_CG.y() ) * ( cg.y() - trs->m_CG.y() ) );

                compIxy += trs->m_Ixy +
                           trs->m_Mass * ( ( cg.x() - trs->m_CG.x() ) * ( cg.y() - trs->m_CG.y() ) );
                compIxz += trs->m_Ixz +
                           trs->m_Mass * ( ( cg.x() - trs->m_CG.x() ) * ( cg.z() - trs->m_CG.z() ) );
                compIyz += trs->m_Iyz +
                           trs->m_Mass * ( ( cg.y() - trs->m_CG.y() ) * ( cg.z() - trs->m_CG.z() ) );
            }
        }

        //==== Load Component Results ====//
        name_vec.push_back( tm->m_NameStr );
        mass_vec.push_back( compMass );
        cg_vec.push_back( cg );
        ixx_vec.push_back( compIxx );
        iyy_vec.push_back( compIyy );
        izz_vec.push_back( compIzz );
        ixy_vec.push_back( compIxy );
        ixz_vec.push_back( compIxz );
        iyz_vec.push_back( compIyz );
        vol_vec.push_back( compVol );
    }

    res->Add( ResData( "Num_Comps", ( int )name_vec.size() ) );
    res->Add( ResData( "Comp_Name", name_vec ) );
    res->Add( ResData( "Comp_ID", id_vec ) );
    res->Add( ResData( "Comp_Mass", mass_vec ) );
    res->Add( ResData( "Comp_CG", cg_vec ) );
    res->Add( ResData( "Comp_Ixx", ixx_vec ) );
    res->Add( ResData( "Comp_Iyy", iyy_vec ) );
    res->Add( ResData( "Comp_Izz", izz_vec ) );
    res->Add( ResData( "Comp_Ixy", ixy_vec ) );
    res->Add( ResData( "Comp_Ixz", ixz_vec ) );
    res->Add( ResData( "Comp_Iyz", iyz_vec ) );
    res->Add( ResData( "Comp_Vol", vol_vec ) );

    //==== Totals ====//
    res->Add( ResData( "Total_Mass", m_TotalMass ) );
    res->Add( ResData( "Total_CG", m_CenterOfGrav ) );
    res->Add( ResData( "Total_Ixx", m_TotalIxx ) );
    res->Add( ResData( "Total_Iyy", m_TotalIyy ) );
    res->Add( ResData( "Total_Izz", m_TotalIzz ) );
    res->Add( ResData( "Total_Ixy", m_TotalIxy ) );
    res->Add( ResData( "Total_Ixz", m_TotalIxz ) );
    res->Add( ResData( "Total_Iyz", m_TotalIyz ) );
    res->Add( ResData( "Total_Volume", totalVol ) );

    //==== Clean Up Mess ====//
    for ( i = 0 ; i < ( int )tetraVec.size() ; i++ )
    {
        delete tetraVec[i];
    }

    for ( i = 0 ; i < ( int )triShellVec.size() ; i++ )
    {
        delete triShellVec[i];
    }

    //==== Get Rid of TMeshes  that are not shells ====//
    vector<TMesh*> newTMeshVec;
    for ( i = 0 ; i < ( int )m_TMeshVec.size() ; i++ )
    {
        if ( m_TMeshVec[i]->m_ShellFlag )
        {
            newTMeshVec.push_back( m_TMeshVec[i] );
        }
        else
        {
            delete m_TMeshVec[i];
        }
    }
    m_TMeshVec = newTMeshVec;

//  res->WriteCSVFile("junk.txt");

    string f_name = m_Vehicle->getExportFileName( vsp::MASS_PROP_TXT_TYPE );
    res->WriteMassProp( f_name );
}

//==== Create a Prism Made of Tetras - Extrude Tri +- len/2 ====//
void MeshGeom::CreatePrism( vector< TetraMassProp* >& tetraVec, TTri* tri, double len )
{
    if ( tri->m_Mass < m_MinTriDen )
    {
        m_MinTriDen = tri->m_Mass;
    }
    if ( tri->m_Mass > m_MaxTriDen )
    {
        m_MaxTriDen = tri->m_Mass;
    }

    vec3d cnt = ( tri->m_N0->m_Pnt + tri->m_N1->m_Pnt + tri->m_N2->m_Pnt ) * ( 1.0 / 3.0 );

    vec3d p0 = tri->m_N0->m_Pnt;
    vec3d p1 = tri->m_N1->m_Pnt;
    vec3d p2 = tri->m_N2->m_Pnt;
    p0.offset_x( len / 2.0 );
    p1.offset_x( len / 2.0 );
    p2.offset_x( len / 2.0 );

    vec3d p3 = tri->m_N0->m_Pnt;
    vec3d p4 = tri->m_N1->m_Pnt;
    vec3d p5 = tri->m_N2->m_Pnt;
    p3.offset_x( -len / 2.0 );
    p4.offset_x( -len / 2.0 );
    p5.offset_x( -len / 2.0 );

    tetraVec.push_back( new TetraMassProp( tri->m_ID, tri->m_Mass, cnt, p0, p1, p2 ) );
    tetraVec.push_back( new TetraMassProp( tri->m_ID, tri->m_Mass, cnt, p3, p4, p5 ) );
    tetraVec.push_back( new TetraMassProp( tri->m_ID, tri->m_Mass, cnt, p0, p1, p3 ) );
    tetraVec.push_back( new TetraMassProp( tri->m_ID, tri->m_Mass, cnt, p3, p4, p1 ) );
    tetraVec.push_back( new TetraMassProp( tri->m_ID, tri->m_Mass, cnt, p1, p2, p4 ) );
    tetraVec.push_back( new TetraMassProp( tri->m_ID, tri->m_Mass, cnt, p4, p5, p2 ) );
    tetraVec.push_back( new TetraMassProp( tri->m_ID, tri->m_Mass, cnt, p0, p2, p3 ) );
    tetraVec.push_back( new TetraMassProp( tri->m_ID, tri->m_Mass, cnt, p3, p5, p2 ) );
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
    m_OneMesh = new TMesh();
    m_OneMesh->m_Color = m_TMeshVec[0]->m_Color;

    for ( m = 0 ; m < ( int )m_TMeshVec.size() ; m++ )
    {
        TMesh* mesh = m_TMeshVec[m];
        for ( t = 0 ; t < ( int )mesh->m_TVec.size() ; t++ )
        {
            if ( mesh->m_TVec[t]->m_SplitVec.size() )
            {
                for ( i = 0 ; i < ( int )mesh->m_TVec[t]->m_SplitVec.size() ; i++ )
                {
                    if ( mesh->m_TVec[t]->m_SplitVec[i]->m_InteriorFlag == 0 )
                    {
                        TTri* tri = mesh->m_TVec[t]->m_SplitVec[i];
                        m_OneMesh->AddTri( tri->m_N0, tri->m_N1, tri->m_N2, mesh->m_TVec[t]->m_Norm );
                    }
                }
            }
            else if ( mesh->m_TVec[t]->m_InteriorFlag == 0 )
            {
                TTri* tri = mesh->m_TVec[t];
                m_OneMesh->AddTri( tri->m_N0, tri->m_N1, tri->m_N2, tri->m_Norm );
            }
        }
    }

    //==== Bound Box with Oct Tree ====//
    m_OneMesh->LoadBndBox();

    m_OneMesh->WaterTightCheck( fid, m_TMeshVec );

    //==== Delete Old Meshes and Add One Mesh ====//
    for ( i = 0 ; i < ( int )m_TMeshVec.size() ; i++ )
    {
        delete m_TMeshVec[i];
    }

    m_TMeshVec.clear();
    m_TMeshVec.push_back( m_OneMesh );
}

void MeshGeom::MergeRemoveOpenMeshes( MeshInfo* info )
{
    int i, j;

    //==== Check If All Closed ====//
    for ( i = 0 ; i < ( int )m_TMeshVec.size() ; i++ )
    {
        m_TMeshVec[i]->CheckIfClosed();
    }

    //==== Try to Merge Non Closed Meshes ====//
    for ( i = 0 ; i < ( int )m_TMeshVec.size() ; i++ )
    {
        for ( j = i + 1 ; j < ( int )m_TMeshVec.size() ; j++ )
        {
            m_TMeshVec[i]->MergeNonClosed( m_TMeshVec[j] );
        }
    }
    for ( i = 0 ; i < ( int )m_TMeshVec.size() ; i++ )
    {
        if ( m_TMeshVec[i]->m_DeleteMeFlag )
        {
            info->m_NumOpenMeshesMerged++;
        }
    }

    for ( i = 0 ; i < ( int )m_TMeshVec.size() ; i++ )
    {
        if ( m_TMeshVec[i]->m_NonClosedTriVec.size() )
        {
            if ( !m_TMeshVec[i]->m_DeleteMeFlag )
            {
                info->m_NumOpenMeshedDeleted++;
            }

            m_TMeshVec[i]->m_DeleteMeFlag = true;
        }
    }

    //==== Remove Merged Meshes ====//
    vector< TMesh* > newTMeshVec;
    for ( i = 0 ; i < ( int )m_TMeshVec.size() ; i++ )
    {
        if ( !m_TMeshVec[i]->m_DeleteMeFlag )
        {
            newTMeshVec.push_back( m_TMeshVec[i] );
        }
        else
        {
            delete m_TMeshVec[i];
        }
    }
    m_TMeshVec = newTMeshVec;

    //==== Remove Any Degenerate Tris ====//
    for ( i = 0 ; i < ( int )m_TMeshVec.size() ; i++ )
    {
        info->m_NumDegenerateTriDeleted += m_TMeshVec[i]->RemoveDegenerate();
    }

}

void MeshGeom::PreMerge()
{
    // This method pre-merges each TMesh in the TMeshVec. This builds node and edge alias maps
    // in order to perform an intersection between non-merged meshes in UW space

    for ( int i = 0; i < ( int )m_TMeshVec.size(); i++ )
    {
        m_TMeshVec[i]->BuildMergeMaps();
    }
}

void MeshGeom::AddHalfBox()
{
    //==== Find Bound Box ====//
    BndBox box;
    for ( int m = 0 ; m < ( int )m_TMeshVec.size() ; m++ )
    {
        for ( int t = 0 ; t < ( int )m_TMeshVec[m]->m_TVec.size() ; t++ )
        {
            box.Update( m_TMeshVec[m]->m_TVec[t]->m_N0->m_Pnt );
        }
    }

    //==== Make The Box a Bit Bigger ===//
    vec3d bscale = vec3d( 2, 2, 2 );
    box.Scale( bscale );

    //==== Build Box Triangles =====//
    TMesh* tm = new TMesh();
    tm->m_HalfBoxFlag = true;

    m_TMeshVec.push_back( tm );

    int num_div = 10;
    double xmin = box.GetMin( 0 );
    double xmax = box.GetMax( 0 );
    double xdel = ( xmax - xmin ) / ( double )( num_div - 1 );

    double zmin = box.GetMin( 2 );
    double zmax = box.GetMax( 2 );
    double zdel = ( zmax - zmin ) / ( double )( num_div - 1 );

    double ymin = box.GetMin( 1 );

    ////==== Y=0 Plane ====//
    //for ( int i = 0 ; i < num_div-1 ; i++ )
    //{
    //  for ( int j = 0 ; j < num_div-1 ; j++ )
    //  {
    //      double x0 = xmin + (double)i*xdel;
    //      double x1 = xmin + (double)(i+1)*xdel;
    //      double z0 = zmin + (double)j*zdel;
    //      double z1 = zmin + (double)(j+1)*zdel;

    //      tm->addTri( vec3d(x0, 0, z0), vec3d( x1, 0, z0 ), vec3d( x0,  0, z1 ), vec3d(0,1,0) );
    //      tm->addTri( vec3d(x1, 0, z0), vec3d( x1, 0, z1 ), vec3d( x0,  0, z1 ), vec3d(0,1,0) );
    //  }
    //}

    //==== Add Other Sides ====//
    vec3d A = vec3d( xmin, 0, zmin );
    vec3d B = vec3d( xmax, 0, zmin );
    vec3d C = vec3d( xmin, 0, zmax );
    vec3d D = vec3d( xmax, 0, zmax );
    vec3d E = vec3d( xmin, ymin, zmin );
    vec3d F = vec3d( xmax, ymin, zmin );
    vec3d G = vec3d( xmin, ymin, zmax );
    vec3d H = vec3d( xmax, ymin, zmax );

    //tm->addTri( E, G, H, vec3d(0,-1,0) );
    //tm->addTri( E, H, F, vec3d(0,-1,0) );

    //tm->addTri( A, B, D, vec3d(0, 1,0) );
    //tm->addTri( A, D, C, vec3d(0, 1,0) );

    //tm->addTri( A, C, E, vec3d( -1, 0, 0) );
    //tm->addTri( C, G, E, vec3d( -1, 0, 0) );

    //tm->addTri( B, F, D, vec3d(  1, 0, 0) );
    //tm->addTri( D, F, H, vec3d(  1, 0, 0) );

    //tm->addTri( C, D, G, vec3d( 0, 0, 1) );
    //tm->addTri( D, H, G, vec3d( 0, 0, 1) );

    //tm->addTri( A, E, B, vec3d( 0, 0, -1) );
    //tm->addTri( B, E, F, vec3d( 0, 0, -1) );

    tm->AddTri( G, E, H, vec3d( 0, -1, 0 ) );
    tm->AddTri( H, E, F, vec3d( 0, -1, 0 ) );

    tm->AddTri( B, A, D, vec3d( 0, 1, 0 ) );
    tm->AddTri( D, A, C, vec3d( 0, 1, 0 ) );

    tm->AddTri( C, A, E, vec3d( -1, 0, 0 ) );
    tm->AddTri( G, C, E, vec3d( -1, 0, 0 ) );

    tm->AddTri( F, B, D, vec3d(  1, 0, 0 ) );
    tm->AddTri( F, D, H, vec3d(  1, 0, 0 ) );

    tm->AddTri( D, C, G, vec3d( 0, 0, 1 ) );
    tm->AddTri( H, D, G, vec3d( 0, 0, 1 ) );

    tm->AddTri( E, A, B, vec3d( 0, 0, -1 ) );
    tm->AddTri( E, B, F, vec3d( 0, 0, -1 ) );

}

vector<TMesh*> MeshGeom::CreateTMeshVec()
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

void MeshGeom::FlattenTMeshVec()
{
    vector<TMesh*> flatTMeshVec;
    flatTMeshVec.resize( m_TMeshVec.size() );
    for ( int i = 0 ; i < ( int )m_TMeshVec.size() ; i++ )
    {
        flatTMeshVec[i] = new TMesh();
        flatTMeshVec[i]->CopyFlatten( m_TMeshVec[i] );
        delete m_TMeshVec[i];
    }
    m_TMeshVec.clear();
    m_TMeshVec = flatTMeshVec;
}

void MeshGeom::FlattenSliceVec()
{
    vector<TMesh*> flatTMeshVec;
    flatTMeshVec.resize( m_SliceVec.size() );
    for ( int i = 0 ; i < ( int )m_SliceVec.size() ; i++ )
    {
        flatTMeshVec[i] = new TMesh();
        flatTMeshVec[i]->CopyFlatten( m_SliceVec[i] );
        delete m_SliceVec[i];
    }
    m_SliceVec.clear();
    m_SliceVec = flatTMeshVec;
}

//==== Get Total Transformation Matrix from Original Points ====//
Matrix4d MeshGeom::GetTotalTransMat()
{
    Matrix4d retMat;
    retMat.initMat( m_ScaleMatrix.data() );
    retMat.postMult( m_ModelMatrix.data() );

    return retMat;
}

//==== Get the Names of the TMeshes ====//
vector< string > MeshGeom::GetTMeshNames()
{
    vector< string > names;
    for ( int i = 0 ; i < ( int )m_TMeshVec.size() ; i++ )
    {
        names.push_back( m_TMeshVec[i]->m_NameStr.append( to_string( ( long long )m_TMeshVec[i]->m_SurfNum ) ) );
    }

    return names;
}

//==== Subtag All Trianlges ====//
void MeshGeom::SubTagTris()
{
    // Clear out the current Subtag Maps
    SubSurfaceMgr.ClearTagMaps();
    SubSurfaceMgr.m_CompNames = GetTMeshNames();
    SubSurfaceMgr.SetSubSurfTags( GetNumNascartParts() );
    SubSurfaceMgr.BuildCompNameMap();

    for ( int i = 0 ; i < ( int )m_TMeshVec.size() ; i++ )
    {
        m_TMeshVec[i]->SubTag( i + 1 );
    }

    SubSurfaceMgr.BuildSingleTagMap();

}

