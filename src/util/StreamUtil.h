// Public Domain code from http://wordaligned.org/articles/cpp-streambufs


#ifndef STREAMUTIL_H
#define STREAMUTIL_H

#include <fstream>
#include <iostream>

class redirecter
{
public:
    redirecter( std::ostream & dst, std::ostream & src ) :
        src( src ), sbuf( src.rdbuf( dst.rdbuf() ) )
    {
    }

    ~redirecter()
    {
        src.rdbuf( sbuf );
    }

private:
    std::ostream & src;
    std::streambuf * const sbuf;
};

class teebuf: public std::streambuf
{
public:
    // Construct a streambuf which tees output to both input
    // streambufs.
    teebuf( std::streambuf * sb1, std::streambuf * sb2 ) :
        sb1( sb1 ), sb2( sb2 )
    {
    }
private:
    // This tee buffer has no buffer. So every character "overflows"
    // and can be put directly into the teed buffers.
    virtual int overflow( int c )
    {
        if ( c == EOF )
        {
            return !EOF;
        }
        else
        {
            int const r1 = sb1->sputc( c );
            int const r2 = sb2->sputc( c );
            return r1 == EOF || r2 == EOF ? EOF : c;
        }
    }

    // Sync both teed buffers.
    virtual int sync()
    {
        int const r1 = sb1->pubsync();
        int const r2 = sb2->pubsync();
        return r1 == 0 && r2 == 0 ? 0 : -1;
    }
private:
    std::streambuf * sb1;
    std::streambuf * sb2;
};

class teestream: public std::ostream
{
public:
    // Construct an ostream which tees output to the supplied
    // ostreams.
    teestream( std::ostream & o1, std::ostream & o2 );
private:
    teebuf tbuf;
};

teestream::teestream( std::ostream & o1, std::ostream & o2 ) :
    std::ostream( &tbuf ), tbuf( o1.rdbuf(), o2.rdbuf() )
{
}

#endif
