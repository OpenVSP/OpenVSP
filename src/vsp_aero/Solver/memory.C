#include "memory.H"

#include "START_NAME_SPACE.H"

/*##############################################################################
#                                                                              #
#                                      gettime                                 #              
#                                                                              #
##############################################################################*/

double mymemory(void)
{
 
#ifdef MYMEMORY
 
    struct rusage Usage;
    
    getrusage(RUSAGE_SELF, &Usage);

    return ((double) Usage.ru_maxrss)/1.e9;;
   
#else

   return 0.;

#endif
           
}

#include "END_NAME_SPACE.H"
