#include <stdio.h>

#ifdef WIN32

#include <time.h>
#include <sys/timeb.h>

#else

#ifndef MYTIME
#include <sys/time.h>
#endif

#endif

#include <errno.h>
#include <stdlib.h>

double myclock(void);
