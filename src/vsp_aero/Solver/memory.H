#include <stdio.h>

#ifdef MYMEMORY
#include "sys/resource.h"
#endif

#include <errno.h>
#include <stdlib.h>
#include "utils.H"

#include "START_NAME_SPACE.H"

double mymemory(void);

#include "END_NAME_SPACE.H"
