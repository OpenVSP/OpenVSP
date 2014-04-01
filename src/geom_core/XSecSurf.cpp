//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// XSecSurf.cpp: implementation of the XSecSurf class.
//
//////////////////////////////////////////////////////////////////////

#include "XSecSurf.h"
#include "StlHelper.h"

//==== Default Constructor ====//
XSecSurf::XSecSurf()
{
    m_rotation.loadIdentity();
    m_center = false;
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
XSec* XSecSurf::FindXSec( string id )
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

//==== Find XSec Given Index ====//
XSec* XSecSurf::FindXSec( int index )
{
    string id = GetXSecID( index );
    return FindXSec( id );
}

//==== Find XSec ID Given Index ====//
string XSecSurf::GetXSecID( int index )
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

//==== Create XSec ====//
XSec* XSecSurf::CreateXSec( int type, int index )
{
    XSec* xsec_ptr = NULL;
    if ( type == XSec::POINT )
    {
        xsec_ptr = new PointXSec( index != 0 );
    }
    else if ( type == XSec::CIRCLE )
    {
        xsec_ptr = new CircleXSec( index != 0 );
    }
    else if ( type == XSec::ELLIPSE )
    {
        xsec_ptr = new EllipseXSec( index != 0 );
    }
    else if ( type == XSec::SUPER_ELLIPSE )
    {
        xsec_ptr = new SuperXSec( index != 0 );
    }
    else if ( type == XSec::ROUNDED_RECTANGLE )
    {
        xsec_ptr = new RoundedRectXSec( index != 0 );
    }
    else if ( type == XSec::GENERAL_FUSE )
    {
        xsec_ptr = new GeneralFuseXSec( index != 0 );
    }
    else if ( type == XSec::FOUR_SERIES )
    {
        xsec_ptr = new FourSeries( index != 0 );
    }
    else if ( type == XSec::SIX_SERIES )
    {
        xsec_ptr = new SixSeries( index != 0 );
    }
    else if ( type == XSec::BICONVEX )
    {
        xsec_ptr = new Biconvex( index != 0 );
    }
    else if ( type == XSec::WEDGE )
    {
        xsec_ptr = new Wedge( index != 0 );
    }
    else if ( type == XSec::FILE_FUSE )
    {
        xsec_ptr = new FileXSec( index != 0 );
    }
    else if ( type == XSec::FILE_AIRFOIL )
    {
        xsec_ptr = new FileAirfoil( index != 0 );
    }

    if ( xsec_ptr )
    {
        xsec_ptr->SetParentContainer( GetID() );
        xsec_ptr->SetTransformation( m_rotation, m_center );
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

    XSec* xs = CreateXSec( type, index );

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

    XSec* xs = CreateXSec( type, NumXSec() );
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
    if ( m_XSecIDDeque.size() <= 2 )
    {
        return;
    }

    if ( index == 0 || index == ( int )m_XSecIDDeque.size() - 1 )
    {
        return;
    }


    XSec* xs = FindXSec( index );
    if ( !xs )
    {
        return;
    }

    //==== Delete Saved XSec ====//
    XSec* saved_xs = FindXSec( m_SavedXSec );
    if ( saved_xs )
    {
        vector_remove_val( m_XSecPtrVec, saved_xs );
        delete saved_xs;
    }

    m_SavedXSec = xs->GetID();
    m_XSecIDDeque.erase( m_XSecIDDeque.begin() + index );
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
    if ( saved_xs && saved_xs->GetType() != xs->GetType() )
    {
        vector_remove_val( m_XSecPtrVec, saved_xs );
        delete saved_xs;
        saved_xs = NULL;
    }

    //==== Saved XSec ====//
    if ( !saved_xs )
    {
        saved_xs = CreateXSec( xs->GetType(), index );
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

    string new_xs_id = InsertXSec( saved_xs->GetType(), index );
    XSec* new_xs = FindXSec( new_xs_id );
    if ( !new_xs )
    {
        return;
    }

    //==== Copy Data ====//
    new_xs->CopyFrom( saved_xs );
    new_xs->m_XLocPercent = xs->m_XLocPercent();
    new_xs->m_YLocPercent = xs->m_YLocPercent();
    new_xs->m_ZLocPercent = xs->m_ZLocPercent();

    deque_remove_val( m_XSecIDDeque, xs->GetID() );
    vector_remove_val( m_XSecPtrVec, xs );

    delete xs;
}

//==== Change XSec Type ====//
void XSecSurf::ChangeXSecType( int index, int type )
{
    //==== Old XSec ====//
    XSec* xs = FindXSec( index );

    if ( !xs )
    {
        return;
    }

    //==== Create New XSec ====//
    XSec* nxs = CreateXSec( type, index );

    if ( nxs )
    {
        //==== Copy Data ====//
        nxs->CopyFrom( xs );

        m_XSecIDDeque.insert( m_XSecIDDeque.begin() + index, nxs->GetID() );

        //==== Remove Old XSec ====//
        m_XSecIDDeque.erase( m_XSecIDDeque.begin() + index + 1 );
        vector_remove_val( m_XSecPtrVec, xs );
        delete xs;
    }
}

//==== Encode XML ====//
xmlNodePtr XSecSurf::EncodeXml( xmlNodePtr & node )
{
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

    xmlNodePtr xsecsurf_node = XmlUtil::GetNode( node, "XSecSurf", 0 );
    if ( xsecsurf_node )
    {
        int num = XmlUtil::GetNumNames( xsecsurf_node, "XSec" );
        for ( int i = 0 ; i < num ; i++ )
        {
            xmlNodePtr xsec_node = XmlUtil::GetNode( xsecsurf_node, "XSec", i );
            if ( xsec_node )
            {
                xmlNodePtr temp_node = XmlUtil::GetNode( xsec_node, "XSec", 0 );
                int type = XmlUtil::FindInt( temp_node, "Type", XSec::POINT );

                //==== Create New Cross Section ====//
                XSec* xsec_ptr = FindXSec( AddXSec( type ) );
                if ( xsec_ptr )
                {
                    xsec_ptr->DecodeXSec( xsec_node );
                }
            }
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

void XSecSurf::SetTransformation( const Matrix4d &mat, bool center )
{
    m_rotation = mat;
    m_center = center;
}
