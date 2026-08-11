//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// UnformattedFile.cpp
//
//////////////////////////////////////////////////////////////////////

#include "UnformattedFile.h"

#include <climits>
#include <cstring>

// Reverse a value in place, one byte at a time.  Only used when writing for a reader of
// the opposite endianness, so it is not worth being clever about.
static void SwapBytes( void* data, size_t nbyte )
{
    unsigned char* p = ( unsigned char* ) data;

    for ( size_t i = 0; i < nbyte / 2; i++ )
    {
        unsigned char t = p[i];
        p[i] = p[ nbyte - 1 - i ];
        p[ nbyte - 1 - i ] = t;
    }
}

UnformattedOut::UnformattedOut()
{
    m_FP = nullptr;
    m_Good = true;

    m_SinglePrecision = false;
    m_ByteSwap = false;

    m_InRecord = false;
    m_RecordStart = 0;
    m_RecordBytes = 0;
}

UnformattedOut::~UnformattedOut()
{
    Close();
}

bool UnformattedOut::HostIsLittleEndian()
{
    const unsigned int one = 1;

    return *( const unsigned char* ) &one == 1;
}

bool UnformattedOut::Open( const string &fname )
{
    Close();

    m_FP = fopen( fname.c_str(), "wb" );
    m_Good = ( m_FP != nullptr );

    return m_Good;
}

void UnformattedOut::Close()
{
    if ( m_FP )
    {
        // A record left open would have no count, so close it rather than write a file
        // that cannot be read.
        if ( m_InRecord )
        {
            EndRecord();
        }

        fclose( m_FP );
        m_FP = nullptr;
    }

    m_InRecord = false;
    m_RecordStart = 0;
    m_RecordBytes = 0;
}

void UnformattedOut::WriteBytes( const void* data, size_t nbyte )
{
    if ( !m_FP || !m_Good )
    {
        return;
    }

    if ( fwrite( data, 1, nbyte, m_FP ) != nbyte )
    {
        m_Good = false;
    }
}

// Put a record's byte count at a given offset, then carry on at the end of the file.
void UnformattedOut::WriteMarker( long offset, long nbyte )
{
    if ( !m_FP || !m_Good )
    {
        return;
    }

    if ( nbyte > ( long )INT_MAX )
    {
        // A four byte marker cannot describe this record.  Fortran runtimes split such
        // records into subrecords, with conventions that vary; rather than guess at one,
        // say the file is no good.
        m_Good = false;
        return;
    }

    long here = ftell( m_FP );

    if ( fseek( m_FP, offset, SEEK_SET ) != 0 )
    {
        m_Good = false;
        return;
    }

    int marker = ( int )nbyte;
    if ( m_ByteSwap )
    {
        SwapBytes( &marker, sizeof( marker ) );
    }

    if ( fwrite( &marker, sizeof( marker ), 1, m_FP ) != 1 )
    {
        m_Good = false;
    }

    if ( fseek( m_FP, here, SEEK_SET ) != 0 )
    {
        m_Good = false;
    }
}

void UnformattedOut::BeginRecord()
{
    if ( !m_FP || !m_Good )
    {
        return;
    }

    if ( m_InRecord )
    {
        EndRecord();
    }

    m_RecordStart = ftell( m_FP );
    m_RecordBytes = 0;
    m_InRecord = true;

    // Leave room for the count; it is filled in by EndRecord.
    int marker = 0;
    WriteBytes( &marker, sizeof( marker ) );
}

void UnformattedOut::EndRecord()
{
    if ( !m_FP || !m_InRecord )
    {
        return;
    }

    m_InRecord = false;

    // The count appears again after the data, so a reader can walk the file backwards.
    int marker = ( int )m_RecordBytes;
    if ( m_ByteSwap )
    {
        SwapBytes( &marker, sizeof( marker ) );
    }
    WriteBytes( &marker, sizeof( marker ) );

    WriteMarker( m_RecordStart, m_RecordBytes );
}

void UnformattedOut::Write( int val )
{
    if ( m_ByteSwap )
    {
        SwapBytes( &val, sizeof( val ) );
    }

    WriteBytes( &val, sizeof( val ) );
    m_RecordBytes += sizeof( val );
}

void UnformattedOut::Write( const vector < int > &vals )
{
    for ( int i = 0; i < ( int )vals.size(); i++ )
    {
        Write( vals[i] );
    }
}

void UnformattedOut::Write( double val )
{
    if ( m_SinglePrecision )
    {
        float f = ( float )val;
        if ( m_ByteSwap )
        {
            SwapBytes( &f, sizeof( f ) );
        }

        WriteBytes( &f, sizeof( f ) );
        m_RecordBytes += sizeof( f );
    }
    else
    {
        double d = val;
        if ( m_ByteSwap )
        {
            SwapBytes( &d, sizeof( d ) );
        }

        WriteBytes( &d, sizeof( d ) );
        m_RecordBytes += sizeof( d );
    }
}

// A float is passed along as a double and then written at whatever width the file is
// set to.  Widening a float to a double and back is exact, so nothing is lost when the
// file is single precision either.
void UnformattedOut::Write( float val )
{
    Write( ( double )val );
}

void UnformattedOut::Write( const vector < double > &vals )
{
    for ( int i = 0; i < ( int )vals.size(); i++ )
    {
        Write( vals[i] );
    }
}

void UnformattedOut::Write( const vector < float > &vals )
{
    for ( int i = 0; i < ( int )vals.size(); i++ )
    {
        Write( ( double )vals[i] );
    }
}

void UnformattedOut::WriteRecord( int val )
{
    BeginRecord();
    Write( val );
    EndRecord();
}

void UnformattedOut::WriteRecord( const vector < int > &vals )
{
    BeginRecord();
    Write( vals );
    EndRecord();
}

void UnformattedOut::WriteRecord( const vector < double > &vals )
{
    BeginRecord();
    Write( vals );
    EndRecord();
}

void UnformattedOut::WriteRecord( const vector < float > &vals )
{
    BeginRecord();
    Write( vals );
    EndRecord();
}

//////////////////////////////////////////////////////////////////////
//========================== UnformattedIn ===========================//
//////////////////////////////////////////////////////////////////////

UnformattedIn::UnformattedIn()
{
    m_FP = nullptr;
    m_Good = true;

    m_SinglePrecision = false;
    m_ByteSwap = false;

    m_InRecord = false;
    m_RecordBytes = 0;
    m_RecordRead = 0;
    m_RecordStart = 0;
}

UnformattedIn::~UnformattedIn()
{
    Close();
}

bool UnformattedIn::Open( const string &fname )
{
    Close();

    m_FP = fopen( fname.c_str(), "rb" );
    m_Good = ( m_FP != nullptr );

    if ( m_Good )
    {
        DetectByteSwap();
    }

    return m_Good;
}

void UnformattedIn::Close()
{
    if ( m_FP )
    {
        fclose( m_FP );
        m_FP = nullptr;
    }

    m_InRecord = false;
    m_RecordBytes = 0;
    m_RecordRead = 0;
    m_RecordStart = 0;
}

bool UnformattedIn::ReadBytes( void* data, size_t nbyte )
{
    if ( !m_FP || !m_Good )
    {
        return false;
    }

    if ( fread( data, 1, nbyte, m_FP ) != nbyte )
    {
        m_Good = false;
        return false;
    }

    return true;
}

bool UnformattedIn::ReadMarker( int &marker )
{
    if ( !ReadBytes( &marker, sizeof( marker ) ) )
    {
        return false;
    }

    if ( m_ByteSwap )
    {
        SwapBytes( &marker, sizeof( marker ) );
    }

    return true;
}

// A record's two markers are the same number, so reading the first one, stepping over
// that many bytes and looking at the second says whether the byte order was guessed
// right.  Try it as written, then swapped.
bool UnformattedIn::DetectByteSwap()
{
    if ( !m_FP )
    {
        return false;
    }

    for ( int swap = 0; swap < 2; swap++ )
    {
        m_ByteSwap = ( swap != 0 );

        if ( fseek( m_FP, 0, SEEK_SET ) != 0 )
        {
            break;
        }

        m_Good = true;

        int lead = 0;
        if ( !ReadMarker( lead ) || lead < 0 )
        {
            continue;
        }

        if ( fseek( m_FP, lead, SEEK_CUR ) != 0 )
        {
            continue;
        }

        int trail = 0;
        if ( !ReadMarker( trail ) )
        {
            continue;
        }

        if ( lead == trail )
        {
            fseek( m_FP, 0, SEEK_SET );
            m_Good = true;
            return true;
        }
    }

    // Neither reading made sense of the first record.  Leave the order alone and let the
    // caller find out when it reads.
    m_ByteSwap = false;
    fseek( m_FP, 0, SEEK_SET );
    m_Good = true;

    return false;
}

bool UnformattedIn::DetectPrecision( long nreal, long nint )
{
    if ( !m_FP || nreal < 1 )
    {
        return false;
    }

    long nbyte = 0;

    if ( m_InRecord )
    {
        nbyte = m_RecordBytes;
    }
    else
    {
        // Look at the next record's marker and put the file back where it was.
        long here = ftell( m_FP );

        int lead = 0;
        bool ok = ReadMarker( lead );

        if ( fseek( m_FP, here, SEEK_SET ) != 0 )
        {
            m_Good = false;
            return false;
        }

        if ( !ok || lead < 0 )
        {
            return false;
        }

        nbyte = lead;
    }

    long fixed = nint * ( long )sizeof( int );
    long as_single = nreal * ( long )sizeof( float ) + fixed;
    long as_double = nreal * ( long )sizeof( double ) + fixed;

    if ( nbyte == as_single )
    {
        m_SinglePrecision = true;
        return true;
    }

    if ( nbyte == as_double )
    {
        m_SinglePrecision = false;
        return true;
    }

    return false;
}

bool UnformattedIn::BeginRecord()
{
    if ( !m_FP || !m_Good )
    {
        return false;
    }

    if ( m_InRecord )
    {
        EndRecord();
    }

    int lead = 0;
    if ( !ReadMarker( lead ) )
    {
        return false;
    }

    if ( lead < 0 )
    {
        m_Good = false;
        return false;
    }

    m_RecordBytes = lead;
    m_RecordRead = 0;
    m_RecordStart = ftell( m_FP );
    m_InRecord = true;

    return true;
}

bool UnformattedIn::EndRecord()
{
    if ( !m_FP || !m_InRecord )
    {
        return false;
    }

    m_InRecord = false;

    // Step over whatever was not read, so the trailing marker is next either way.
    if ( fseek( m_FP, m_RecordStart + m_RecordBytes, SEEK_SET ) != 0 )
    {
        m_Good = false;
        return false;
    }

    int trail = 0;
    if ( !ReadMarker( trail ) )
    {
        return false;
    }

    if ( trail != m_RecordBytes )
    {
        m_Good = false;
        return false;
    }

    return true;
}

void UnformattedIn::Read( int &val )
{
    if ( m_InRecord && GetRecordRemaining() < ( long )sizeof( val ) )
    {
        m_Good = false;
        return;
    }

    if ( ReadBytes( &val, sizeof( val ) ) )
    {
        if ( m_ByteSwap )
        {
            SwapBytes( &val, sizeof( val ) );
        }

        m_RecordRead += sizeof( val );
    }
}

void UnformattedIn::Read( vector < int > &vals )
{
    for ( int i = 0; i < ( int )vals.size(); i++ )
    {
        Read( vals[i] );
    }
}

// Take one real out of the file at whatever width the file holds, and hand it back as a
// double for the caller to put where it likes.
double UnformattedIn::ReadRealValue()
{
    if ( m_SinglePrecision )
    {
        if ( m_InRecord && GetRecordRemaining() < ( long )sizeof( float ) )
        {
            m_Good = false;
            return 0.0;
        }

        float f = 0.0f;
        if ( !ReadBytes( &f, sizeof( f ) ) )
        {
            return 0.0;
        }

        if ( m_ByteSwap )
        {
            SwapBytes( &f, sizeof( f ) );
        }

        m_RecordRead += sizeof( f );
        return f;
    }

    if ( m_InRecord && GetRecordRemaining() < ( long )sizeof( double ) )
    {
        m_Good = false;
        return 0.0;
    }

    double d = 0.0;
    if ( !ReadBytes( &d, sizeof( d ) ) )
    {
        return 0.0;
    }

    if ( m_ByteSwap )
    {
        SwapBytes( &d, sizeof( d ) );
    }

    m_RecordRead += sizeof( d );
    return d;
}

void UnformattedIn::Read( double &val )
{
    val = ReadRealValue();
}

void UnformattedIn::Read( float &val )
{
    val = ( float )ReadRealValue();
}

void UnformattedIn::Read( vector < double > &vals )
{
    for ( int i = 0; i < ( int )vals.size(); i++ )
    {
        vals[i] = ReadRealValue();
    }
}

void UnformattedIn::Read( vector < float > &vals )
{
    for ( int i = 0; i < ( int )vals.size(); i++ )
    {
        vals[i] = ( float )ReadRealValue();
    }
}

bool UnformattedIn::ReadRecord( int &val )
{
    if ( !BeginRecord() )
    {
        return false;
    }

    Read( val );

    return EndRecord() && m_Good;
}

bool UnformattedIn::ReadRecord( vector < int > &vals )
{
    if ( !BeginRecord() )
    {
        return false;
    }

    vals.resize( m_RecordBytes / sizeof( int ) );
    Read( vals );

    return EndRecord() && m_Good;
}

bool UnformattedIn::ReadRecord( vector < double > &vals )
{
    if ( !BeginRecord() )
    {
        return false;
    }

    size_t realsize = sizeof( double );
    if ( m_SinglePrecision )
    {
        realsize = sizeof( float );
    }
    vals.resize( m_RecordBytes / realsize );
    Read( vals );

    return EndRecord() && m_Good;
}

bool UnformattedIn::ReadRecord( vector < float > &vals )
{
    if ( !BeginRecord() )
    {
        return false;
    }

    size_t realsize = sizeof( double );
    if ( m_SinglePrecision )
    {
        realsize = sizeof( float );
    }
    vals.resize( m_RecordBytes / realsize );
    Read( vals );

    return EndRecord() && m_Good;
}
