//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

#include "LocaleUtil.h"

#include <clocale>
#include <cstdio>
#include <cstring>

#if !defined( _WIN32 )
#  include <langinfo.h>
#endif

#if defined( _WIN32 )

bool CNumericLocaleGuard::IsCNumeric()
{
    char buf[16];
    snprintf( buf, sizeof( buf ), "%.1f", 0.5 );
    return strcmp( buf, "0.5" ) == 0;
}

// The thread is given a locale of its own, a copy of the process-wide one, and only its
// numeric category is changed.
CNumericLocaleGuard::CNumericLocaleGuard() : m_Active( false ), m_PrevConfig( 0 )
{
    if ( IsCNumeric() )
    {
        return;
    }

    m_PrevConfig = _configthreadlocale( _ENABLE_PER_THREAD_LOCALE );

    const char* prev = setlocale( LC_NUMERIC, nullptr );
    if ( prev )
    {
        m_PrevNumeric = prev;
    }

    setlocale( LC_NUMERIC, "C" );
    m_Active = true;
}

CNumericLocaleGuard::~CNumericLocaleGuard()
{
    if ( !m_Active )
    {
        return;
    }

    if ( !m_PrevNumeric.empty() )
    {
        setlocale( LC_NUMERIC, m_PrevNumeric.c_str() );
    }

    if ( m_PrevConfig == _DISABLE_PER_THREAD_LOCALE )
    {
        _configthreadlocale( _DISABLE_PER_THREAD_LOCALE );
    }
}

#else

// nl_langinfo reads the calling thread's locale.
bool CNumericLocaleGuard::IsCNumeric()
{
    const char* radix = nl_langinfo( RADIXCHAR );
    return radix && strcmp( radix, "." ) == 0;
}

// The thread's locale -- its own, or the process-wide one -- is copied with only the numeric
// category changed, so character handling and everything else stay as they were.
CNumericLocaleGuard::CNumericLocaleGuard() : m_Active( false ), m_Prev( ( locale_t ) 0 ), m_Loc( ( locale_t ) 0 )
{
    if ( IsCNumeric() )
    {
        return;
    }

    m_Prev = uselocale( ( locale_t ) 0 );

    locale_t base = duplocale( m_Prev );
    if ( base == ( locale_t ) 0 )
    {
        return;
    }

    // newlocale frees base when it succeeds and leaves it alone when it fails.
    m_Loc = newlocale( LC_NUMERIC_MASK, "C", base );
    if ( m_Loc == ( locale_t ) 0 )
    {
        freelocale( base );
        return;
    }

    uselocale( m_Loc );
    m_Active = true;
}

CNumericLocaleGuard::~CNumericLocaleGuard()
{
    if ( !m_Active )
    {
        return;
    }

    uselocale( m_Prev );
    freelocale( m_Loc );
}

#endif
