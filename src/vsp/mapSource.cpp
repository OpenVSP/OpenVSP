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

bool LongMapTargetLengthCompare(const MapSource &a, const MapSource &b)
{
    return ( *(b.m_strptr) < *(a.m_strptr) );
}

bool ShortMapTargetLengthCompare4D(const MapSource4D &a, const MapSource4D &b)
{
    return ( *(a.m_strptr) < *(b.m_strptr) );
}

bool LongMapTargetLengthCompare4D(const MapSource4D &a, const MapSource4D &b)
{
    return ( *(b.m_strptr) < *(a.m_strptr) );
}


void MSCloud::sort()
{
	std::sort( sources.begin(), sources.end(), ShortMapTargetLengthCompare );
}

void MSCloudFourD::sort()
{
	std::sort( sources.begin(), sources.end(), ShortMapTargetLengthCompare4D );
}

// Returns the distance between the vector "p1[0:size-1]" and the data point with index "idx_p2" stored in the class:
double MSCloudFourD::kdtree_distance( const double *p1, const size_t idx_p2, size_t size) const
{
	const double d0 = p1[0] - sources[idx_p2].m_pt.x();
	const double d1 = p1[1] - sources[idx_p2].m_pt.y();
	const double d2 = p1[2] - sources[idx_p2].m_pt.z();

	const double r2 = d0*d0 + d1*d1 + d2*d2;
	const double r = sqrt( r2 );

	const double targetstr = sources[idx_p2].m_initstr;

	const double ds = *str - targetstr;

	const double deltaS = ds + r * grm1;

	const double R = deltaT + deltaS;

	return R;
}


void MSCloudFourD::LimitTargetMap( MSTreeFourD &ms_tree, GridDensity* grid_den )
{
	int nsrc = sources.size();

	grm1 = grid_den->GetGrowRatio() - 1.0;
	double tmin = *( sources[ 0 ].m_strptr );
	double tmax = *( sources[ nsrc - 1 ].m_strptr );
	deltaT = tmax - tmin;

	SearchParams params;
	params.sorted = false;

	for ( int i = 0 ; i < nsrc ; i++ )
	{
		if( !sources[i].m_dominated )
		{
			double *query_pt = sources[i].m_pt.v;
			str = sources[i].m_strptr;

			MSTreeResults ms_matches;

			int nMatches = ms_tree.radiusSearch( query_pt, deltaT, ms_matches, params );

			for ( int j = 0; j < nMatches; j++ )
			{
				int imatch = ms_matches[j].first;
				double R = ms_matches[j].second;

				double deltaS = R - deltaT;

				double targetinitstr = sources[imatch].m_initstr;

				double targetadjust = targetinitstr + deltaS;

				if( targetadjust < ( *( sources[imatch].m_strptr ) ) )
				{
					*( sources[imatch].m_strptr ) = targetadjust;
					sources[imatch].m_dominated = true;
				}
			}
		}
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

void MSCloudFourD::free_strengths()
{
	int nsrc = sources.size();
	for ( int i = 0 ; i < nsrc ; i++ )
	{
		double *strptr = sources[i].m_strptr;
		delete strptr;
	}
}
