//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

#include "ProcessUtil.h"
#include "MessageMgr.h"
#include "StringUtil.h"

#include <ctime>

#ifdef __APPLE__

#endif

#ifdef WIN32

#else

#include <sys/wait.h>

#include <fcntl.h>
#include <csignal>
#endif


void SleepForMilliseconds( unsigned int sleep_time)
{
#ifdef WIN32
    Sleep( sleep_time );
#else
    usleep( sleep_time*1000 );
#endif
}

ProcessUtil::ProcessUtil()
{
#ifdef WIN32
    m_ThreadID = 0;
    m_Handle = nullptr;

    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    ZeroMemory( &pi, sizeof(pi) );

    m_StdoutPipe[PIPE_READ] = nullptr;
    m_StdoutPipe[PIPE_WRITE] = nullptr;
#else
    childPid = -1;

    m_StdoutPipe[PIPE_READ] = -1;
    m_StdoutPipe[PIPE_WRITE] = -1;
#endif
}

#ifndef WIN32
// Wrapper for handling arguments for exec*** routines.
// Note, this automatically makes cmd the first argument in the list.
// This also automatically nullptr terminates the list of arguments.
const char** opt2argv( const string &cmd, const vector< string > &options )
{
    int narg = options.size();
    const char **argv = new const char*[narg + 2];

    argv[0] = cmd.c_str();

    for ( int i = 0;  i < narg;  i++ )
    {
        argv[i+1] = options[i].c_str();
    }
    argv[narg + 1] = nullptr;

    return argv;
}

// C++ wrapper for execv.
int cppexecv( const string &cmd, const vector< string > &options )
{
    const char **argv = opt2argv( cmd, options );

    int retval = execv( cmd.c_str(), (char **)argv );

    delete[] argv;

    return retval;
}

// C++ wrapper for execvp -- searches path.
int cppexecvp( const string &cmd, const vector< string > &options )
{
    const char **argv = opt2argv( cmd, options );

    int retval = execvp( cmd.c_str(), (char **)argv );

    delete[] argv;

    return retval;
}
#endif

int ProcessUtil::SystemCmd( const string &path, const string &cmd, const vector<string> &opts )
{
#ifdef WIN32
    string command = string( "start " );
    if ( !path.empty() )
    {
        command += path + string("\\");
    }
    command += cmd;

    for( unsigned int i = 0; i < opts.size(); i++ )
    {
        command += string(" ") + opts[i];
    }

    return system( command.c_str() );

#else
    string command;
    if ( !path.empty() )
    {
        command += path + string( "/" );
    }
    command += cmd;

    for( unsigned int i = 0; i < opts.size(); i++ )
    {
        command += string(" ") + opts[i];
    }
    command += string("&");

    return system( command.c_str() );
#endif
}

#ifdef WIN32
string ProcessUtil::QuoteString( const string &str )
{
    string qstr;
    if( str.empty() == false &&
        str.find_first_of( " \t\n\v\"" ) == str.npos )
    {
        return str;
    }
    else
    {

        qstr.push_back( '"' );

        for (auto It = str.begin () ; ; ++It)
        {
            unsigned NumberBackslashes = 0;

            while( It != str.end () && *It == '\\' )
            {
                ++It;
                ++NumberBackslashes;
            }

            if( It == str.end () )
            {
                qstr.append( NumberBackslashes * 2, '\\' );
                break;
            }
            else if( *It == '"' )
            {
                qstr.append( NumberBackslashes * 2 + 1, '\\' );
                qstr.push_back( *It );
            }
            else
            {
                qstr.append( NumberBackslashes, '\\' );
                qstr.push_back( *It );
            }
        }

        qstr.push_back( '"' );
    }
    return qstr;
}
#endif

int ProcessUtil::ForkCmd( const string &path, const string &cmd, const vector<string> &opts )
{
    clock_t tstart = clock();

#ifdef WIN32


    SECURITY_ATTRIBUTES saAttr;

    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = nullptr;

    if ( !CreatePipe( &m_StdoutPipe[PIPE_READ], &m_StdoutPipe[PIPE_WRITE], &saAttr, 0 ) )
    {
        printf( "Error StdoutRd CreatePipe\n" );
        exit( 0 );
    }

    if ( !SetHandleInformation( m_StdoutPipe[PIPE_READ], HANDLE_FLAG_INHERIT, 0 ) )
    {
        printf( "Stdout SetHandleInformation\n" );
        exit( 0 );
    }

    string command;
    if ( !path.empty() )
    {
        command += path + string("\\");
    }
    command = QuoteString( command + cmd );

    for( int i = 0; i < opts.size(); i++ )
    {
        command += string(" ") + QuoteString( opts[i] );
    }

    char *cmdstr = (char*) malloc( (command.size()+1) * sizeof(char) );
    strcpy( cmdstr, command.c_str() );

    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    si.hStdError = m_StdoutPipe[PIPE_WRITE];
    si.hStdOutput = m_StdoutPipe[PIPE_WRITE];
    si.dwFlags |= STARTF_USESTDHANDLES;

    ZeroMemory( &pi, sizeof(pi) );

    if( !CreateProcess( nullptr,    // No module name (use command line).
        TEXT( cmdstr ),          // Command line.
        nullptr,                    // Process handle not inheritable.
        nullptr,                    // Thread handle not inheritable.
        TRUE,                    // Set handle inheritance to FALSE.
        0,                       // No creation flags.
        nullptr,                    // Use parent's environment block.
        nullptr,                    // Use parent's starting directory.
        &si,                     // Pointer to STARTUPINFO structure.
        &pi )                    // Pointer to PROCESS_INFORMATION structure.
    )
    {
        printf( "CreateProcess failed (%d).\n", GetLastError() );
        return 0;
    }

#else

    if( pipe( m_StdoutPipe ) < 0 )
    {
        printf( "Error allocating pipe for child output redirect\n");
        return -1;
    }

    fcntl( m_StdoutPipe[PIPE_WRITE] , F_SETFL, O_NONBLOCK );
    fcntl( m_StdoutPipe[PIPE_READ] , F_SETFL, O_NONBLOCK );

    if ( ( childPid = fork() ) == 0 )
    {

        if( dup2( m_StdoutPipe[PIPE_WRITE], STDOUT_FILENO ) == -1 ||
            dup2( m_StdoutPipe[PIPE_WRITE], STDERR_FILENO ) == -1 )
        {
            printf( "Error redirecting child stdout\n" );
            exit( 0 );
        }

        close( m_StdoutPipe[PIPE_READ] );
        close( m_StdoutPipe[PIPE_WRITE] );

        if ( path.empty() ) // Search OS path for cmd
        {
            if( cppexecvp( cmd, opts ) < 0 ) {
                printf( "execvp error\n" );
            }
        }
        else // Full path to cmd specified.
        {
            string command;
            if ( !path.empty() )
            {
                command = path + string("/");
            }
            command += cmd;

            if( cppexecv( command, opts ) < 0 ) {
                printf( "execv error\n" );
            }
        }
    }
    else if (childPid < 0)
    {
        close( m_StdoutPipe[PIPE_READ] );
        close( m_StdoutPipe[PIPE_WRITE] );

        printf( "Fork failed (%d).\n", childPid );
        return 0;
    }
#endif

    // Attempt at forking has taken less than one second, so delay to make up for it.  This gives the monitor thread
    // a fair chance to connect.  Threads that terminate too quickly do not get their output captured because they
    // terminate before the monitor is working.  Delay occurs before the output pipe is closed.
    double tmin = 1.0;
    double telapsed = ( clock() - tstart ) / (double)CLOCKS_PER_SEC;
    if ( telapsed < tmin )
    {
        SleepForMilliseconds( 1000.0 * ( tmin - telapsed ) );
    }

#ifdef WIN32
    CloseHandle( m_StdoutPipe[PIPE_WRITE] );
    m_StdoutPipe[PIPE_WRITE] = nullptr;

    free( cmdstr );
#else
    close( m_StdoutPipe[PIPE_WRITE] );
#endif

    return 0;
}

void ProcessUtil::WaitCmd( void *(*updatefun)( void *data ), void *data )
{
    int waitFlag = 1;

    while ( waitFlag )
    {
        // Close process and thread handles.
#ifdef WIN32
        int ret = WaitForSingleObject( pi.hProcess, 100 ); // (milliseconds)

        if ( ret == WAIT_OBJECT_0 )
        {
            CloseHandle( pi.hProcess );
            CloseHandle( pi.hThread );
            ZeroMemory( &pi, sizeof(pi) );

            waitFlag = 0;
        }
#else
        int status;
        pid_t retpid = waitpid( childPid, &status, WNOHANG );

        if  (retpid == childPid )
        {
            childPid = -1;
            waitFlag = 0;
        }
        else
        {
            usleep( 100000 ); // (microseconds)
        }
#endif

        if ( updatefun )
        {
            updatefun( data );
        }
    }
}

void ProcessUtil::Kill()
{
    if( IsRunning() )
    {
#ifdef WIN32
        TerminateProcess( pi.hProcess, 0 );
#else
        kill( childPid, SIGTERM );
#endif
    }
}

bool ProcessUtil::IsRunning()
{
#ifdef WIN32

    if( pi.dwProcessId == 0 )
    {
        return false;
    }
    else
    {
        int ret = WaitForSingleObject( pi.hProcess, 0 ); // (milliseconds)

        if ( ret == WAIT_OBJECT_0 )
        {
            CloseHandle( pi.hProcess );
            CloseHandle( pi.hThread );
            ZeroMemory( &pi, sizeof(pi) );

            return false;
        }
    }
#else
    if( childPid == -1 )
    {
        return false;
    }
    else
    {
        int status;
        pid_t retpid = waitpid( childPid, &status, WNOHANG );

        if  (retpid == childPid )
        {
            childPid = -1;
            return false;
        }
    }
#endif
    return true;
}

#ifdef WIN32
void ProcessUtil::StartThread( LPTHREAD_START_ROUTINE threadfun, LPVOID data )
{
    HANDLE m_Handle = CreateThread( 0, 0, threadfun, data, 0, &m_ThreadID );

    if(m_Handle==nullptr)
    {
        //THREAD CREATION FAILED
        printf("ERROR: Thread creation failed \n\tFile: %s \tLine:%d\n",__FILE__,__LINE__);
    }
}
#else
void ProcessUtil::StartThread( void *(*threadfun)( void *data ), void *data )
{
// Example code for setting thread stack size.
//    pthread_attr_t thread_attr;
//    size_t tmp_size=0;
//    pthread_attr_init( &thread_attr );
//    pthread_attr_setstacksize( &thread_attr , 0x80000 * 2 ); // 0x80000 // Default
//    pthread_attr_getstacksize( &thread_attr , &tmp_size );
//    pthread_create( &m_Thread, &thread_attr, threadfun, data );

    pthread_create( &m_Thread, nullptr, threadfun, data );

    //TODO return thread creation success/failure
}
#endif

void ProcessUtil::ReadStdoutPipe(char * bufptr, int bufsize, BUF_READ_TYPE * nreadptr )
{
    bufptr[0] = 0;
#ifdef WIN32
    ReadFile( m_StdoutPipe[PIPE_READ], bufptr, bufsize, nreadptr, nullptr);
#else
    *nreadptr = read( m_StdoutPipe[PIPE_READ], bufptr, bufsize );
#endif
}

/* PrettyCmd( path, cmd, opts )
    Returns a command string that could be used on the command line
*/
string ProcessUtil::PrettyCmd( const string &path, const string &cmd, const vector<string> &opts )
{
#ifdef WIN32
    string command;
    if ( !path.empty() )
    {
        command += path + string("\\");
    }
    command = QuoteString( command + cmd );

    for( int i = 0; i < opts.size(); i++ )
    {
        command += string(" ") + QuoteString( opts[i] );
    }
#else
    string command;
    if ( !path.empty() )
    {
        command = path + string("/");
    }
    command += cmd;

    for( unsigned int i = 0; i < opts.size(); i++ )
    {
        command += string(" ") + opts[i];
    }
#endif

    //add line feed ending
    command += string("\n");

    return command;
}

void MonitorProcess( FILE * logFile, ProcessUtil *process, const string &msgLabel )
{
    // ==== MonitorSolverProcess ==== //
    int bufsize = 1000;
    char *buf;
    buf = ( char* ) malloc( sizeof( char ) * ( bufsize + 1 ) );
    if ( !buf )
    {
        return;
    }
    BUF_READ_TYPE nread = 1;
    bool runflag = process->IsRunning();
    while ( runflag || nread > 0 )
    {
        process->ReadStdoutPipe( buf, bufsize, &nread );
        if( nread > 0 )
        {
            if ( buf )
            {
                buf[nread] = 0;
                if( logFile )
                {
                    fprintf( logFile, "%s", buf );
                }
                else
                {
                    MessageData data;
                    data.m_String = msgLabel;
                    data.m_StringVec.push_back( string( buf ) );
                    MessageMgr::getInstance().Send( "ScreenMgr", nullptr, data );
                }
            }
        }

        SleepForMilliseconds( 100 );
        runflag = process->IsRunning();
    }

    if( logFile )
    {
        fprintf( logFile, "Done\n" );
    }
    else
    {
        MessageData data;
        data.m_String = msgLabel;
        data.m_StringVec.push_back( string( "Done\n" ) );
        MessageMgr::getInstance().Send( "ScreenMgr", nullptr, data );
    }

#ifdef WIN32
    CloseHandle( process->m_StdoutPipe[0] );
    process->m_StdoutPipe[0] = nullptr;
#else
    close( process->m_StdoutPipe[0] );
    process->m_StdoutPipe[0] = -1;
#endif

    free( buf );
}
