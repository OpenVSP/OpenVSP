//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

//******************************************************************************
//
//   The "C" numeric locale for the calling thread.
//
//   Every number OpenVSP writes or reads as text uses a '.' decimal point.  printf,
//   strtod, atof and sscanf follow the numeric locale, so a program that hosts
//   OpenVSP and sets a locale with a comma decimal point (a GUI toolkit, MATLAB,
//   setlocale( LC_ALL, "" ) from user code) would have them write and read commas.
//
//   A CNumericLocaleGuard sets the calling thread's numeric locale to "C" for as
//   long as it lives and puts back what was there before.  The process-wide locale
//   and every other thread are left alone.  When the thread already has a '.'
//   decimal point it does nothing at all.
//
//******************************************************************************

#ifndef LOCALEUTIL_H
#define LOCALEUTIL_H

#if defined( _WIN32 )
#  include <string>
#else
#  include <locale.h>
#  if defined( __APPLE__ )
#    include <xlocale.h>
#  endif
#endif

class CNumericLocaleGuard
{
public:

    CNumericLocaleGuard();
    ~CNumericLocaleGuard();

    CNumericLocaleGuard( const CNumericLocaleGuard & ) = delete;
    CNumericLocaleGuard & operator=( const CNumericLocaleGuard & ) = delete;

    // Whether printf on the calling thread writes a '.' decimal point.
    static bool IsCNumeric();

private:

    bool m_Active;

#if defined( _WIN32 )
    int m_PrevConfig;
    std::string m_PrevNumeric;
#else
    locale_t m_Prev;
    locale_t m_Loc;
#endif
};

#endif
