//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

#include "FastFormat.h"

#include <charconv>
#include <cmath>
#include <cstdio>

// std::to_chars for floating point is missing from older standard libraries, and Apple's
// library offers it only from macOS 13.3.  Without it every number goes through snprintf.
#if defined( __APPLE__ )
#  if defined( __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ )
#    if __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ >= 130300
#      define FASTFORMAT_TO_CHARS_FLOAT
#    endif
#  endif
#elif defined( _GLIBCXX_RELEASE )
#  if _GLIBCXX_RELEASE >= 11
#    define FASTFORMAT_TO_CHARS_FLOAT
#  endif
#elif defined( _LIBCPP_VERSION )
#  if _LIBCPP_VERSION >= 170000
#    define FASTFORMAT_TO_CHARS_FLOAT
#  endif
#elif defined( _MSC_VER )
#  if _MSC_VER >= 1924
#    define FASTFORMAT_TO_CHARS_FLOAT
#  endif
#endif

namespace
{

void AppendPrintf( std::string &out, const char* fmt, int width, int prec, double x )
{
    char buf[512];
    snprintf( buf, sizeof( buf ), fmt, width, prec, x );
    out += buf;
}

#ifdef FASTFORMAT_TO_CHARS_FLOAT

// Right justify in a field of width, as printf does.
void AppendPadded( std::string &out, const char* b, const char* e, int width )
{
    int n = ( int )( e - b );
    if ( n < width )
    {
        out.append( width - n, ' ' );
    }
    out.append( b, e );
}

// Whether the three characters before e read "500": the digits past the last one kept are
// those of a value exactly half way between two roundings.  printf implementations do not all
// round or trim such a value the same way, so it is left to printf itself.
bool EndsHalfway( const char* b, const char* e )
{
    return e - b >= 3 && e[-3] == '5' && e[-2] == '0' && e[-1] == '0';
}

// x is m 2^e with m odd; e is returned.  x must be finite and not zero.
int LowBitExponent( double x, int &e2 )
{
    double f = std::frexp( std::fabs( x ), &e2 );
    unsigned long long m = ( unsigned long long )std::ldexp( f, 53 );
    int tz = 0;
    while ( !( m & 1ULL ) )
    {
        m >>= 1;
        tz++;
    }
    return e2 - 53 + tz;
}

// Whether x rounded to nsig significant digits may sit on a tie.
//
// With x = m 2^e, m odd and e negative, x written out in full is an integer ending in 5,
// times 10^e, so it has E10 + 1 - e significant digits, E10 being its decimal exponent.  It
// is a tie at nsig digits only when that count is nsig + 1.  Only when the cheap estimate of
// E10 cannot rule that out is the number written out to see.
bool MaybeTieSig( double x, int nsig )
{
    if ( x == 0.0 )
    {
        return false;
    }

    int e2 = 0;
    int e = LowBitExponent( x, e2 );

    if ( e < 0 )
    {
        // 2^( e2 - 1 ) <= |x| < 2^e2
        const double log10_2 = 0.30102999566398120;
        int lo = ( int )std::floor( ( e2 - 1 ) * log10_2 ) - 1;
        int hi = ( int )std::floor( e2 * log10_2 ) + 1;
        int want = nsig + e;
        if ( want < lo || want > hi )
        {
            return false;
        }
    }
    else if ( std::fabs( x ) < std::pow( 10.0, nsig ) )
    {
        // An integer of nsig digits or fewer is written exactly.
        return false;
    }

    char buf[64];
    std::to_chars_result r = std::to_chars( buf, buf + sizeof( buf ), x, std::chars_format::scientific, nsig + 2 );
    if ( r.ec != std::errc() )
    {
        return true;
    }

    const char* end = buf;
    while ( end < r.ptr && *end != 'e' )
    {
        end++;
    }
    return EndsHalfway( buf, end );
}

// Whether x rounded to prec decimal places may sit on a tie: x written out in full has -e
// decimal places, so only prec + 1 of them can be.
bool MaybeTieFixed( double x, int prec )
{
    if ( x == 0.0 )
    {
        return false;
    }

    int e2 = 0;
    return LowBitExponent( x, e2 ) == -( prec + 1 );
}

#endif

}

void FastFormat::AppendInt( std::string &out, long long v )
{
    char buf[32];
    std::to_chars_result r = std::to_chars( buf, buf + sizeof( buf ), v );
    out.append( buf, r.ptr );
}

void FastFormat::AppendG( std::string &out, double x, int width, int prec )
{
#ifdef FASTFORMAT_TO_CHARS_FLOAT
    if ( std::isfinite( x ) && prec > 0 && prec <= 40 && !MaybeTieSig( x, prec ) )
    {
        char buf[64];
        std::to_chars_result r = std::to_chars( buf, buf + sizeof( buf ), x, std::chars_format::general, prec );
        if ( r.ec == std::errc() )
        {
            AppendPadded( out, buf, r.ptr, width );
            return;
        }
    }
#endif
    AppendPrintf( out, "%*.*g", width, prec, x );
}

void FastFormat::AppendE( std::string &out, double x, int width, int prec )
{
#ifdef FASTFORMAT_TO_CHARS_FLOAT
    if ( std::isfinite( x ) && prec >= 0 && prec <= 40 && !MaybeTieSig( x, prec + 1 ) )
    {
        char buf[64];
        std::to_chars_result r = std::to_chars( buf, buf + sizeof( buf ), x, std::chars_format::scientific, prec );
        if ( r.ec == std::errc() )
        {
            AppendPadded( out, buf, r.ptr, width );
            return;
        }
    }
#endif
    AppendPrintf( out, "%*.*e", width, prec, x );
}

void FastFormat::AppendF( std::string &out, double x, int width, int prec )
{
#ifdef FASTFORMAT_TO_CHARS_FLOAT
    // Past 1e15 the integer digits alone outgrow the buffer's comfort; printf takes those.
    if ( std::isfinite( x ) && std::fabs( x ) < 1.0e15 && prec >= 0 && prec <= 40 && !MaybeTieFixed( x, prec ) )
    {
        char buf[96];
        std::to_chars_result r = std::to_chars( buf, buf + sizeof( buf ), x, std::chars_format::fixed, prec );
        if ( r.ec == std::errc() )
        {
            AppendPadded( out, buf, r.ptr, width );
            return;
        }
    }
#endif
    AppendPrintf( out, "%*.*f", width, prec, x );
}
