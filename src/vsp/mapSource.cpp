//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

//******************************************************************************
//
//   Map Source and KD-Tree
//
//   Rob McDonald 4/4/12
//
//******************************************************************************

#include "mapSource.h"

bool ShortMapTargetLengthCompare(const MapSource &a, const MapSource &b)
{
    return ( *(a.m_strptr) < *(b.m_strptr) );
}

void MSCloud::sort()
{
	std::sort( sources.begin(), sources.end(), ShortMapTargetLengthCompare );
}

void MSCloudProg::sort()
{
	std::sort( sources.begin(), sources.end(), ShortMapTargetLengthCompare );
}

// Returns the distance between the vector "p1[0:size-1]" and the data point with index "idx_p2" stored in the class:
double MSCloudProg::kdtree_distance( const double *p1, const size_t idx_p2, size_t size) const
{
	double rmax = ( *str - tmin ) / grm1;
	double r2max = rmax * rmax;

	const double d0 = p1[0] - sources[idx_p2].m_pt.x();
	const double d1 = p1[1] - sources[idx_p2].m_pt.y();
	const double d2 = p1[2] - sources[idx_p2].m_pt.z();

	double r2 = d0*d0 + d1*d1 + d2*d2;

	double r = sqrt( r2 );

	double targetstr = *( sources[idx_p2].m_strptr );

	double targetlocalstr = targetstr + grm1 * r;

	if( targetlocalstr < *str )
	{
		*str = targetlocalstr;
		rmax = ( *str - tmin ) / grm1;
		r2max = rmax * rmax;
	}

	return r2/r2max;
}


void MSCloudProg::LimitTargetMap( MSTreeProg &ms_tree, GridDensity* grid_den )
{
	int nsrc = sources.size();

	grm1 = grid_den->GetGrowRatio() - 1.0;
	tmin = *( sources[ 0 ].m_strptr );
//	double tmax = *( sources[ nsrc - 1 ].m_strptr );

	SearchParams params;
	params.sorted = false;

	for ( int i = 0 ; i < nsrc ; i++ )
	{
		double *query_pt = sources[i].m_pt.v;
		str = sources[i].m_strptr ;

		MSTreeResults ms_matches;

		int nMatches = ms_tree.radiusSearch( query_pt, 1.0, ms_matches, params );
	}
}

void MSCloud::prune_map_sources( MSTree &ms_tree, GridDensity* grid_den )
{
	int nsrc = sources.size();
	vector<bool> remove (nsrc, false);

	double grm1 = grid_den->GetGrowRatio() - 1.0;
	double tmin = grid_den->GetMinLen();

	SearchParams params;
	params.sorted = false;

	for ( int i = 0 ; i < nsrc ; i++ )
	{
		double *query_pt = sources[i].m_pt.v;
		double localstr = *( sources[i].m_strptr );

		MSTreeResults es_matches;

		double rmax = ( localstr - tmin ) / grm1;
		double r2max = rmax * rmax;

		int nMatches = ms_tree.radiusSearch( query_pt, r2max, es_matches, params );

		for ( int j = 0; j < nMatches; j++ )
		{
			int imatch = es_matches[j].first;
			double r = sqrt( es_matches[j].second );

			double targetstr = *( sources[imatch].m_strptr );

			double targetlocalstr = targetstr + grm1 * r;

			if( targetlocalstr < localstr )
			{
				remove[i] = true;
				break;
			}
		}
	}

	vector< MapSource > new_sources;
	new_sources.reserve( nsrc );

	for ( int i = 0 ; i < nsrc ; i++ )
	{
		if( !remove[i] )
			new_sources.push_back( sources[i] );
	}

	sources.swap( new_sources );
}

void MSCloud::free_strengths()
{
	int nsrc = sources.size();
	for ( int i = 0 ; i < nsrc ; i++ )
	{
		double *strptr = sources[i].m_strptr;
		delete strptr;
	}
}

void MSCloudProg::free_strengths()
{
	int nsrc = sources.size();
	for ( int i = 0 ; i < nsrc ; i++ )
	{
		double *strptr = sources[i].m_strptr;
		delete strptr;
	}
}
