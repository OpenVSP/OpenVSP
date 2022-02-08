//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// XSecSurf.cpp: implementation of the XSecSurf class.
//
//////////////////////////////////////////////////////////////////////

#include "XSecSurf.h"
#include "WingGeom.h"
#include "CustomGeom.h"
#include "PropGeom.h"
#include "StlHelper.h"

using namespace vsp;


//==== Default Constructor ====//
XSecSurf::XSecSurf()
{
    m_XSecType = -1;
    m_PrincipalDir = -1;
    m_WidthDir = -1;
    m_WidthShift = -1;
    m_FlipUD = false;
    m_CutMinNumXSecs = 2;
    m_SavedXSecCurve = NULL;

    // Assign default values different from -1 above.
    SetXSecType( XSEC_FUSE );
    SetBasicOrientation( X_DIR, Y_DIR, XS_SHIFT_MID, false );

    m_GlobalXForm.loadIdentity();

//  m_TestParm.Init( "Test", "XSecSurf", this, 0.0, 1.0e-8, 1.0e12 );
}

//==== Destructor ====//
XSecSurf::~XSecSurf()
{
    DeleteAllXSecs();
}

void XSecSurf::ChangeID( string id )
{
    ParmContainer::ChangeID( id );

    for ( int i = 0 ; i < ( int )m_XSecPtrVec.size() ; i++ )
    {
        m_XSecPtrVec[i]->SetParentContainer( id );
    }
}

//==== Find XSec Given ID ====//
XSec* XSecSurf::FindXSec( string id ) const
{
    for ( int i = 0 ; i < ( int )m_XSecPtrVec.size() ; i++ )
    {
        if ( m_XSecPtrVec[i]->GetID() == id )
        {
            return m_XSecPtrVec[i];
        }
    }
    return NULL;
}

const XSec* XSecSurf::FindConstXSec( string id ) const
{
    return FindXSec( id );
}

//==== Find XSec Given Index ====//
XSec* XSecSurf::FindXSec( int index ) const
{
    string id = GetXSecID( index );
    return FindXSec( id );
}

const XSec* XSecSurf::FindConstXSec( int index ) const
{
    return FindXSec( index );
}

//==== Find XSec Index Given ID ====//
int XSecSurf::FindXSecIndex( string ID ) const
{
    for( int i = 0; i < static_cast<int>( m_XSecIDDeque.size() ); i++ )
    {
        if( m_XSecIDDeque[i] == ID )
        {
            return i;
        }
    }
    return -1;
}

//==== Find XSec ID Given Index ====//
string XSecSurf::GetXSecID( int index ) const
{
    string id;
    if ( index >= 0 && index < ( int )m_XSecIDDeque.size() )
    {
        id = m_XSecIDDeque[index];
    }
    return id;
}

//==== Change XSecID ====//
void XSecSurf::ChangeXSecID( string oldID, string newID )
{
    for( int i = 0; i < static_cast<int>( m_XSecIDDeque.size() ); i++ )
    {
        if( m_XSecIDDeque[i] == oldID )
        {
            m_XSecIDDeque[i] = newID;
        }
    }

    if( m_SavedXSec == oldID )
    {
        m_SavedXSec = newID;
    }
}

XSecCurve* XSecSurf::CreateXSecCurve( int type )
{
    XSecCurve* xscrv_ptr = NULL;
    if ( type == XS_POINT )
    {
        xscrv_ptr = new PointXSec( );
    }
    else if ( type == XS_CIRCLE )
    {
        xscrv_ptr = new CircleXSec( );
    }
    else if ( type == XS_ELLIPSE )
    {
        xscrv_ptr = new EllipseXSec( );
    }
    else if ( type == XS_SUPER_ELLIPSE )
    {
        xscrv_ptr = new SuperXSec( );
    }
    else if ( type == XS_ROUNDED_RECTANGLE )
    {
        xscrv_ptr = new RoundedRectXSec( );
    }
    else if ( type == XS_GENERAL_FUSE )
    {
        xscrv_ptr = new GeneralFuseXSec( );
    }
    else if ( type == XS_FOUR_SERIES )
    {
        xscrv_ptr = new FourSeries( );
    }
    else if ( type == XS_SIX_SERIES )
    {
        xscrv_ptr = new SixSeries( );
    }
    else if ( type == XS_BICONVEX )
    {
        xscrv_ptr = new Biconvex( );
    }
    else if ( type == XS_WEDGE )
    {
        xscrv_ptr = new Wedge( );
    }
    else if ( type == XS_FILE_FUSE )
    {
        xscrv_ptr = new FileXSec( );
    }
    else if ( type == XS_FILE_AIRFOIL )
    {
        xscrv_ptr = new FileAirfoil( );
    }
    else if ( type == XS_CST_AIRFOIL )
    {
        xscrv_ptr = new CSTAirfoil( );
    }
    else if ( type == XS_VKT_AIRFOIL )
    {
        xscrv_ptr = new VKTAirfoil( );
    }
    else if ( type == XS_FOUR_DIGIT_MOD )
    {
        xscrv_ptr = new FourDigMod( );
    }
    else if ( type == XS_FIVE_DIGIT )
    {
        xscrv_ptr = new FiveDig( );
    }
    else if ( type == XS_FIVE_DIGIT_MOD )
    {
        xscrv_ptr = new FiveDigMod( );
    }
    else if ( type == XS_ONE_SIX_SERIES )
    {
        xscrv_ptr = new OneSixSeries( );
    }
    else if ( type == XS_EDIT_CURVE )
    {
        xscrv_ptr = new EditCurveXSec( );
    }

    return xscrv_ptr;
}

//==== Create XSecCurve ====//
XSec* XSecSurf::CreateXSec( int type )
{
    XSecCurve* xscrv_ptr = CreateXSecCurve( type );

    XSec* xsec_ptr = NULL;
    if ( xscrv_ptr )
    {
        if ( m_XSecType == XSEC_FUSE )
        {
            xsec_ptr = ( XSec* ) new FuseXSec( xscrv_ptr );
        }
        else if ( m_XSecType == XSEC_STACK )
        {
            xsec_ptr = ( XSec* ) new StackXSec( xscrv_ptr );
        }
        else if ( m_XSecType == XSEC_WING )
        {
             xsec_ptr = ( XSec* ) new WingSect( xscrv_ptr );
        }
        else if ( m_XSecType == XSEC_CUSTOM )
        {
             xsec_ptr = ( XSec* ) new CustomXSec( xscrv_ptr );
        }
        else if ( m_XSecType == XSEC_PROP )
        {
             xsec_ptr = ( XSec* ) new PropXSec( xscrv_ptr );
        }
        else
        {
            fprintf( stderr, "Undefined or unknown XSec type in XSecSurf::CreateXSec\n");
            assert(false);
        }

        xsec_ptr->SetParentContainer( GetID() );
        m_XSecPtrVec.push_back( xsec_ptr );
    }

    return xsec_ptr;
}

//==== Delete All XSecs ====//
void XSecSurf::DeleteAllXSecs()
{
    //==== Delete Any Constructed XSecs ====//
    for ( int i = 0 ; i < ( int )m_XSecPtrVec.size() ; i++ )
    {
        delete m_XSecPtrVec[i];
    }
    m_XSecPtrVec.clear();
    m_XSecIDDeque.clear();
    m_SavedXSec.clear();
}

//==== Insert XSec After Index ====//
string XSecSurf::InsertXSec( int type, int index )
{
    string id;

    XSec* xs = CreateXSec( type );

    if ( xs )               // Valid XSec?
    {
        id = xs->GetID();
        if ( index < ( int )m_XSecIDDeque.size() )
        {
            m_XSecIDDeque.insert( m_XSecIDDeque.begin() + index + 1, xs->GetID() );
        }
        else
        {
            m_XSecIDDeque.push_back( xs->GetID() );
        }
    }
    return id;
}

//==== Insert XSec After Index ====//
string XSecSurf::AddXSec( int type )
{
    string id;

    XSec* xs = CreateXSec( type );
    if ( xs )
    {
        id = xs->GetID();
        m_XSecIDDeque.push_back( id );
    }
    return id;
}

//==== Cut XSec ====//
void XSecSurf::CutXSec( int index )
{
    if ( (int)m_XSecIDDeque.size() <= m_CutMinNumXSecs )
    {
        return;
    }

    XSec* xs = FindXSec( index );
    if ( !xs )
    {
        return;
    }

    CopyXSec( index );

    vector_remove_val( m_XSecPtrVec, xs );
    m_XSecIDDeque.erase( m_XSecIDDeque.begin() + index );
    delete xs;
}

//==== Copy XSec ====//
void XSecSurf::CopyXSec( int index )
{
    XSec* xs = FindXSec( index );
    if ( !xs )
    {
        return;
    }

    XSec* saved_xs = FindXSec( m_SavedXSec );

    //==== Create Saved XSec ====//
    if ( saved_xs && ( saved_xs->GetType() != xs->GetType()
            || saved_xs->GetXSecCurve()->GetType() != xs->GetXSecCurve()->GetType() ) )
    {
        vector_remove_val( m_XSecPtrVec, saved_xs );
        delete saved_xs;
        saved_xs = NULL;
    }

    //==== Saved XSec ====//
    if ( !saved_xs )
    {
        saved_xs = CreateXSec( xs->GetXSecCurve()->GetType() );
    }

    //==== Copy Data ====//
    if ( saved_xs )
    {
        saved_xs->CopyFrom( xs );
        m_SavedXSec = saved_xs->GetID();
    }
}

//==== Paste XSec ====//
void XSecSurf::PasteXSec( int index )
{
    XSec* xs = FindXSec( index );
    if ( !xs )
    {
        return;
    }

    XSec* saved_xs = FindXSec( m_SavedXSec );
    if ( !saved_xs )
    {
        return;
    }

    string new_xs_id = InsertXSec( saved_xs->GetXSecCurve()->GetType(), index );
    XSec* new_xs = FindXSec( new_xs_id );
    if ( !new_xs )
    {
        return;
    }

    //==== Copy Data ====//
    new_xs->CopyFrom( saved_xs );

    //==== Copy Position from xsec being replaced ====//
    new_xs->CopyBasePos( xs );

    deque_remove_val( m_XSecIDDeque, xs->GetID() );
    vector_remove_val( m_XSecPtrVec, xs );

    delete xs;
}

//==== Copy XSec Curve====//
void XSecSurf::CopyXSecCurve( int index )
{
    XSec* xs = FindXSec( index );
    if ( !xs )
    {
        return;
    }

    if ( m_SavedXSecCurve && m_SavedXSecCurve->GetType() != xs->GetXSecCurve()->GetType() )
    {
        delete m_SavedXSecCurve;
        m_SavedXSecCurve = NULL;
    }

    if ( !m_SavedXSecCurve )
    {
        m_SavedXSecCurve = CreateXSecCurve( xs->GetXSecCurve()->GetType() );
    }

    if ( m_SavedXSecCurve )
    {
        m_SavedXSecCurve->CopyFrom( xs->GetXSecCurve() );
    }
}

//==== Paste XSec Curve====//
void XSecSurf::PasteXSecCurve( int index )
{
    XSec* xs = FindXSec( index );
    if ( !xs )
        return;

    if ( !m_SavedXSecCurve )
        return;

    XSecCurve* duplicate_saved_crv = CreateXSecCurve( m_SavedXSecCurve->GetType() );
    if ( duplicate_saved_crv )
    {
        duplicate_saved_crv->CopyFrom( m_SavedXSecCurve );
    }

    xs->SetXSecCurve( duplicate_saved_crv );


}

//==== Change XSec Type ====//
void XSecSurf::ChangeXSecShape( int index, int type )
{
    //==== Old XSec ====//
    XSec* xs = FindXSec( index );

    if ( !xs )
    {
        return;
    }

    //==== Create New XSec ====//
    XSec* nxs = CreateXSec( type );

    if ( nxs )
    {
        //==== Copy Data ====//
        nxs->CopyFrom( xs );

        m_XSecIDDeque.insert( m_XSecIDDeque.begin() + index, nxs->GetID() );

        //==== Remove Old XSec ====//
        m_XSecIDDeque.erase( m_XSecIDDeque.begin() + index + 1 );
        vector_remove_val( m_XSecPtrVec, xs );
        delete xs;
        
        XSecCurve* xsc = nxs->GetXSecCurve();

        if ( xsc )
        {
            xsc->Update();
        }
    }
}

void XSecSurf::GetBasicTransformation( double w, Matrix4d &mat )
{
    if ( m_PrincipalDir != -1 )
    {
        GetBasicTransformation( m_PrincipalDir, m_WidthDir, m_WidthShift, m_FlipUD, w, mat );
    }
    else
    {
        printf( "Must call XSecSurf::SetBasicOrientation before use.\n" );
        assert( false );
    }
}

void XSecSurf::GetBasicTransformation( int pdir, int wdir, int wshift, bool flip, double w, Matrix4d &mat )
{
    double *m = mat.data();

    for ( int i = 0; i < 16; i++ )
    {
        m[i] = 0;
    }

    int prow = pdir;
    // Principal direction of base curves +Z
    m[ prow + ( Z_DIR * 4 ) ] = 1;

    int wrow = wdir;
    // Width direction of base curves +X
    m[ wrow + ( X_DIR * 4 ) ] = 1;

    // Remaining row via clever math
    int row = 3 - ( prow + wrow );

    // Cross product to ensure right handed system
    int r1 = Y_DIR, r2 = Z_DIR;
    switch( row )
    {
    case X_DIR:
        r1 = Y_DIR;
        r2 = Z_DIR;
        break;
    case Y_DIR:
        r1 = Z_DIR;
        r2 = X_DIR;
        break;
    case Z_DIR:
        r1 = X_DIR;
        r2 = Y_DIR;
        break;
    }

    int flipflag = 1;
    if ( flip )
    {
        flipflag = -1;
    }

    // Specialized cross product with known zeros.
    m[ row + ( Y_DIR * 4 ) ] = flipflag * ( m[ r1 + ( Z_DIR * 4 ) ] * m[ r2 + ( X_DIR * 4 ) ] -
                                            m[ r1 + ( X_DIR * 4 ) ] * m[ r2 + ( Z_DIR * 4 ) ] );

    // Shift in width direction if required.
    m[ wrow + 12 ] = -w * wshift / 2.0;
}


//==== Encode XML ====//
xmlNodePtr XSecSurf::EncodeXml( xmlNodePtr & node )
{
    ParmContainer::EncodeXml( node );

    xmlNodePtr xsecsurf_node = xmlNewChild( node, NULL, BAD_CAST "XSecSurf", NULL );
    if ( xsecsurf_node )
    {
        for ( int i = 0 ; i < NumXSec() ; i++ )
        {
            XSec* xsec = FindXSec( i );
            if ( xsec  )
            {
                xsec->EncodeXSec( xsecsurf_node );
            }
        }
    }
    return xsecsurf_node;
}

//==== Decode XML ====//
xmlNodePtr XSecSurf::DecodeXml( xmlNodePtr & node )
{
    DeleteAllXSecs();

    ParmContainer::DecodeXml( node );

    xmlNodePtr xsecsurf_node = XmlUtil::GetNode( node, "XSecSurf", 0 );
    if ( xsecsurf_node )
    {
        int num = XmlUtil::GetNumNames( xsecsurf_node, "XSec" );

        xmlNodePtr xsec_node = xsecsurf_node->xmlChildrenNode;
        while( xsec_node != NULL )
        {
            if ( !xmlStrcmp( xsec_node->name, ( const xmlChar * )"XSec" ) )
            {
                if ( xsec_node )
                {
                    xmlNodePtr temp_node = NULL;
                    temp_node = XmlUtil::GetNode( xsec_node, "XSec", 0 );

                    if ( temp_node )
                    {
                        XmlUtil::FindInt( temp_node, "Type", XSEC_FUSE );

                        xmlNodePtr xscrv_node = XmlUtil::GetNode( temp_node, "XSecCurve", 0 );
                        if ( xscrv_node )
                        {
                            xmlNodePtr temp2_node = XmlUtil::GetNode( xscrv_node, "XSecCurve", 0 );
                            int xsc_type = XmlUtil::FindInt( temp2_node, "Type", XS_POINT );

                            //==== Create New Cross Section ====//
                            XSec* xsec_ptr = FindXSec( AddXSec( xsc_type ) );
                            if ( xsec_ptr )
                            {
                                xsec_ptr->DecodeXSec( xsec_node );
                            }
                        }
                    }
                }
            }
            xsec_node = xsec_node->next;
        }
    }
    return xsecsurf_node;
}

////==== Set Parent ID For All XSecs ====//
//void XSecSurf::SetParentID(std::string id)
//{
//  for ( int i = 0 ; i < (int)m_XSecPtrVec.size() ; i++ )
//  {
//      m_XSecPtrVec[i]->SetParentGeomID( id );
//  }
//}

//==== Trigger Updates Because A Parm Changed ====//
void XSecSurf::ParmChanged( Parm* parm_ptr, int type )
{
    ParmContainer* pc = GetParentContainerPtr();

    if ( pc )
    {
        pc->ParmChanged( parm_ptr, type );
    }

}

//==== Look Though All Parms and Load Linkable Ones ===//
void XSecSurf::AddLinkableParms( vector< string > & parm_vec, const string & link_container_id )
{
    ParmContainer::AddLinkableParms( parm_vec, link_container_id );

    for ( int i = 0 ; i < NumXSec() ; i++ )
    {
        XSec* xsec = FindXSec( i );
        if ( xsec  )
        {
            xsec->AddLinkableParms( parm_vec, link_container_id );
        }
    }
}

string XSecSurf::GetName() const
{
    ParmContainer* pc = GetParentContainerPtr();

    if ( pc )
    {
        return pc->GetName();
    }
    return ParmContainer::GetName();
}
