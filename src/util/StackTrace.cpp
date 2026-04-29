//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

#include "StackTrace.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <string>
using std::string;
using std::ostringstream;

static const int MAX_FRAMES = 64;

// =============================================================================
// Windows / MSVC  — DbgHelp gives symbol names + file/line with debug info
// =============================================================================
#ifdef WIN32

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <dbghelp.h>

string GetStackTrace( int skip )
{
    HANDLE proc = GetCurrentProcess();

    static bool initialized = false;
    if ( !initialized )
    {
        SymInitialize( proc, NULL, TRUE );
        SymSetOptions( SYMOPT_LOAD_LINES | SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS );
        initialized = true;
    }

    void*  stack[MAX_FRAMES] = {};
    USHORT nframes = CaptureStackBackTrace( (ULONG)( skip + 1 ), MAX_FRAMES, stack, NULL );

    char         symBuf[sizeof( SYMBOL_INFO ) + MAX_SYM_NAME * sizeof( TCHAR )] = {};
    SYMBOL_INFO* sym = reinterpret_cast<SYMBOL_INFO*>( symBuf );
    sym->SizeOfStruct = sizeof( SYMBOL_INFO );
    sym->MaxNameLen   = MAX_SYM_NAME;

    IMAGEHLP_LINE64 lineInfo = {};
    lineInfo.SizeOfStruct = sizeof( IMAGEHLP_LINE64 );

    ostringstream oss;
    for ( USHORT i = 0; i < nframes; ++i )
    {
        DWORD64 addr     = reinterpret_cast<DWORD64>( stack[i] );
        DWORD64 dispSym  = 0;
        DWORD   dispLine = 0;

        string funcName = "??";
        string fileName = "??";
        int    lineNo   = 0;

        if ( SymFromAddr( proc, addr, &dispSym, sym ) )
        {
            funcName = sym->Name;
        }

        if ( SymGetLineFromAddr64( proc, addr, &dispLine, &lineInfo ) )
        {
            fileName = lineInfo.FileName;
            size_t slash = fileName.find_last_of( "\\/" );
            if ( slash != string::npos )
            {
                fileName = fileName.substr( slash + 1 );
            }
            lineNo = static_cast<int>( lineInfo.LineNumber );
        }

        char buf[1024];
        if ( lineNo > 0 )
        {
            snprintf( buf, sizeof( buf ), "#%-3d  %-60s  %s:%d\n",
                      (int)i, funcName.c_str(), fileName.c_str(), lineNo );
        }
        else
        {
            snprintf( buf, sizeof( buf ), "#%-3d  %s\n", (int)i, funcName.c_str() );
        }
        oss << buf;
    }

    return oss.str();
}

// =============================================================================
// POSIX — macOS (Clang, Intel + ARM64) and Linux (GCC)
// =============================================================================
#else

#include <vector>
#include <execinfo.h>
#include <cxxabi.h>
#include <dlfcn.h>
#include <unistd.h>
using std::vector;

#ifdef __APPLE__
#   include <mach-o/dyld.h>
#else
#   include <elf.h>
#endif

// ─── Helpers ─────────────────────────────────────────────────────────────────

static string demangle( const char* sym )
{
    if ( !sym || sym[0] == '\0' )
    {
        return "??";
    }
    int   status = 0;
    char* d = abi::__cxa_demangle( sym, nullptr, nullptr, &status );
    if ( status == 0 && d )
    {
        string result( d );
        free( d );
        return result;
    }
    return sym;
}

static void rtrim( string& s )
{
    while ( !s.empty() && ( s.back() == '\n' || s.back() == '\r' || s.back() == ' ' ) )
    {
        s.pop_back();
    }
}

static string run_cmd( const string& cmd )
{
    FILE* fp = popen( cmd.c_str(), "r" );
    if ( !fp )
    {
        return "";
    }
    char   buf[512];
    string out;
    while ( fgets( buf, sizeof( buf ), fp ) )
    {
        out += buf;
    }
    pclose( fp );
    return out;
}

// ─── Per-frame data ───────────────────────────────────────────────────────────

struct FrameInfo
{
    void*     addr   = nullptr;
    string    module;       // path to the ELF / dylib containing this frame
    uintptr_t base   = 0;   // dli_fbase (load address of the module)
    string    func;         // demangled function name
    string    file;         // source file name (filled by subprocess)
    int       line   = 0;   // source line number (filled by subprocess)
    bool      pie    = false; // Linux: true → addresses are relative to base
};

// ─── macOS: batch resolve via atos ───────────────────────────────────────────
#ifdef __APPLE__

// atos accepts multiple addresses per invocation:
//   atos -o <module> -l 0x<load_base> 0x<addr1> 0x<addr2> ...
// With debug info:    "FuncName(args) (FileName.cpp:42)"
// Without debug info: "FuncName (in ModuleName) (load_addr + offset)"
static void resolve_lines( vector<FrameInfo>& frames,
                            int begin, int end,
                            const string& module, uintptr_t base )
{
    ostringstream cmd;
    cmd << std::hex;
    cmd << "atos -o '" << module << "' -l 0x" << base;
    for ( int i = begin; i < end; ++i )
    {
        cmd << " 0x" << (uintptr_t)frames[i].addr;
    }
    cmd << " 2>/dev/null";

    string out = run_cmd( cmd.str() );
    if ( out.empty() )
    {
        return;
    }

    int    fi  = begin;
    size_t pos = 0;
    while ( fi < end && pos < out.size() )
    {
        size_t nl  = out.find( '\n', pos );
        string row = out.substr( pos, nl == string::npos ? string::npos : nl - pos );
        rtrim( row );
        pos = ( nl == string::npos ) ? out.size() : nl + 1;

        // Look for trailing "(filename.cpp:NN)".
        if ( !row.empty() && row.back() == ')' )
        {
            size_t rparen = row.size() - 1;
            size_t lparen = row.rfind( '(' );
            size_t colon  = row.rfind( ':' );
            if ( lparen != string::npos && colon != string::npos &&
                 lparen < colon && colon < rparen )
            {
                string linestr = row.substr( colon + 1, rparen - colon - 1 );
                bool isNum = !linestr.empty();
                for ( char c : linestr )
                {
                    if ( !isdigit( (unsigned char)c ) )
                    {
                        isNum = false;
                        break;
                    }
                }
                if ( isNum )
                {
                    string filepart = row.substr( lparen + 1, colon - lparen - 1 );
                    size_t sl = filepart.rfind( '/' );
                    if ( sl != string::npos )
                    {
                        filepart = filepart.substr( sl + 1 );
                    }
                    // Function is everything before the " (file:line)" suffix.
                    string funcpart = ( lparen > 0 ) ? row.substr( 0, lparen - 1 )
                                                     : frames[fi].func;
                    rtrim( funcpart );
                    frames[fi].func = funcpart;
                    frames[fi].file = filepart;
                    frames[fi].line = std::stoi( linestr );
                }
            }
        }
        ++fi;
    }
}

// ─── Linux: batch resolve via addr2line ──────────────────────────────────────
#else

// Return true if the ELF at path is position-independent (ET_DYN), false for
// a fixed-address executable (ET_EXEC).  This determines how to form the
// query address for addr2line:
//   PIE     → addr2line expects  virtual_address − load_base
//   non-PIE → addr2line expects  virtual_address  (link-time absolute)
static bool is_pie_elf( const char* path )
{
    FILE* f = fopen( path, "rb" );
    if ( !f )
    {
        return true;    // assume PIE if we cannot read the file
    }
    unsigned char ident[EI_NIDENT];
    uint16_t      etype = ET_DYN;
    if ( fread( ident, 1, EI_NIDENT, f ) == EI_NIDENT &&
         ident[0] == ELFMAG0 && ident[1] == ELFMAG1 &&
         ident[2] == ELFMAG2 && ident[3] == ELFMAG3 )
    {
        // e_type immediately follows e_ident in both ELF32 and ELF64.
        fread( &etype, sizeof( etype ), 1, f );
    }
    fclose( f );
    return etype == ET_DYN;
}

// addr2line output per queried address (with -f -s):
//   line 1: demangled function name (or "??")
//   line 2: filename:linenumber   (or "??:0")
static void resolve_lines( vector<FrameInfo>& frames,
                            int begin, int end,
                            const string& module,
                            uintptr_t base, bool pie )
{
    ostringstream cmd;
    cmd << std::hex;
    cmd << "addr2line -e '" << module << "' -f -s -C -i";
    for ( int i = begin; i < end; ++i )
    {
        uintptr_t uaddr = (uintptr_t)frames[i].addr;
        uintptr_t query = pie ? ( uaddr - base ) : uaddr;
        cmd << " 0x" << query;
    }
    cmd << " 2>/dev/null";

    string out = run_cmd( cmd.str() );
    if ( out.empty() )
    {
        return;
    }

    int    fi  = begin;
    size_t pos = 0;
    while ( fi < end && pos < out.size() )
    {
        // Line 1: function name (demangled by -C).
        size_t nl1   = out.find( '\n', pos );
        string fname = out.substr( pos, nl1 == string::npos ? string::npos : nl1 - pos );
        rtrim( fname );
        pos = ( nl1 == string::npos ) ? out.size() : nl1 + 1;

        // Line 2: file:line.
        size_t nl2   = out.find( '\n', pos );
        string floc  = out.substr( pos, nl2 == string::npos ? string::npos : nl2 - pos );
        rtrim( floc );
        pos = ( nl2 == string::npos ) ? out.size() : nl2 + 1;

        if ( fname != "??" && !fname.empty() )
        {
            frames[fi].func = fname;
        }

        size_t colon = floc.rfind( ':' );
        if ( colon != string::npos )
        {
            string linestr = floc.substr( colon + 1 );
            if ( !linestr.empty() && linestr != "?" && linestr != "0" )
            {
                frames[fi].file = floc.substr( 0, colon );
                frames[fi].line = std::stoi( linestr );
            }
        }
        ++fi;
    }
}

#endif  // __APPLE__ / Linux

// ─── Common POSIX entry point ─────────────────────────────────────────────────

string GetStackTrace( int skip )
{
    void*  raw[MAX_FRAMES] = {};
    int    nframes = backtrace( raw, MAX_FRAMES );
    char** syms    = backtrace_symbols( raw, nframes );

    // Step 1: Populate per-frame info using dladdr.
    vector<FrameInfo> frames;
    frames.reserve( nframes );

    for ( int i = skip + 1; i < nframes; ++i )
    {
        FrameInfo fi;
        fi.addr = raw[i];

        Dl_info di = {};
        if ( dladdr( raw[i], &di ) && di.dli_fname )
        {
            fi.module = di.dli_fname;
            fi.base   = (uintptr_t)di.dli_fbase;
            fi.func   = di.dli_sname ? demangle( di.dli_sname )
                                     : ( syms ? syms[i] : "??" );
#ifndef __APPLE__
            fi.pie = is_pie_elf( di.dli_fname );
#endif
        }
        else
        {
            fi.func = syms ? syms[i] : "??";
        }
        frames.push_back( fi );
    }

    free( syms );

    // Step 2: One subprocess call per contiguous run of frames in the same
    //         module (typically just one call for a single-executable program).
    int groupStart = 0;
    for ( int i = 1; i <= (int)frames.size(); ++i )
    {
        bool last          = ( i == (int)frames.size() );
        bool moduleChanged = !last && ( frames[i].module != frames[groupStart].module );

        if ( ( last || moduleChanged ) && !frames[groupStart].module.empty() )
        {
#ifdef __APPLE__
            resolve_lines( frames, groupStart, i,
                           frames[groupStart].module,
                           frames[groupStart].base );
#else
            resolve_lines( frames, groupStart, i,
                           frames[groupStart].module,
                           frames[groupStart].base,
                           frames[groupStart].pie );
#endif
        }

        if ( moduleChanged )
        {
            groupStart = i;
        }
    }

    // Step 3: Format output.
    ostringstream oss;
    for ( int i = 0; i < (int)frames.size(); ++i )
    {
        const FrameInfo& fi = frames[i];
        char prefix[16];
        snprintf( prefix, sizeof( prefix ), "#%-3d  ", i );
        oss << prefix << fi.func;
        if ( fi.line > 0 )
        {
            oss << "  (" << fi.file << ":" << fi.line << ")";
        }
        oss << "\n";
    }

    return oss.str();
}

#endif  // WIN32

// =============================================================================
// Common
// =============================================================================

void PrintStackTrace( int skip )
{
    fprintf( stderr, "\n--- Stack Trace ---\n%s--- End Stack Trace ---\n\n",
             GetStackTrace( skip + 1 ).c_str() );
}
