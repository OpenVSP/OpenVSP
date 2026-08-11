//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// UnformattedFile.h
//
// Writing of Fortran sequential unformatted files.
//
//////////////////////////////////////////////////////////////////////

#if !defined(UNFORMATTED_FILE__INCLUDED_)
#define UNFORMATTED_FILE__INCLUDED_

#include <cstdio>
#include <string>
#include <vector>

using std::string;
using std::vector;

// Writes the files a Fortran program produces with FORM='UNFORMATTED'.  Every Fortran
// WRITE statement becomes one record, and a record is the count of its data bytes as a
// four byte integer, then the data, then that same count again.  A reader walks the file
// by those markers, so the records have to match the READ statements on the other side
// one for one -- not just the data.
//
// So this
//
//     WRITE(IU) N
//     WRITE(IU) X,Y,Z
//
// is
//
//     UnformattedOut fp;
//     fp.Open( fname );
//     fp.WriteRecord( n );
//
//     fp.BeginRecord();
//     fp.Write( x );
//     fp.Write( y );
//     fp.Write( z );
//     fp.EndRecord();
//
// Records are not buffered.  The count is left blank and filled in once the record ends,
// so a record can be as large as the reader will accept.
class UnformattedOut
{
public:

    UnformattedOut();
    virtual ~UnformattedOut();

    bool Open( const string &fname );
    void Close();

    bool IsOpen() const                             { return m_FP != nullptr; }

    // True until something goes wrong: a write fails, or a record grows past what a four
    // byte marker can describe.
    bool IsGood() const                             { return m_Good; }

    // Write reals as four byte floats rather than eight byte doubles, for a reader built
    // in single precision.  This setting alone decides how wide a real lands in the file;
    // the type handed to Write is only what the caller happens to be holding, and is
    // promoted or narrowed to suit.
    void SetSinglePrecision( bool flag )            { m_SinglePrecision = flag; }
    bool GetSinglePrecision() const                 { return m_SinglePrecision; }

    // Reverse the byte order on the way out, for a reader of the opposite endianness.
    void SetByteSwap( bool flag )                   { m_ByteSwap = flag; }
    bool GetByteSwap() const                        { return m_ByteSwap; }

    static bool HostIsLittleEndian();

    //==== One Record At A Time ====//

    void BeginRecord();
    void EndRecord();

    // The overload picks the on-disk type, so these read like the Fortran they have to
    // match.  An integer type other than int has to be cast: how wide an integer lands in
    // the file is the caller's decision to make, not one to arrive at by whichever type a
    // count happened to be held in.
    void Write( int val );
    void Write( const vector < int > &vals );

    void Write( double val );
    void Write( float val );
    void Write( const vector < double > &vals );
    void Write( const vector < float > &vals );

    //==== Whole Records ====//

    void WriteRecord( int val );
    void WriteRecord( const vector < int > &vals );
    void WriteRecord( const vector < double > &vals );
    void WriteRecord( const vector < float > &vals );

protected:

    void WriteBytes( const void* data, size_t nbyte );
    void WriteMarker( long offset, long nbyte );

    FILE* m_FP;
    bool m_Good;

    bool m_SinglePrecision;
    bool m_ByteSwap;

    bool m_InRecord;
    long m_RecordStart;     // file offset of the record's leading marker
    long m_RecordBytes;     // data bytes written into the record so far
};

// Reads the files UnformattedOut writes, and the ones a Fortran program produces.  It
// walks the same records: the count, the data, the count again.
//
// So this
//
//     READ(IU) N
//     READ(IU) X,Y,Z
//
// is
//
//     UnformattedIn fp;
//     fp.Open( fname );
//     fp.ReadRecord( n );
//
//     fp.BeginRecord();
//     fp.Read( x );
//     fp.Read( y );
//     fp.Read( z );
//     fp.EndRecord();
//
// A file does not say which byte order it was written in, so Open works it out by
// checking whether a record's two markers agree when read each way.
//
// Nor does it say how wide its reals are, and no amount of looking at the bytes will
// tell: ninety six bytes is as good a run of twelve doubles as it is of twenty four
// floats.  What does settle it is the count of values the record is supposed to hold,
// which structured formats state ahead of the data -- a Plot3D block gives its
// dimensions in an earlier record.  DetectPrecision takes that count and reads the
// answer off the record's length.
class UnformattedIn
{
public:

    UnformattedIn();
    virtual ~UnformattedIn();

    bool Open( const string &fname );
    void Close();

    bool IsOpen() const                             { return m_FP != nullptr; }

    // True until a read runs off the end of the file or of the open record, or a record
    // turns out to be malformed.
    bool IsGood() const                             { return m_Good; }

    // Whether the file holds four byte floats rather than eight byte doubles.  Reals are
    // handed back as whatever the caller asked for either way.
    void SetSinglePrecision( bool flag )            { m_SinglePrecision = flag; }
    bool GetSinglePrecision() const                 { return m_SinglePrecision; }

    void SetByteSwap( bool flag )                   { m_ByteSwap = flag; }
    bool GetByteSwap() const                        { return m_ByteSwap; }

    // Set the byte order by reading the first record both ways and seeing which one has
    // markers that agree.  Called by Open; only needed directly on a file whose first
    // record was not written by a matching writer.
    bool DetectByteSwap();

    // Set the real width from the length of a record known to hold nreal reals and nint
    // ints.  Uses the open record if there is one, otherwise looks ahead to the next
    // without disturbing the file.  False, and no change, if the record's length fits
    // neither reading.
    //
    // The answer is only as good as the count.  A record of nreal doubles is the same
    // length as one of twice that many floats, so a count wrong by a factor of two
    // returns true and sets the wrong width.  Give it a count the file stated, not one
    // arrived at by dividing the record up.
    bool DetectPrecision( long nreal, long nint = 0 );

    //==== One Record At A Time ====//

    // Step to the next record.  False at the end of the file, or if the record is
    // malformed.
    bool BeginRecord();

    // Leave the record, whether or not all of it was read, and check that it closed with
    // the count it opened with.
    bool EndRecord();

    long GetRecordBytes() const                     { return m_RecordBytes; }
    long GetRecordRemaining() const                 { return m_RecordBytes - m_RecordRead; }

    void Read( int &val );
    void Read( vector < int > &vals );      // as many as the vector already holds room for

    void Read( double &val );
    void Read( float &val );
    void Read( vector < double > &vals );
    void Read( vector < float > &vals );

    //==== Whole Records ====//

    // These take the whole of the next record, sizing the vector from its byte count.
    bool ReadRecord( int &val );
    bool ReadRecord( vector < int > &vals );
    bool ReadRecord( vector < double > &vals );
    bool ReadRecord( vector < float > &vals );

protected:

    bool ReadBytes( void* data, size_t nbyte );
    bool ReadMarker( int &marker );

    double ReadRealValue();

    FILE* m_FP;
    bool m_Good;

    bool m_SinglePrecision;
    bool m_ByteSwap;

    bool m_InRecord;
    long m_RecordBytes;     // data bytes in the open record
    long m_RecordRead;      // how many of them have been read
    long m_RecordStart;     // file offset of the open record's data
};

#endif // !defined(UNFORMATTED_FILE__INCLUDED_)
