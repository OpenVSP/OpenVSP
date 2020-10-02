//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// SimpleTess.cpp:
// Rob McDonald
//
//////////////////////////////////////////////////////////////////////

#include "SimpleTess.h"

SimpleTess::SimpleTess()
{
    m_FlipNormal = false;
}

SimpleTess::~SimpleTess()
{
}

void SimpleTess::Transform( const Matrix4d & mat )
{
    // Transform points
    for ( int i = 0; i < m_pnts.size(); i++ )
    {
        for ( int j = 0; j < m_pnts[i].size(); j++ )
        {
            mat.xformvec( m_pnts[i][j] );
        }
    }

    // Transform normal vectors
    for ( int i = 0; i < m_norms.size(); i++ )
    {
        for ( int j = 0; j < m_norms[i].size(); j++ )
        {
            mat.xformnormvec( m_norms[i][j] );
        }
    }
}

void SimpleTess::GetBoundingBox( BndBox &bb ) const
{
    bb.Reset();

    for ( int i = 0; i < m_pnts.size(); i++ )
    {
        for ( int j = 0; j < m_pnts[i].size(); j++ )
        {
            for ( int k = 0; k < m_pnts[i][j].size(); k++ )
            {
                bb.Update( m_pnts[i][j][k] );
            }
        }
    }
}

void SimpleTess::CalcTexCoords( )
{
    int nu = m_nufeat - 1;
    int nv = m_nvfeat - 1;

    unsigned int n = nu * nv;

    m_utex.resize(n);
    m_vtex.resize(n);

    int k = 0;
    for ( int i = 0; i < nu; i++ )
    {

        for ( int j = 0; j < nv; j++ )
        {
            unsigned int nui = m_pnts[k].size();

            m_utex[k].resize(nui);
            m_vtex[k].resize(nui);
            for ( int ii = 0; ii < nui; ii++ )
            {
                unsigned int nvj = m_pnts[k][0].size();

                m_utex[k][ii].resize(nvj);
                m_vtex[k][ii].resize(nvj);


                for ( int jj = 0; jj < nvj; jj++ )
                {
                    if ( ii == 0 )
                    {
                        if ( i == 0 )
                        {
                            m_utex[k][ii][jj] = 0.0;
                        }
                        else
                        {
                            int ilast = m_utex[(i-1)*nv+j].size() - 1;
                            m_utex[k][ii][jj] = m_utex[(i-1)*nv+j][ilast][jj]; // previous kpatch iend;
                        }
                    }
                    else
                    {
                        double du = dist( m_pnts[k][ii][jj], m_pnts[k][ii-1][jj] );
                        if ( du < 1e-6 )
                        {
                            du = 1.0;
                        }
                        m_utex[k][ii][jj] = m_utex[k][ii-1][jj] + du;
                    }

                    if ( jj == 0 )
                    {
                        if ( j == 0 )
                        {
                            m_vtex[k][ii][jj] = 0.0;
                        }
                        else
                        {
                            int jlast = m_vtex[i*nv+j-1][ii].size() - 1;
                            m_vtex[k][ii][jj] = m_vtex[i*nv+j-1][ii][jlast]; // previous kpatch jend;
                        }
                    }
                    else
                    {
                        double dv = dist( m_pnts[k][ii][jj], m_pnts[k][ii][jj-1] );
                        if ( dv < 1e-6 )
                        {
                            dv = 1.0;
                        }
                        m_vtex[k][ii][jj] = m_vtex[k][ii][jj-1] + dv;
                    }
                }
            }
            k++;
        }
    }

    k = 0;
    for ( int i = 0; i < nu; i++ )
    {
        for ( int j = 0; j < nv; j++ )
        {
            int nui = m_pnts[k].size();

            for ( int ii = 0; ii < nui; ii++ )
            {
                int nvj = m_pnts[k][0].size();

                for ( int jj = 0; jj < nvj; jj++ )
                {
                    int kjlast = i*nv+nv-1;
                    int kilast = (nu-1)*nv+j;

                    int imax = m_utex[kilast].size()-1;
                    int jmax = m_vtex[kjlast][0].size()-1;

                    m_utex[k][ii][jj] /= m_utex[kilast][imax][jj];
                    m_vtex[k][ii][jj] /= m_vtex[kjlast][ii][jmax];
                }
            }
            k++;
        }
    }
}
