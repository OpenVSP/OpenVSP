//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

#if !defined(PROCESS_UTIL__INCLUDED_)
#define PROCESS_UTIL__INCLUDED_

#ifdef WIN32
#undef _HAS_STD_BYTE
#define _HAS_STD_BYTE 0
#include <windows.h>
#else
#include <unistd.h>
#include <sys/types.h>
#include <pthread.h>
#endif

#ifdef WIN32
#define BUF_READ_TYPE DWORD
#else
#define BUF_READ_TYPE int
#endif

#include <string>
#include <vector>
using std::string;
using std::vector;

#define PIPE_READ 0
#define PIPE_WRITE 1

class ProcessUtil;

void SleepForMilliseconds( unsigned int sleep_time);
void MonitorProcess( FILE * logFile, ProcessUtil *process, const string &msgLabel );

class ProcessUtil
{
public:

    ProcessUtil();

    static int SystemCmd( const string &path, const string &cmd, const vector<string> &opts );

    int ForkCmd( const string &path, const string &cmd, const vector<string> &opts );

    void WaitCmd( void *(*updatefun)( void * ), void *data );
    void Kill();

    bool IsRunning();

    void ReadStdoutPipe(char * buf, int bufsize, BUF_READ_TYPE * nread );

    static string PrettyCmd( const string &path, const string &cmd, const vector<string> &opts ); //returns a command string that could be used on the command line

#ifdef WIN32
    static string QuoteString( const string &str );
    void StartThread( LPTHREAD_START_ROUTINE threadfun, LPVOID data );

    HANDLE m_StdoutPipe[2];
#else
    void StartThread( void *(*threadfun)( void * ), void *data );

    int m_StdoutPipe[2];
#endif

private:

#ifdef WIN32
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    DWORD m_ThreadID;
    HANDLE m_Handle;
#else
    pid_t childPid;
    pthread_t m_Thread;
#endif

};

#endif

