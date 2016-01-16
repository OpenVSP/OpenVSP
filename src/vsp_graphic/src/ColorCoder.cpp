#include "ColorCoder.h"
#include "ByteOperationUtil.h"

namespace VSPGraphic
{
namespace ColorCode
{
static unsigned int endofblock = 0x00000001;
}

ColorCoder::ColorCoder()
{
}
ColorCoder::~ColorCoder()
{
}

void ColorCoder::genCodeBlock( int size, unsigned int * start_out, unsigned int * end_out, unsigned char * block_out )
{
    unsigned int newStart;
    int i;

    newStart = ColorCode::endofblock;

    // Check recyclable blocks.
    if( !_freelist.empty() )
    {
        for( i = 0; i < ( int )_freelist.size(); i++ )
        {
            int blocksize = _freelist[i].end - _freelist[i].start + 1;
            if( blocksize >= size )
            {
                newStart = _freelist[i].start;

                _freelist[i].start += size;
                if( _freelist[i].start == _freelist[i].end )
                {
                    _freelist.erase( _freelist.begin() + i );
                }
            }
        }
    }

    // If nothing fits, push to the back of the block.
    if( newStart == ColorCode::endofblock )
    {
        ColorCode::endofblock += size;
    }

    // Update start_out, end_out, and block_out.

    *start_out = newStart;
    *end_out = newStart + size - 1;

    unsigned int iterator = newStart;
    for( i = 0; i < size; i++ )
    {
        unsigned char bytes[sizeof( unsigned int )];
        uIntToBytes( iterator, bytes );
        for( int n = 0; n < sizeof( unsigned int ); n++ )
        {
            block_out[i * sizeof( unsigned int ) + n] = bytes[n];
        }
        iterator++;
    }
}

void ColorCoder::freeCodeBlock( unsigned int start, unsigned int end )
{
    if( (start <= 0) || (end < start) )
    {
        return;
    }

    int i;
    _freeblock block;

    block.start = start;
    block.end = end;

    // Merge right.
    for( i = 0; i < ( int )_freelist.size(); i++ )
    {
        if( block.end + 1 == _freelist[i].start )
        {
            block.end = _freelist[i].end;

            _freelist.erase( _freelist.begin() + i );
        }
    }

    // Merge left.
    for( i = 0; i < ( int )_freelist.size(); i++ )
    {
        if( block.start == _freelist[i].end + 1 )
        {
            block.start = _freelist[i].start;

            _freelist.erase( _freelist.begin() + i );
        }
    }

    if( block.end + 1 == ColorCode::endofblock )
    {
        ColorCode::endofblock = block.start;
    }
    else
    {
        _freelist.push_back( block );
    }
}
}
