//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

//******************************************************************************
//
//   Merge Near Pnts 
//
//   J.R. Gloudemans 8/28/12
//
//******************************************************************************

#include "PntNodeMerge.h"

	
void PntNodeCloud::AddPntNodes( vector< vec3d > & pnts )
{
	for ( size_t i = 0 ; i < pnts.size() ; i++ )
	{
		m_PntNodes.push_back( PntNode( pnts[i] ) );
	}
}

bool PntNodeCloud::UsedNode( int i )
{
	assert ( i >= 0 && i < (int)m_PntNodes.size() );

	if ( i == m_PntNodes[i].m_Index )
		return true;

	return false;
}

int PntNodeCloud::GetNodeUsedIndex( int i )
{
	assert ( i >= 0 && i < (int)m_PntNodes.size() );

	int ind = m_PntNodes[i].m_Index;

	return m_PntNodes[ind].m_UsedIndex;

}

void IndexPntNodes( PntNodeCloud & cloud, double tol )
{
	PNTree index( 3, cloud, KDTreeSingleIndexAdaptorParams(10)  );
	index.buildIndex();

	//==== Find Close Point Groups ====//
	int cnt = 0;
	for ( size_t i = 0 ; i < cloud.m_PntNodes.size() ; i++ )
	{
		if ( cloud.m_PntNodes[i].m_Index == -1 )
		{
			std::vector<std::pair<size_t,double> >   ret_matches;

			nanoflann::SearchParams params;
			const size_t nMatches = 
				index.radiusSearch( &cloud.m_PntNodes[i].m_Pnt[0], tol, ret_matches, params);

			for ( size_t j = 0 ; j < ret_matches.size() ; j++ )
			{
				size_t m_ind = ret_matches[j].first;
				cloud.m_PntNodes[ m_ind ].m_Index = i;
			}
			cloud.m_PntNodes[i].m_UsedIndex = cnt;
			cnt++;
		}
	}
}



