#include <stdio.h>

#ifdef WIN32

#include <time.h>
#include <sys/timeb.h>

#else

#ifdef MYTIME
#include <sys/time.h>
#endif

#endif

#include <errno.h>
#include <stdlib.h>
#include "utils.H"

#include "START_NAME_SPACE.H"

VSPAERO_DOUBLE myclock(void);

#include "END_NAME_SPACE.H"
