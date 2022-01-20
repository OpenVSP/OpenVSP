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
#include "PtCloudGeom.h"
#include "LinkMgr.h"
#include "Vehicle.h"
#include "PntNodeMerge.h"

#include "StringUtil.h"
#include "StlHelper.h"

#include "SubSurfaceMgr.h"
#include "VspUtil.h"

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

    m_TotalTheoArea = m_TotalWetArea = 0.0;
    m_TotalTheoVol  = m_TotalWetVol  = 0.0;

    m_CenterOfGrav = vec3d( 0, 0, 0 );
    m_TotalMass = 0.0;
    m_TotalIxx = 0.0;
    m_TotalIyy = 0.0;
    m_TotalIzz = 0.0;
    m_TotalIxy = 0.0;
    m_TotalIxz = 0.0;
    m_TotalIyz = 0.0;

    m_ScaleMatrix.loadIdentity();
    m_ScaleFromOrig.Init( "Scale_From_Original", "XForm", this, 1, 1.0e-5, 1.0e12 );

    m_ViewMeshFlag.Init( "MeshFlag", "Draw", this, true, 0, 1 );
    m_ViewSliceFlag.Init( "SliceFlag", "Draw", this, true, 0, 1 );

    m_StartColorDegree.Init( "StartColorDegree", "Draw", this, 0, 0, 359 ); 
    m_StartColorDegree.SetDescript( "Color degree on color wheel for 1st mesh, where 0 degrees is red, 120 degrees is green, 240 degrees is blue" );

    // Debug
    m_DrawType.Init( "Draw_Type", "Draw", this, DRAW_XYZ, DRAW_XYZ, DRAW_TAGS );
    m_DrawSubSurfs.Init( "Draw_Sub_UV", "Debug", this, 0, 0, 1 );
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
    if ( ( fp = fopen( file_name, "r" ) ) == ( FILE * )NULL )
    {
        return 0;
    }

    //==== Read first Line of file and compare against expected header ====//
    fscanf( fp, "%s INPUT FILE\n\n", str );
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

        //==== Convert CrossSections to Triangles ====//
        for ( int i = 1 ; i < ( int )crossVec.size() ; i++ )
            for ( int j = 1 ; j < ( int )crossVec[i].size() ; j++ )
            {
                tMesh->AddTri( crossVec[i - 1][j - 1], crossVec[i][j - 1], crossVec[i][j] );

                tMesh->AddTri( crossVec[i - 1][j - 1], crossVec[i][j], crossVec[i - 1][j] );
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

                fpos_t pos;
                fgetpos( file_id, &pos );

                if ( EOF == fscanf( file_id, "%s %*s\n", str ) )
                {
                    break;
                }

                if ( strcmp( str, "endsolid" ) == 0 )
                {
                    fgets( str, 255, file_id );
                    if ( feof( file_id ) )
                    {
                        break;
                    }
                }
                else
                {
                    fsetpos( file_id, &pos );
                }
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
                tPtr->m_Norm = vec3d( nx, ny, nz );
                tMesh->m_TVec.push_back( tPtr );

                //==== Put Nodes Into Tri ====//
                tPtr->m_N0 = new TNode();
                tPtr->m_N1 = new TNode();
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

void MeshGeom::WriteStl( FILE* file_id, int tag )
{
    //==== Write Out Tris ====//
    for ( int i = 0 ; i < ( int )m_IndexedTriVec.size() ; i++ )
    {
        TTri* ttri = m_IndexedTriVec[i];

        int t = SubSurfaceMgr.GetTag( ttri->m_Tags );

        if ( t == tag )
        {
            vec3d p0 = ttri->m_N0->m_Pnt;
            vec3d p1 = ttri->m_N1->m_Pnt;
            vec3d p2 = ttri->m_N2->m_Pnt;
            vec3d v10 = p1 - p0;
            vec3d v20 = p2 - p1;
            vec3d norm = cross( v10, v20 );
            norm.normalize();

            fprintf( file_id, " facet normal  %2.10le %2.10le %2.10le\n",  norm.x(), norm.y(), norm.z() );
            fprintf( file_id, "   outer loop\n" );

            fprintf( file_id, "     vertex %2.10le %2.10le %2.10le\n", p0.x(), p0.y(), p0.z() );
            fprintf( file_id, "     vertex %2.10le %2.10le %2.10le\n", p1.x(), p1.y(), p1.z() );
            fprintf( file_id, "     vertex %2.10le %2.10le %2.10le\n", p2.x(), p2.y(), p2.z() );

            fprintf( file_id, "   endloop\n" );
            fprintf( file_id, " endfacet\n" );
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

void MeshGeom::InitIndexedMesh( const vector < TMesh* > &meshvec, int & offset )
{
    //==== Find All Exterior and Split Tris =====//
    for ( int m = 0 ; m < meshvec.size() ; m++ )
    {
        for ( int t = 0 ; t < ( int )meshvec[m]->m_TVec.size() ; t++ )
        {
            TTri* tri = meshvec[m]->m_TVec[t];
            if ( tri->m_SplitVec.size() )
            {
                for ( int s = 0 ; s < ( int )tri->m_SplitVec.size() ; s++ )
                {
                    if ( !tri->m_SplitVec[s]->m_IgnoreTriFlag )
                    {
                        char str[80];
                        sprintf( str, "%d", offset );
                        tri->m_SplitVec[s]->m_ID = string( str );
                        m_IndexedTriVec.push_back( tri->m_SplitVec[s] );
                    }
                }
            }
            else if ( !tri->m_IgnoreTriFlag )
            {
                char str[80];
                sprintf( str, "%d", offset );
                tri->m_ID = string( str );
                m_IndexedTriVec.push_back( tri );
            }
        }
        offset++;
    }
}

//==== Build Indexed Mesh ====//
void MeshGeom::BuildIndexedMesh( int partOffset, bool half_flag )
{
    m_IndexedTriVec.clear();
    m_IndexedNodeVec.clear();

    partOffset++;

    if ( m_ViewMeshFlag() )
    {
        InitIndexedMesh( m_TMeshVec, partOffset );
    }

    if ( m_ViewSliceFlag() )
    {
        InitIndexedMesh( m_SliceVec, partOffset );
    }

    //==== Collect All Points ====//
    vector< TNode* > allNodeVec;
    allNodeVec.reserve( m_IndexedTriVec.size() * 3 );
    for ( int t = 0 ; t < ( int )m_IndexedTriVec.size() ; t++ )
    {
        m_IndexedTriVec[t]->m_N0->m_ID = ( int )allNodeVec.size();
        allNodeVec.push_back( m_IndexedTriVec[t]->m_N0 );
        m_IndexedTriVec[t]->m_N1->m_ID = ( int )allNodeVec.size();
        allNodeVec.push_back( m_IndexedTriVec[t]->m_N1 );
        m_IndexedTriVec[t]->m_N2->m_ID = ( int )allNodeVec.size();
        allNodeVec.push_back( m_IndexedTriVec[t]->m_N2 );
    }
    vector< vec3d > allPntVec( allNodeVec.size() );
    for ( int i = 0 ; i < ( int )allNodeVec.size() ; i++ )
    {
        allPntVec[i] = allNodeVec[i]->m_Pnt;
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
    m_IndexedNodeVec.reserve( pnCloud.m_NumUsedPts );
    for ( int i = 0 ; i < ( int )allNodeVec.size() ; i++ )
    {
        if ( pnCloud.UsedNode( i ) )
        {
            m_IndexedNodeVec.push_back( allNodeVec[i] );
        }
    }

    //==== Set Adjusted Node IDs ====//
    for ( int i = 0 ; i < ( int )allNodeVec.size() ; i++ )
    {
        allNodeVec[i]->m_ID = pnCloud.GetNodeUsedIndex( i );
    }

    //==== Remove Any Bogus Tris ====//
    vector< TTri* > goodTriVec;
    goodTriVec.reserve( m_IndexedTriVec.size() );
    //==== Write Out Tris ====//
    for ( int t = 0 ; t < ( int )m_IndexedTriVec.size() ; t++ )
    {
        TTri* ttri = m_IndexedTriVec[t];
        if( ttri )
        {
            if ( ttri->m_N0->m_ID != ttri->m_N1->m_ID &&
                    ttri->m_N0->m_ID != ttri->m_N2->m_ID &&
                    ttri->m_N1->m_ID != ttri->m_N2->m_ID )
            {
                if ( half_flag && ( ( ( ttri->m_N0->GetXYZPnt() + ttri->m_N1->GetXYZPnt() + ttri->m_N2->GetXYZPnt() ) / 3. ).y() < 0 ) )
                {
                    // Don't keep tris with enter with -Y component
                    continue;
                }

                goodTriVec.push_back( ttri );
            }
        }
    }
    // Swap instead of assign to avoid copy.
    // m_IndexedTriVec = goodTriVec;
    swap( m_IndexedTriVec, goodTriVec );

    Update();
}

void MeshGeom::WriteNascartPnts( FILE* fp )
{
    vec3d v;
    Matrix4d XFormMat = GetTotalTransMat();
    //==== Write Out Nodes ====//
    for ( int i = 0 ; i < ( int )m_IndexedNodeVec.size() ; i++ )
    {
        TNode* tnode = m_IndexedNodeVec[i];
        // Apply Transformations
        if( tnode )
        {
            v = XFormMat.xform( tnode->m_Pnt );
            fprintf( fp, "%16.10g %16.10g %16.10g\n", v.x(), v.z(), -v.y() );
        }
    }
}

void MeshGeom::WriteCart3DPnts( FILE* fp )
{
    //==== Write Out Nodes ====//
    vec3d v;
    Matrix4d XFormMat = GetTotalTransMat();
    for ( int i = 0 ; i < ( int )m_IndexedNodeVec.size() ; i++ )
    {
        TNode* tnode = m_IndexedNodeVec[i];
        // Apply Transformations
        if( tnode )
        {
            v = XFormMat.xform( tnode->m_Pnt );
            fprintf( fp, "%16.10g %16.10g %16.10g\n", v.x(), v.y(),  v.z() );
        }
    }
}

void MeshGeom::WriteOBJPnts( FILE* fp )
{
    //==== Write Out Nodes ====//
    vec3d v;
    Matrix4d XFormMat = GetTotalTransMat();
    for ( int i = 0 ; i < ( int )m_IndexedNodeVec.size() ; i++ )
    {
        TNode* tnode = m_IndexedNodeVec[i];
        // Apply Transformations
        if( tnode )
        {
            v = XFormMat.xform( tnode->m_Pnt );
            fprintf( fp, "v %16.10g %16.10g %16.10g\n", v.x(), v.y(),  v.z() );
        }
    }
}

void MeshGeom::WriteVSPGeomPnts( FILE* file_id )
{
    //==== Write Out Nodes ====//
    vec3d v;
    Matrix4d XFormMat = GetTotalTransMat();
    for ( int i = 0 ; i < ( int )m_IndexedNodeVec.size() ; i++ )
    {
        TNode* tnode = m_IndexedNodeVec[i];
        // Apply Transformations
        v = XFormMat.xform( tnode->m_Pnt );
        fprintf( file_id, "%16.10g %16.10g %16.10g\n", v.x(), v.y(), v.z() ); // , tnode->m_UWPnt.x(), tnode->m_UWPnt.y() );
    }
}

int MeshGeom::WriteGMshNodes( FILE* fp, int node_offset )
{
    vec3d v;
    Matrix4d XFormMat = GetTotalTransMat();
    for ( int i = 0 ; i < ( int )m_IndexedNodeVec.size() ; i++ )
    {
        TNode* tnode = m_IndexedNodeVec[i];
        // Apply Transformations
        if( tnode )
        {
            v = XFormMat.xform( tnode->m_Pnt );
            fprintf( fp, "%d %16.10f %16.10f %16.10f\n", i + node_offset + 1,
                     v.x(), v.y(), v.z() );
        }
    }
    return node_offset + ( int )m_IndexedNodeVec.size();
}

void MeshGeom::WriteFacetNodes( FILE* fp )
{
    //==== Write Out Nodes ====//
    vec3d v;
    Matrix4d XFormMat = GetTotalTransMat();
    for ( int i = 0; i < (int)m_IndexedNodeVec.size(); i++ )
    {
        TNode* tnode = m_IndexedNodeVec[i];
        // Apply Transformations
        v = XFormMat.xform( tnode->m_Pnt );
        fprintf( fp, "%16.10g %16.10g %16.10g\n", v.x(), v.y(), v.z() );
    }
}

int MeshGeom::WriteNascartTris( FILE* fp, int off )
{
    //==== Write Out Tris ====//
    for ( int t = 0 ; t < ( int )m_IndexedTriVec.size() ; t++ )
    {
        TTri* ttri = m_IndexedTriVec[t];
        if( ttri )
        {
            fprintf( fp, "%d %d %d %d.0\n", ttri->m_N0->m_ID + 1 + off,  ttri->m_N2->m_ID + 1 + off,
                     ttri->m_N1->m_ID + 1 + off, SubSurfaceMgr.GetTag( ttri->m_Tags ) );
        }
    }

    return ( off + m_IndexedNodeVec.size() );
}

int MeshGeom::WriteCart3DTris( FILE* fp, int off )
{
    //==== Write Out Tris ====//
    for ( int t = 0 ; t < ( int )m_IndexedTriVec.size() ; t++ )
    {
        TTri* ttri = m_IndexedTriVec[t];
        if( ttri )
        {
            fprintf( fp, "%d %d %d\n", ttri->m_N0->m_ID + 1 + off,  ttri->m_N1->m_ID + 1 + off, ttri->m_N2->m_ID + 1 + off );
        }
    }

    return ( off + m_IndexedNodeVec.size() );
}

int MeshGeom::WriteOBJTris( FILE* fp, int off )
{
    //==== Write Out Tris ====//
    for ( int t = 0 ; t < ( int )m_IndexedTriVec.size() ; t++ )
    {
        TTri* ttri = m_IndexedTriVec[t];
        if( ttri )
        {
            fprintf( fp, "f %d %d %d\n", ttri->m_N0->m_ID + 1 + off,  ttri->m_N1->m_ID + 1 + off, ttri->m_N2->m_ID + 1 + off );
        }
    }

    return ( off + m_IndexedNodeVec.size() );
}

int MeshGeom::WriteVSPGeomTris( FILE* file_id, int offset )
{
    //==== Write Out Tris ====//
    for ( int t = 0 ; t < ( int )m_IndexedTriVec.size() ; t++ )
    {
        TTri* ttri = m_IndexedTriVec[t];
        fprintf(file_id, "3 %d %d %d\n", ttri->m_N0->m_ID + 1 + offset, ttri->m_N1->m_ID + 1 + offset, ttri->m_N2->m_ID + 1 + offset );
    }

    return ( offset + m_IndexedNodeVec.size() );
}

int MeshGeom::WriteGMshTris( FILE* fp, int node_offset, int tri_offset )
{
    //==== Write Out Tris ====//
    for ( int t = 0 ; t < ( int )m_IndexedTriVec.size() ; t++ )
    {
        TTri* ttri = m_IndexedTriVec[t];
        if( ttri )
        {
            fprintf( fp, "%d 2 0 %d %d %d\n", t + tri_offset + 1,
                     ttri->m_N0->m_ID + 1 + node_offset,  ttri->m_N2->m_ID + 1 + node_offset, ttri->m_N1->m_ID + 1 + node_offset );
        }
    }
    return ( tri_offset + m_IndexedTriVec.size() );
}

void MeshGeom::WriteFacetTriParts( FILE* fp, int &offset, int &tri_count, int &part_count )
{
    vector < string > geom_ID_vec;
    geom_ID_vec.resize( m_TMeshVec.size() );

    for ( unsigned int i = 0; i < m_TMeshVec.size(); i++ )
    {
        geom_ID_vec[i] = m_TMeshVec[i]->m_PtrID;
    }

    vector < int > tri_offset; // vector of number of tris for each tag

    int materialID = 0; // Default Material ID of PEC (Referred to as "iCoat" in XPatch facet file documentation)

    vector < int > all_tag_vec = SubSurfaceMgr.GetAllTags(); // vector of tags, where each tag identifies a part or group of facets

    //==== Get # of facets for each part ====//
    for ( unsigned int i = 0; i < all_tag_vec.size(); i++ )
    {
        int tag_count = 0;

        for ( unsigned int j = 0; j < m_IndexedTriVec.size(); j++ )
        {
            if ( all_tag_vec[i] == SubSurfaceMgr.GetTag( m_IndexedTriVec[j]->m_Tags ) )
            {
                tag_count++;
            }
        }

        tri_offset.push_back( tag_count );
    }

    // Remove indexes of tri_offset that contain no tris
    for ( unsigned int j = 0; j < tri_offset.size(); j++ )
    {
        if ( tri_offset[j] == 0 ) // This indicates no tris for the tag index. 
        {
            // Erase to avoid writing and counting parts with no tris
            tri_offset.erase( tri_offset.begin() + j );
            all_tag_vec.erase( all_tag_vec.begin() + j );
            j--;
        }
    }

    fprintf( fp, "%ld \n", tri_offset.size() ); // # of "Small" parts, based on the total number of tags

    //==== Write Out Tris ====//
    for ( unsigned int i = 0; i < all_tag_vec.size(); i++ )
    {
        int curr_tag = all_tag_vec[i];
        bool new_section = true; // flag to write small part section header

        for ( unsigned int j = 0; j < m_IndexedTriVec.size(); j++ )
        {
            if ( curr_tag == SubSurfaceMgr.GetTag( m_IndexedTriVec[j]->m_Tags ) ) // only write out current tris for surrent tag
            {
                if ( new_section ) // write small part header and get material ID for small part
                {
                    string name = SubSurfaceMgr.GetTagNames( m_IndexedTriVec[j]->m_Tags );
                    fprintf( fp, "%s\n", name.c_str() ); // Write name of small part
                    fprintf( fp, "%d 3\n", tri_offset[i] ); // Number of facets for the part, 3 nodes per facet

                    new_section = false;
                }

                TTri* ttri = m_IndexedTriVec[j];

                tri_count++; // counter for number of tris/facets

                // 3 nodes of facet, material ID, component ID, running facet #:
                fprintf( fp, "%d %d %d %d %u %d\n", ttri->m_N0->m_ID + 1 + offset, ttri->m_N1->m_ID + 1 + offset, ttri->m_N2->m_ID + 1 + offset, materialID, i + 1 + part_count, tri_count );
            }
        }
    }

    part_count += tri_offset.size();
    offset += m_IndexedNodeVec.size();
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

int MeshGeom::WriteCart3DParts( FILE* fp  )
{
    //==== Write Component IDs for each Tri =====//
    int tag;
    for ( int t = 0 ; t < ( int )m_IndexedTriVec.size() ; t++ )
    {
        tag = SubSurfaceMgr.GetTag( m_IndexedTriVec[t]->m_Tags );

        fprintf( fp, "%d \n",  tag );
    }
    return 0;
}

int MeshGeom::WriteVSPGeomParts( FILE* file_id  )
{
    //==== Write Component IDs for each Tri =====//
    int tag;
    for ( int t = 0 ; t < ( int )m_IndexedTriVec.size() ; t++ )
    {
        TTri* ttri = m_IndexedTriVec[t];
        tag = SubSurfaceMgr.GetTag( ttri->m_Tags );

        fprintf( file_id, "%d %16.10g %16.10g %16.10g %16.10g %16.10g %16.10g\n", tag,
                 ttri->m_N0->m_UWPnt.x(), ttri->m_N0->m_UWPnt.y(),
                 ttri->m_N1->m_UWPnt.x(), ttri->m_N1->m_UWPnt.y(),
                 ttri->m_N2->m_UWPnt.x(), ttri->m_N2->m_UWPnt.y() );
    }
    return 0;
}

// Wake edges are created such that N0.u < N1.u.
// This comparator sorts first by sgn(N0.y), abs(N0.y), then N0.u and N1.u.
bool OrderWakeEdges ( const TEdge &a, const TEdge &b )
{
    if ( sgn( a.m_N0->m_Pnt.y() ) < sgn( b.m_N0->m_Pnt.y() ) ) return true;
    if ( sgn( b.m_N0->m_Pnt.y() ) < sgn( a.m_N0->m_Pnt.y() ) ) return false;

    if ( abs( a.m_N0->m_Pnt.y() ) < abs( b.m_N0->m_Pnt.y() ) ) return true;
    if ( abs( b.m_N0->m_Pnt.y() ) < abs( a.m_N0->m_Pnt.y() ) ) return false;

    if ( a.m_N0->m_UWPnt.x() < b.m_N0->m_UWPnt.x() ) return true;
    if ( b.m_N0->m_UWPnt.x() < a.m_N0->m_UWPnt.x() ) return false;

    if ( a.m_N1->m_UWPnt.x() < b.m_N1->m_UWPnt.x() ) return true;
    if ( b.m_N1->m_UWPnt.x() < a.m_N1->m_UWPnt.x() ) return false;

    if ( a.m_N0->m_Pnt.x() < b.m_N0->m_Pnt.x() ) return true;
    if ( b.m_N0->m_Pnt.x() < a.m_N0->m_Pnt.x() ) return false;

    if ( a.m_N0->m_Pnt.z() < b.m_N0->m_Pnt.z() ) return true;
    if ( b.m_N0->m_Pnt.z() < a.m_N0->m_Pnt.z() ) return false;

    if ( sgn( a.m_N1->m_Pnt.y() ) < sgn( b.m_N1->m_Pnt.y() ) ) return true;
    if ( sgn( b.m_N1->m_Pnt.y() ) < sgn( a.m_N1->m_Pnt.y() ) ) return false;

    if ( abs( a.m_N1->m_Pnt.y() ) < abs( b.m_N1->m_Pnt.y() ) ) return true;
    if ( abs( b.m_N1->m_Pnt.y() ) < abs( a.m_N1->m_Pnt.y() ) ) return false;

    if ( a.m_N1->m_Pnt.x() < b.m_N1->m_Pnt.x() ) return true;
    if ( b.m_N1->m_Pnt.x() < a.m_N1->m_Pnt.x() ) return false;

    if ( a.m_N1->m_Pnt.z() < b.m_N1->m_Pnt.z() ) return true;
    if ( b.m_N1->m_Pnt.z() < a.m_N1->m_Pnt.z() ) return false;

    return false;
}

bool AboutEqualWakeNodes ( TNode *a, TNode *b )
{
    if ( aboutequal( a->m_Pnt.y(), b->m_Pnt.y() )
      && aboutequal( a->m_Pnt.x(), b->m_Pnt.x() )
      && aboutequal( a->m_Pnt.z(), b->m_Pnt.z() )
      && aboutequal( a->m_UWPnt.x(), b->m_UWPnt.x() )
      && aboutequal( a->m_UWPnt.y(), b->m_UWPnt.y() ) ) return true;

    return false;
}

bool AboutEqualWakeEdges ( const TEdge &a, const TEdge &b )
{
    if ( AboutEqualWakeNodes( a.m_N0, b.m_N0 )
      && AboutEqualWakeNodes( a.m_N1, b.m_N1 ) ) return true;

    return false;
}

int MeshGeom::WriteVSPGeomWakes( FILE* file_id, int offset )
{
    vector < TEdge > wakeedges;

    for ( int t = 0 ; t < ( int )m_IndexedTriVec.size() ; t++ )
    {
        TTri *ttri = m_IndexedTriVec[t];
        int we = ttri->WakeEdge();

        if ( we > 0 )
        {
            TEdge e;
            if ( we == 1 )
            {
                e = TEdge( ttri->m_N0, ttri->m_N1, ttri );
            }
            else if ( we == 2 )
            {
                e = TEdge( ttri->m_N1, ttri->m_N2, ttri );
            }
            else
            {
                e = TEdge( ttri->m_N2, ttri->m_N0, ttri );
            }
            e.SortNodesByU();
            wakeedges.push_back( e );
        }
    }

    sort( wakeedges.begin(), wakeedges.end(), OrderWakeEdges );

    vector < TEdge >::iterator it;
    it = unique( wakeedges.begin(), wakeedges.end(), AboutEqualWakeEdges );

    wakeedges.resize( distance( wakeedges.begin(), it ) );

    list < TEdge > wlist( wakeedges.begin(), wakeedges.end() );

    vector < deque < TEdge > > wakes;
    int iwake = 0;

    while ( !wlist.empty() )
    {
        list < TEdge >::iterator wit = wlist.begin();

        iwake = wakes.size();
        wakes.resize( iwake + 1 );
        wakes[iwake].push_back( *wit );
        wit = wlist.erase( wit );

        while ( wit != wlist.end() )
        {
            if ( AboutEqualWakeNodes( wakes[iwake].back().m_N1, (*wit).m_N0 ) )
            {
                wakes[iwake].push_back( *wit );
                wlist.erase( wit );
                wit = wlist.begin();
                continue;
            }
            else if ( AboutEqualWakeNodes( wakes[iwake].begin()->m_N0, (*wit).m_N1 ) )
            {
                wakes[iwake].push_front( *wit );
                wlist.erase( wit );
                wit = wlist.begin();
                continue;
            }
            wit++;
        }
    }


    int nwake = wakes.size();

    m_PolyVec.resize( nwake );
    fprintf( file_id, "%d\n", nwake );

    for ( iwake = 0; iwake < nwake; iwake++ )
    {
        int iprt = 0;
        int iwe;
        int nwe = wakes[iwake].size();
        m_PolyVec[iwake].resize( nwe + 1 );
        fprintf( file_id, "%d ", nwe + 1 );

        for ( iwe = 0; iwe < nwe; iwe++ )
        {
            fprintf( file_id, "%d", wakes[iwake][iwe].m_N0->m_ID + 1 + offset );
            m_PolyVec[iwake][iwe] = wakes[iwake][iwe].m_N0->m_Pnt;

            if ( iprt < 9 )
            {
                fprintf( file_id, " " );
                iprt++;
            }
            else
            {
                fprintf( file_id, "\n" );
                iprt = 0;
            }
        }
        fprintf( file_id, "%d\n", wakes[iwake][iwe - 1].m_N1->m_ID + 1 + offset );
        m_PolyVec[iwake][iwe] = wakes[iwake][iwe - 1].m_N1->m_Pnt;
    }

    return ( offset + m_IndexedNodeVec.size() );
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

//==== Generate Cross Sections =====//
void MeshGeom::UpdateBBox()
{
    int i, j;
    m_BBox.Reset();
    Matrix4d transMat = GetTotalTransMat();
    if ( m_TMeshVec.size() > 0 )
    {
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
    else
    {
        m_BBox.Update( vec3d( 0.0, 0.0, 0.0 ));
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

            map<int, DrawObj*> tag_dobj_map;
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

        m_FeatureDrawObj_vec[i].m_PntVec.resize( m_PolyVec[i].size() );
        m_FeatureDrawObj_vec[i].m_LineWidth = 5;
        m_FeatureDrawObj_vec[i].m_LineColor = rgb;

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
            m_WireShadeDrawObj_vec[i].m_Type = DrawObj::VSP_HIDDEN_TRIS;
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


    for ( int i = 0; i < m_FeatureDrawObj_vec.size(); i++ )
    {
        m_FeatureDrawObj_vec[i].m_Type = DrawObj::VSP_LINE_STRIP;
    }

}

//==== Create And Load Tris into Results Data Structures ====//
void MeshGeom::CreateGeomResults( Results* res )
{
    if ( m_TMeshVec.size() && m_SliceVec.size() )
    {
        res->Add( NameValData( "Type", vsp::MESH_INDEX_AND_SLICE_TRI ) );
    }
    else if ( m_TMeshVec.size() )
    {
        res->Add( NameValData( "Type", vsp::MESH_INDEXED_TRI ) );
    }
    else if ( m_SliceVec.size() )
    {
        res->Add( NameValData( "Type", vsp::MESH_SLICE_TRI ) );
    }

    //==== Add Index Tris =====//
    if ( m_TMeshVec.size() )
    {
        BuildIndexedMesh( 0 );

        vector< vec3d > pvec;
        Matrix4d XFormMat = GetTotalTransMat();
        //==== Write Out Nodes ====//
        for ( int i = 0 ; i < ( int )m_IndexedNodeVec.size() ; i++ )
        {
            TNode* tnode = m_IndexedNodeVec[i];
            pvec.push_back( XFormMat.xform( tnode->m_Pnt ) );
        }
        res->Add( NameValData( "Num_Pnts", ( int )m_IndexedNodeVec.size() ) );
        res->Add( NameValData( "Tri_Pnts", pvec ) );

        //==== Write Out Tris ====//
        vector< int > id0_vec;
        vector< int > id1_vec;
        vector< int > id2_vec;
        for ( int t = 0 ; t < ( int )m_IndexedTriVec.size() ; t++ )
        {
            TTri* ttri = m_IndexedTriVec[t];

            id0_vec.push_back( ttri->m_N0->m_ID );
            id1_vec.push_back( ttri->m_N1->m_ID );
            id2_vec.push_back( ttri->m_N2->m_ID );
        }
        res->Add( NameValData( "Num_Tris", ( int )m_IndexedTriVec.size() ) );
        res->Add( NameValData( "Tri_Index0", id0_vec ) );
        res->Add( NameValData( "Tri_Index1", id1_vec ) );
        res->Add( NameValData( "Tri_Index2", id2_vec ) );
    }

    //==== Add Slices =====//
    if ( m_SliceVec.size() )
    {
        //==== Load m_SliceVec ====//
        res->Add( NameValData( "Num_Slices", ( int )m_SliceVec.size() ) );
        for ( int i = 0; i < ( int )m_SliceVec.size(); i++ )
        {
            res->Add( NameValData( "Num_Slice_Tris", ( int )( int )m_SliceVec[i]->m_TVec.size() ) );
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
            res->Add( NameValData( "Slice_Tris_Pnt_0", slice_tri_n0_vec ) );
            res->Add( NameValData( "Slice_Tris_Pnt_1", slice_tri_n1_vec ) );
            res->Add( NameValData( "Slice_Tris_Pnt_2", slice_tri_n2_vec ) );
        }
    }
}

void MeshGeom::CreatePtCloudGeom()
{
    BuildIndexedMesh( 0 );
    vector < TNode* > nvec = m_IndexedNodeVec;
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
            m_Vehicle->UpdateGui();

        }
    }
}

void MeshGeom::Scale()
{
    double currentScale = m_Scale() / m_LastScale();
    m_ScaleFromOrig *= currentScale;
    m_ScaleMatrix.loadIdentity();
    m_ScaleMatrix.scale( m_ScaleFromOrig() );
    m_LastScale = m_Scale();
}

void MeshGeom::ApplyScale()
{
    if ( std::abs( m_LastScale() - m_Scale() ) < 0.0000001 )
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
                m_TMeshVec[i]->m_XYZPnts[j][k] = m_TMeshVec[i]->m_XYZPnts[j][k] * ( m_Scale() / m_LastScale() );
            }
        }
    }
    map<TNode*, int >::const_iterator iter;
    for ( iter = nodeMap.begin() ; iter != nodeMap.end() ; ++iter )
    {
        TNode* n = iter->first;
        n->m_Pnt = n->m_Pnt * ( m_Scale() / m_LastScale() );
    }

    m_LastScale = m_Scale();
}

void MeshGeom::TransformMeshVec( vector<TMesh*> & meshVec, const Matrix4d & TransMat ) const
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
    for ( iter = nodeMap.begin() ; iter != nodeMap.end() ; ++iter )
    {
        TNode* n = iter->first;
        n->m_Pnt = TransMat.xform( n->m_Pnt );
    }

    vec3d zeroV = vec3d( 0.0, 0.0, 0.0 );
    zeroV = TransMat.xform( zeroV );

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
                    tri->m_Norm = TransMat.xform( tri->m_Norm ) - zeroV;
                }
            }
            else
            {
                TTri* tri = meshVec[i]->m_TVec[j];
                tri->m_Norm = TransMat.xform( tri->m_Norm ) - zeroV;
            }
        }

       // Apply Transformation to Mesh's area center
       meshVec[i]->m_AreaCenter = TransMat.xform( meshVec[i]->m_AreaCenter );
    }
}

void MeshGeom::IntersectTrim( vector< DegenGeom > &degenGeom, bool degen, int halfFlag, int intSubsFlag )
{
    int i, j;

    // Temporary variable -- likely pass up to top levels and make an option.
    bool deleteopen = false;

    //==== Check For Open Meshes and Merge or Delete Them ====//
    MeshInfo info;

    // This call is made later (after subsurface intersection) for the !degen case.
    // Perhaps this can be moved down there to match.
    if ( degen )
    {
        MergeRemoveOpenMeshes( &info, deleteopen );
    }

    int numTris = 0;
    if ( !degen )
    {
        //==== Count Tris ====//
        for ( i = 0 ; i < ( int )m_TMeshVec.size() ; i++ )
        {
            if ( !m_TMeshVec[i]->m_HalfBoxFlag )
            {
                numTris += m_TMeshVec[i]->m_TVec.size();
            }
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

    Results* res = NULL;
    if ( !degen )
    {
        //==== Create Results ====//
        res = ResultsMgr.CreateResults( "Comp_Geom" );
        res->Add( NameValData( "Num_Comps", ( int )compIdVec.size() ) );
        res->Add( NameValData( "Total_Num_Meshes", ( int )m_TMeshVec.size() ) );
        res->Add( NameValData( "Total_Num_Tris", numTris ) );
        res->Add( NameValData( "Mesh_GeomID", this->GetID() ) );
    }

    //==== Scale To 10 Units ====//
    UpdateBBox();
    m_LastScale = 1.0;
    m_Scale = 1000.0 / m_BBox.GetLargestDist();
    ApplyScale();

    if ( !degen )
    {
        //==== Intersect Subsurfaces to make clean lines ====//
        if ( intSubsFlag )
        {
            for ( i = 0 ; i < ( int )m_TMeshVec.size() ; i++ )
            {
                vector< TMesh* > sub_surf_meshes;
                vector< SubSurface* > sub_surf_vec = SubSurfaceMgr.GetSubSurfs( m_TMeshVec[i]->m_PtrID, m_TMeshVec[i]->m_SurfNum );
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
        }
        // Tag meshes before regular intersection
        SubTagTris( (bool)intSubsFlag );

        MergeRemoveOpenMeshes( &info, deleteopen );
    }

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
    //update_xformed_bbox();          // Load Xform BBox

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

    // Fill vector of cfdtypes so we don't have to pass TMeshVec all the way down.
    vector < int > bTypes( m_TMeshVec.size() );
    vector < bool > thicksurf( m_TMeshVec.size() );
    for ( i = 0 ; i < ( int )m_TMeshVec.size() ; i++ )
    {
        bTypes[i] = m_TMeshVec[i]->m_SurfCfdType;
        thicksurf[i] = m_TMeshVec[i]->m_ThickSurf;
    }

    //==== Mark which triangles to ignore ====//
    for ( i = 0 ; i < ( int )m_TMeshVec.size() ; i++ )
    {
        m_TMeshVec[i]->SetIgnoreTriFlag( m_TMeshVec, bTypes, thicksurf );
    }

    // Only ever set to true for !degen case.
    // The code is harmless for a !halfFlag case (optimization).
    // So, both protections could be eliminated.
    // This is the only appearance of halfFalg in this function, argument could be eliminated.
    if ( !degen )
    {
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

    double guessTotalWetVol = 0;
    for ( i = 0 ; i < ( int )m_TMeshVec.size() ; i++ )
    {
        m_TMeshVec[i]->m_GuessVol = m_TMeshVec[i]->m_TheoVol * m_TMeshVec[i]->m_WetArea / m_TMeshVec[i]->m_TheoArea;      // Guess
        m_TMeshVec[i]->m_WetVol = 0.0;
        guessTotalWetVol += m_TMeshVec[i]->m_GuessVol;
    }

    double leftOver = m_TotalWetVol;
    int leftOverCnt = 20;
    while ( leftOverCnt > 0 )
    {
        leftOverCnt--;

        double sumWetVol = 0.0;
        for ( i = 0 ; i < ( int )m_TMeshVec.size() ; i++ )
        {
            double perWetVol = m_TMeshVec[i]->m_GuessVol / guessTotalWetVol;
            m_TMeshVec[i]->m_WetVol += perWetVol * ( leftOver );

            int neg_vol_mult = 1; // Negative volume multiplier
            if ( m_TMeshVec[i]->m_SurfCfdType == vsp::CFD_NEGATIVE )
            {
                neg_vol_mult = -1;
            }

            if ( neg_vol_mult * m_TMeshVec[i]->m_WetVol > neg_vol_mult * m_TMeshVec[i]->m_TheoVol )
            {
                m_TMeshVec[i]->m_WetVol = m_TMeshVec[i]->m_TheoVol;
            }
            sumWetVol += m_TMeshVec[i]->m_WetVol;
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

    if ( degen )
    {
        bool matchFlag;
        vector< bool > matchVec( m_TMeshVec.size(), false );
        // For each degenGeom
        for ( i = 0; i < ( int )degenGeom.size(); i++ )
        {
            matchFlag = false;
            DegenPoint degenPoint = degenGeom[i].getDegenPoint();

            // Loop through tmesh vector
            for ( j = 0; j < m_TMeshVec.size(); j++ )
            {
                if ( matchVec[j] == false )
                {
                    // If its pointer id matches the current degenGeom
                    if ( degenGeom[i].getParentGeom()->GetID() == m_TMeshVec[j]->m_PtrID &&
                         degenGeom[i].getSurfNum() == m_TMeshVec[j]->m_SurfNum )
                    {
                        degenPoint.area.push_back( m_TMeshVec[j]->m_TheoArea );
                        degenPoint.areaWet.push_back( m_TMeshVec[j]->m_WetArea );
                        degenPoint.vol.push_back( m_TMeshVec[j]->m_TheoVol );
                        degenPoint.volWet.push_back( m_TMeshVec[j]->m_WetVol );
                        matchVec[j] = true;
                        matchFlag = true;
                    }
                }
            }
            if ( !matchFlag )
            {
                degenPoint.area.push_back( 0.0 );
                degenPoint.areaWet.push_back( 0.0 );
                degenPoint.vol.push_back( 0.0 );
                degenPoint.volWet.push_back( 0.0 );
            }

            degenGeom[i].setDegenPoint( degenPoint );
        }
    }

    if ( !degen )
    {

        int ntags = -1;
        vector < double > tagTheoAreaVec;
        vector < double > tagWetAreaVec;
        vector < string > tagNameVec;
        vector < string > tagIDVec;

        if ( intSubsFlag )
        {
            // Subtract off dummy tag.
            ntags = SubSurfaceMgr.GetNumTags() - 1;

            tagTheoAreaVec.resize( ntags, 0.0 );
            tagWetAreaVec.resize( ntags, 0.0 );
            tagNameVec.resize( ntags );
            tagIDVec.resize( ntags );

            for ( i = 0 ; i < ( int )m_TMeshVec.size() ; i++ )
            {
                for ( j = 0; j < ntags; j++ )
                {
                    tagTheoAreaVec[j] += m_TMeshVec[i]->m_TagTheoAreaVec[j];
                    tagWetAreaVec[j] += m_TMeshVec[i]->m_TagWetAreaVec[j];
                }
            }

            vector < int > tags = SubSurfaceMgr.GetAllTags();

            assert( tags.size() == ntags );

            for ( j = 0; j < ntags; j++ )
            {
                tagNameVec[j] = SubSurfaceMgr.GetTagNames( j );
                tagIDVec[j] = SubSurfaceMgr.GetTagIDs( j );
            }
        }

        //==== Add Results ====//
        vector< string > name_vec;
        vector< double > theo_area_vec;
        vector< double > wet_area_vec;
        vector< double > theo_vol_vec;
        vector< double > wet_vol_vec;

        res->Add( NameValData( "Num_Meshes", ( int )m_TMeshVec.size() ) );
        for ( i = 0 ; i < ( int )m_TMeshVec.size() ; i++ )
        {
            TMesh* tmsh = m_TMeshVec[i];
            name_vec.push_back( tmsh->m_NameStr );
            theo_area_vec.push_back( tmsh->m_TheoArea );
            wet_area_vec.push_back( tmsh->m_WetArea );
            theo_vol_vec.push_back( tmsh->m_TheoVol );
            wet_vol_vec.push_back( tmsh->m_WetVol );
        }

        res->Add( NameValData( "Comp_Name", name_vec ) );
        res->Add( NameValData( "Theo_Area", theo_area_vec ) );
        res->Add( NameValData( "Wet_Area", wet_area_vec ) );
        res->Add( NameValData( "Theo_Vol", theo_vol_vec ) );
        res->Add( NameValData( "Wet_Vol", wet_vol_vec ) );

        res->Add( NameValData( "Num_Tags", ntags ) );
        res->Add( NameValData( "Tag_Name", tagNameVec ) );
        res->Add( NameValData( "Tag_ID", tagIDVec ) );
        res->Add( NameValData( "Tag_Theo_Area", tagTheoAreaVec ) );
        res->Add( NameValData( "Tag_Wet_Area", tagWetAreaVec ) );

        res->Add( NameValData( "Total_Theo_Area", m_TotalTheoArea ) );
        res->Add( NameValData( "Total_Wet_Area", m_TotalWetArea ) );
        res->Add( NameValData( "Total_Theo_Vol", m_TotalTheoVol ) );
        res->Add( NameValData( "Total_Wet_Vol", m_TotalWetVol ) );

        res->Add( NameValData( "Num_Degen_Tris_Removed", info.m_NumDegenerateTriDeleted ) );
        res->Add( NameValData( "Num_Open_Meshes_Removed", info.m_NumOpenMeshedDeleted ) );
        res->Add( NameValData( "Num_Open_Meshes_Merged", info.m_NumOpenMeshesMerged ) );
        res->Add( NameValData( "Meshes_Removed_Names", info.m_DeletedMeshes ) );
        res->Add( NameValData( "Meshes_Merged_Names", info.m_MergedMeshes ) );

        string txtfn = m_Vehicle->getExportFileName( vsp::COMP_GEOM_TXT_TYPE );
        res->WriteCompGeomTxtFile( txtfn );

        //==== Write CSV File ====//
        if ( m_Vehicle->getExportCompGeomCsvFile() )
        {
            string csvfn = m_Vehicle->getExportFileName( vsp::COMP_GEOM_CSV_TYPE );
            res->WriteCompGeomCsvFile( csvfn );
        }

    }
}

//==== Call After BndBoxes Have Been Create But Before Intersect ====//
void MeshGeom::AreaSlice( int numSlices , vec3d norm_axis,
                          bool autoBounds, double start, double end )
{
    int tesselate = 0; // WARNING: Always false
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
    double tempMat[16] = {};
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
    MergeRemoveOpenMeshes( &info, false );

    //==== Create Results ====//
    Results* res = ResultsMgr.CreateResults( "Slice" );
    res->Add( NameValData( "Num_Degen_Triangles_Removed", info.m_NumDegenerateTriDeleted ) );
    res->Add( NameValData( "Num_Open_Meshes_Removed", info.m_NumOpenMeshedDeleted ) );
    res->Add( NameValData( "Num_Open_Meshes_Merged", info.m_NumOpenMeshesMerged ) );
    res->Add( NameValData( "Meshes_Removed_Names", info.m_DeletedMeshes ) );
    res->Add( NameValData( "Meshes_Merged_Names", info.m_MergedMeshes ) );
    res->Add( NameValData( "Mesh_GeomID", this->GetID() ) );


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

    res->Add( NameValData( "Num_Comps", ( int )compIdVec.size() ) );
    res->Add( NameValData( "Num_Meshes", ( int )m_TMeshVec.size() ) );
    res->Add( NameValData( "Num_Tris", numTris ) );
    res->Add( NameValData( "Axis_Vector", norm_axis ) );

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

    vec3d norm( 1, 0, 0 );

    double dxSlice = 1.0;
    if ( numSlices > 1 )
    {
        dxSlice = ( xMax - xMin ) / ( double )( numSlices - 1 );
    }

    vector< double > loc_vec;
    for ( s = 0 ; s < numSlices ; s++ )
    {
        TMesh* tm = new TMesh();
        m_SliceVec.push_back( tm );

        tm->m_ThickSurf = false;
        tm->m_SurfCfdType = vsp::CFD_STRUCTURE;

        double x = xMin + ( double )s * dxSlice;
        loc_vec.push_back( x );

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

    // Fill vector of cfdtypes so we don't have to pass TMeshVec all the way down.
    vector < int > bTypes( m_TMeshVec.size() );
    vector < bool > thicksurf( m_TMeshVec.size() );
    for ( i = 0 ; i < ( int )m_TMeshVec.size() ; i++ )
    {
        bTypes[i] = m_TMeshVec[i]->m_SurfCfdType;
        thicksurf[i] = m_TMeshVec[i]->m_ThickSurf;
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

            m_TMeshVec[i]->RemoveIsectEdges();
        }

        //==== Split Intersected Tri in Mesh ====//
        tm->Split();

        //==== Determine Which Triangle Are Interior/Exterior ====//
        tm->DeterIntExt( m_TMeshVec );

        //==== Mark which triangles to ignore ====//
        tm->SetIgnoreTriFlag( m_TMeshVec, bTypes, thicksurf );

    }

    TransMat.affineInverse();

    vector< double > area_vec;
    vector < vec3d > AreaCenter;
    for ( s = 0 ; s < ( int )m_SliceVec.size() ; s++ )
    {
        m_SliceVec[s]->ComputeWetArea();
        area_vec.push_back( m_SliceVec[s]->m_WetArea );
        AreaCenter.push_back( TransMat.xform( m_SliceVec[s]->m_AreaCenter ) );
    }
    res->Add( NameValData( "Slice_Area_Center", AreaCenter ) );
    res->Add( NameValData( "Num_Slices", ( int )m_SliceVec.size() ) );
    res->Add( NameValData( "Slice_Loc", loc_vec ) );
    res->Add( NameValData( "Slice_Area", area_vec ) );

    string filename = m_Vehicle->getExportFileName( vsp::SLICE_TXT_TYPE );
    res->WriteSliceFile( filename );

    //==== TransForm Slices and Mesh to Match Original Coord Sys ====//
    TransformMeshVec( m_SliceVec, TransMat );
    TransformMeshVec( m_TMeshVec, TransMat );
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

                    double xcon = beta * cos( theta - 0.5 * PI ) * ( pt.y() - center.y() ) + beta * sin( theta + 0.5 * PI ) * ( pt.z() - center.z() );
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
        double tubedist = 0.5 * size * sin( 0.5*PI - sliceAngle) / sin( sliceAngle );
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

    for ( int i = 0 ; i < ( int )m_TMeshVec.size() ; i++ )
    {
        // Get vector of all subsurface pointers in current TMesh
        vector< SubSurface* > sub_surf_vec = SubSurfaceMgr.GetSubSurfs( m_TMeshVec[i]->m_PtrID, m_TMeshVec[i]->m_SurfNum );

        for ( int ssv = 0 ; ssv < ( int )sub_surf_vec.size(); ssv++ )
        {
            // Populate vector of TMesh* for current subsurface
            vector< TMesh* > sub_surf_meshes;
            string subsurf_id = sub_surf_vec[ssv]->GetID();
            if ( vector_contains_val( Flow_vec, subsurf_id ) )
            {
                vector< TMesh* > tmp_vec = sub_surf_vec[ssv]->CreateTMeshVec();
                sub_surf_meshes.insert( sub_surf_meshes.end(), tmp_vec.begin(), tmp_vec.end() );
            }

            if ( !sub_surf_meshes.size() )
            {
                continue;
            }

            // Create tube extension location
            double tloc = 0;
            bool ambflag = 0;
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
                    ambflag = 1;
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
                            tempmesh->AddTri( tt0_N0, tt0_N1, tt0_N2, tt0_norm );
                            tempmesh->AddTri( tt1_N0, tt1_N1, tt1_N2, tt1_norm );
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
    SubTagTris( 1 );

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
        string id = m_TMeshVec[i]->m_PtrID;
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
    MergeRemoveOpenMeshes( &info );

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
                gp[m].rotate_y( cos(-( 0.5*PI-sliceAngle)), sin(-( 0.5*PI-sliceAngle)) );
                // Rotate plane to current theta
                gp[m].rotate_x( cos(theta), sin(theta) );
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
            tm->AddTri( gp[2], gp[3], gp[0], gpnorm );
            tm->AddTri( gp[2], gp[0], gp[1], gpnorm );
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
        tm->AddTri( gp[2], gp[3], gp[0], gpnorm );
        tm->AddTri( gp[2], gp[0], gp[1], gpnorm );
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
        tm->SetIgnoreTriFlag( m_TMeshVec, bTypes, thicksurf );
    }

    //==== Pushback slice and area results ====//
    // Make ID lookup map.
    std::map< string, int > compIdMap;
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
void MeshGeom::MassSliceX( int numSlices, bool writefile )
{
    int i, j, s;

    //==== Check For Open Meshes and Merge or Delete Them ====//
    MeshInfo info;
    MergeRemoveOpenMeshes( &info );

    //==== Create Results ====//
    Results* res = ResultsMgr.CreateResults( "Mass_Properties" );
    res->Add( NameValData( "Num_Degen_Triangles_Removed", info.m_NumDegenerateTriDeleted ) );
    res->Add( NameValData( "Num_Open_Meshes_Removed", info.m_NumOpenMeshedDeleted ) );
    res->Add( NameValData( "Num_Open_Meshes_Merged", info.m_NumOpenMeshesMerged ) );
    res->Add( NameValData( "Meshes_Removed_Names", info.m_DeletedMeshes ) );
    res->Add( NameValData( "Meshes_Merged_Names", info.m_MergedMeshes ) );
    res->Add( NameValData( "Mesh_GeomID", this->GetID() ) );

    //==== Count Tris ====//
    int numTris = 0;
    for ( i = 0 ; i < ( int )m_TMeshVec.size() ; i++ )
    {
        numTris += m_TMeshVec[i]->m_TVec.size();
    }

    //==== Augment ID with index to make symmetric copies unique. ====//
    for ( i = 0 ; i < ( int )m_TMeshVec.size() ; i++ )
    {
        m_TMeshVec[i]->m_PtrID.append( std::to_string( (long long) i ) );
    }

    res->Add( NameValData( "Num_Total_Meshes", ( int )m_TMeshVec.size() ) );
    res->Add( NameValData( "Num_Total_Tris", numTris ) );

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

        tm->m_ThickSurf = false;
        tm->m_SurfCfdType = vsp::CFD_STRUCTURE;

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

    // Fill vector of cfdtypes so we don't have to pass TMeshVec all the way down.
    vector < int > bTypes( m_TMeshVec.size() );
    vector < bool > thicksurf( m_TMeshVec.size() );
    for ( i = 0 ; i < ( int )m_TMeshVec.size() ; i++ )
    {
        bTypes[i] = m_TMeshVec[i]->m_SurfCfdType;
        thicksurf[i] = m_TMeshVec[i]->m_ThickSurf;
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

            m_TMeshVec[i]->RemoveIsectEdges();
        }

        //==== Split Intersected Tri in Mesh ====//
        tm->Split();

        //==== Determine Which Triangle Are Interior/Exterior ====//
        tm->DeterIntExt( m_TMeshVec );

        //==== Mark which triangles to ignore ====//
        tm->SetIgnoreTriFlag( m_TMeshVec, bTypes, thicksurf );

    }

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

    //==== Mark which triangles to ignore ====//
    for ( i = 0 ; i < ( int )m_TMeshVec.size() ; i++ )
    {
        m_TMeshVec[i]->SetIgnoreTriFlag( m_TMeshVec, bTypes, thicksurf );
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
                        if ( !tri->m_SplitVec[j]->m_IgnoreTriFlag )
                        {
                            TriShellMassProp* tsmp = new TriShellMassProp( tm->m_PtrID, tm->m_ShellMassArea,
                                    tri->m_SplitVec[j]->m_N0->m_Pnt,
                                    tri->m_SplitVec[j]->m_N1->m_Pnt,
                                    tri->m_SplitVec[j]->m_N2->m_Pnt );
                            triShellVec.push_back( tsmp );
                        }
                    }
                }
                else if ( !tri->m_IgnoreTriFlag )
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
                    if ( !tri->m_SplitVec[j]->m_IgnoreTriFlag )
                    {
                        CreatePrism( tetraVec, tri->m_SplitVec[j], prismLength );
                    }
                }
            }
            else if ( !tri->m_IgnoreTriFlag )
            {
                CreatePrism( tetraVec, tri, prismLength );
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
        totalVol += std::abs( tetraVec[i]->m_Vol );
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
                compVol += std::abs( tetraVec[i]->m_Vol );
            }
        }

        cg = vec3d( 0, 0, 0 );
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
            if ( !tet->m_CompId.compare( id ) )
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
            if ( !trs->m_CompId.compare( id ) )
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
    
    for ( int i = 0; i < m_PointMassVec.size(); i++ )
    {
        id_vec.push_back( m_PointMassVec[i]->m_CompId );

        Geom* geom = m_Vehicle->FindGeom( m_PointMassVec[i]->m_CompId );
        if ( geom )
        {
            name_vec.push_back( geom->GetName() );
        }
        else
        {
            name_vec.push_back( "" );
        }

        mass_vec.push_back( m_PointMassVec[i]->m_Mass );
        cg_vec.push_back( m_PointMassVec[i]->m_CG );
        ixx_vec.push_back( m_PointMassVec[i]->m_Ixx );
        iyy_vec.push_back( m_PointMassVec[i]->m_Iyy );
        izz_vec.push_back( m_PointMassVec[i]->m_Izz );
        ixy_vec.push_back( m_PointMassVec[i]->m_Ixy );
        ixz_vec.push_back( m_PointMassVec[i]->m_Ixz );
        iyz_vec.push_back( m_PointMassVec[i]->m_Iyz );
        vol_vec.push_back( m_PointMassVec[i]->m_Vol );
    }

    res->Add( NameValData( "Num_Comps", (int)name_vec.size() ) );
    res->Add( NameValData( "Comp_Name", name_vec ) );
    res->Add( NameValData( "Comp_ID", id_vec ) );
    res->Add( NameValData( "Comp_Mass", mass_vec ) );
    res->Add( NameValData( "Comp_CG", cg_vec ) );
    res->Add( NameValData( "Comp_Ixx", ixx_vec ) );
    res->Add( NameValData( "Comp_Iyy", iyy_vec ) );
    res->Add( NameValData( "Comp_Izz", izz_vec ) );
    res->Add( NameValData( "Comp_Ixy", ixy_vec ) );
    res->Add( NameValData( "Comp_Ixz", ixz_vec ) );
    res->Add( NameValData( "Comp_Iyz", iyz_vec ) );
    res->Add( NameValData( "Comp_Vol", vol_vec ) );

    //==== Totals ====//
    res->Add( NameValData( "Total_Mass", m_TotalMass ) );
    res->Add( NameValData( "Total_CG", m_CenterOfGrav ) );
    res->Add( NameValData( "Total_Ixx", m_TotalIxx ) );
    res->Add( NameValData( "Total_Iyy", m_TotalIyy ) );
    res->Add( NameValData( "Total_Izz", m_TotalIzz ) );
    res->Add( NameValData( "Total_Ixy", m_TotalIxy ) );
    res->Add( NameValData( "Total_Ixz", m_TotalIxz ) );
    res->Add( NameValData( "Total_Iyz", m_TotalIyz ) );
    res->Add( NameValData( "Total_Volume", totalVol ) );

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

    if( writefile )
    {
        string f_name = m_Vehicle->getExportFileName( vsp::MASS_PROP_TXT_TYPE );
        res->WriteMassProp( f_name );
    }
}

void MeshGeom::degenGeomMassSliceX( vector< DegenGeom > &degenGeom )
{
    int i, j, s, numSlices = 250;

    //==== Check For Open Meshes and Merge or Delete Them ====//
    MeshInfo info;
    MergeRemoveOpenMeshes( &info );

    //==== Augment ID with index to make symmetric copies unique. ====//
    for ( i = 0 ; i < ( int )m_TMeshVec.size() ; i++ )
    {
        m_TMeshVec[i]->m_PtrID.append( std::to_string( (long long) i ) );
    }

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
    //update_xformed_bbox();          // Load Xform BBox

    double xMin = m_BBox.GetMin( 0 );
    double xMax = m_BBox.GetMax( 0 );

    double sliceW = ( xMax - xMin ) / ( double )( numSlices );


    //==== Build Slice Mesh Object =====//
    for ( s = 0 ; s < numSlices ; s++ )
    {
        TMesh* tm = new TMesh();
        m_SliceVec.push_back( tm );

        tm->m_ThickSurf = false;
        tm->m_SurfCfdType = vsp::CFD_STRUCTURE;

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

    // Fill vector of cfdtypes so we don't have to pass TMeshVec all the way down.
    vector < int > bTypes( m_TMeshVec.size() );
    vector < bool > thicksurf( m_TMeshVec.size() );
    for ( i = 0 ; i < ( int )m_TMeshVec.size() ; i++ )
    {
        bTypes[i] = m_TMeshVec[i]->m_SurfCfdType;
        thicksurf[i] = m_TMeshVec[i]->m_ThickSurf;
    }

    //==== Load Bounding Box ====//
    for ( s = 0 ; s < ( int )m_SliceVec.size() ; s++ )
    {
        TMesh* tm = m_SliceVec[s];
        tm->LoadBndBox();

        //==== Intersect All Mesh Geoms ====//
        for ( i = 0 ; i < ( int )m_TMeshVec.size() ; i++ )
        {
            tm->Intersect( m_TMeshVec[i] );

            m_TMeshVec[i]->RemoveIsectEdges();
        }

        //==== Split Intersected Tri in Mesh ====//
        tm->Split();

        //==== Determine Which Triangle Are Interior/Exterior ====//
        tm->DeterIntExt( m_TMeshVec );

        //==== Mark which triangles to ignore ====//
        tm->SetIgnoreTriFlag( m_TMeshVec, bTypes, thicksurf );

    }


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

    //==== Mark which triangles to ignore ====//
    for ( i = 0 ; i < ( int )m_TMeshVec.size() ; i++ )
    {
        m_TMeshVec[i]->SetIgnoreTriFlag( m_TMeshVec, bTypes, thicksurf );
    }

    //==== Do Shell Calcs ====//
    vector< DegenGeomTriShellMassProp* > triShellVec;
    for ( s = 0 ; s < ( int )m_TMeshVec.size() ; s++ )
    {
        TMesh* tm = m_TMeshVec[s];
        for ( i = 0 ; i < ( int )tm->m_TVec.size() ; i++ )
        {
            TTri* tri = tm->m_TVec[i];
            if ( tri->m_SplitVec.size() )
            {
                for ( j = 0 ; j < ( int )tri->m_SplitVec.size() ; j++ )
                {
                    if ( !tri->m_SplitVec[j]->m_IgnoreTriFlag )
                    {
                        DegenGeomTriShellMassProp* tsmp = new DegenGeomTriShellMassProp( tm->m_PtrID, tri->m_SplitVec[j]->m_N0->m_Pnt,
                                tri->m_SplitVec[j]->m_N1->m_Pnt,
                                tri->m_SplitVec[j]->m_N2->m_Pnt );
                        triShellVec.push_back( tsmp );
                    }
                }
            }
            else if ( !tri->m_IgnoreTriFlag )
            {
                DegenGeomTriShellMassProp* tsmp = new DegenGeomTriShellMassProp( tm->m_PtrID, tri->m_N0->m_Pnt, tri->m_N1->m_Pnt, tri->m_N2->m_Pnt );
                triShellVec.push_back( tsmp );
            }
        }
    }

    //==== Build Tetrahedrons ====//
    double prismLength = sliceW;
    vector< DegenGeomTetraMassProp* > tetraVec;

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
                    if ( !tri->m_SplitVec[j]->m_IgnoreTriFlag )
                    {
                        createDegenGeomPrism( tetraVec, tri->m_SplitVec[j], prismLength );
                    }
                }
            }
            else if ( !tri->m_IgnoreTriFlag )
            {
                createDegenGeomPrism( tetraVec, tri, prismLength );
            }
        }
    }

    //==== Calculate Properties on a Per Component Basis ====//

    vector<vec3d> compSolidCg, compShellCg;
    vector< vector<double> > compSolidI, compShellI;

    for ( s = 0 ; s < ( int )m_TMeshVec.size() ; s++ )
    {
        TMesh* tm = m_TMeshVec[s];
        string id = tm->m_PtrID;

        vec3d cgSolid( 0, 0, 0 ), cgShell( 0, 0, 0 );
        double compMassSolid = 0.0, compMassShell = 0.0;
        for ( i = 0 ; i < ( int )tetraVec.size() ; i++ )
        {
            if ( !tetraVec[i]->m_CompId.compare( id ) )
            {
                compMassSolid += tetraVec[i]->m_Vol;
                cgSolid = cgSolid + tetraVec[i]->m_CG * tetraVec[i]->m_Vol;
            }
        }
        for ( i = 0 ; i < ( int )triShellVec.size() ; i++ )
        {
            if ( !triShellVec[i]->m_CompId.compare( id ) )
            {
                compMassShell += triShellVec[i]->m_TriArea;
                cgShell = cgShell + triShellVec[i]->m_CG * triShellVec[i]->m_TriArea;
            }
        }

        if ( compMassSolid )
        {
            cgSolid = cgSolid * ( 1.0 / compMassSolid );
        }
        if ( compMassShell )
        {
            cgShell = cgShell * ( 1.0 / compMassShell );
        }

        compSolidCg.push_back( cgSolid );
        compShellCg.push_back( cgShell );

        double compIxx = 0.0;
        double compIyy = 0.0;
        double compIzz = 0.0;
        double compIxy = 0.0;
        double compIxz = 0.0;
        double compIyz = 0.0;
        for ( i = 0 ; i < ( int )tetraVec.size() ; i++ )
        {
            DegenGeomTetraMassProp* tet = tetraVec[i];
            if ( !tet->m_CompId.compare( id ) )
            {
                compIxx += tet->m_Ixx +
                           tet->m_Vol * ( ( cgSolid.y() - tet->m_CG.y() ) * ( cgSolid.y() - tet->m_CG.y() ) + ( cgSolid.z() - tet->m_CG.z() ) * ( cgSolid.z() - tet->m_CG.z() ) );
                compIyy += tet->m_Iyy +
                           tet->m_Vol * ( ( cgSolid.x() - tet->m_CG.x() ) * ( cgSolid.x() - tet->m_CG.x() ) + ( cgSolid.z() - tet->m_CG.z() ) * ( cgSolid.z() - tet->m_CG.z() ) );
                compIzz += tet->m_Izz +
                           tet->m_Vol * ( ( cgSolid.x() - tet->m_CG.x() ) * ( cgSolid.x() - tet->m_CG.x() ) + ( cgSolid.y() - tet->m_CG.y() ) * ( cgSolid.y() - tet->m_CG.y() ) );

                compIxy += tet->m_Ixy +
                           tet->m_Vol * ( ( cgSolid.x() - tet->m_CG.x() ) * ( cgSolid.y() - tet->m_CG.y() ) );
                compIxz += tet->m_Ixz +
                           tet->m_Vol * ( ( cgSolid.x() - tet->m_CG.x() ) * ( cgSolid.z() - tet->m_CG.z() ) );
                compIyz += tet->m_Iyz +
                           tet->m_Vol * ( ( cgSolid.y() - tet->m_CG.y() ) * ( cgSolid.z() - tet->m_CG.z() ) );
            }
        }
        vector<double> tempSolidI;
        tempSolidI.push_back( compIxx );
        tempSolidI.push_back( compIyy );
        tempSolidI.push_back( compIzz );
        tempSolidI.push_back( compIxy );
        tempSolidI.push_back( compIxz );
        tempSolidI.push_back( compIyz );

        compSolidI.push_back( tempSolidI );

        compIxx = 0.0;
        compIyy = 0.0;
        compIzz = 0.0;
        compIxy = 0.0;
        compIxz = 0.0;
        compIyz = 0.0;
        for ( i = 0 ; i < ( int )triShellVec.size() ; i++ )
        {
            DegenGeomTriShellMassProp* trs = triShellVec[i];
            if ( !trs->m_CompId.compare( id ) )
            {
                compIxx += trs->m_Ixx +
                           trs->m_TriArea * ( ( cgShell.y() - trs->m_CG.y() ) * ( cgShell.y() - trs->m_CG.y() ) + ( cgShell.z() - trs->m_CG.z() ) * ( cgShell.z() - trs->m_CG.z() ) );
                compIyy += trs->m_Iyy +
                           trs->m_TriArea * ( ( cgShell.x() - trs->m_CG.x() ) * ( cgShell.x() - trs->m_CG.x() ) + ( cgShell.z() - trs->m_CG.z() ) * ( cgShell.z() - trs->m_CG.z() ) );
                compIzz += trs->m_Izz +
                           trs->m_TriArea * ( ( cgShell.x() - trs->m_CG.x() ) * ( cgShell.x() - trs->m_CG.x() ) + ( cgShell.y() - trs->m_CG.y() ) * ( cgShell.y() - trs->m_CG.y() ) );

                compIxy += trs->m_Ixy +
                           trs->m_TriArea * ( ( cgShell.x() - trs->m_CG.x() ) * ( cgShell.y() - trs->m_CG.y() ) );
                compIxz += trs->m_Ixz +
                           trs->m_TriArea * ( ( cgShell.x() - trs->m_CG.x() ) * ( cgShell.z() - trs->m_CG.z() ) );
                compIyz += trs->m_Iyz +
                           trs->m_TriArea * ( ( cgShell.y() - trs->m_CG.y() ) * ( cgShell.z() - trs->m_CG.z() ) );
            }
        }

        vector<double> tempShellI;
        tempShellI.push_back( compIxx );
        tempShellI.push_back( compIyy );
        tempShellI.push_back( compIzz );
        tempShellI.push_back( compIxy );
        tempShellI.push_back( compIxz );
        tempShellI.push_back( compIyz );

        compShellI.push_back( tempShellI );

    }

    bool matchFlag;
    vector< bool > matchVec( m_TMeshVec.size(), false );
    // For each degenGeom
    for ( i = 0; i < ( int )degenGeom.size(); i++ )
    {
        matchFlag = false;
        DegenPoint degenPoint = degenGeom[i].getDegenPoint();

        // Loop through tmesh vector
        for ( j = 0; j < m_TMeshVec.size(); j++ )
        {
            if ( matchVec[j] == false )
            {
                // If its pointer id matches the current degenGeom
                string geomid = degenGeom[i].getParentGeom()->GetID();
                if ( geomid.compare( 0, geomid.size(), m_TMeshVec[j]->m_PtrID, 0, geomid.size() ) == 0 &&
                     degenGeom[i].getSurfNum() == m_TMeshVec[j]->m_SurfNum )
                {
                    degenPoint.Isolid.push_back( compSolidI[j] );
                    degenPoint.Ishell.push_back( compShellI[j] );
                    degenPoint.xcgSolid.push_back( compSolidCg[j] );
                    degenPoint.xcgShell.push_back( compShellCg[j] );
                    matchVec[j] = true;

                    matchFlag = true;
                }
            }
        }
        if ( !matchFlag )
        {
            degenPoint.Isolid.push_back( vector<double>( 6, 0.0 ) );
            degenPoint.Ishell.push_back( vector<double>( 6, 0.0 ) );
            degenPoint.xcgSolid.push_back( vec3d( 0.0, 0.0, 0.0 ) );
            degenPoint.xcgShell.push_back( vec3d( 0.0, 0.0, 0.0 ) );
        }

        degenGeom[i].setDegenPoint( degenPoint );
    }

    //==== Clean Up Mess ====//
    for ( i = 0 ; i < ( int )tetraVec.size() ; i++ )
    {
        delete tetraVec[i];
    }

    for ( i = 0 ; i < ( int )triShellVec.size() ; i++ )
    {
        delete triShellVec[i];
    }

}

//==== Create a Prism Made of Tetras - Extrude Tri +- len/2 ====//
void MeshGeom::CreatePrism( vector< TetraMassProp* >& tetraVec, TTri* tri, double len )
{
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

    tetraVec.push_back( new TetraMassProp( tri->m_ID, tri->m_Density, cnt, p0, p1, p2 ) );
    tetraVec.push_back( new TetraMassProp( tri->m_ID, tri->m_Density, cnt, p3, p4, p5 ) );
    tetraVec.push_back( new TetraMassProp( tri->m_ID, tri->m_Density, cnt, p0, p1, p3 ) );
    tetraVec.push_back( new TetraMassProp( tri->m_ID, tri->m_Density, cnt, p3, p4, p1 ) );
    tetraVec.push_back( new TetraMassProp( tri->m_ID, tri->m_Density, cnt, p1, p2, p4 ) );
    tetraVec.push_back( new TetraMassProp( tri->m_ID, tri->m_Density, cnt, p4, p5, p2 ) );
    tetraVec.push_back( new TetraMassProp( tri->m_ID, tri->m_Density, cnt, p0, p2, p3 ) );
    tetraVec.push_back( new TetraMassProp( tri->m_ID, tri->m_Density, cnt, p3, p5, p2 ) );
}

//==== Create a Prism Made of DegenGeomTetras - Extrude Tri +- len/2 ====//
void MeshGeom::createDegenGeomPrism( vector< DegenGeomTetraMassProp* >& tetraVec, TTri* tri, double len )
{
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

    tetraVec.push_back( new DegenGeomTetraMassProp( tri->m_ID, cnt, p0, p1, p2 ) );
    tetraVec.push_back( new DegenGeomTetraMassProp( tri->m_ID, cnt, p3, p4, p5 ) );
    tetraVec.push_back( new DegenGeomTetraMassProp( tri->m_ID, cnt, p0, p1, p3 ) );
    tetraVec.push_back( new DegenGeomTetraMassProp( tri->m_ID, cnt, p3, p4, p1 ) );
    tetraVec.push_back( new DegenGeomTetraMassProp( tri->m_ID, cnt, p1, p2, p4 ) );
    tetraVec.push_back( new DegenGeomTetraMassProp( tri->m_ID, cnt, p4, p5, p2 ) );
    tetraVec.push_back( new DegenGeomTetraMassProp( tri->m_ID, cnt, p0, p2, p3 ) );
    tetraVec.push_back( new DegenGeomTetraMassProp( tri->m_ID, cnt, p3, p5, p2 ) );
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
    oneMesh->m_Color = m_TMeshVec[0]->m_Color;

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
                        oneMesh->AddTri( tri->m_N0, tri->m_N1, tri->m_N2, mesh->m_TVec[t]->m_Norm );
                    }
                }
            }
            else if ( !mesh->m_TVec[t]->m_IgnoreTriFlag )
            {
                TTri* tri = mesh->m_TVec[t];
                oneMesh->AddTri( tri->m_N0, tri->m_N1, tri->m_N2, tri->m_Norm );
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

void MeshGeom::MergeRemoveOpenMeshes( MeshInfo* info, bool deleteopen )
{
    int i, j;

    //==== Check If All Closed ====//
    for ( i = 0 ; i < ( int )m_TMeshVec.size() ; i++ )
    {
        m_TMeshVec[i]->CheckIfClosed();
    }

    //==== Try to Merge Non Closed Meshes ====//
    // Marks mesh un-used after merge for deletion
    for ( i = 0 ; i < ( int )m_TMeshVec.size() ; i++ )
    {
        for ( j = i + 1 ; j < ( int )m_TMeshVec.size() ; j++ )
        {
            m_TMeshVec[i]->MergeNonClosed( m_TMeshVec[j] );
        }
    }
    // Keep track of merged meshes in info.
    for ( i = 0 ; i < ( int )m_TMeshVec.size() ; i++ )
    {
        if ( m_TMeshVec[i]->m_DeleteMeFlag )
        {
            info->m_NumOpenMeshesMerged++;
            info->m_MergedMeshes.push_back( m_TMeshVec[i]->m_NameStr );
        }
    }

    // Mark any still open meshes for deletion.  Perhaps make this optional.
    if ( deleteopen )
    {
        for ( i = 0 ; i < ( int )m_TMeshVec.size() ; i++ )
        {
            if ( m_TMeshVec[i]->m_NonClosedTriVec.size() )
            {
                if ( !m_TMeshVec[i]->m_DeleteMeFlag )
                {
                    info->m_NumOpenMeshedDeleted++;
                    info->m_DeletedMeshes.push_back( m_TMeshVec[i]->m_NameStr );
                }

                m_TMeshVec[i]->m_DeleteMeFlag = true;
            }
        }
    }

    //==== Remove meshes marked for deletion ====//
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

    double xmin = box.GetMin( 0 );
    double xmax = box.GetMax( 0 );

    double zmin = box.GetMin( 2 );
    double zmax = box.GetMax( 2 );

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

void MeshGeom::CreateDegenGeom( vector<DegenGeom> &dgs, bool preview )
{
    unsigned int num_meshes = m_TMeshVec.size();

    for ( int i = 0; i < num_meshes; i++ )
    {
        DegenGeom degenGeom;

        degenGeom.setType( DegenGeom::MESH_TYPE );

        degenGeom.setParentGeom( this );
        degenGeom.setSurfNum( i );
        degenGeom.setFlipNormal( false );
        degenGeom.setMainSurfInd( 0 );
        degenGeom.setSymCopyInd( 0 );

        Matrix4d trans = GetTotalTransMat();

        vector < double > tmatvec;
        for ( int j = 0; j < 16; j++ )
        {
            tmatvec.push_back( trans.data()[ j ] );
        }
        degenGeom.setTransMat( tmatvec );

        degenGeom.setNumXSecs( 0 );
        degenGeom.setNumPnts( 0 );
        degenGeom.setName( GetName() );

        dgs.push_back(degenGeom);
    }
}

vector<TMesh*> MeshGeom::CreateTMeshVec() const
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
Matrix4d MeshGeom::GetTotalTransMat() const
{
    Matrix4d retMat;
    retMat.initMat( m_ScaleMatrix );
    retMat.postMult( m_ModelMatrix );

    return retMat;
}

//==== Get the Names of the TMeshes ====//
vector< string > MeshGeom::GetTMeshNames()
{
    vector< string > names;
    for ( int i = 0 ; i < ( int )m_TMeshVec.size() ; i++ )
    {
        names.push_back( m_TMeshVec[i]->m_NameStr + "_Surf" + to_string( ( long long )m_TMeshVec[i]->m_SurfNum ) );
    }

    return names;
}

vector< string > MeshGeom::GetTMeshIDs()
{
    vector< string > ids;
    for ( int i = 0; i < (int)m_TMeshVec.size(); i++ )
    {
        ids.push_back( m_TMeshVec[i]->m_PtrID + "_Surf" + to_string( (long long)m_TMeshVec[i]->m_SurfNum ) );
    }

    return ids;
}

set < string > MeshGeom::GetTMeshPtrIDs()
{
    set < string > ids;
    for ( size_t i = 0; i < m_TMeshVec.size(); i++ )
    {
        ids.insert( m_TMeshVec[i]->m_PtrID );
    }

    return ids;
}

//==== Subtag All Triangles ====//
void MeshGeom::SubTagTris( bool tag_subs )
{
    // Clear out the current Subtag Maps
    SubSurfaceMgr.ClearTagMaps();
    SubSurfaceMgr.m_CompNames = GetTMeshNames();
    SubSurfaceMgr.m_CompIDs = GetTMeshIDs();
    SubSurfaceMgr.SetSubSurfTags( GetNumIndexedParts() );
    SubSurfaceMgr.BuildCompNameMap();
    SubSurfaceMgr.BuildCompIDMap();

    for ( int i = 0 ; i < ( int )m_TMeshVec.size() ; i++ )
    {
        m_TMeshVec[i]->SubTag( i + 1, tag_subs );
    }

    SubSurfaceMgr.BuildSingleTagMap();

}

