//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

//******************************************************************************
//
//   Edge Source and KD-Tree
//
//   Rob McDonald 4/4/12
//
//******************************************************************************

#include "edgeSource.h"

void ESCloud::prune_edge_sources( ESTree &es_tree, GridDensity* grid_den )
{
	int nsrc = sources.size();
	vector<bool> remove (nsrc, false);

	double grm1 = grid_den->GetGrowRatio() - 1.0;

	double rmax = grid_den->GetBaseLen() / ( grid_den->GetGrowRatio() - 1.0 );

	SearchParams params;
	params.sorted = false;

	for ( int i = 0 ; i < nsrc ; i++ )
	{
		double *query_pt = sources[i].m_pt.v;
		double localstr = sources[i].m_str;

		ESTreeResults es_matches;

		int nMatches = es_tree.radiusSearch( query_pt, rmax, es_matches, params );

		for ( int j = 0; j < nMatches; j++ )
		{
			int imatch = es_matches[j].first;
			double r = es_matches[j].second;

			double targetstr = sources[imatch].m_str;

			double remotestr = localstr + grm1 * r;

			if( targetstr > remotestr )
				remove[imatch] = true;
		}
	}

	vector< EdgeSource > new_sources;
	new_sources.reserve( nsrc );

	for ( int i = 0 ; i < nsrc ; i++ )
	{
		if( !remove[i] )
			new_sources.push_back( sources[i] );
	}

	sources.swap( new_sources );
}
