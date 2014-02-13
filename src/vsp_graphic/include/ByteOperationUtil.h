#ifndef _VSP_GRAPHIC_BYTE_OPERATION_UTIL_H
#define _VSP_GRAPHIC_BYTE_OPERATION_UTIL_H

namespace VSPGraphic
{
/*!
* Check the Endianness.
*/
static int isLittleEndian()
{
    union IntBytes
    {
        unsigned int uInt;
        unsigned char bytes[sizeof( int )];
    }
    EndianTest = {1};

    return EndianTest.bytes[0] == 1;
}

/*!
* Convert Byte Array to Unsigned Integer.
*/
static void bytesToUInt( const unsigned char * bytes, unsigned int * uInt_out )
{
    int i;
    union bytesToUInt
    {
        unsigned int unsigned_Int;
        unsigned char bytes[sizeof( unsigned int )];
    }
    convert;

    if( isLittleEndian() )
    {
        for( i = 0; i < sizeof( unsigned int ); i++ )
        {
            convert.bytes[i] = bytes[sizeof( unsigned int ) - 1 - i];
        }
    }
    else
    {
        for( i = 0; i < sizeof( unsigned int ); i++ )
        {
            convert.bytes[i] = bytes[i];
        }
    }
    *uInt_out = convert.unsigned_Int;
}

/*!
* Convert Unsigned Integer to Byte Array.
*/
static void uIntToBytes( const unsigned int uInt, unsigned char * bytes_out )
{
    int i;
    union uIntToBytes
    {
        unsigned int unsigned_Int;
        unsigned char bytes[sizeof( unsigned int )];
    }
    convert = {uInt};

    if( isLittleEndian() )
    {
        for( i = 0; i < sizeof( uInt ); i++ )
        {
            bytes_out[i] = convert.bytes[sizeof( uInt ) - 1 - i];
        }
    }
    else
    {
        for( i = 0; i < sizeof( uInt ); i++ )
        {
            bytes_out[i] = convert.bytes[i];
        }
    }
}
}
#endif