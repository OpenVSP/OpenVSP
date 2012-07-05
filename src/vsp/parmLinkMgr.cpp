//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// Parm Link Mgr
//
//////////////////////////////////////////////////////////////////////

#include "parmLinkMgr.h"
#include "parmLinkScreen.h"

#include "geom.h"
#include "aircraft.h"
#include "util.cpp"			// To get the templates to work??

bool ParmNameCompare(const Parm* a, const Parm* b) 
{
	string strA = string( a->get_name().get_char_star() );
	string strB = string( b->get_name().get_char_star() );

	int c = strA.compare( strB );
	if ( c < 0 )
		return true;

	return false;
}


//==== Singleton ====//
PLM_Single::PLM_Single()
{
	static ParmLinkMgr* ptr = 0; 

    if (!ptr) 
		ptr = new ParmLinkMgr();

	parmLinkMgr = ptr;     
}

//==== Singleton ====//
PM_Single::PM_Single()
{
	static ParmMgr* ptr = 0;

    if (!ptr)
		ptr = new ParmMgr();

	parmMgr = ptr;
}


//====================================================================
//====================================================================
UserGeom::UserGeom(Aircraft* aptr) : Geom(aptr)
{
	type = USER_GEOM_TYPE;
	type_str = Stringc("hrm");

	setName( Stringc("UserGeom") );

	setNoShowFlag(1);

	userParm1.initialize(this, "UserParm1", 0.0);
    userParm1.set_lower_upper( -10000000, 10000000 );
	userParm2.initialize(this, "UserParm2", 0.0);
    userParm2.set_lower_upper( -10000000, 10000000 );
	userParm3.initialize(this, "UserParm3", 0.0);
    userParm3.set_lower_upper( -10000000, 10000000 );
	userParm4.initialize(this, "UserParm4", 0.0);
    userParm4.set_lower_upper( -10000000, 10000000 );
	userParm5.initialize(this, "UserParm5", 0.0);
    userParm5.set_lower_upper( -10000000, 10000000 );
	userParm6.initialize(this, "UserParm6", 0.0);
    userParm6.set_lower_upper( -10000000, 10000000 );
	userParm7.initialize(this, "UserParm7", 0.0);
    userParm7.set_lower_upper( -10000000, 10000000 );
	userParm8.initialize(this, "UserParm8", 0.0);
    userParm8.set_lower_upper( -10000000, 10000000 );

}

UserGeom::~UserGeom()
{
}

void UserGeom::copy( Geom* fromGeom )
{
	Geom::copy( fromGeom );

	UserGeom* g = (UserGeom*)fromGeom;			// Upcast
	userParm1.set( g->userParm1() );
	userParm2.set( g->userParm2() );
	userParm3.set( g->userParm3() );
	userParm4.set( g->userParm4() );
	userParm5.set( g->userParm5() );
	userParm6.set( g->userParm6() );
	userParm7.set( g->userParm7() );
	userParm8.set( g->userParm8() );

}

void UserGeom::write(xmlNodePtr root)
{
	xmlAddStringNode( root, "Type", "User");

	xmlNodePtr user_node = xmlNewChild( root, NULL, (const xmlChar *)"User_Parms", NULL );

	xmlAddDoubleNode( user_node, "User1", userParm1() );
	xmlAddDoubleNode( user_node, "User2", userParm2() );
	xmlAddDoubleNode( user_node, "User3", userParm3() );
	xmlAddDoubleNode( user_node, "User4", userParm4() );
	xmlAddDoubleNode( user_node, "User5", userParm5() );
	xmlAddDoubleNode( user_node, "User6", userParm6() );
	xmlAddDoubleNode( user_node, "User7", userParm7() );
	xmlAddDoubleNode( user_node, "User8", userParm8() );
}

void UserGeom::read(xmlNodePtr root)
{
	xmlNodePtr node;
	node = xmlGetNode( root, "User_Parms", 0 );

	if ( node )
	{
		userParm1    =  xmlFindDouble( node, "User1", userParm1() );
		userParm2    =  xmlFindDouble( node, "User2", userParm2() );
		userParm3    =  xmlFindDouble( node, "User3", userParm3() );
		userParm4    =  xmlFindDouble( node, "User4", userParm4() );
		userParm5    =  xmlFindDouble( node, "User5", userParm5() );
		userParm6    =  xmlFindDouble( node, "User6", userParm6() );
		userParm7    =  xmlFindDouble( node, "User7", userParm7() );
		userParm8    =  xmlFindDouble( node, "User8", userParm8() );
	}

}

void UserGeom::parm_changed(Parm*)
{

}

void UserGeom::LoadLinkableParms( vector< Parm* > & parmVec )
{
	Geom::AddLinkableParm( &userParm1, parmVec, this, "User" );
	Geom::AddLinkableParm( &userParm2, parmVec, this, "User" );
	Geom::AddLinkableParm( &userParm3, parmVec, this, "User" );
	Geom::AddLinkableParm( &userParm4, parmVec, this, "User" );
	Geom::AddLinkableParm( &userParm5, parmVec, this, "User" );
	Geom::AddLinkableParm( &userParm6, parmVec, this, "User" );
	Geom::AddLinkableParm( &userParm7, parmVec, this, "User" );
	Geom::AddLinkableParm( &userParm8, parmVec, this, "User" );
}

void UserGeom::SetUserParmValue( int id, double val )
{
	if      ( id == 1 )		userParm1.set( val );
	else if ( id == 2 )		userParm2.set( val );
	else if ( id == 3 )		userParm3.set( val );
	else if ( id == 4 )		userParm4.set( val );
	else if ( id == 5 )		userParm5.set( val );
	else if ( id == 6 )		userParm6.set( val );
	else if ( id == 7 )		userParm7.set( val );
	else if ( id == 8 )		userParm8.set( val );
}


//=============================================================//
//=============================================================//
ParmLink::ParmLink()
{
	m_ParmA = m_ParmB = NULL;
	m_OffsetFlag = true;
	m_Offset = 0.0;
	m_ScaleFlag = false;
	m_Scale = 1.0;
	m_LowerLimitFlag = false;
	m_LowerLimit = -1.0e12;
	m_UpperLimitFlag = false;
	m_UpperLimit =  1.0e12;
	
}


ParmLink::~ParmLink()
{


}

void ParmLink::SetParmA( Parm* p )
{
	if ( p != m_ParmA )
	{
		m_ParmA = p;
//		InitOffsetScale();
	}
}

void ParmLink::SetParmB( Parm* p )
{
	if ( p != m_ParmB )
	{
		m_ParmB = p;
//		InitOffsetScale();
	}
}

void ParmLink::InitOffsetScale()
{
	if ( m_ParmA && m_ParmB )
	{
		double valA = m_ParmA->get();
		double valB = m_ParmB->get();

		//==== Initialize Offset and Scale ====//
		m_Offset = valB - valA;
		m_Scale  = 1.0;
		if ( fabs(valA) > 0.0000001 )
			m_Scale = valB/valA;

		m_LowerLimit = m_ParmB->get_lower();
		m_UpperLimit = m_ParmB->get_upper();
	}
}



//=============================================================//
//=============================================================//

ParmLinkMgr::ParmLinkMgr()
{
	m_CurrParmLinkIndex = -1;

	aircraftPtr = NULL;
	m_WorkingParmLink.SetParmA( NULL );
	m_WorkingParmLink.SetParmB( NULL );
	m_WorkingParmLink.SetOffsetFlag( true );
	m_WorkingParmLink.SetOffset( 0.0 );
	m_WorkingParmLink.SetScaleFlag( false );
	m_WorkingParmLink.SetScale( 1.0 );

	m_DefaultParm = NULL;
}

ParmLinkMgr::~ParmLinkMgr()
{

}

void ParmLinkMgr::WriteLinks( xmlNodePtr root )
{
	xmlNodePtr link_list_node = xmlNewChild( root, NULL, (const xmlChar *)"ParmLink_List", NULL );

	for ( int i = 0 ; i < (int)m_ParmLinkVec.size() ; i++ )
	{
		ParmLink* pl = m_ParmLinkVec[i];
		xmlNodePtr link_node = xmlNewChild( link_list_node, NULL, (const xmlChar *)"Link", NULL );
		xmlAddIntNode( link_node, "GeomA", ((Geom*)pl->GetParmA()->get_geom_base())->getPtrID() );
		xmlAddStringNode( link_node, "GroupA", pl->GetParmA()->get_group_name().get_char_star() );
		xmlAddStringNode( link_node, "ParmA",  pl->GetParmA()->get_name().get_char_star() );

		xmlAddIntNode( link_node, "GeomB", ((Geom*)pl->GetParmB()->get_geom_base())->getPtrID() );
		xmlAddStringNode( link_node, "GroupB", pl->GetParmB()->get_group_name().get_char_star() );
		xmlAddStringNode( link_node, "ParmB",  pl->GetParmB()->get_name().get_char_star() );

		xmlAddIntNode( link_node, "OffsetFlag", pl->GetOffsetFlag() );
		xmlAddDoubleNode( link_node, "Offset",  pl->GetOffset() );
		xmlAddIntNode( link_node, "ScaleFlag",  pl->GetScaleFlag() );
		xmlAddDoubleNode( link_node, "Scale",   pl->GetScale() );

		xmlAddIntNode( link_node, "LowerLimitFlag", pl->GetLowerLimitFlag() );
		xmlAddDoubleNode( link_node, "LowerLimit",  pl->GetLowerLimit() );
		xmlAddIntNode( link_node, "UpperLimitFlag", pl->GetUpperLimitFlag() );
		xmlAddDoubleNode( link_node, "UpperLimit",  pl->GetUpperLimit() );

	}
}

void ParmLinkMgr::ReadLinks(xmlNodePtr root, vector< Geom* > & geomVec)
{
	vector< Geom* > gVec = geomVec;
	gVec.push_back( aircraftPtr->getUserGeom() );

	xmlNodePtr node_list = xmlGetNode( root, "ParmLink_List", 0 );
	if ( node_list  )
	{
		int num_links = xmlGetNumNames( node_list, "Link" );
		for ( int i = 0 ; i < num_links ; i++ )
		{
			xmlNodePtr link_node = xmlGetNode( node_list, "Link", i );
			if ( link_node )
			{
				int geomA_id = xmlFindInt( link_node, "GeomA", 0 );
				Stringc groupA_name = Stringc( xmlFindString( link_node, "GroupA", " " ) );
				Stringc parmA_name =  Stringc( xmlFindString( link_node, "ParmA", " " ) );

				Parm* parmA = parmMgrPtr->FindParm( gVec, geomA_id, groupA_name, parmA_name );

				int geomB_id = xmlFindInt( link_node, "GeomB", 0 );
				Stringc groupB_name = Stringc( xmlFindString( link_node, "GroupB", " " ) );
				Stringc parmB_name = Stringc( xmlFindString( link_node, "ParmB", " " ) );

				Parm* parmB = parmMgrPtr->FindParm( gVec, geomB_id, groupB_name, parmB_name );


				if ( parmA && parmB )
				{
					ParmLink* pl = new ParmLink();

					pl->SetParmA( parmA );
					pl->SetParmB( parmB );

					pl->SetOffsetFlag( !!xmlFindInt( link_node, "OffsetFlag", 0 ) );
					pl->SetOffset( xmlFindDouble( link_node, "Offset", 0.0 ) );
					pl->SetScaleFlag( !!xmlFindInt( link_node, "ScaleFlag", 0 ) );
					pl->SetScale( xmlFindDouble( link_node, "Scale", 1.0 ) );

					pl->SetLowerLimitFlag( !!xmlFindInt( link_node, "LowerLimitFlag", 0 ) );
					pl->SetLowerLimit( xmlFindDouble( link_node, "LowerLimit", 0.0 ) );
					pl->SetUpperLimitFlag( !!xmlFindInt( link_node, "UpperLimitFlag", 0 ) );
					pl->SetUpperLimit( xmlFindDouble( link_node, "UpperLimit", 0.0 ) );

					pl->GetParmA()->set_linked_flag( true );
					pl->GetParmB()->set_linked_flag( true );

					m_ParmLinkVec.push_back( pl );
					m_CurrParmLinkIndex = (int)m_ParmLinkVec.size() - 1;
				}
			}
		}
	}
}

void ParmLinkMgr::SwapGeom( Geom* gOld, Geom* gNew )
{
	for ( int i = 0 ; i < (int)m_ParmLinkVec.size() ; i++ )
	{
		Parm* pA = m_ParmLinkVec[i]->GetParmA();
		if ( pA->get_geom_base() == gOld )
		{
			Stringc group_name = pA->get_group_name();
			Stringc parm_name  = pA->get_name();
			Parm* p = parmMgrPtr->FindParm( gNew, group_name, parm_name );
			if ( p )
				m_ParmLinkVec[i]->SetParmA( p );
		}
		Parm* pB = m_ParmLinkVec[i]->GetParmB();
		if ( pB->get_geom_base() == gOld )
		{
			Stringc group_name = pB->get_group_name();
			Stringc parm_name  = pB->get_name();
			Parm* p = parmMgrPtr->FindParm( gNew, group_name, parm_name );
			if ( p )
				m_ParmLinkVec[i]->SetParmB( p );
		}
		m_ParmLinkVec[i]->GetParmA()->set_linked_flag( true );
		m_ParmLinkVec[i]->GetParmB()->set_linked_flag( true );
	}
}


void ParmLinkMgr::RemoveAllReferencesLink( Geom* geomPtr )
{
	vector < Parm* > parmVec;
	geomPtr->LoadLinkableParms( parmVec );

	for ( int i = 0 ; i < (int)parmVec.size() ; i++ )
	{
		RemoveParmReferencesLink( parmVec[i] );
	}
	if ( aircraftPtr->getScreenMgr() )
		aircraftPtr->getScreenMgr()->getParmLinkScreen()->RemoveAllRefs( geomPtr );

}

void ParmLinkMgr::RemoveParmReferencesLink( Parm* parmPtr )
{
	//==== Remove From Parm Link Vec ====//
	vector< ParmLink* > tempVec;
	for ( int i = 0 ; i < (int)m_ParmLinkVec.size() ; i++ )
	{
		if ( m_ParmLinkVec[i]->GetParmA() != parmPtr && m_ParmLinkVec[i]->GetParmB() != parmPtr )
			tempVec.push_back( m_ParmLinkVec[i] );
	}
	m_ParmLinkVec = tempVec;

	if ( m_DefaultParm == parmPtr )
		m_DefaultParm = NULL;
	if ( m_WorkingParmLink.GetParmA() == parmPtr || m_WorkingParmLink.GetParmB() == parmPtr )
		ResetWorkingParmLink();

	parmPtr->set_linked_flag( false );
	parmPtr->clear_all_devs();

	m_CurrParmLinkIndex = -1;
}

void ParmLinkMgr::RebuildAllLink()
{
	RebuildParmLinkMap();

	if ( aircraftPtr->getScreenMgr() )
		aircraftPtr->getScreenMgr()->getParmLinkScreen()->update();
}

void ParmLinkMgr::RebuildParmLinkMap()
{
	map< Parm*, vector< ParmLink* > >::iterator itr;
	for ( itr = m_ParmLinkMap.begin() ; itr != m_ParmLinkMap.end() ; itr++ )
		itr->second.clear();

	m_ParmLinkMap.clear();
	for ( int i = 0 ; i < (int)m_ParmLinkVec.size() ; i++ )
	{
		ParmLink* pl = m_ParmLinkVec[i];
		m_ParmLinkMap[pl->GetParmA()].push_back( pl );
		pl->GetParmA()->set_linked_flag( true );
		pl->GetParmB()->set_linked_flag( true );

	}
}


void ParmLinkMgr::SetCurrParmLinkIndex( int i )
{
	m_CurrParmLinkIndex = i;
}

ParmLink* ParmLinkMgr::GetCurrParmLink()
{
	if (  m_CurrParmLinkIndex >= 0 && m_CurrParmLinkIndex < (int)m_ParmLinkVec.size() )
		return m_ParmLinkVec[ m_CurrParmLinkIndex ];

	return &m_WorkingParmLink;
}

vector< ParmLink* > ParmLinkMgr::GetParmLinkVec()
{
	return m_ParmLinkVec;
}



void ParmLinkMgr::SetParmA( Parm* p )
{
	m_WorkingParmLink.SetParmA( p );
	m_WorkingParmLink.InitOffsetScale();
	RebuildAllLink();
}

void ParmLinkMgr::SetParmB( Parm* p )
{
	m_WorkingParmLink.SetParmB( p );
	m_WorkingParmLink.InitOffsetScale();
	RebuildAllLink();
}

void ParmLinkMgr::SetParm( bool flagA, int comp_ind, int group_ind, int parm_ind )
{
	vector< Geom* > compVec = aircraftPtr->getGeomVec();
	compVec.push_back( aircraftPtr->getUserGeom() );

	if ( compVec.size() == 0 )
	{
		m_WorkingParmLink.SetParmA( NULL );
		m_WorkingParmLink.SetParmB( NULL );
		return;
	}

	if ( comp_ind >= (int)compVec.size() ) comp_ind = 0;
	Geom* gPtr = compVec[comp_ind];

	string group_name = parmMgrPtr->GetGroupName( gPtr, group_ind );

	vector< Parm* > parmVec = parmMgrPtr->GetParmVec( gPtr, group_name );

	if ( parmVec.size() == 0 )
	{
		if ( flagA )	m_WorkingParmLink.SetParmA( NULL );
		else			m_WorkingParmLink.SetParmB( NULL );
		return;
	}

	if ( parm_ind >= (int)parmVec.size() )
		parm_ind = 0;

	if ( flagA )
		m_WorkingParmLink.SetParmA( parmVec[parm_ind] );
	else
		m_WorkingParmLink.SetParmB( parmVec[parm_ind] );

	m_WorkingParmLink.InitOffsetScale();

}


bool ParmLinkMgr::AddCurrLink()
{
	//==== Check if Modifying Already Add Link ====//
	if (  m_CurrParmLinkIndex >= 0 && m_CurrParmLinkIndex < (int)m_ParmLinkVec.size() )
		return false;

	if ( m_WorkingParmLink.GetParmA() == m_WorkingParmLink.GetParmB() )
		return false;

	if ( CheckForDuplicateLink( m_WorkingParmLink.GetParmA(), m_WorkingParmLink.GetParmB() ) )
		return false;

	ParmLink* pl = new ParmLink();
	*pl = m_WorkingParmLink;

	pl->GetParmA()->set_linked_flag( true );
	pl->GetParmB()->set_linked_flag( true );

	m_ParmLinkVec.push_back( pl );
	m_CurrParmLinkIndex = (int)m_ParmLinkVec.size() - 1;

	m_ParmLinkMap[pl->GetParmA()].push_back( pl );
	ParmChanged( pl->GetParmA(), true );				// Trigger Update

	return true;
}

bool ParmLinkMgr::CheckForDuplicateLink( Parm* pA, Parm* pB )
{
	for ( int i = 0 ; i < (int)m_ParmLinkVec.size() ; i++ )
	{
		if ( m_ParmLinkVec[i]->GetParmA() == pA && 	
			 m_ParmLinkVec[i]->GetParmB() == pB )
		{
			return true;
		}
	}
	return false;
}



bool ParmLinkMgr::LinkAllComp()
{
	Parm* pA =  m_WorkingParmLink.GetParmA();
	Parm* pB =  m_WorkingParmLink.GetParmB();

	if ( pA->get_geom_base() == pB->get_geom_base() )
		return false;

	//==== For All Parms in GeomA, Try to Find Match in GeomB and Link ====//
	vector< string > groupAVec = parmMgrPtr->GetGroupNameVec( pA->get_geom_base() );
	vector< string > groupBVec = parmMgrPtr->GetGroupNameVec( pB->get_geom_base() );

	for ( int i = 0 ; i < (int)groupAVec.size() ; i++ )
	{
		for ( int j = 0 ; j < (int)groupBVec.size() ; j++ )
		{
			if ( groupAVec[i].compare( groupBVec[j] ) == 0 )
			{
				vector< Parm* > parmAVec = parmMgrPtr->GetParmVec( pA->get_geom_base(), groupAVec[i] );
				vector< Parm* > parmBVec = parmMgrPtr->GetParmVec( pB->get_geom_base(), groupBVec[j] );

				for ( int k = 0 ; k < (int)parmAVec.size() ; k++ )
				{
					for ( int n = 0 ; n < (int)parmBVec.size() ; n++ )
					{
						Parm* parmA = parmAVec[k];
						Parm* parmB = parmBVec[n];

						if ( parmA->get_name() == parmB->get_name() )
						{
							AddLink( parmA, parmB );
						}
					}
				}
			}
		}
	}

	return true;
}

void ParmLinkMgr::AddLink( Parm* pA, Parm* pB )
{
	//==== Make Sure Parm Are Not Already Linked ====//
	for ( int i = 0 ; i < (int)m_ParmLinkVec.size() ; i++ )
	{
		if ( m_ParmLinkVec[i]->GetParmA() == pA && 
			 m_ParmLinkVec[i]->GetParmB() == pB  )
		{
			return;
		}
	}

	ParmLink* pl = new ParmLink();

	pl->SetParmA( pA );
	pl->SetParmB( pB );
	pl->SetOffsetFlag( true );
	pl->SetOffset( pB->get() - pA->get() );
	pl->SetScaleFlag( false );
	pl->SetScale( 1.0 );

	pA->set_linked_flag( true );
	pB->set_linked_flag( true );

	m_ParmLinkVec.push_back( pl );
	m_CurrParmLinkIndex = (int)m_ParmLinkVec.size() - 1;

	m_ParmLinkMap[pl->GetParmA()].push_back( pl );
	ParmChanged( pl->GetParmA(), true );				// Trigger Update
}

bool ParmLinkMgr::LinkAllGroup()
{
	Parm* pA =  m_WorkingParmLink.GetParmA();
	Parm* pB =  m_WorkingParmLink.GetParmB();

	if ( pA->get_geom_base() == pB->get_geom_base() )
		return false;

	string gnameA = string( pA->get_group_name().get_char_star() );
	string gnameB = string( pB->get_group_name().get_char_star() );

	vector< Parm* > parmAVec = parmMgrPtr->GetParmVec( pA->get_geom_base(), gnameA );
	vector< Parm* > parmBVec = parmMgrPtr->GetParmVec( pB->get_geom_base(), gnameB );

	for ( int k = 0 ; k < (int)parmAVec.size() ; k++ )
	{
		for ( int n = 0 ; n < (int)parmBVec.size() ; n++ )
		{
			Parm* parmA = parmAVec[k];
			Parm* parmB = parmBVec[n];

			if ( parmA->get_name() == parmB->get_name() )
			{
				AddLink( parmA, parmB );
			}
		}
	}

	return true;
}

void ParmLinkMgr::DelCurrLink()
{
	//==== Remove From Parm Link Vec ====//
	vector< ParmLink* > tempVec;
	for ( int i = 0 ; i < (int)m_ParmLinkVec.size() ; i++ )
	{
		if ( i == m_CurrParmLinkIndex )
		{
			m_ParmLinkVec[i]->GetParmA()->set_linked_flag( false );
			m_ParmLinkVec[i]->GetParmB()->set_linked_flag( false );
			delete m_ParmLinkVec[i];
		}
		else 
			tempVec.push_back( m_ParmLinkVec[i] );
	}
	m_ParmLinkVec = tempVec;

	for ( int i = 0 ; i < (int)m_ParmLinkVec.size() ; i++ )
	{
		m_ParmLinkVec[i]->GetParmA()->set_linked_flag( true );
		m_ParmLinkVec[i]->GetParmB()->set_linked_flag( true );
	}


	RebuildParmLinkMap();
}

void ParmLinkMgr::DelAllLinks()
{
	for ( int i = 0 ; i < (int)m_ParmLinkVec.size() ; i++ )
	{
		m_ParmLinkVec[i]->GetParmA()->set_linked_flag( false );
		m_ParmLinkVec[i]->GetParmB()->set_linked_flag( false );
		delete m_ParmLinkVec[i];
	}
	m_ParmLinkVec.clear();

	RebuildParmLinkMap();
}
	

ParmLink* ParmLinkMgr::ResetWorkingParmLink()
{
	m_CurrParmLinkIndex = -1;
	m_WorkingParmLink.SetParmA( m_DefaultParm );
	m_WorkingParmLink.SetParmB( m_DefaultParm );

	m_WorkingParmLink.SetScale( 1.0 );
	m_WorkingParmLink.SetScaleFlag( false );

	m_WorkingParmLink.SetOffset( 0.0 );
	m_WorkingParmLink.SetOffsetFlag( true );

	return &m_WorkingParmLink;
}

void ParmLinkMgr::ParmChanged( Parm* parmPtr, bool start_flag  )
{
	int i;
	map< Parm*, vector< ParmLink* > >::iterator itr;

	//==== Look for Links and Modify Linked Parms ====//
	itr = m_ParmLinkMap.find( parmPtr );

	if ( itr == m_ParmLinkMap.end() )
		return;

	if ( parmPtr->get_link_update_flag() == true )
		return;

	vector< ParmLink* > plVec = itr->second;

	if ( plVec.size() == 0 )
		return;

	parmPtr->set_link_update_flag( true );
	m_UpdatedParmVec.push_back( parmPtr );

	for ( i = 0 ; i < (int)plVec.size() ; i++ )
	{
		ParmLink* pl = plVec[i];

		if ( pl->GetParmB()->get_link_update_flag() == false )			// Prevent Circular 
		{
			double offset = 0.0;
			if ( pl->GetOffsetFlag() )	offset = pl->GetOffset();
			double scale = 1.0;
			if ( pl->GetScaleFlag() )	scale = pl->GetScale();

			double val = parmPtr->get()*scale + offset;

			if ( pl->GetLowerLimitFlag() && val < pl->GetLowerLimit() )		// Constraints
				val = pl->GetLowerLimit();

			if ( pl->GetUpperLimitFlag() && val > pl->GetUpperLimit() )		// Constraints
				val = pl->GetUpperLimit();

			pl->GetParmB()->set_from_link( val );
			pl->GetParmB()->get_geom()->parm_changed( pl->GetParmB() );
		}
	}

	if ( start_flag )										// Clean Up
	{
		for ( i = 0 ; i < (int)m_UpdatedParmVec.size() ; i++ )
			m_UpdatedParmVec[i]->set_link_update_flag( false );

		m_UpdatedParmVec.clear();
	}
}


//=============================================================//
//=============================================================//



ParmMgr::ParmMgr()
{
	aircraftPtr = NULL;
}

ParmMgr::~ParmMgr()
{

}


Parm* ParmMgr::FindParm( vector< Geom* > & gVec, int ptrID, Stringc& group_name, Stringc& parm_name )
{
	Parm* pPtr = NULL;
	Geom* gPtr = NULL;
	for ( int i = 0 ; i < (int)gVec.size() ; i++ )
	{
		if ( gVec[i]->getPtrID() == ptrID )
			gPtr = gVec[i];
	}
	//==== Check for User Geom ====//
	if ( gPtr == NULL )
	{
		if ( group_name == Stringc("User") )
		{
			gPtr = aircraftPtr->getUserGeom();
		}
	}

	pPtr = FindParm( gPtr, group_name, parm_name );

	return pPtr;
}

Parm* ParmMgr::FindParm( Geom* gPtr, Stringc& group_name, Stringc& parm_name )
{
	Parm* pPtr = NULL;
	if ( gPtr )
	{
		vector< Parm* > pVec;
		gPtr->LoadLinkableParms( pVec );
		for ( int i = 0 ; i < (int)pVec.size() ; i++ )
		{
			if ( pVec[i]->get_name() == parm_name &&
				 pVec[i]->get_group_name() == group_name )
			{
				pPtr = pVec[i];
			}
		}
	}
	return pPtr;
}

void ParmMgr::RebuildAll()
{
	LoadAllParms();
	parmLinkMgrPtr->RebuildAllLink();
}

void ParmMgr::LoadAllParms()
{
	//==== Clear All Existing Parms ====//
	map< GeomBase*, map< string, vector< Parm* > > >::iterator g;
	map< string, vector<Parm*> >::iterator itr;
	for ( g = m_ParmMap.begin() ; g != m_ParmMap.end() ; g++ )
	{
		for ( itr = g->second.begin() ; itr != g->second.end() ; itr++ )
		{
			itr->second.clear();
		}
		g->second.clear();
	}
	m_ParmMap.clear();

	if ( aircraftPtr == NULL )
		return;

	//==== Loop Thru All Geoms ====//
	vector< Geom* > geomVec = aircraftPtr->getGeomVec();
	geomVec.push_back( aircraftPtr->getUserGeom() );
	for ( int i = 0 ; i < (int)geomVec.size() ; i++ )
	{
		vector < Parm* > parmVec;
		geomVec[i]->LoadLinkableParms( parmVec );
		for ( int j = 0 ; j < (int)parmVec.size() ; j++ )
		{
			Parm* p = parmVec[j];
			p->set_linked_flag( false );
			Register( p, p->get_geom_base(), p->get_group_name().get_char_star() );
		}
	}
}

void ParmMgr::Register( Parm* parmPtr, GeomBase* geomPtr, string groupName )
{
	if ( parmPtr == NULL || geomPtr == NULL )
		return;

	parmPtr->set_group_name( groupName.c_str() );
	parmPtr->set_base_geom( geomPtr );

	m_ParmMap[geomPtr][groupName].push_back( parmPtr );

	if ( !parmLinkMgrPtr->GetDefaultParm() )
		parmLinkMgrPtr->SetDefaultParm( parmPtr );
}

void ParmMgr::RegisterParmButton( ParmButton* b )
{
	if ( aircraftPtr->getScreenMgr() )
		aircraftPtr->getScreenMgr()->getParmLinkScreen()->RegisterParmButton( b );
}

void ParmMgr::RemoveAllReferences( Geom* geomPtr )
{
	parmLinkMgrPtr->RemoveAllReferencesLink( geomPtr );
}

void ParmMgr::RemoveParmReferences( Parm* parmPtr )
{
	parmLinkMgrPtr->RemoveParmReferencesLink( parmPtr );
}

vector< string > ParmMgr::GetGroupNameVec( GeomBase* geomPtr )
{
	vector< string > groupNameVec;

	map< string, vector< Parm* > >::iterator itr;

	for ( itr = m_ParmMap[geomPtr].begin() ; itr != m_ParmMap[geomPtr].end() ; ++itr )
	{
		groupNameVec.push_back( itr->first );

	}
	return groupNameVec;
}

vector< Parm* > ParmMgr::GetParmVec( GeomBase* geomPtr, string group_name  )
{
	return m_ParmMap[geomPtr][group_name];
}

string ParmMgr::GetGroupName( GeomBase* geomPtr, int name_index )
{
	vector< string > group_vec = GetGroupNameVec( geomPtr );
	if ( name_index >= (int)group_vec.size() )
		name_index = (int)group_vec.size()-1;
	if ( name_index >= 0 /*&& name_index < (int)group_vec.size()*/ )
		return group_vec[name_index];

	return string("Design");
}

int ParmMgr::GetCurrGeomNameVec( Parm* parmPtr, vector< string > & nameVec )
{
	int index = 0;
	vector< Geom* > geomVec = aircraftPtr->getGeomVec();
	geomVec.push_back( aircraftPtr->getUserGeom() );

	for ( int i = 0 ; i < (int)geomVec.size() ; i++ )
	{
		nameVec.push_back( geomVec[i]->getName().get_char_star() );
		if ( parmPtr && parmPtr->get_geom_base() == geomVec[i] )
			index = i;
	}
	return index;
}

int ParmMgr::GetCurrGroupNameVec( Parm* parmPtr, vector< string > & nameVec )
{
	int index = 0;
	if ( !parmPtr )
		return index;

	nameVec = GetGroupNameVec( parmPtr->get_geom_base() );

	string name = string( parmPtr->get_group_name().get_char_star() );

	for ( int i = 0 ; i < (int)nameVec.size() ; i++ )
	{
		if ( name.compare( nameVec[i] ) == 0 )
			index = i;
	}

	return index;
}

int	ParmMgr::GetCurrParmNameVec( Parm* parmPtr, vector< string > & nameVec )
{
	int index = 0;

	if ( !parmPtr )
		return index;

	string group_name = string( parmPtr->get_group_name().get_char_star() );
	vector< Parm* > pVec = GetParmVec( parmPtr->get_geom_base(), group_name );

	for ( int i = 0 ; i < (int)pVec.size() ; i++ )
	{
		if ( pVec[i] == parmPtr )
			index = i;

		nameVec.push_back( string( pVec[i]->get_name().get_char_star() ) );
	}

	return index;
}


