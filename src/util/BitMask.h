//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// BitMask.h
//
// Created by Rob McDonald on 10/12/22.

#ifndef VSP_TOP_BITMASK_H
#define VSP_TOP_BITMASK_H

#include <vector>
#include <string>

class BitMask
{
public:
    BitMask()
    {
        m_Store = 0;
    }

    BitMask( int val )
    {
        m_Store = val;
    }

    BitMask( const std::vector < bool > &boolvec )
    {
        m_Store = 0;

        for ( int i = 0; i < boolvec.size(); i++ )
        {
            if ( boolvec[i] )
            {
                SetBit( i );
            }
        }
    }

    void SetBit( unsigned i )
    {
        m_Store |= 1 << i;
    }

    void ClearBit( unsigned i )
    {
        m_Store &= ~( 1 << i );
    }

    void FlipBit( unsigned i )
    {
        m_Store ^= 1 << i;
    }

    bool CheckBit( unsigned i )
    {
        return m_Store & (1 << i );
    }

    void Or( const BitMask &bm )
    {
        m_Store |= bm.m_Store;
    }

    std::vector < bool > AsBoolVec( int n )
    {
        std::vector < bool > boolvec( n, false );

        for ( int i = 0; i < n; i++ )
        {
            boolvec[i] = CheckBit(i);
        }

        return boolvec;
    }

    unsigned AsNum()
    {
        return m_Store;
    }

    std::string AsNASTRAN()
    {
        std::string str;

        for ( int i = 0; i < 6; i++ )
        {
            if ( CheckBit(i) )
            {
                str += std::to_string( i + 1 );
            }
        }

        return str;
    }

protected:
    unsigned m_Store;
};


#endif //VSP_TOP_BITMASK_H
